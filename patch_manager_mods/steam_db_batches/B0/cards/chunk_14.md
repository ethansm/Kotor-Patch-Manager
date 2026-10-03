# Review cards B0 chunk 14

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-14-01

GOG function: Gob::RemoveAnimationFlag  gog 0x008fd290  callconv=(blank)  param_size_bytes=4  gog_size=82  gog_retn=4
Candidate Steam address: 0x004e3160 (size 82, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3110:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fd240 Gob::AddAnimationFlag -> 0x004e3110  gate_class=REVIEW
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

## Card B0-14-02

GOG function: Global::AurPostString  gog 0x00866b40  callconv=__stdcall  param_size_bytes=16  gog_size=133  gog_retn=0
Candidate Steam address: 0x00474c00 (size 133, retn 0)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Other::AurPostString
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (MainLoop 0x781be0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00474c90:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00866b40 Other::AurPostString -> 0x00474c00  gate_class=REVIEW
- GOG-order neighbour after:  0x00867040 Global::CommandCompletion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00866410 PostedString::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x434*} | Steam {0x4*,0xc*,0x434*}

### abi
```
GOG param bytes: 16; callconv: stdcall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 16 bytes -> x86 slots 16; ASM RET 0)
```

### prologue
```
prologue for 0x00474c00: 16-byte aligned: YES
  00474c00  55             PUSH EBP
  00474c01  8B EC          MOV EBP,ESP
  00474c03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00474c00
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x004744D0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00474c00 FUN_00474c00 size=133 ret=0 callers=7 callees=2

void FUN_00474c00(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094a6fb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x434,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 != 0) {
    FUN_004744d0(param_1,param_2,param_3,param_4);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00474c90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00474c90: 16-byte aligned: YES
  00474c90  55             PUSH EBP
  00474c91  8B EC          MOV EBP,ESP
  00474c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00474c90
string literals (0):
direct callees (5):
  0x0091EFED  (unnamed)
  0x00919723  operator new
  0x00919F10  _strlen
  0x004744D0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 8):
  +0x40C [dec 1036]  x1  via ECX
  +0x410 [dec 1040]  x1  via EDX
  +0x414 [dec 1044]  x1  via EAX
  +0x418 [dec 1048]  x1  via ECX
  +0x41C [dec 1052]  x1  via EDX
  +0x420 [dec 1056]  x1  via EAX
  +0x424 [dec 1060]  x1  via ECX
  +0x428 [dec 1064]  x1  via EDX
```

## Card B0-14-03

GOG function: CSWPartyTable::GetNPCSelectability  gog 0x00700cb0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x005fb1a0 (size 3508, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::SaveTableInfo
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fb170:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700c60 CSWPartyTable::GetIsAvailable -> 0x005fb0f0  gate_class=AUTO
- GOG-order neighbour after:  0x00700ce0 CSWPartyTable::SetNPCSelectability -> 0x005fb170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x7c*} | Steam {0x4,0x8,0xc,0x10,0x2c,0x34,0x4c,0x7c*,0x88,0xc4,0xc8,0x108,0x10c,0x110,...(+23)}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0xa,0xc,0x10,0x17,0x21,0xa0,0x13c,0x240,...(+1)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fb1a0: 16-byte aligned: YES
  005fb1a0  55             PUSH EBP
  005fb1a1  8B EC          MOV EBP,ESP
  005fb1a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005fb1a0
string literals (59):
  0x00997268  at 005fb2b5  'PARTYTABLE'
  0x0099725C  at 005fb39f  'PT_PCNAME'
  0x00997254  at 005fb3bc  'PT_GOLD'
  0x00997240  at 005fb3da  'PT_ITEM_COMPONENT'
  0x0099722C  at 005fb3f8  'PT_ITEM_CHEMICAL'
  0x00997220  at 005fb416  'PT_SWOOP1'
  0x00997214  at 005fb434  'PT_SWOOP2'
  0x00997208  at 005fb452  'PT_SWOOP3'
  0x009971FC  at 005fb470  'PT_XP_POOL'
  0x009971E8  at 005fb48e  'PT_PLAYEDSECONDS'
  0x009971D4  at 005fb4ac  'PT_CONTROLLED_NPC'
  0x009971C8  at 005fb4dc  'PT_SOLOMODE'
direct callees (30):
  0x00733540  CExoString::CExoString(empty)
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00733780  CExoString::~CExoString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x005FEE60  CSWPartyTable::GetTotalPlayedSeconds
  0x005FF000  CSWPartyTable::ResetCurrentSessionStartTime
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00719BF0  CResGFF::WriteFieldINT
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719740  CResGFF::AddList
  0x00719800  CResGFF::AddListElement
  0x00719940  CResGFF::AddStructToStruct
  0x0073F750  ClientApp::GetGuiInGame
  0x0071A1B0  CResGFF::WriteFieldVOID
  0x007C7D20  CGuiInGame::GetMessageBuffer
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x34 [dec 52]  x2  via ECX
  +0x114 [dec 276]  x2  via ECX,EDX
  +0x118 [dec 280]  x2  via ECX,EDX
  +0x1A8 [dec 424]  x2  via EDX
  +0x23C [dec 572]  x2  via EAX,EDX
  +0x2C [dec 44]  x1  via EAX
  +0x7C [dec 124]  x1  via EAX
  +0xC4 [dec 196]  x1  via EDX
  +0x108 [dec 264]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005fb1a0 FUN_005fb1a0 size=3508 ret=4 callers=2 callees=30

void __fastcall FUN_005fb1a0(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 *local_138;
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  int local_90;
  undefined4 *local_8c;
  int local_88;
  int local_84;
  undefined1 local_80 [40];
  int local_58;
  int local_54;
  undefined1 local_50 [8];
  uint local_48;
  undefined4 local_44;
  undefined4 *local_40;
  int local_3c;
  undefined1 local_38 [4];
  int local_34;
  undefined1 local_30 [4];
  undefined1 local_2c [20];
  uint local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954ab8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = uVar1;
  FUN_00733540(uVar1);
  local_8 = 0;
  local_44 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  local_48 = 0;
  local_90 = FUN_00919723(0xa0);
  local_8._0_1_ = 1;
  if (local_90 == 0) {
    local_138 = (undefined4 *)0x0;
  }
  else {
    local_138 = (undefined4 *)FUN_00716e60();
  }
  local_8c = local_138;
  local_8._0_1_ = 0;
  local_40 = local_138;
  FUN_00733570(&DAT_009938e4);
  local_8._0_1_ = 2;
  FUN_00733570(&DAT_00997274);
  local_8._0_1_ = 3;
  local_3c = FUN_0071a3d0(local_30,local_a0,local_98);
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 0;
  FUN_00733780();
  if (local_3c == 1) {
    FUN_00733570("PARTYTABLE");
    local_8._0_1_ = 4;
    FUN_00733570(&DAT_00993e10);
    local_8._0_1_ = 5;
    FUN_00733df0(local_b0,local_a8);
    local_8._0_1_ = 6;
    uVar2 = FUN_00733df0(local_c0,local_b8);
    local_8._0_1_ = 7;
    FUN_007337c0(uVar2);
    local_8._0_1_ = 6;
    FUN_00733780();
    local_8._0_1_ = 5;
/* ... 151 more lines truncated by --max-lines */
```

### decoy: 0x005fb170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fb170: 16-byte aligned: YES
  005fb170  55             PUSH EBP
  005fb171  8B EC          MOV EBP,ESP
  005fb173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fb170
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

## Card B0-14-04

GOG function: CSWCArea::ClearAllMapPins  gog 0x00457310  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007aa940 (size 497, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::RestartMiniGame
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aa910:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004571f0 CSWCArea::GetIsDark -> 0x007aa7f0  gate_class=AUTO
- GOG-order neighbour after:  0x00457340 CSWCArea::RestartMiniGame -> 0x007aa940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0xc,0x20,0x3c,0x48,0x60,0x64,0x268}
imm sets (* = in both, 0 common): GOG {0x23c,0x248} | Steam {0x4,0x10,0x60,0x11c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007aa940: 16-byte aligned: YES
  007aa940  55             PUSH EBP
  007aa941  8B EC          MOV EBP,ESP
  007aa943  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007aa940
string literals (0):
direct callees (10):
  0x00919723  operator new
  0x00839260  (unnamed)
  0x0083B720  (unnamed)
  0x0083B7B0  (unnamed)
  0x0083B940  (unnamed)
  0x00710840  (unnamed)
  0x0083B9A0  (unnamed)
  0x0083DB30  (unnamed)
  0x0083BBB0  (unnamed)
  0x008398F0  (unnamed)
struct offsets touched (top 7):
  +0x268 [dec 616]  x10  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x60 [dec 96]  x1  via ECX
  +0x64 [dec 100]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007aa940 FUN_007aa940 size=497 ret=0 callers=1 callees=10

undefined4 __fastcall FUN_007aa940(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_6c;
  undefined1 local_48 [16];
  undefined1 local_38 [12];
  int local_2c;
  undefined4 local_28;
  int *local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096825b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x268) != 0) {
    local_2c = FUN_00919723(0x11c,uVar1);
    local_8 = 0;
    if (local_2c == 0) {
      local_6c = 0;
    }
    else {
      local_6c = FUN_00839260(param_1);
    }
    local_28 = local_6c;
    local_8 = 0xffffffff;
    local_1c = local_6c;
    local_20 = *(int *)(*(int *)(param_1 + 0x268) + 0x3c);
    for (local_14 = 0; local_14 < local_20; local_14 = local_14 + 1) {
      local_18 = FUN_0083b720(local_14);
      FUN_0083b7b0(local_18,local_18 + 0x10);
    }
    local_20 = *(int *)(*(int *)(param_1 + 0x268) + 0x48);
    for (local_14 = 0; local_14 < local_20; local_14 = local_14 + 1) {
      iVar2 = FUN_0083b940(local_14);
      local_24 = *(int **)(iVar2 + 0x60);
      uVar3 = (**(code **)(*local_24 + 0xc))();
      FUN_00710840(uVar3);
      uVar3 = (**(code **)(*local_24 + 0x64 [dec 100] {int*: byte +0x190}))(local_38);
      FUN_0083b9a0(local_48,uVar3);
    }
    iVar2 = FUN_0083db30();
    if (iVar2 == 0) {
      ExceptionList = local_10;
      return 0;
    }
    FUN_0083bbb0(*(undefined4 *)(*(int *)(param_1 + 0x268) + 0x20));
    if (*(undefined4 **)(param_1 + 0x268) == (undefined4 *)0x0) {
      uVar3 = 0;
    }
    else {
      uVar3 = (**(code **)**(undefined4 **)(param_1 + 0x268))(1);
    }
    *(undefined4 *)(param_1 + 0x268) = local_1c;
    FUN_008398f0(uVar1,uVar3);
  }
  ExceptionList = local_10;
  return *(undefined4 *)(param_1 + 0x268);
}
```

### decoy: 0x007aa910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aa910: 16-byte aligned: YES
  007aa910  55             PUSH EBP
  007aa911  8B EC          MOV EBP,ESP
  007aa913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aa910
string literals (0):
direct callees (2):
  0x00474030  (unnamed)
  0x004292D0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-14-05

GOG function: Gob::Render  gog 0x0089aba0  callconv=__thiscall  param_size_bytes=1  gog_size=1673  gog_retn=4
Candidate Steam address: 0x004b3fe0 (size 1673, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 3 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'GetPath'@0098dac4; 'GetCreatureRadius'@0098dacc; 'GetPersonalRadius'@0098dae0; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004a81d0:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0089a940 Gob::ProxyPartDraw  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0089b230 Global::AurGrassMaterialInUse -> 0x0050c3e0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0081b710 CAurObjectReference::Destructor -> Steam ?  in candidate callees: n/a
  0x008235c0 GLRender::DrawSphereBound -> Steam ?  in candidate callees: n/a
  0x00857940 CHitInfo::Constructor -> Steam ?  in candidate callees: n/a
  0x00883220 unnamed -> Steam ?  in candidate callees: n/a
  0x008834f0 unnamed -> Steam ?  in candidate callees: n/a
  0x0089a940 Gob::ProxyPartDraw -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x008f4ff0 Material::BindTexture0 -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (4): 'GetPath'; 'GetCreatureRadius'; 'GetPersonalRadius'; '=propagaterender'
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x13*,0x50*,0x84*,0x8c*,0x98*,0xf4*,0x108*,0x164*,0x181*,0x190*,0x192*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x13*,0x50*,0x84*,0x8c*,0x98*,0xf4*,0x108*,0x164*,0x181*,0x190*,0x192*,...(+3)}
imm sets (* = in both, 22 common): GOG {0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0xa4*,0xa8*,0xb0*,0xb4*,0xc0*,0xcc*,0xf0*,...(+8)} | Steam {0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0xa4*,0xa8*,0xb0*,0xb4*,0xc0*,0xcc*,0xf0*,...(+8)}

### abi
```
GOG param bytes: 1; callconv: thiscall
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004b3fe0: 16-byte aligned: YES
  004b3fe0  55             PUSH EBP
  004b3fe1  8B EC          MOV EBP,ESP
  004b3fe3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004b3fe0
string literals (3):
  0x0098DAE0  at 004b43a1  'GetPersonalRadius'
  0x0098DACC  at 004b43c8  'GetCreatureRadius'
  0x0098DAC4  at 004b43f0  'GetPath'
direct callees (10):
  0x00465560  (unnamed)
  0x00473E80  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x0049C930  (unnamed)
  0x004DAE60  (unnamed)
  0x004DB1F0  (unnamed)
  0x004B3D80  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x00430EA0  (unnamed)
  0x0049C660  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x20  via EAX,ECX,EDX
  +0x8 [dec 8]  x20  via EAX,ECX,EDX
  +0x108 [dec 264]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x1B4 [dec 436]  x4  via EAX,EDX
  +0x1E0 [dec 480]  x4  via EAX,ECX,EDX
  +0x84 [dec 132]  x3  via EAX,ECX
  +0x50 [dec 80]  x2  via ECX,EDX
  +0x98 [dec 152]  x2  via EAX
  +0x194 [dec 404]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004b3fe0 FUN_004b3fe0 size=1673 ret=4 callers=1 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004b3fe0(int *param_1,char param_2)

{
  uint uVar1;
  int iVar2;
  float local_100;
  float local_fc;
  float local_f8;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined1 local_88 [93];
  undefined1 local_2b;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094bc13;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (DAT_009f654c != 0) {
    if (param_1[0x26 = byte +0x98] == 0) {
      param_1[0x30 = byte +0xc0] = param_1[0x29 = byte +0xa4];
      param_1[0x31 = byte +0xc4] = param_1[0x2a = byte +0xa8];
      param_1[0x32 = byte +0xc8] = param_1[0x2b = byte +0xac];
      param_1[0x33 = byte +0xcc] = param_1[0x2c = byte +0xb0];
      param_1[0x34 = byte +0xd0] = param_1[0x2d = byte +0xb4];
      param_1[0x35 = byte +0xd4] = param_1[0x2e = byte +0xb8];
      param_1[0x36 = byte +0xd8] = param_1[0x2f = byte +0xbc];
    }
    else if (((DAT_009f6548 == 0) && (param_1[0x21 = byte +0x84] != 0)) &&
            ((*(ushort *)(param_1[0x21 = byte +0x84] + 0x50) & 4) != 0)) {
      param_1[0x30 = byte +0xc0] = param_1[0x29 = byte +0xa4];
      param_1[0x31 = byte +0xc4] = param_1[0x2a = byte +0xa8];
      param_1[0x32 = byte +0xc8] = param_1[0x2b = byte +0xac];
      param_1[0x33 = byte +0xcc] = param_1[0x2c = byte +0xb0];
      param_1[0x34 = byte +0xd0] = param_1[0x2d = byte +0xb4];
      param_1[0x35 = byte +0xd4] = param_1[0x2e = byte +0xb8];
      param_1[0x36 = byte +0xd8] = param_1[0x2f = byte +0xbc];
    }
    else if ((*(char *)((int)param_1 + 0x192) == '\0') && (DAT_00a73708 == 0)) {
      param_1[0x30 = byte +0xc0] = param_1[0x29 = byte +0xa4];
      param_1[0x31 = byte +0xc4] = param_1[0x2a = byte +0xa8];
      param_1[0x32 = byte +0xc8] = param_1[0x2b = byte +0xac];
      param_1[0x33 = byte +0xcc] = param_1[0x2c = byte +0xb0];
      param_1[0x34 = byte +0xd0] = param_1[0x2d = byte +0xb4];
      param_1[0x35 = byte +0xd4] = param_1[0x2e = byte +0xb8];
      param_1[0x36 = byte +0xd8] = param_1[0x2f = byte +0xbc];
    }
    else {
      if ((DAT_00a73750 != 0) &&
         (((*(ushort *)(param_1[0x21 = byte +0x84] + 0x50) & 8) != 0 && (DAT_00a20020 != 0)))) {
        local_94 = *(undefined4 *)(DAT_00a32a48 + 0xa8);
        local_90 = *(undefined4 *)(DAT_00a32a48 + 0xac);
        local_8c = *(undefined4 *)(DAT_00a32a48 + 0xb0);
        local_1c = *(undefined4 *)(DAT_00a20020 + 0xb4);
        local_18 = *(undefined4 *)(DAT_00a20020 + 0xb8);
        local_14 = *(undefined4 *)(DAT_00a20020 + 0xbc);
        FUN_00465560(0,0xbfffffff,0);
        local_8 = 0;
        local_2b = 0;
        iVar2 = (**(code **)(*param_1 + 0x8c))
                          (local_94,local_90,local_8c,local_1c,local_18,local_14,&local_1c,local_88)
        ;
        if (iVar2 == 0) {
          param_1[0x42 = byte +0x108] = (int)(_DAT_009f62b0 / _DAT_009f6220 + (float)param_1[0x42 = byte +0x108]);
        }
        else {
          param_1[0x42 = byte +0x108] = (int)((float)param_1[0x42 = byte +0x108] - _DAT_009f62b0 / _DAT_009f6220);
/* ... 70 more lines truncated by --max-lines */
```

### decoy: 0x004a81d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004a81d0: 16-byte aligned: YES
  004a81d0  55             PUSH EBP
  004a81d1  8B EC          MOV EBP,ESP
  004a81d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004a81d0
string literals (0):
direct callees (10):
  0x0049C930  (unnamed)
  0x00428F80  (unnamed)
  0x00432EB0  (unnamed)
  0x00432EE0  (unnamed)
  0x0091DBA0  _memcpy
  0x00432F10  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00435410  (unnamed)
  0x00513DA0  (unnamed)
  0x00513E20  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x29  via EAX,ECX,EDX
  +0x8 [dec 8]  x29  via EAX,ECX,EDX
  +0x4C [dec 76]  x13  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x74 [dec 116]  x4  via EAX,ECX,EDX
  +0x78 [dec 120]  x3  via EAX,EDX
  +0x50 [dec 80]  x2  via EAX,ECX
  +0x6C [dec 108]  x2  via ECX
  +0x80 [dec 128]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-14-06

GOG function: CSWGuiBorder::Destructor  gog 0x00912fc0  callconv=__thiscall  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00414bf0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00414bc0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00912f70 CRes::GetData  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00912ff0 CSWGuiBorder::Initialize -> 0x00414c20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x00414bf0: 16-byte aligned: YES
  00414bf0  55             PUSH EBP
  00414bf1  8B EC          MOV EBP,ESP
  00414bf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00414bf0
string literals (0):
direct callees (2):
  0x00414C60  CSWGuiBorder::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00414bf0 FUN_00414bf0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00414bf0(undefined4 param_1,uint param_2)

{
  FUN_00414c60();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00414bc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414bc0: 16-byte aligned: YES
  00414bc0  55             PUSH EBP
  00414bc1  8B EC          MOV EBP,ESP
  00414bc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414bc0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-14-07

GOG function: ConsoleFunc::Constructor  gog 0x00867740  callconv=(blank)  param_size_bytes=8  gog_size=160  gog_retn=8
Candidate Steam address: 0x004757d0 (size 160, retn 8)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ConsoleFunc::NoParamConstructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475870:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867740 ConsoleFunc::NoParamConstructor -> 0x004757d0  gate_class=REVIEW
- GOG-order neighbour after:  0x008677e0 ConsoleFunc::StringConstructor -> 0x00475870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00866e80 unnamed -> Steam ?  in candidate callees: n/a
  0x008684e0 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x004757d0: 16-byte aligned: YES
  004757d0  55             PUSH EBP
  004757d1  8B EC          MOV EBP,ESP
  004757d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004757d0 FUN_004757d0 size=160 ret=8 callers=11 callees=4

int __thiscall FUN_004757d0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0091cff0(param_1,param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar1 = FUN_00919723(8);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_00476570(param_3);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x50) = local_20;
  FUN_00474f10(param_1);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00475870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475870: 16-byte aligned: YES
  00475870  55             PUSH EBP
  00475871  8B EC          MOV EBP,ESP
  00475873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00475870
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765C0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-14-08

GOG function: Gob::RenderBlur  gog 0x0089a850  callconv=(blank)  param_size_bytes=0  gog_size=231  gog_retn=0
Candidate Steam address: 0x004b3c90 (size 231, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004b33e0:STR0/CALLEE0/SIG0.394/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00899fa0 Gob::PartVolumeProject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0089a940 Gob::ProxyPartDraw  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008834f0 unnamed -> Steam ?  in candidate callees: n/a
  0x0089a940 Gob::ProxyPartDraw -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x98*,0x1e0*} | Steam {0x4*,0x8*,0xc*,0x98*,0x1e0*}
imm sets (* = in both, 7 common): GOG {0xc*,0x44*,0x4c*,0xa4*,0xb0*,0xc0*,0xcc*} | Steam {0xc*,0x44*,0x4c*,0xa4*,0xb0*,0xc0*,0xcc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004b3c90: 16-byte aligned: YES
  004b3c90  55             PUSH EBP
  004b3c91  8B EC          MOV EBP,ESP
  004b3c93  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004b3c90
string literals (0):
direct callees (2):
  0x0049C930  (unnamed)
  0x004B3D80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x1E0 [dec 480]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004b3c90 FUN_004b3c90 size=231 ret=0 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004b3c90(int param_1)

{
  glPushMatrix();
  FUN_0049c930(param_1 + 0xa4,param_1 + 0xb0,1);
  if (*(float *)(param_1 + 0x1e0) != (float)_DAT_00986b50) {
    glScalef(*(undefined4 *)(param_1 + 0x1e0),*(undefined4 *)(param_1 + 0x1e0),
             *(undefined4 *)(param_1 + 0x1e0));
  }
  FUN_004b3d80(*(undefined4 *)(param_1 + 0x98),1);
  *(undefined4 *)(param_1 + 0xc0) = *(undefined4 *)(param_1 + 0xa4);
  *(undefined4 *)(param_1 + 0xc4) = *(undefined4 *)(param_1 + 0xa8);
  *(undefined4 *)(param_1 + 0xc8 [dec 200]) = *(undefined4 *)(param_1 + 0xac);
  *(undefined4 *)(param_1 + 0xcc) = *(undefined4 *)(param_1 + 0xb0);
  *(undefined4 *)(param_1 + 0xd0) = *(undefined4 *)(param_1 + 0xb4);
  *(undefined4 *)(param_1 + 0xd4) = *(undefined4 *)(param_1 + 0xb8);
  *(undefined4 *)(param_1 + 0xd8) = *(undefined4 *)(param_1 + 0xbc);
  glPopMatrix();
  return;
}
```

### decoy: 0x004b33e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004b33e0: 16-byte aligned: YES
  004b33e0  55             PUSH EBP
  004b33e1  8B EC          MOV EBP,ESP
  004b33e3  81 EC 58 01 00 00 SUB ESP,0x158
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004b33e0
string literals (0):
direct callees (5):
  0x0049C930  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00514C40  (unnamed)
  0x004A9720  (unnamed)
  0x004B33E0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x15  via ECX,EDX
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x1E0 [dec 480]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via ECX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x5C [dec 92]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
```

## Card B0-14-09

GOG function: Gob::EndLookAtAnimate  gog 0x008f6e70  callconv=(blank)  param_size_bytes=0  gog_size=48  gog_retn=0
Candidate Steam address: 0x004dcce0 (size 48, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005046e0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f6c10 Gob::BeginLookAtAnimate  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f6ea0 Global::AnimationParseField  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x1a0*} | Steam {0x8*,0x1a0*}
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
prologue for 0x004dcce0: 16-byte aligned: YES
  004dcce0  55             PUSH EBP
  004dcce1  8B EC          MOV EBP,ESP
  004dcce3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dcce0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1A0 [dec 416]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004dcce0 FUN_004dcce0 size=48 ret=0 callers=0 callees=0

void __fastcall FUN_004dcce0(int param_1)

{
  if (*(int *)(param_1 + 0x1a0) != 0) {
    (**(code **)(**(int **)(param_1 + 0x1a0) + 8))();
  }
  return;
}
```

### decoy: 0x005046e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005046e0: 16-byte aligned: YES
  005046e0  55             PUSH EBP
  005046e1  8B EC          MOV EBP,ESP
  005046e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005046e0
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 3):
  +0x1B4 [dec 436]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
```

## Card B0-14-10

GOG function: CResGFF::WriteFieldBYTE  gog 0x00625b30  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719800 (size 306, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::AddListElement
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.35 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719940:STR0/CALLEE2/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625aa0 CResGFF::AddStructToStruct -> 0x00719940  gate_class=AUTO
- GOG-order neighbour after:  0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x4,0x8*,0x70,0x98*}
imm sets (* = in both, 1 common): GOG {0x8,0xc*} | Steam {0x4,0xc*,0xf,0x1c}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719800: 16-byte aligned: YES
  00719800  55             PUSH EBP
  00719801  8B EC          MOV EBP,ESP
  00719803  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719800
string literals (0):
direct callees (6):
  0x00717AA0  CResGFF::InitializeForWriting
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717880  CResGFF::GetDataLayoutList
  0x00718580  CResGFF::AddDataLayoutList
  0x00717F30  CResGFF::AddStruct
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x70 [dec 112]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719800 FUN_00719800 size=306 ret=12 callers=44 callees=6

undefined4 __thiscall FUN_00719800(int param_1,int *param_2,int param_3,undefined4 param_4)

{
  undefined4 uVar1;
  int iVar2;
  uint local_18;
  int *local_14;
  int local_10;
  int *local_c;
  int *local_8;
  
  if ((param_3 == 0) || (param_2 == (int *)0x0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    uVar1 = FUN_007178e0(param_3,param_3 + 4);
    local_8 = (int *)FUN_00717770(param_3,uVar1);
    if ((local_8 == (int *)0x0) || (*local_8 != 0xf)) {
      uVar1 = 0;
    }
    else {
      local_14 = (int *)FUN_00717880(local_8,&local_18);
      if ((local_14 == (int *)0x0) || (local_18 < 4)) {
        uVar1 = 0;
      }
      else {
        local_10 = *local_14;
        if (local_18 < local_10 * 4 + 4U) {
          uVar1 = 0;
        }
        else {
          local_c = (int *)FUN_00718580(local_8[2 = byte +0x8],local_10 * 4 + 4,local_10 * 4 + 8);
          if (local_c == (int *)0x0) {
            uVar1 = 0;
          }
          else {
            local_8[2 = byte +0x8] = (int)local_c - *(int *)(param_1 + 0x70);
            iVar2 = FUN_00717f30(param_4);
            *param_2 = iVar2;
            if (*param_2 == -1) {
              uVar1 = 0;
            }
            else {
              *local_c = local_10 + 1;
              local_c[local_10 + 1] = *param_2;
              uVar1 = 1;
            }
          }
        }
      }
    }
  }
  return uVar1;
}
```

### decoy: 0x00719940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719940: 16-byte aligned: YES
  00719940  55             PUSH EBP
  00719941  8B EC          MOV EBP,ESP
  00719943  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719940
string literals (0):
direct callees (3):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
  0x00717F30  CResGFF::AddStruct
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-14-11

GOG function: CSWGuiPanel::Constructor  gog 0x0090cb30  callconv=__thiscall  param_size_bytes=4  gog_size=423  gog_retn=4
Candidate Steam address: 0x0040e5a0 (size 423, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 4 (Constructor 0x414a50,Constructor 0x893950,Constructor 0x8ba980); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040e480:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090ca10 CSWGuiExtent::Union  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090cce0 CSWGuiPanel::Destructor -> 0x0040e860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00912e20 CSWGuiBorder::Constructor -> Steam 0x00414a50  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0xc*,0x18*,0x1c*,0x20*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*} | Steam {0x4*,0x8*,0xc*,0x18*,0x1c*,0x20*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*}
imm sets (* = in both, 11 common): GOG {0x2*,0x4*,0x18*,0x24*,0x40*,0x54*,0x78*,0xfffffcff*,0xffffff7f*,0xffffffcf*,0xfffffffd*} | Steam {0x2*,0x4*,0x18*,0x24*,0x40*,0x54*,0x78*,0xfffffcff*,0xffffff7f*,0xffffffcf*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0040e5a0: 16-byte aligned: YES
  0040e5a0  55             PUSH EBP
  0040e5a1  8B EC          MOV EBP,ESP
  0040e5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040e5a0
string literals (0):
direct callees (3):
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x00414A50  CSWGuiBorder::Constructor
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x4C [dec 76]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e5a0 FUN_0040e5a0 size=423 ret=4 callers=75 callees=3

undefined4 * __thiscall FUN_0040e5a0(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009482ae;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4 = byte +0x10] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[1 = byte +0x4] = 0;
  local_8 = 0;
  *param_1 = CSWGuiPanel::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  param_1[0x15 = byte +0x54] = 0;
  param_1[0x16 = byte +0x58] = 0;
  param_1[0x17 = byte +0x5c] = 0;
  iVar2 = FUN_00919723(0x78,uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  if (iVar2 == 0) {
    local_28 = 0;
  }
  else {
    local_28 = FUN_00414a50();
  }
  param_1[0x18 = byte +0x60] = local_28;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xfffffffd;
  param_1[8 = byte +0x20] = 0;
  param_1[7 = byte +0x1c] = param_2;
  param_1[0xc = byte +0x30] = 0;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 4;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 0x40;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xffffffcf;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xfffffcff;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xffffff7f;
  param_1[0x14 = byte +0x50] = 0x3f800000;
  *(undefined1 *)(param_1 + 0x13) = 0;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 1;
  *(undefined1 *)(param_1 + 0x19) = 0xff;
  param_1[6 = byte +0x18] = 1;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0040e480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

## Card B0-14-12

GOG function: CSWSEffectListHandler::Destructor  gog 0x0064ceb0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0051d480 (size 45, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d450:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ccf0 CServerAIMaster::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064cee0 CSWSItemPropertyHandler::Destructor -> 0x0051d4e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00736240 CSWSEffectListHandler::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051d480: 16-byte aligned: YES
  0051d480  55             PUSH EBP
  0051d481  8B EC          MOV EBP,ESP
  0051d483  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d480
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051d480 FUN_0051d480 size=45 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_0051d480(undefined4 *param_1,uint param_2)

{
  *param_1 = CItemPropertyApplierRemover::vftable;
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0051d450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d450: 16-byte aligned: YES
  0051d450  55             PUSH EBP
  0051d451  8B EC          MOV EBP,ESP
  0051d453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d450
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-14-13

GOG function: ConsoleFunc::Constructor_3  gog 0x00867880  callconv=(blank)  param_size_bytes=8  gog_size=160  gog_retn=8
Candidate Steam address: 0x00475910 (size 160, retn 8)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ConsoleFunc::IntConstructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00475870:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867880 ConsoleFunc::IntConstructor -> 0x00475910  gate_class=REVIEW
- GOG-order neighbour after:  0x00867920 ConsoleFunc::Destructor -> 0x004759b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00866e80 unnamed -> Steam ?  in candidate callees: n/a
  0x00868540 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00475910: 16-byte aligned: YES
  00475910  55             PUSH EBP
  00475911  8B EC          MOV EBP,ESP
  00475913  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00475910
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765F0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00475910 FUN_00475910 size=160 ret=8 callers=2 callees=4

int __thiscall FUN_00475910(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0091cff0(param_1,param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar1 = FUN_00919723(8);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_004765f0(param_3);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x50) = local_20;
  FUN_00474f10(param_1);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00475870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475870: 16-byte aligned: YES
  00475870  55             PUSH EBP
  00475871  8B EC          MOV EBP,ESP
  00475873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00475870
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765C0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-14-14

GOG function: CSWSEffectListHandler::Destructor  gog 0x0064ceb0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0051d4e0 (size 44, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::Destructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d4b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ccf0 CServerAIMaster::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064cee0 CSWSItemPropertyHandler::Destructor -> 0x0051d4e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00736240 CSWSEffectListHandler::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051d4e0: 16-byte aligned: YES
  0051d4e0  55             PUSH EBP
  0051d4e1  8B EC          MOV EBP,ESP
  0051d4e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d4e0
string literals (0):
direct callees (2):
  0x005AA3C0  CSWSItemPropertyHandler::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051d4e0 FUN_0051d4e0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0051d4e0(undefined4 param_1,uint param_2)

{
  FUN_005aa3c0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0051d4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d4b0: 16-byte aligned: YES
  0051d4b0  55             PUSH EBP
  0051d4b1  8B EC          MOV EBP,ESP
  0051d4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d4b0
string literals (0):
direct callees (2):
  0x00593960  CSWSEffectListHandler::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-14-15

GOG function: CResGFF::WriteFieldBYTE  gog 0x00625b30  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719a40 (size 97, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::WriteFieldCHAR
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007199d0:STR0/CALLEE2/SIG1/SRV0/VT0
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

## Card B0-14-16

GOG function: CSWPartyTable::GetNPCSelectability  gog 0x00700cb0  callconv=(blank)  param_size_bytes=4  gog_size=42  gog_retn=4
Candidate Steam address: 0x005fb0f0 (size 65, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::GetIsAvailable
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fb140:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00700c60 CSWPartyTable::GetIsAvailable -> 0x005fb0f0  gate_class=AUTO
- GOG-order neighbour after:  0x00700ce0 CSWPartyTable::SetNPCSelectability -> 0x005fb170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007004d0 CSWPartyTable::GetIsNPCAvailable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x7c} | Steam {0x1c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0xc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005fb0f0: 16-byte aligned: YES
  005fb0f0  55             PUSH EBP
  005fb0f1  8B EC          MOV EBP,ESP
  005fb0f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fb0f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fb0f0 FUN_005fb0f0 size=65 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_005fb0f0(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (0xb < local_8) {
      return 0;
    }
    if (param_2 == *(int *)(param_1 + 0x1c + local_8 * 4)) break;
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### decoy: 0x005fb140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fb140: 16-byte aligned: YES
  005fb140  55             PUSH EBP
  005fb141  8B EC          MOV EBP,ESP
  005fb143  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fb140
string literals (0):
direct callees (1):
  0x005FA960  CSWPartyTable::GetIsNPCAvailable
struct offsets touched (top 1):
  +0x7C [dec 124]  x1  via ECX
```

## Card B0-14-17

GOG function: ConsoleFunc::Destructor  gog 0x00867920  callconv=(blank)  param_size_bytes=0  gog_size=72  gog_retn=0
Candidate Steam address: 0x004759b0 (size 72, retn 0)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00475910:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867880 ConsoleFunc::Constructor_3 -> 0x00475910  gate_class=REVIEW
- GOG-order neighbour after:  0x00867a20 Global::AurResGet  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00866f10 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004759b0: 16-byte aligned: YES
  004759b0  55             PUSH EBP
  004759b1  8B EC          MOV EBP,ESP
  004759b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004759b0
string literals (0):
direct callees (1):
  0x00474FA0  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004759b0 FUN_004759b0 size=72 ret=0 callers=35 callees=1

void __fastcall FUN_004759b0(int param_1)

{
  FUN_00474fa0(param_1);
  if (*(undefined4 **)(param_1 + 0x50) != (undefined4 *)0x0) {
    (**(code **)**(undefined4 **)(param_1 + 0x50))(1);
  }
  return;
}
```

### decoy: 0x00475910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475910: 16-byte aligned: YES
  00475910  55             PUSH EBP
  00475911  8B EC          MOV EBP,ESP
  00475913  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00475910
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765F0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

## Card B0-14-18

GOG function: Gob::AddAnimationFlag  gog 0x008fd240  callconv=(blank)  param_size_bytes=4  gog_size=67  gog_retn=4
Candidate Steam address: 0x004e3110 (size 67, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3160:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fcfb7 Gob::Animate_6  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fd290 Gob::RemoveAnimationFlag -> 0x004e3160  gate_class=REVIEW

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
prologue for 0x004e3110: 16-byte aligned: YES
  004e3110  55             PUSH EBP
  004e3111  8B EC          MOV EBP,ESP
  004e3113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004e3110
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xDC [dec 220]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004e3110 FUN_004e3110 size=67 ret=4 callers=0 callees=0

void __thiscall FUN_004e3110(int param_1,uint param_2)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xe0) + -1;
  if (-1 < iVar1) {
    iVar1 = *(int *)(*(int *)(param_1 + 0xdc) + iVar1 * 4);
    *(uint *)(iVar1 + 8) = *(uint *)(iVar1 + 8) | param_2;
  }
  return;
}
```

### decoy: 0x004e3160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004e3160: 16-byte aligned: YES
  004e3160  55             PUSH EBP
  004e3161  8B EC          MOV EBP,ESP
  004e3163  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004e3160
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xDC [dec 220]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

## Card B0-14-19

GOG function: CSWVirtualMachineCommands::ExecuteCommandCutsceneMove  gog 0x00783260  callconv=__thiscall  param_size_bytes=8  gog_size=165  gog_retn=8
Candidate Steam address: 0x0066dba0 (size 212, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.70 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066dc80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00783180 CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack -> 0x0066dba0  gate_class=REVIEW
- GOG-order neighbour after:  0x00783310 Global::ExecuteCommandBarkString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640c40 CVirtualMachine::StackPopVector -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006810b0 CSWSCreature::AddCutsceneMoveActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0x8*,0x20,0xfffff82f*} | Steam {0x8*,0x1c,0x7f000000,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066dba0: 16-byte aligned: YES
  0066dba0  55             PUSH EBP
  0066dba1  8B EC          MOV EBP,ESP
  0066dba3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066dba0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005683D0  CSWSCreature::AddCutsceneAttackActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0066dba0 FUN_0066dba0 size=212 ret=8 callers=0 callees=4

undefined4 __fastcall FUN_0066dba0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0x7f000000;
  local_c = 0;
  local_18 = 0;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_006fd9a0(&local_18);
      if (iVar1 == 0) {
        uVar2 = 0xfffff82f;
      }
      else {
        iVar1 = FUN_006fd9a0(&local_8);
        if (iVar1 == 0) {
          uVar2 = 0xfffff82f;
        }
        else {
          local_14 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
          if (local_14 != 0) {
            FUN_005683d0(local_10,local_c,local_18,local_8);
          }
          uVar2 = 0;
        }
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0066dc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066dc80: 16-byte aligned: YES
  0066dc80  55             PUSH EBP
  0066dc81  8B EC          MOV EBP,ESP
  0066dc83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066dc80
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDA20  CVirtualMachine::StackPopVector
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568480  CSWSCreature::AddCutsceneMoveActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B0-14-20

GOG function: ConsoleFunc::Constructor_2  gog 0x008677e0  callconv=(blank)  param_size_bytes=8  gog_size=160  gog_retn=8
Candidate Steam address: 0x00475870 (size 160, retn 8)
Candidate sources: branch    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=ConsoleFunc::StringConstructor
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004757d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008677e0 ConsoleFunc::StringConstructor -> 0x00475870  gate_class=REVIEW
- GOG-order neighbour after:  0x00867880 ConsoleFunc::IntConstructor -> 0x00475910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00866e80 unnamed -> Steam ?  in candidate callees: n/a
  0x00868510 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924c20 Global::strcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x50*} | Steam {0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00475870: 16-byte aligned: YES
  00475870  55             PUSH EBP
  00475871  8B EC          MOV EBP,ESP
  00475873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00475870
string literals (0):
direct callees (4):
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004765C0  (unnamed)
  0x00474F10  (unnamed)
struct offsets touched (top 1):
  +0x50 [dec 80]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00475870 FUN_00475870 size=160 ret=8 callers=22 callees=4

int __thiscall FUN_00475870(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0091cff0(param_1,param_2,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar1 = FUN_00919723(8);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_004765c0(param_3);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x50) = local_20;
  FUN_00474f10(param_1);
  ExceptionList = local_10;
  return param_1;
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

## Card B0-14-21

GOG function: Scene::GetObjectScale  gog 0x00864850  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00472610 (size 22, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004725e0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00864820 Scene::SetGeomFadeProperties -> 0x004725e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00865bc5 Global::Catch_All@00865bc5  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1e0*} | Steam {0x1e0*}
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
prologue for 0x00472610: 16-byte aligned: YES
  00472610  55             PUSH EBP
  00472611  8B EC          MOV EBP,ESP
  00472613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00472610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1E0 [dec 480]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00472610 FUN_00472610 size=22 ret=4 callers=0 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

float10 FUN_00472610(int param_1)

{
  return (float10)*(float *)(param_1 + 0x1e0);
}
```

### decoy: 0x004725e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004725e0: 16-byte aligned: YES
  004725e0  55             PUSH EBP
  004725e1  8B EC          MOV EBP,ESP
  004725e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004725e0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-14-22

GOG function: CSWCArea::ClearAllMapPins  gog 0x00457310  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007aa7f0 (size 279, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::GetIsDark
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007aa910:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004571f0 CSWCArea::GetIsDark -> 0x007aa7f0  gate_class=AUTO
- GOG-order neighbour after:  0x00457340 CSWCArea::RestartMiniGame -> 0x007aa940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x188,0x238}
imm sets (* = in both, 0 common): GOG {0x23c,0x248} | Steam {0x8,0x40,0x41}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007aa7f0: 16-byte aligned: YES
  007aa7f0  55             PUSH EBP
  007aa7f1  8B EC          MOV EBP,ESP
  007aa7f3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007aa7f0
string literals (0):
direct callees (3):
  0x0073F450  CClientExoApp::GetControlledObject
  0x00774A80  CSWCCreature::GetCreatureLight
  0x0044B8A0  (unnamed)
struct offsets touched (top 3):
  +0x238 [dec 568]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x188 [dec 392]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007aa7f0 FUN_007aa7f0 size=279 ret=0 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_007aa7f0(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  
  local_14 = 0;
  local_10 = 0;
  local_c = 0;
  if (*(int *)(param_1 + 0x188) == 0) {
    uVar3 = 0;
  }
  else {
    iVar4 = FUN_0073f450();
    if (iVar4 != 0) {
      FUN_00774a80(0);
    }
    local_14 = 0x3f800000;
    local_10 = 0x3f800000;
    local_c = 0x3f800000;
    fVar5 = (float10)FUN_0044b8a0(&local_14,&local_14);
    fVar1 = (float)fVar5;
    if (_DAT_00a10700 * (float)_DAT_0099fd88 * _DAT_00a10700 * (float)_DAT_0099fd88 < fVar1) {
      fVar2 = _DAT_00a10704 * (float)_DAT_0099fd88 * _DAT_00a10704 * (float)_DAT_0099fd88;
      if (fVar2 < fVar1 == (fVar2 == fVar1)) {
        local_18 = *(undefined4 *)(param_1 + 0x238);
      }
      else {
        local_18 = 0;
      }
    }
    else {
      local_18 = 1;
    }
    *(undefined4 *)(param_1 + 0x238) = local_18;
    uVar3 = *(undefined4 *)(param_1 + 0x238);
  }
  return uVar3;
}
```

### decoy: 0x007aa910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007aa910: 16-byte aligned: YES
  007aa910  55             PUSH EBP
  007aa911  8B EC          MOV EBP,ESP
  007aa913  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007aa910
string literals (0):
direct callees (2):
  0x00474030  (unnamed)
  0x004292D0  (unnamed)
struct offsets touched (top 0):
```

## Card B0-14-23

GOG function: CSWVirtualMachineCommands::ExecuteCommandCutsceneMove  gog 0x00783260  callconv=__thiscall  param_size_bytes=8  gog_size=165  gog_retn=8
Candidate Steam address: 0x0066dd30 (size 567, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandBarkString
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066dc80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00783180 CSWVirtualMachineCommands::ExecuteCommandCutsceneAttack -> 0x0066dba0  gate_class=REVIEW
- GOG-order neighbour after:  0x00783310 Global::ExecuteCommandBarkString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640c40 CVirtualMachine::StackPopVector -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x006810b0 CSWSCreature::AddCutsceneMoveActions -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {0x4,0x4c,0xcc,0x1d8,0x1dc}
imm sets (* = in both, 2 common): GOG {0x8*,0x20,0xfffff82f*} | Steam {0x2,0x3,0x8*,0x78,0x7f000000,0xfffff82f*,0xfffff830}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066dd30: 16-byte aligned: YES
  0066dd30  55             PUSH EBP
  0066dd31  8B EC          MOV EBP,ESP
  0066dd33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0066dd30
string literals (0):
direct callees (12):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F750  ClientApp::GetGuiInGame
  0x00733540  CExoString::CExoString(empty)
  0x007107F0  (unnamed)
  0x0071DF90  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x007CA6E0  CGuiInGame::ShowBarkBubble
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x00740260  CClientExoApp::BarkString
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x4C [dec 76]  x1  via EAX
  +0xCC [dec 204]  x1  via ECX
  +0x1D8 [dec 472]  x1  via ECX
  +0x1DC [dec 476]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0066dd30 FUN_0066dd30 size=567 ret=8 callers=0 callees=12

void FUN_0066dd30(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 extraout_var;
  undefined1 local_5c [8];
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined1 local_44 [8];
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  uint local_28;
  undefined4 local_24;
  undefined4 local_20;
  uint local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959d48;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = uVar1;
  iVar2 = FUN_006fdaf0(&local_1c);
  if ((iVar2 != 0) && (uVar5 = extraout_var, iVar2 = FUN_006fd9a0(&local_20), iVar2 != 0)) {
    local_24 = 0xffffffff;
    local_14 = 0xffffffff;
    if ((param_2 < 3) ||
       ((iVar2 = FUN_006fd9a0(&local_24), iVar2 != 0 &&
        (iVar2 = FUN_006fd9a0(&local_14), iVar2 != 0)))) {
      iVar2 = FUN_0073f750(uVar1,uVar5);
      local_18 = *(int *)(iVar2 + 0x4c);
      if (local_18 != 0) {
        *(undefined4 *)(local_18 + 0x1d8) = local_24;
        *(undefined4 *)(local_18 + 0x1dc) = local_14;
      }
      iVar2 = FUN_0073f750();
      if (*(int *)(iVar2 + 0xcc) == 0) {
        FUN_00733540();
        local_8 = 0;
        FUN_007107f0();
        local_8 = 1;
        FUN_0071df90(local_20,local_44,DAT_0099c4a2);
        local_54 = local_3c;
        local_50 = local_38;
        local_4c = local_34;
        local_48 = local_30;
        FUN_007335f0(local_44);
        local_8 = CONCAT31(local_8._1_3_,2);
        if (local_1c == 0x7f000000) {
          uVar5 = 0;
          puVar4 = &local_54;
          puVar3 = local_5c;
          uVar1 = local_1c;
          FUN_0073f750(0x7f000000,puVar3,puVar4,0);
          FUN_007ca6e0(uVar1,puVar3,puVar4,uVar5);
        }
        else {
          local_1c = FUN_0073fec0(local_1c);
          Concurrency::details::SchedulerProxy::DoHillClimbing
                    (*(SchedulerProxy **)(DAT_00a1b4a4 + 4),local_1c,(uint)local_5c,(uint)&local_54,
                     0);
        }
        local_8 = CONCAT31(local_8._1_3_,1);
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
/* ... 7 more lines truncated by --max-lines */
```

### decoy: 0x0066dc80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066dc80: 16-byte aligned: YES
  0066dc80  55             PUSH EBP
  0066dc81  8B EC          MOV EBP,ESP
  0066dc83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066dc80
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDA20  CVirtualMachine::StackPopVector
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00568480  CSWSCreature::AddCutsceneMoveActions
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B0-14-24

GOG function: Scene::SetGeomFadeProperties  gog 0x00864820  callconv=(blank)  param_size_bytes=4  gog_size=45  gog_retn=4
Candidate Steam address: 0x004725e0 (size 45, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00472610:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008647f0 Scene::SetObjectScale -> 0x004725b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00864850 Scene::GetObjectScale -> 0x00472610  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*} | Steam {0x2*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004725e0: 16-byte aligned: YES
  004725e0  55             PUSH EBP
  004725e1  8B EC          MOV EBP,ESP
  004725e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004725e0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004725e0 FUN_004725e0 size=45 ret=4 callers=0 callees=0

undefined4 FUN_004725e0(int param_1)

{
  undefined4 uVar1;
  
  if (((param_1 == 1) || (param_1 == 2)) || (param_1 == 4)) {
    DAT_009f5d28 = param_1;
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00472610 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00472610: 16-byte aligned: YES
  00472610  55             PUSH EBP
  00472611  8B EC          MOV EBP,ESP
  00472613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00472610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1E0 [dec 480]  x1  via EAX
```

## Card B0-14-25

GOG function: CSWGuiControl::AddChildControl  gog 0x009175d0  callconv=__thiscall  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00419330 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00419350:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00917540 CSWGuiControl::HandleRMouseUp  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009175f0 CSWGuiControl::GetIsChild -> 0x00419350  gate_class=AUTO

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
