# Review cards B0 chunk 11

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-11-01

GOG function: CSWSItemPropertyHandler::ApplyDecreaseAbilityScore  gog 0x007c8270  callconv=__thiscall  param_size_bytes=12  gog_size=485  gog_retn=20
Candidate Steam address: 0x005b0e60 (size 485, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0b50:STR0/CALLEE0/SIG0.722/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c80d0 CSWSItemPropertyHandler::RemoveDamageVulnerability -> 0x005b0cc0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8470 CSWSItemPropertyHandler::RemoveDecreaseAbilityScore -> 0x005b1060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 7 common): GOG {0x4*,0x5*,0x14*,0x15*,0x3c*,0x8c*,0xfffffff8*} | Steam {0x4*,0x5*,0x14*,0x15*,0x3c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b0e60: 16-byte aligned: YES
  005b0e60  55             PUSH EBP
  005b0e61  8B EC          MOV EBP,ESP
  005b0e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0e60
string literals (1):
  0x00992E8C  at 005b0e88  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0e60 FUN_005b0e60 size=485 ret=20 callers=0 callees=9

undefined4
FUN_005b0e60(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e03;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_18;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x15);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_18 = -local_18;
  if (local_18 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_48;
    local_34 = (undefined2)DAT_00995fb0;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    switch(*(undefined2 *)(param_2 + 2)) {
    case 0:
      FUN_005e4e80(0,DAT_009960f8);
      break;
    case 1:
      FUN_005e4e80(0,DAT_009960fc);
      break;
    case 2:
      FUN_005e4e80(0,DAT_00996100);
      break;
    case 3:
      FUN_005e4e80(0,DAT_00996104);
      break;
    case 4:
      FUN_005e4e80(0,DAT_00996108);
      break;
    case 5:
      FUN_005e4e80(0,DAT_0099610c);
    }
    FUN_005e4e80(1,local_18);
    FUN_00544210(local_14,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0b50: 16-byte aligned: YES
  005b0b50  55             PUSH EBP
  005b0b51  8B EC          MOV EBP,ESP
  005b0b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0b50
string literals (1):
  0x00992E8C  at 005b0c14  'Value'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-02

GOG function: CSWSItemPropertyHandler::ApplyAbilityBonus  gog 0x007c4f00  callconv=__thiscall  param_size_bytes=12  gog_size=479  gog_retn=20
Candidate Steam address: 0x005adaf0 (size 479, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ad9b0:STR0/CALLEE0/SIG0.472/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4ee0 CSWSItemPropertyHandler::RemoveLight -> 0x005adad0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5100 CSWSItemPropertyHandler::RemoveAbilityBonus -> 0x005adcf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x4*,0x5*,0x14*,0x3c*,0x8c*,0xfffffff8*} | Steam {0x4*,0x5*,0x14*,0x3c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005adaf0: 16-byte aligned: YES
  005adaf0  55             PUSH EBP
  005adaf1  8B EC          MOV EBP,ESP
  005adaf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adaf0
string literals (1):
  0x00992E8C  at 005adb18  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005adaf0 FUN_005adaf0 size=479 ret=20 callers=0 callees=9

undefined4
FUN_005adaf0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e03;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_14 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_48;
    local_34 = (undefined2)DAT_00995fac;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    switch(*(undefined2 *)(param_2 + 2)) {
    case 0:
      FUN_005e4e80(0,DAT_009960f8);
      break;
    case 1:
      FUN_005e4e80(0,DAT_009960fc);
      break;
    case 2:
      FUN_005e4e80(0,DAT_00996100);
      break;
    case 3:
      FUN_005e4e80(0,DAT_00996104);
      break;
    case 4:
      FUN_005e4e80(0,DAT_00996108);
      break;
    case 5:
      FUN_005e4e80(0,DAT_0099610c);
    }
    FUN_005e4e80(1,local_14);
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ad9b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ad9b0: 16-byte aligned: YES
  005ad9b0  55             PUSH EBP
  005ad9b1  8B EC          MOV EBP,ESP
  005ad9b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ad9b0
string literals (0):
direct callees (6):
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-11-03

GOG function: CSWSItem::AddSubraceRequirements  gog 0x0066d900  callconv=(blank)  param_size_bytes=4  gog_size=1056  gog_retn=4
Candidate Steam address: 0x0060a4f0 (size 1177, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::AddPCRequirements
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.70 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0060a0d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0066d4e0 CSWSItem::AddGenderRequirements -> 0x00609cb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0066dd20 CSWSItem::AddPCRequirements -> 0x0060a4f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x11*,0x130*,0x28c*,0x2d4*} | Steam {0x2*,0x4*,0x8,0x11*,0x130*,0x134,0x28c*,0x2d4*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3d,0xe0,...(+2)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe,0x3e,...(+3)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0060a4f0: 16-byte aligned: YES
  0060a4f0  55             PUSH EBP
  0060a4f1  8B EC          MOV EBP,ESP
  0060a4f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0060a4f0
string literals (0):
direct callees (11):
  0x00733540  CExoString::CExoString(empty)
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042A0  CSWSItem::GetPassiveProperty
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005355F0  (unnamed)
  0x0071BCF0  C2DA::GetINTEntry_2
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x11 [dec 17]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x134 [dec 308]  x1  via ECX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0060a4f0 FUN_0060a4f0 size=1177 ret=4 callers=1 callees=11

void __fastcall FUN_0060a4f0(int param_1)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  int local_40;
  uint local_3c;
  int local_38;
  undefined1 local_2c [8];
  undefined4 local_24;
  int local_20;
  short *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00955724;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,2);
  local_38 = 0;
  iVar2 = FUN_00603f60(0x3e,0);
  if (iVar2 != 0) {
    for (local_20 = 0; local_20 < *(int *)(param_1 + 0x28c); local_20 = local_20 + 1) {
      local_14 = (short *)FUN_006042a0(local_20);
      if ((*(byte *)((int)local_14 + 0x11) == 0xff) ||
         ((1 << (*(byte *)((int)local_14 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (*local_14 == 0x3e)) {
        local_3c = (uint)(ushort)local_14[1];
        if (local_38 == 0) {
          FUN_00733570(&DAT_00997608);
          local_8._0_1_ = 3;
          uVar3 = FUN_007408f0(local_48,0xbb9a);
          local_8._0_1_ = 4;
          FUN_00733df0(local_50,uVar3);
          local_8._0_1_ = 5;
          uVar3 = FUN_00733df0(local_60,local_58);
          local_8._0_1_ = 6;
          FUN_007337c0(uVar3);
          local_8._0_1_ = 5;
          FUN_00733780();
          local_8._0_1_ = 4;
          FUN_00733780();
          local_8._0_1_ = 3;
          FUN_00733780();
          local_8 = CONCAT31(local_8._1_3_,2);
          FUN_00733780();
          local_38 = 1;
        }
        if (local_3c == 0) {
          local_40 = FUN_0051c8d0();
          if (local_40 != 0) {
            uVar3 = FUN_005355f0(local_68);
            local_8._0_1_ = 7;
            FUN_007337c0(uVar3);
/* ... 52 more lines truncated by --max-lines */
```

### decoy: 0x0060a0d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0060a0d0: 16-byte aligned: YES
  0060a0d0  55             PUSH EBP
  0060a0d1  8B EC          MOV EBP,ESP
  0060a0d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0060a0d0
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042A0  CSWSItem::GetPassiveProperty
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071BCF0  C2DA::GetINTEntry_2
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via ECX
  +0x130 [dec 304]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via ECX
  +0x11 [dec 17]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

## Card B0-11-04

GOG function: CSWSItemPropertyHandler::ApplyDamageResistance  gog 0x007c7c30  callconv=__thiscall  param_size_bytes=12  gog_size=359  gog_retn=20
Candidate Steam address: 0x005b0820 (size 359, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b04f0:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c7a70 CSWSItemPropertyHandler::RemoveDamageReduction -> 0x005b0660  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7da0 CSWSItemPropertyHandler::RemoveDamageResistance -> 0x005b0990  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Amount'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x4*,0x7*,0x14*,0x38*,0x8c*,0xfffffff8*} | Steam {0x4*,0x7*,0x14*,0x38*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b0820: 16-byte aligned: YES
  005b0820  55             PUSH EBP
  005b0821  8B EC          MOV EBP,ESP
  005b0823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0820
string literals (1):
  0x0099626C  at 005b08d9  'Amount'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0820 FUN_005b0820 size=359 ret=20 callers=0 callees=9

undefined4
FUN_005b0820(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  int local_48;
  undefined2 local_30;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  undefined4 local_1c;
  undefined2 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009530a3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_005e4920(1);
  }
  local_20 = local_48;
  local_8 = 0xffffffff;
  local_14 = local_48;
  local_30 = (undefined2)DAT_00995f24;
  *(undefined2 *)(local_48 + 8) = local_30;
  *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_00733570("Amount");
  local_8 = 1;
  puVar3 = &local_1c;
  puVar2 = local_2c;
  uVar1 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(7);
  FUN_0071bcf0(uVar1,puVar2,puVar3);
  local_8 = 0xffffffff;
  FUN_00733780();
  local_18 = *(undefined2 *)(param_2 + 2);
  FUN_005e4e80(0,1 << ((byte)local_18 & 0x1f));
  FUN_005e4e80(1,local_1c);
  FUN_00544210(local_14,param_5,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b04f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b04f0: 16-byte aligned: YES
  005b04f0  55             PUSH EBP
  005b04f1  8B EC          MOV EBP,ESP
  005b04f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b04f0
string literals (1):
  0x0099626C  at 005b05a9  'Amount'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-11-05

GOG function: CSWSItemPropertyHandler::ApplyDamagePenalty  gog 0x007c72b0  callconv=__thiscall  param_size_bytes=20  gog_size=823  gog_retn=20
Candidate Steam address: 0x005afea0 (size 823, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b01e0:STR0/CALLEE0/SIG0.515/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c7020 CSWSItemPropertyHandler::RemoveDamageImmunity -> 0x005afc10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c75f0 CSWSItemPropertyHandler::RemoveDamagePenalty -> 0x005b01e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*} | Steam {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x10*,0x14*,0x20*,0x74*,0x8c*,0x4000*,0x8000*,0x10000*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x10*,0x14*,0x20*,0x74*,0x8c*,0x4000*,0x8000*,0x10000*,0xfffffff8*}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005afea0: 16-byte aligned: YES
  005afea0  55             PUSH EBP
  005afea1  8B EC          MOV EBP,ESP
  005afea3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005afea0
string literals (1):
  0x00992E8C  at 005aff32  'Value'
direct callees (14):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00606BD0  CSWSItem::GetDamageFlags
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005afea0 FUN_005afea0 size=823 ret=20 callers=0 callees=14

undefined4 FUN_005afea0(int param_1,int param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 local_80;
  int local_7c;
  undefined2 local_58;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095300e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  local_2c = FUN_00919723(0x8c,uVar1);
  local_8 = 0;
  if (local_2c == 0) {
    local_7c = 0;
  }
  else {
    local_7c = FUN_005e4920(1);
  }
  local_28 = local_7c;
  local_8 = 0xffffffff;
  local_18 = local_7c;
  FUN_00733570("Value");
  local_8 = 1;
  piVar4 = &local_14;
  puVar3 = local_34;
  uVar1 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar1,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780();
  iVar2 = local_18;
  local_14 = -local_14;
  if (local_14 == 0) {
    if (local_18 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar2);
    }
  }
  else {
    local_58 = (undefined2)DAT_00995f54;
    *(undefined2 *)(local_18 + 8) = local_58;
    *(ushort *)(local_18 + 0xa [dec 10]) = *(ushort *)(local_18 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    if (param_4 == 0x10) {
      FUN_005e4e80(5,DAT_009961a4);
    }
    else if (param_4 == 0x20) {
      FUN_005e4e80(5,DAT_009961a8);
    }
    else if (param_4 == 0x4000) {
      FUN_005e4e80(5,DAT_009961ac);
    }
    else if (param_4 == 0x8000) {
      FUN_005e4e80(5,DAT_009961b0);
    }
    else if (param_4 == 0x10000) {
      FUN_005e4e80(5,DAT_009961b4);
    }
    else if (param_4 == 8) {
      FUN_005e4e80(5,DAT_009961b8);
    }
/* ... 25 more lines truncated by --max-lines */
```

### decoy: 0x005b01e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b01e0: 16-byte aligned: YES
  005b01e0  55             PUSH EBP
  005b01e1  8B EC          MOV EBP,ESP
  005b01e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b01e0
string literals (1):
  0x00992E8C  at 005b0209  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00606BD0  CSWSItem::GetDamageFlags
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13C [dec 316]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-11-06

GOG function: CSWSItem::RestoreUsedActiveProperties  gog 0x00667710  callconv=(blank)  param_size_bytes=2  gog_size=63  gog_retn=4
Candidate Steam address: 0x00603dd0 (size 245, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00603ee0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667600 CSWSItem::GetPropertyByTypeExists -> 0x00603dd0  gate_class=REVIEW
- GOG-order neighbour after:  0x00667a90 CSWSItem::GetActiveProperty -> 0x00604260  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x6,0xc,0x10,0x288}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0xa*,0x2b} | Steam {0x2,0x3,0x4*,0x5,0x8*,0xa*,0x20,0xff}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00603dd0: 16-byte aligned: YES
  00603dd0  55             PUSH EBP
  00603dd1  8B EC          MOV EBP,ESP
  00603dd3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00603dd0 FUN_00603dd0 size=245 ret=4 callers=1 callees=1

void __thiscall FUN_00603dd0(int param_1,int param_2)

{
  short *psVar1;
  int local_c;
  
  for (local_c = 0; local_c < *(int *)(param_1 + 0x288); local_c = local_c + 1) {
    psVar1 = (short *)FUN_00604260(local_c);
    if (((psVar1 != (short *)0x0) && (*psVar1 == 10)) &&
       ((param_2 == 0 || ((param_2 != 0 && ((char)psVar1[8] == -1)))))) {
      switch(psVar1[3]) {
      case 8:
        *(undefined1 *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10}) = 1;
        break;
      case 9:
        *(undefined1 *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10}) = 2;
        break;
      case 10:
        *(undefined1 *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10}) = 3;
        break;
      case 0xb:
        *(undefined1 *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10}) = 4;
        break;
      case 0xc:
        *(undefined1 *)(psVar1 + 0x8 [dec 8] {short*: byte +0x10}) = 5;
      }
      psVar1[6] = 1;
      psVar1[7] = 0;
    }
  }
  return;
}
```

### decoy: 0x00603ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603ee0: 16-byte aligned: YES
  00603ee0  55             PUSH EBP
  00603ee1  8B EC          MOV EBP,ESP
  00603ee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00603ee0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x603F28 [dec 6307624]  x1  via EDX
```

## Card B0-11-07

GOG function: CSWSItemPropertyHandler::OnItemPropertyApplied  gog 0x007c1e20  callconv=(blank)  param_size_bytes=20  gog_size=92  gog_retn=20
Candidate Steam address: 0x005aaa10 (size 92, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aaa70:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c18a0 CSWSItemPropertyHandler::InitializeItemProperties -> 0x005aa490  gate_class=REVIEW
- GOG-order neighbour after:  0x007c1e80 CSWSItemPropertyHandler::OnItemPropertyRemoved -> 0x005aaa70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0x46*} | Steam {0x8*,0x14*,0x46*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005aaa10: 16-byte aligned: YES
  005aaa10  55             PUSH EBP
  005aaa11  8B EC          MOV EBP,ESP
  005aaa13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aaa10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aaa10 FUN_005aaa10 size=92 ret=20 callers=0 callees=0

undefined4 __thiscall
FUN_005aaa10(int param_1,undefined4 param_2,ushort *param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  ushort uVar1;
  undefined4 uVar2;
  
  uVar1 = *param_3;
  if ((uVar1 < 0x46) && (*(int *)(*(int *)(param_1 + 4) + (uint)uVar1 * 4) != 0)) {
    uVar2 = (**(code **)(*(int *)(param_1 + 4) + (uint)uVar1 * 4))
                      (param_2,param_3,param_4,param_5,param_6);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x005aaa70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aaa70: 16-byte aligned: YES
  005aaa70  55             PUSH EBP
  005aaa71  8B EC          MOV EBP,ESP
  005aaa73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005aaa70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B0-11-08

GOG function: CSWSItemPropertyHandler::ApplySpecialWalk  gog 0x007c6260  callconv=__thiscall  param_size_bytes=12  gog_size=453  gog_retn=20
Candidate Steam address: 0x005aee50 (size 453, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af1b0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c60c0 CSWSItemPropertyHandler::RemoveChangedSavingThrow -> 0x005aecb0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6430 CSWSItemPropertyHandler::RemoveSpecialWalk -> 0x005af020  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x2*,0x8*,0xa*,0x14*} | Steam {0x2*,0x8*,0xa*,0x14*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x14*,0x3c*,0x8c*,0xffffffe7*,0xfffffff8*} | Steam {0x4*,0x8*,0x14*,0x3c*,0x8c*,0xffffffe7*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005aee50: 16-byte aligned: YES
  005aee50  55             PUSH EBP
  005aee51  8B EC          MOV EBP,ESP
  005aee53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aee50
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005aee50 FUN_005aee50 size=453 ret=20 callers=0 callees=5

undefined4
FUN_005aee50(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  int local_4c;
  int local_48;
  undefined2 local_3c;
  undefined2 local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950046;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  local_30 = (undefined2)DAT_00996004;
  *(undefined2 *)(local_48 + 8) = local_30;
  FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
  *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_00544210(local_48,param_5,param_5);
  iVar1 = FUN_00919723(0x8c);
  local_8 = 1;
  if (iVar1 == 0) {
    local_4c = 0;
  }
  else {
    local_4c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  *(ushort *)(local_4c + 0xa [dec 10]) = *(ushort *)(local_4c + 0xa [dec 10]) & 0xffe7 | 8;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  local_3c = (undefined2)DAT_00996008;
  *(undefined2 *)(local_4c + 8) = local_3c;
  *(ushort *)(local_4c + 0xa [dec 10]) = *(ushort *)(local_4c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_00544210(local_4c,param_5,param_5);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005af1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af1b0: 16-byte aligned: YES
  005af1b0  55             PUSH EBP
  005af1b1  8B EC          MOV EBP,ESP
  005af1b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005af1b0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-11-09

GOG function: CSWSItemPropertyHandler::ApplyChangedSavingThrow  gog 0x007c5f30  callconv=(blank)  param_size_bytes=20  gog_size=397  gog_retn=20
Candidate Steam address: 0x005aeb20 (size 397, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aee50:STR0/CALLEE0/SIG0.6/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c5e70 CSWSItemPropertyHandler::RemoveReducedSavingThrow -> 0x005aea60  gate_class=REVIEW
- GOG-order neighbour after:  0x007c60c0 CSWSItemPropertyHandler::RemoveChangedSavingThrow -> 0x005aecb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x8*,0xa*,0x14*,0x122*} | Steam {0x2*,0x8*,0xa*,0x14*,0x122*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x14*,0x30*,0x8c*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x14*,0x30*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005aeb20: 16-byte aligned: YES
  005aeb20  55             PUSH EBP
  005aeb21  8B EC          MOV EBP,ESP
  005aeb23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aeb20
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aeb20 FUN_005aeb20 size=397 ret=20 callers=2 callees=5

undefined4 FUN_005aeb20(int param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_3c;
  undefined2 local_28;
  undefined2 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952f1b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_3c = 0;
  }
  else {
    local_3c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if (param_2 < 0) {
    local_28 = (undefined2)DAT_00995f88;
    *(undefined2 *)(local_3c + 8) = local_28;
    param_2 = -param_2;
  }
  else {
    local_24 = (undefined2)DAT_00995f84;
    *(undefined2 *)(local_3c + 8) = local_24;
  }
  *(ushort *)(local_3c + 0xa [dec 10]) = *(ushort *)(local_3c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  sVar1 = *(short *)(param_3 + 2);
  if (sVar1 == 1) {
    FUN_005e4e80(1,1);
  }
  else if (sVar1 == 2) {
    FUN_005e4e80(1,2);
  }
  else if (sVar1 == 3) {
    FUN_005e4e80(1,3);
  }
  FUN_005e4e80(0,param_2);
  FUN_005e4e80(2,0);
  FUN_005e4e80(3,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
  FUN_00544210(local_3c,param_5,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005aee50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aee50: 16-byte aligned: YES
  005aee50  55             PUSH EBP
  005aee51  8B EC          MOV EBP,ESP
  005aee53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aee50
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
```

## Card B0-11-10

GOG function: CSWSItemPropertyHandler::ApplyImprovedSavingThrow  gog 0x007c5c30  callconv=__thiscall  param_size_bytes=12  gog_size=184  gog_retn=20
Candidate Steam address: 0x005ae820 (size 184, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ae9a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5a90 CSWSItemPropertyHandler::RemoveImprovedForceResistance -> 0x005ae680  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5cf0 CSWSItemPropertyHandler::RemoveImprovedSavingThrow -> 0x005ae8e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 2 common): GOG {0x6*,0x130*} | Steam {0x6*,0x130*}
imm sets (* = in both, 2 common): GOG {0x14*,0x18*} | Steam {0x14*,0x18*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005ae820: 16-byte aligned: YES
  005ae820  55             PUSH EBP
  005ae821  8B EC          MOV EBP,ESP
  005ae823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae820
string literals (1):
  0x00992E8C  at 005ae848  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae820 FUN_005ae820 size=184 ret=20 callers=0 callees=5

undefined4
FUN_005ae820(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aeb20(param_1,local_14,param_2,param_3,param_5);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae9a0: 16-byte aligned: YES
  005ae9a0  55             PUSH EBP
  005ae9a1  8B EC          MOV EBP,ESP
  005ae9a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae9a0
string literals (1):
  0x00992E8C  at 005ae9c8  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-11

GOG function: CSWSItemPropertyHandler::ApplyACBonus  gog 0x007c53c0  callconv=__thiscall  param_size_bytes=12  gog_size=686  gog_retn=20
Candidate Steam address: 0x005adfb0 (size 686, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ae260:STR0/CALLEE0/SIG0.488/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5100 CSWSItemPropertyHandler::RemoveAbilityBonus -> 0x005adcf0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5670 CSWSItemPropertyHandler::RemoveACBonus -> 0x005ae260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x8b*,0x122*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x8b*,0x122*,0x130*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x5*,0x14*,0x64*,0x8c*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x5*,0x14*,0x64*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005adfb0: 16-byte aligned: YES
  005adfb0  55             PUSH EBP
  005adfb1  8B EC          MOV EBP,ESP
  005adfb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adfb0
string literals (1):
  0x00992E8C  at 005ae01e  'Value'
direct callees (12):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x006D5200  (unnamed)
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via ECX
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005adfb0 FUN_005adfb0 size=686 ret=20 callers=0 callees=12

undefined4
FUN_005adfb0(int param_1,short *param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  int local_6c;
  undefined2 local_40;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  uint local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e33;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_6c = 0;
  }
  else {
    local_6c = FUN_005e4920(1);
  }
  local_20 = local_6c;
  local_8 = 0xffffffff;
  local_18 = local_6c;
  FUN_00733570("Value");
  local_8 = 1;
  piVar4 = &local_14;
  puVar3 = local_2c;
  uVar2 = (uint)(ushort)param_2[3];
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780();
  iVar1 = local_18;
  if (local_14 == 0) {
    if (local_18 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar1);
    }
  }
  else {
    local_40 = (undefined2)DAT_00995fdc;
    *(undefined2 *)(local_18 + 8) = local_40;
    *(ushort *)(local_18 + 0xa [dec 10]) = *(ushort *)(local_18 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    local_1c = (uint)*(byte *)(iVar1 + 0x8b);
    FUN_005e4e80(0,local_1c);
    FUN_005e4e80(1,local_14);
    FUN_005e4e80(5,DAT_0099614c);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    if (*param_2 == 2) {
      if ((param_2[1] != 0) && ((ushort)param_2[1] < 4)) {
        FUN_005e4e80(4,param_2[1]);
      }
    }
    else if (*param_2 == 3) {
      FUN_005e4e80(5,param_2[1]);
    }
    else if (*param_2 == 4) {
      FUN_005e4e80(2,param_2[1]);
    }
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae260: 16-byte aligned: YES
  005ae260  55             PUSH EBP
  005ae261  8B EC          MOV EBP,ESP
  005ae263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae260
string literals (1):
  0x00992E8C  at 005ae293  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x006D5200  (unnamed)
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via ECX
  +0xA [dec 10]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
```

## Card B0-11-12

GOG function: CSWSItem::RestoreUsedActiveProperties  gog 0x00667710  callconv=(blank)  param_size_bytes=2  gog_size=63  gog_retn=4
Candidate Steam address: 0x00604140 (size 133, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::SetNumActiveProperties
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00603f60:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667600 CSWSItem::GetPropertyByTypeExists -> 0x00603dd0  gate_class=REVIEW
- GOG-order neighbour after:  0x00667a90 CSWSItem::GetActiveProperty -> 0x00604260  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x288,0x290}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0xa,0x2b} | Steam {0x4*,0xc,0x1c}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00604140: 16-byte aligned: YES
  00604140  55             PUSH EBP
  00604141  8B EC          MOV EBP,ESP
  00604143  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00604140
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x00919723  operator new
struct offsets touched (top 2):
  +0x290 [dec 656]  x4  via EAX,EDX
  +0x288 [dec 648]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00604140 FUN_00604140 size=133 ret=4 callers=1 callees=2

void __thiscall FUN_00604140(int param_1,int param_2)

{
  longlong lVar1;
  undefined4 uVar2;
  
  *(int *)(param_1 + 0x288) = param_2;
  if (*(int *)(param_1 + 0x290) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x290));
  }
  if (param_2 == 0) {
    *(undefined4 *)(param_1 + 0x290) = 0;
  }
  else {
    lVar1 = (ulonglong)*(uint *)(param_1 + 0x288) * 0x1c;
    uVar2 = FUN_00919723(-(uint)((int)((ulonglong)lVar1 >> 0x20) != 0) | (uint)lVar1);
    *(undefined4 *)(param_1 + 0x290) = uVar2;
  }
  return;
}
```

### decoy: 0x00603f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603f60: 16-byte aligned: YES
  00603f60  55             PUSH EBP
  00603f61  8B EC          MOV EBP,ESP
  00603f63  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00603f60
string literals (0):
direct callees (2):
  0x00604260  CSWSItem::GetActiveProperty
  0x006042A0  CSWSItem::GetPassiveProperty
struct offsets touched (top 5):
  +0x2 [dec 2]  x2  via EDX
  +0x11 [dec 17]  x2  via EDX
  +0x2D4 [dec 724]  x2  via EAX
  +0x288 [dec 648]  x1  via ECX
  +0x28C [dec 652]  x1  via ECX
```

## Card B0-11-13

GOG function: CSWSItemPropertyHandler::ApplyEnhancementBonus  gog 0x007c1ee0  callconv=__thiscall  param_size_bytes=20  gog_size=1809  gog_retn=20
Candidate Steam address: 0x005aaad0 (size 1809, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ab1f0:STR0/CALLEE0/SIG0.586/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1e80 CSWSItemPropertyHandler::OnItemPropertyRemoved -> 0x005aaa70  gate_class=REVIEW
- GOG-order neighbour after:  0x007c2600 CSWSItemPropertyHandler::RemoveEnhancementBonus -> 0x005ab1f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x38*,0x122*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x38*,0x122*,0x130*}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x10*,0x14*,0x20*,0x8c*,0xc4*,0x4000*,0x8000*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x10*,0x14*,0x20*,0x8c*,0xc4*,0x4000*,0x8000*,...(+4)}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005aaad0: 16-byte aligned: YES
  005aaad0  55             PUSH EBP
  005aaad1  8B EC          MOV EBP,ESP
  005aaad3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aaad0
string literals (1):
  0x00992E8C  at 005aabd9  'Value'
direct callees (11):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x2 [dec 2]  x8  via EAX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x4  via ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x38 [dec 56]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005aaad0 FUN_005aaad0 size=1809 ret=20 callers=0 callees=11

undefined4
FUN_005aaad0(int param_1,short *param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  undefined4 local_d4;
  undefined4 local_d0;
  int local_cc;
  int local_c8;
  undefined2 local_98;
  undefined2 local_6c;
  undefined1 local_44 [8];
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952c14;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if ((param_4 != 0x80000) && (param_4 != 0x40000)) {
    local_20 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    local_34 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (local_34 == 0) {
      local_c8 = 0;
    }
    else {
      local_c8 = FUN_005e4920(1);
    }
    local_30 = local_c8;
    local_8 = 0xffffffff;
    local_14 = local_c8;
    local_3c = FUN_00919723(0x8c,uVar1);
    local_8 = 1;
    if (local_3c == 0) {
      local_cc = 0;
    }
    else {
      local_cc = FUN_005e4920(1);
    }
    local_38 = local_cc;
    local_8 = 0xffffffff;
    local_1c = local_cc;
    FUN_00733570("Value");
    local_8 = 2;
    FUN_0071bcf0(param_2[3],local_44,&local_18);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_18 != 0) {
      local_6c = (undefined2)DAT_00995f44;
      *(undefined2 *)(local_14 + 8) = local_6c;
      *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
      FUN_005e4e80(0,local_18);
      if (param_4 == 0x10) {
        FUN_005e4e80(1,DAT_00996188);
      }
      else if (param_4 == 0x20) {
        FUN_005e4e80(1,DAT_0099618c);
      }
      else if (param_4 == 0x4000) {
        FUN_005e4e80(1,DAT_00996190);
      }
      else if (param_4 == 0x8000) {
        FUN_005e4e80(1,DAT_00996194);
/* ... 101 more lines truncated by --max-lines */
```

### decoy: 0x005ab1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ab1f0: 16-byte aligned: YES
  005ab1f0  55             PUSH EBP
  005ab1f1  8B EC          MOV EBP,ESP
  005ab1f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ab1f0
string literals (1):
  0x00992E8C  at 005ab21f  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x2 [dec 2]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x13C [dec 316]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x122 [dec 290]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
```

## Card B0-11-14

GOG function: CSWSItemPropertyHandler::ApplyDamageVulnerability  gog 0x007c7f60  callconv=__thiscall  param_size_bytes=12  gog_size=360  gog_retn=20
Candidate Steam address: 0x005b0b50 (size 360, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0820:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c7da0 CSWSItemPropertyHandler::RemoveDamageResistance -> 0x005b0990  gate_class=REVIEW
- GOG-order neighbour after:  0x007c80d0 CSWSItemPropertyHandler::RemoveDamageVulnerability -> 0x005b0cc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x4*,0x14*,0x16*,0x38*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x16*,0x38*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b0b50: 16-byte aligned: YES
  005b0b50  55             PUSH EBP
  005b0b51  8B EC          MOV EBP,ESP
  005b0b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0b50
string literals (1):
  0x00992E8C  at 005b0c14  'Value'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0b50 FUN_005b0b50 size=360 ret=20 callers=0 callees=9

undefined4
FUN_005b0b50(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  int local_48;
  undefined2 local_30;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  undefined2 local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009530a3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_005e4920(1);
  }
  local_20 = local_48;
  local_8 = 0xffffffff;
  local_18 = local_48;
  local_1c = *(undefined2 *)(param_2 + 2);
  local_30 = (undefined2)DAT_00995f60;
  *(undefined2 *)(local_48 + 8) = local_30;
  *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_00733570("Value");
  local_8 = 1;
  puVar3 = &local_14;
  puVar2 = local_2c;
  uVar1 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x16);
  FUN_0071bcf0(uVar1,puVar2,puVar3);
  local_8 = 0xffffffff;
  FUN_00733780();
  FUN_005e4e80(0,1 << ((byte)local_1c & 0x1f));
  FUN_005e4e80(1,local_14);
  FUN_00544210(local_18,param_5,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0820: 16-byte aligned: YES
  005b0820  55             PUSH EBP
  005b0821  8B EC          MOV EBP,ESP
  005b0823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0820
string literals (1):
  0x0099626C  at 005b08d9  'Amount'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-11-15

GOG function: CSWSItemPropertyHandler::RemoveLight  gog 0x007c4ee0  callconv=__thiscall  param_size_bytes=0  gog_size=31  gog_retn=16
Candidate Steam address: 0x005adad0 (size 31, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ad9b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4dc0 CSWSItemPropertyHandler::ApplyLight -> 0x005ad9b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4f00 CSWSItemPropertyHandler::ApplyAbilityBonus -> 0x005adaf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 16)
```

### prologue
```
prologue for 0x005adad0: 16-byte aligned: YES
  005adad0  55             PUSH EBP
  005adad1  8B EC          MOV EBP,ESP
  005adad3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005adad0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005adad0 FUN_005adad0 size=31 ret=16 callers=0 callees=0

undefined4 FUN_005adad0(void)

{
  return 0;
}
```

### decoy: 0x005ad9b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ad9b0: 16-byte aligned: YES
  005ad9b0  55             PUSH EBP
  005ad9b1  8B EC          MOV EBP,ESP
  005ad9b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ad9b0
string literals (0):
direct callees (6):
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-11-16

GOG function: CSWSItemPropertyHandler::RemoveAttackBonus  gog 0x007c43d0  callconv=__thiscall  param_size_bytes=16  gog_size=1017  gog_retn=16
Candidate Steam address: 0x005acfc0 (size 1017, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005acc60:STR0/CALLEE0/SIG0.575/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c4070 CSWSItemPropertyHandler::ApplyAttackBonus -> 0x005acc60  gate_class=REVIEW
- GOG-order neighbour after:  0x007c47d0 CSWSItemPropertyHandler::ApplyAttackPenalty -> 0x005ad3c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 12 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x7*,0x8*,0x10*,0x20*,0x26*,0x27*,0x28*,0x60*,0x148*,0x4000*,0x8000*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x7*,0x8*,0x10*,0x20*,0x26*,0x27*,0x28*,0x60*,0x148*,0x4000*,0x8000*,...(+1)}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005acfc0: 16-byte aligned: YES
  005acfc0  55             PUSH EBP
  005acfc1  8B EC          MOV EBP,ESP
  005acfc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005acfc0
string literals (1):
  0x00992E8C  at 005acfe8  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x38 [dec 56]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005acfc0 FUN_005acfc0 size=1017 ret=16 callers=0 callees=5

undefined4 FUN_005acfc0(int param_1,short *param_2,int param_3,int param_4)

{
  byte bVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952d88;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar5 = extraout_var;
  FUN_0071bcf0(param_2[3],local_28,&local_14);
  local_8 = 0xffffffff;
  FUN_00733780(uVar3,uVar5);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005ad06f:
  while( true ) {
    while( true ) {
      if (local_18[1 = byte +0x4] <= (int)local_20) {
        ExceptionList = local_10;
        return 0;
      }
      if ((int)DAT_00995f44 < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) {
        ExceptionList = local_10;
        return 0;
      }
      local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
      if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f44) &&
           ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
          (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar4 = FUN_005e4e60(0), iVar4 == local_14)
         ) break;
      local_20 = local_20 + 1;
    }
    if (param_4 != 0x10) break;
    iVar4 = FUN_005e4e60(1);
    if (iVar4 == DAT_00996188) goto LAB_005ad235;
    local_20 = local_20 + 1;
  }
  if (param_4 == 0x20) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_0099618c) {
      local_20 = local_20 + 1;
      goto LAB_005ad06f;
    }
  }
  else if (param_4 == 0x4000) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_00996190) {
      local_20 = local_20 + 1;
      goto LAB_005ad06f;
    }
  }
  else if (param_4 == 0x8000) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_00996194) {
      local_20 = local_20 + 1;
      goto LAB_005ad06f;
    }
  }
  else if (param_4 == 0x10000) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_00996198) {
      local_20 = local_20 + 1;
/* ... 48 more lines truncated by --max-lines */
```

### decoy: 0x005acc60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005acc60: 16-byte aligned: YES
  005acc60  55             PUSH EBP
  005acc61  8B EC          MOV EBP,ESP
  005acc63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005acc60
string literals (1):
  0x00992E8C  at 005accce  'Value'
direct callees (10):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-11-17

GOG function: CSWSItemPropertyHandler::RemoveACBonus  gog 0x007c5670  callconv=__thiscall  param_size_bytes=12  gog_size=717  gog_retn=16
Candidate Steam address: 0x005ae260 (size 717, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005adfb0:STR0/CALLEE0/SIG0.488/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c53c0 CSWSItemPropertyHandler::ApplyACBonus -> 0x005adfb0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5940 CSWSItemPropertyHandler::ApplyImprovedForceResistance -> 0x005ae530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 13 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0xc*,0x14*,0x18*,0x34*,0x8b*,0x130*,0x13a*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0xc*,0x14*,0x18*,0x34*,0x8b*,0x130*,0x13a*,0x1198*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0x10*,0x70*,0x148*} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0x10*,0x70*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005ae260: 16-byte aligned: YES
  005ae260  55             PUSH EBP
  005ae261  8B EC          MOV EBP,ESP
  005ae263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae260
string literals (1):
  0x00992E8C  at 005ae293  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x006D5200  (unnamed)
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via ECX
  +0xA [dec 10]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae260 FUN_005ae260 size=717 ret=16 callers=0 callees=7

undefined4 FUN_005ae260(int param_1,short *param_2,int param_3)

{
  ushort uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined1 local_2c [8];
  uint local_24;
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e58;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = (int *)(param_3 + 0x148);
  FUN_00733570("Value");
  local_8 = 0;
  piVar6 = &local_14;
  puVar5 = local_2c;
  uVar4 = (uint)(ushort)param_2[3];
  FUN_006f0190(1);
  FUN_0071bcf0(uVar4,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2);
  iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  local_20 = (uint)*(byte *)(iVar3 + 0x8b);
  local_24 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13a);
LAB_005ae345:
  while( true ) {
    while( true ) {
      if (local_18[1 = byte +0x4] <= (int)local_24) {
        ExceptionList = local_10;
        return 0;
      }
      if ((int)DAT_00995fdc < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_24 * 4) + 8)) {
        ExceptionList = local_10;
        return 0;
      }
      local_1c = *(undefined4 **)(*local_18 + local_24 * 4);
      if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fdc) &&
           ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
          (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((uVar2 = FUN_005e4e60(0), uVar2 == local_20 &&
          (iVar3 = FUN_005e4e60(1), iVar3 == local_14)))) break;
      local_24 = local_24 + 1;
    }
    if (*param_2 != 2) break;
    if (((param_2[1] == 0) || (3 < (ushort)param_2[1])) ||
       (uVar1 = param_2[1], uVar2 = FUN_005e4e60(4), uVar2 == uVar1)) goto LAB_005ae4ec;
    local_24 = local_24 + 1;
  }
  if (*param_2 == 4) {
    uVar1 = param_2[1];
    uVar2 = FUN_005e4e60(2);
    if (uVar2 != uVar1) {
      local_24 = local_24 + 1;
      goto LAB_005ae345;
    }
  }
  else if ((*param_2 == 3) && (uVar1 = param_2[1], uVar2 = FUN_005e4e60(5), uVar2 != uVar1)) {
    local_24 = local_24 + 1;
    goto LAB_005ae345;
  }
LAB_005ae4ec:
  FUN_00544790(*local_1c,local_1c[1 = byte +0x4]);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005adfb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adfb0: 16-byte aligned: YES
  005adfb0  55             PUSH EBP
  005adfb1  8B EC          MOV EBP,ESP
  005adfb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adfb0
string literals (1):
  0x00992E8C  at 005ae01e  'Value'
direct callees (12):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x006D5200  (unnamed)
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via ECX
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x34 [dec 52]  x1  via EAX
  +0x8B [dec 139]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-11-18

GOG function: CSWSItemPropertyHandler::RemoveImprovedForceResistance  gog 0x007c5a90  callconv=__thiscall  param_size_bytes=12  gog_size=407  gog_retn=16
Candidate Steam address: 0x005ae680 (size 407, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ae530:STR0/CALLEE0/SIG0.328/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5940 CSWSItemPropertyHandler::ApplyImprovedForceResistance -> 0x005ae530  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5c30 CSWSItemPropertyHandler::ApplyImprovedSavingThrow -> 0x005ae820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 9 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x14e*,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x14e*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x7*,0xb*,0x10*,0x48*,0x148*} | Steam {0x7*,0xb*,0x10*,0x48*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005ae680: 16-byte aligned: YES
  005ae680  55             PUSH EBP
  005ae681  8B EC          MOV EBP,ESP
  005ae683  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae680
string literals (1):
  0x00992E8C  at 005ae6a8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 9):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x14E [dec 334]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae680 FUN_005ae680 size=407 ret=16 callers=0 callees=6

undefined4 FUN_005ae680(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined4 extraout_var;
  undefined4 uVar6;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952eb8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_28;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar6 = extraout_var;
  FUN_006f0190(0xb);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar6);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x14e);
  while( true ) {
    if (local_18[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fa0 < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) break;
    local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
    if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fa0) &&
         ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar3 = FUN_005e4e60(0), iVar3 == local_14))
    {
      FUN_00544790(*local_1c,local_1c[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_20 = local_20 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae530: 16-byte aligned: YES
  005ae530  55             PUSH EBP
  005ae531  8B EC          MOV EBP,ESP
  005ae533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae530
string literals (1):
  0x00992E8C  at 005ae558  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-19

GOG function: CSWSItemPropertyHandler::RemoveAttackPenalty  gog 0x007c4ad0  callconv=__thiscall  param_size_bytes=16  gog_size=740  gog_retn=16
Candidate Steam address: 0x005ad6c0 (size 740, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ad3c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c47d0 CSWSItemPropertyHandler::ApplyAttackPenalty -> 0x005ad3c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4dc0 CSWSItemPropertyHandler::ApplyLight -> 0x005ad9b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 11 common): GOG {0x2*,0x7*,0x8*,0x10*,0x14*,0x20*,0x4c*,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x7*,0x8*,0x10*,0x14*,0x20*,0x4c*,0x148*,0x4000*,0x8000*,0x10000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005ad6c0: 16-byte aligned: YES
  005ad6c0  55             PUSH EBP
  005ad6c1  8B EC          MOV EBP,ESP
  005ad6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ad6c0
string literals (1):
  0x00992E8C  at 005ad6e8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13C [dec 316]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ad6c0 FUN_005ad6c0 size=740 ret=16 callers=0 callees=6

undefined4 FUN_005ad6c0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined4 extraout_var;
  undefined4 uVar7;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097c358;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar6 = &local_1c;
  puVar5 = local_28;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  uVar7 = extraout_var;
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar3,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2,uVar7);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005ad778:
  while( true ) {
    while( true ) {
      if (local_14[1 = byte +0x4] <= (int)local_20) {
        ExceptionList = local_10;
        return 0;
      }
      if (DAT_00995f54 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) {
        ExceptionList = local_10;
        return 0;
      }
      local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
      if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f48) &&
           ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
          (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((iVar4 = FUN_005e4e60(0), iVar4 == local_1c &&
          (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar2 = FUN_005e4e60(2), uVar2 == bVar1))))
      break;
      local_20 = local_20 + 1;
    }
    if (param_4 != 0x10) break;
    iVar4 = FUN_005e4e60(1);
    if (iVar4 == DAT_00996188) goto LAB_005ad963;
    local_20 = local_20 + 1;
  }
  if (param_4 == 0x20) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_0099618c) {
      local_20 = local_20 + 1;
      goto LAB_005ad778;
    }
  }
  else if (param_4 == 0x4000) {
    iVar4 = FUN_005e4e60(1);
    if (iVar4 != DAT_00996190) {
      local_20 = local_20 + 1;
      goto LAB_005ad778;
    }
  }
  else if (param_4 == 0x8000) {
    iVar4 = FUN_005e4e60(1);
/* ... 28 more lines truncated by --max-lines */
```

### decoy: 0x005ad3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ad3c0: 16-byte aligned: YES
  005ad3c0  55             PUSH EBP
  005ad3c1  8B EC          MOV EBP,ESP
  005ad3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ad3c0
string literals (1):
  0x00992E8C  at 005ad42e  'Value'
direct callees (13):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-11-20

GOG function: CSWSItemPropertyHandler::ApplyImprovedForceResistance  gog 0x007c5940  callconv=__thiscall  param_size_bytes=12  gog_size=335  gog_retn=20
Candidate Steam address: 0x005ae530 (size 335, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ae680:STR0/CALLEE0/SIG0.328/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5670 CSWSItemPropertyHandler::RemoveACBonus -> 0x005ae260  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5a90 CSWSItemPropertyHandler::RemoveImprovedForceResistance -> 0x005ae680  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 5 common): GOG {0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x4*,0xb*,0x14*,0x34*,0x8c*,0xfffffff8*} | Steam {0x4*,0xb*,0x14*,0x34*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005ae530: 16-byte aligned: YES
  005ae530  55             PUSH EBP
  005ae531  8B EC          MOV EBP,ESP
  005ae533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae530
string literals (1):
  0x00992E8C  at 005ae558  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae530 FUN_005ae530 size=335 ret=20 callers=0 callees=9

undefined4
FUN_005ae530(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_44;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e93;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0xb);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_14 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_44;
    local_34 = (undefined2)DAT_00995fa0;
    *(undefined2 *)(local_44 + 8) = local_34;
    *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_14);
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae680: 16-byte aligned: YES
  005ae680  55             PUSH EBP
  005ae681  8B EC          MOV EBP,ESP
  005ae683  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae680
string literals (1):
  0x00992E8C  at 005ae6a8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 9):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x14E [dec 334]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-11-21

GOG function: CGameEffect::GetString  gog 0x00734be0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x005e4f60 (size 32, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGameEffect::SetString
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734bc0 CGameEffect::SetObjectID -> 0x005e4f00  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c20 CGameEffect::SetString -> 0x005e4f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x005e4f60: 16-byte aligned: YES
  005e4f60  55             PUSH EBP
  005e4f61  8B EC          MOV EBP,ESP
  005e4f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f60
string literals (0):
direct callees (1):
  0x007337C0  CExoString::operator=
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f60 FUN_005e4f60 size=32 ret=8 callers=3 callees=1
THUNK -> 0x007337C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e4f60(undefined4 param_1,undefined4 param_2)

{
  FUN_007337c0(param_2);
  return;
}
```

### decoy: 0x005e4f20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f20: 16-byte aligned: YES
  005e4f20  55             PUSH EBP
  005e4f21  8B EC          MOV EBP,ESP
  005e4f23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f20
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

## Card B0-11-22

GOG function: CSWSItemPropertyHandler::ApplyBonusFeat  gog 0x007c6c70  callconv=__thiscall  param_size_bytes=12  gog_size=278  gog_retn=20
Candidate Steam address: 0x005af860 (size 278, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005afa80:STR0/CALLEE0/SIG0.607/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6b20 CSWSItemPropertyHandler::RemoveFreedomOfMovement -> 0x005af710  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6d90 CSWSItemPropertyHandler::RemoveBonusFeat -> 0x005af980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x8*,0xa*,0x14*,0x1198*} | Steam {0x2*,0x8*,0xa*,0x14*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x14*,0x24*,0x8c*,0xfffffff8*} | Steam {0x3*,0x4*,0x14*,0x24*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005af860: 16-byte aligned: YES
  005af860  55             PUSH EBP
  005af861  8B EC          MOV EBP,ESP
  005af863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005af860
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005af860 FUN_005af860 size=278 ret=20 callers=0 callees=5

undefined4 FUN_005af860(int param_1,int param_2,int param_3,undefined4 param_4,undefined4 param_5)

{
  int iVar1;
  int local_34;
  undefined2 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a17b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_3 + 0x1198) != 0) {
    iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_34 = 0;
    }
    else {
      local_34 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    if (local_34 != 0) {
      local_24 = (undefined2)DAT_00996068;
      *(undefined2 *)(local_34 + 8) = local_24;
      *(ushort *)(local_34 + 0xa [dec 10]) = *(ushort *)(local_34 + 0xa [dec 10]) & 0xfff8 | 3;
      FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
      FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
      FUN_00544210(local_34,param_5,param_5);
    }
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005afa80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005afa80: 16-byte aligned: YES
  005afa80  55             PUSH EBP
  005afa81  8B EC          MOV EBP,ESP
  005afa83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005afa80
string literals (1):
  0x00992E8C  at 005afae8  'Value'
direct callees (9):
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-23

GOG function: CSWSItemPropertyHandler::ApplyDecreaseSkillModifier  gog 0x007c8ac0  callconv=__thiscall  param_size_bytes=12  gog_size=397  gog_retn=20
Candidate Steam address: 0x005b16b0 (size 397, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b1320:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c88d0 CSWSItemPropertyHandler::RemoveDecreaseAC -> 0x005b14c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8c50 CSWSItemPropertyHandler::RemoveDecreaseSkillModifier -> 0x005b1840  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 7 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*,0x130*}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x14*,0x15*,0x38*,0x8c*,0xfffffff8*} | Steam {0x2*,0x4*,0x14*,0x15*,0x38*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b16b0: 16-byte aligned: YES
  005b16b0  55             PUSH EBP
  005b16b1  8B EC          MOV EBP,ESP
  005b16b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b16b0
string literals (1):
  0x00992E8C  at 005b16d8  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b16b0 FUN_005b16b0 size=397 ret=20 callers=0 callees=9

undefined4
FUN_005b16b0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953133;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_18;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x15);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_18 = -local_18;
  if (local_18 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_48;
    local_34 = (undefined2)DAT_00995ffc;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,local_18);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    FUN_00544210(local_14,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b1320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b1320: 16-byte aligned: YES
  005b1320  55             PUSH EBP
  005b1321  8B EC          MOV EBP,ESP
  005b1323  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b1320
string literals (1):
  0x00992E8C  at 005b1348  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-24

GOG function: CSWSItemPropertyHandler::OnItemPropertyRemoved  gog 0x007c1e80  callconv=(blank)  param_size_bytes=16  gog_size=88  gog_retn=16
Candidate Steam address: 0x005aaa70 (size 88, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aaa10:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1e20 CSWSItemPropertyHandler::OnItemPropertyApplied -> 0x005aaa10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c1ee0 CSWSItemPropertyHandler::ApplyEnhancementBonus -> 0x005aaad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0x46*} | Steam {0x8*,0x10*,0x46*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005aaa70: 16-byte aligned: YES
  005aaa70  55             PUSH EBP
  005aaa71  8B EC          MOV EBP,ESP
  005aaa73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aaa70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005aaa70 FUN_005aaa70 size=88 ret=16 callers=0 callees=0

undefined4 __thiscall
FUN_005aaa70(int param_1,undefined4 param_2,ushort *param_3,undefined4 param_4,undefined4 param_5)

{
  ushort uVar1;
  undefined4 uVar2;
  
  uVar1 = *param_3;
  if ((uVar1 < 0x46) && (*(int *)(*(int *)(param_1 + 8) + (uint)uVar1 * 4) != 0)) {
    uVar2 = (**(code **)(*(int *)(param_1 + 8) + (uint)uVar1 * 4))(param_2,param_3,param_4,param_5);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x005aaa10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aaa10: 16-byte aligned: YES
  005aaa10  55             PUSH EBP
  005aaa11  8B EC          MOV EBP,ESP
  005aaa13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005aaa10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

## Card B0-11-25

GOG function: CResGFF::WriteFieldINT  gog 0x00625d50  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00719b90 (size 96, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::WriteFieldDWORD
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719bf0:STR0/CALLEE2/SIG1/SRV0/VT0
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
imm sets (* = in both, 2 common): GOG {0x5,0x8*,0xc*} | Steam {0x4,0x8*,0xc*}

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

### decoy: 0x00719bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719bf0: 16-byte aligned: YES
  00719bf0  55             PUSH EBP
  00719bf1  8B EC          MOV EBP,ESP
  00719bf3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719bf0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-11-26

GOG function: CSWSItemPropertyHandler::RemoveAbilityBonus  gog 0x007c5100  callconv=__thiscall  param_size_bytes=12  gog_size=669  gog_retn=16
Candidate Steam address: 0x005adcf0 (size 669, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005adaf0:STR0/CALLEE0/SIG0.35/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4f00 CSWSItemPropertyHandler::ApplyAbilityBonus -> 0x005adaf0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c53c0 CSWSItemPropertyHandler::ApplyACBonus -> 0x005adfb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x150*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x150*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x5*,0x7*,0x10*,0x50*,0x148*} | Steam {0x5*,0x7*,0x10*,0x50*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005adcf0: 16-byte aligned: YES
  005adcf0  55             PUSH EBP
  005adcf1  8B EC          MOV EBP,ESP
  005adcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adcf0
string literals (1):
  0x00992E8C  at 005add18  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x150 [dec 336]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005adcf0 FUN_005adcf0 size=669 ret=16 callers=0 callees=6

undefined4 FUN_005adcf0(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953038;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_28;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x150);
LAB_005adda0:
  while( true ) {
    if (local_18[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fac < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
    if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fac) &&
         ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar3 = FUN_005e4e60(1), iVar3 == local_14))
    break;
    local_20 = local_20 + 1;
  }
  switch(*(undefined2 *)(param_2 + 2)) {
  case 0:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960f8) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 1:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960fc) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 2:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996100) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
  case 3:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996104) {
      local_20 = local_20 + 1;
      goto LAB_005adda0;
    }
    break;
/* ... 17 more lines truncated by --max-lines */
```

### decoy: 0x005adaf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adaf0: 16-byte aligned: YES
  005adaf0  55             PUSH EBP
  005adaf1  8B EC          MOV EBP,ESP
  005adaf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adaf0
string literals (1):
  0x00992E8C  at 005adb18  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-27

GOG function: CResGFF::WriteFieldINT  gog 0x00625d50  callconv=(blank)  param_size_bytes=12  gog_size=96  gog_retn=12
Candidate Steam address: 0x00719ce0 (size 96, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::WriteFieldFLOAT
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.83
- best decoy (neighbour/other candidate) scores: 0x00719c50:STR0/CALLEE2/SIG0.45/SRV0/VT0
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
imm sets (* = in both, 2 common): GOG {0x5,0x8*,0xc*} | Steam {0x8*,0xc*}

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

## Card B0-11-28

GOG function: CSWSItemPropertyHandler::RemoveChangedSavingThrow  gog 0x007c60c0  callconv=(blank)  param_size_bytes=16  gog_size=402  gog_retn=16
Candidate Steam address: 0x005aecb0 (size 402, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af020:STR0/CALLEE0/SIG0.548/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> 0x005aeb20  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6260 CSWSItemPropertyHandler::ApplySpecialWalk -> 0x005aee50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x122*} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x122*}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x7*,0x10*,0x44*,0x148*} | Steam {0x2*,0x3*,0x7*,0x10*,0x44*,0x148*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005aecb0: 16-byte aligned: YES
  005aecb0  55             PUSH EBP
  005aecb1  8B EC          MOV EBP,ESP
  005aecb3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005aecb0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x122 [dec 290]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005aecb0 FUN_005aecb0 size=402 ret=16 callers=2 callees=2

undefined4 FUN_005aecb0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  short sVar2;
  undefined4 *puVar3;
  int iVar4;
  uint uVar5;
  int local_18;
  
  sVar2 = *(short *)(param_3 + 2);
  local_18 = 0;
  while( true ) {
    if (*(int *)(param_4 + 0x14c) <= local_18) {
      return 0;
    }
    iVar4 = *(int *)(*(int *)(param_4 + 0x148) + local_18 * 4);
    if ((((((*(ushort *)(iVar4 + 8) == DAT_00995f84) || (*(ushort *)(iVar4 + 8) == DAT_00995f88)) &&
          ((*(ushort *)(iVar4 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
         ((*(int *)(iVar4 + 0x18) == *(int *)(param_2 + 0x14) &&
          (iVar4 = FUN_005e4e60(0), iVar4 == param_1)))) &&
        ((iVar4 = FUN_005e4e60(1), iVar4 == 0 &&
         (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar5 = FUN_005e4e60(3), uVar5 == bVar1)))) &&
       ((((iVar4 = FUN_005e4e60(1), iVar4 == 1 && (sVar2 == 1)) || ((iVar4 == 3 && (sVar2 == 3))))
        || ((iVar4 == 2 && (sVar2 == 2)))))) break;
    local_18 = local_18 + 1;
  }
  puVar3 = *(undefined4 **)(*(int *)(param_4 + 0x148) + local_18 * 4);
  FUN_00544790(*puVar3,puVar3[1 = byte +0x4]);
  return 0;
}
```

### decoy: 0x005af020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af020: 16-byte aligned: YES
  005af020  55             PUSH EBP
  005af021  8B EC          MOV EBP,ESP
  005af023  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af020
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x15C [dec 348]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-11-29

GOG function: CSWSItemPropertyHandler::RemoveDecreaseAC  gog 0x007c88d0  callconv=__thiscall  param_size_bytes=12  gog_size=485  gog_retn=16
Candidate Steam address: 0x005b14c0 (size 485, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b1840:STR0/CALLEE0/SIG0.774/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8730 CSWSItemPropertyHandler::ApplyDecreaseAC -> 0x005b1320  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8ac0 CSWSItemPropertyHandler::ApplyDecreaseSkillModifier -> 0x005b16b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 11 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13a*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13a*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x7*,0x10*,0x14*,0x4c*,0x148*} | Steam {0x2*,0x7*,0x10*,0x14*,0x4c*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b14c0: 16-byte aligned: YES
  005b14c0  55             PUSH EBP
  005b14c1  8B EC          MOV EBP,ESP
  005b14c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b14c0
string literals (1):
  0x00992E8C  at 005b14e8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13A [dec 314]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b14c0 FUN_005b14c0 size=485 ret=16 callers=0 callees=6

undefined4 FUN_005b14c0(int param_1,int param_2,int param_3)

{
  byte bVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined1 *puVar6;
  int *piVar7;
  undefined4 extraout_var;
  undefined4 uVar8;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097c358;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar7 = &local_1c;
  puVar6 = local_28;
  uVar4 = (uint)*(ushort *)(param_2 + 6);
  uVar8 = extraout_var;
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar4,puVar6,piVar7);
  local_8 = 0xffffffff;
  FUN_00733780(uVar3,uVar8);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13a);
  while( true ) {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fe0 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) break;
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fe0) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       (((uVar2 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar2 &&
         (iVar5 = FUN_005e4e60(1), iVar5 == local_1c)) &&
        (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar3 = FUN_005e4e60(2), uVar3 == bVar1)))) {
      FUN_00544790(*local_18,local_18[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_20 = local_20 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b1840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b1840: 16-byte aligned: YES
  005b1840  55             PUSH EBP
  005b1841  8B EC          MOV EBP,ESP
  005b1843  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b1840
string literals (1):
  0x00992E8C  at 005b1868  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x164 [dec 356]  x1  via EDX
```

## Card B0-11-30

GOG function: CSWSItemPropertyHandler::ApplySniperBonus  gog 0x007c9f20  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2b40 (size 34, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyRapidShotBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2b10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9ef0 CSWSItemPropertyHandler::ApplyDoorSabering -> 0x005b2ae0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f50 CSWSItemPropertyHandler::ApplyRapidShotBonus -> 0x005b2b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2e8} | Steam {0x2e4}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b40: 16-byte aligned: YES
  005b2b40  55             PUSH EBP
  005b2b41  8B EC          MOV EBP,ESP
  005b2b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E4 [dec 740]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b40 FUN_005b2b40 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2b40(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e4) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2b10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b10: 16-byte aligned: YES
  005b2b10  55             PUSH EBP
  005b2b11  8B EC          MOV EBP,ESP
  005b2b13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E8 [dec 744]  x1  via EAX
```

## Card B0-11-31

GOG function: CSWSItemPropertyHandler::RemoveBonusFeat  gog 0x007c6d90  callconv=__thiscall  param_size_bytes=12  gog_size=255  gog_retn=16
Candidate Steam address: 0x005af980 (size 255, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af710:STR0/CALLEE0/SIG0.675/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6c70 CSWSItemPropertyHandler::ApplyBonusFeat -> 0x005af860  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6e90 CSWSItemPropertyHandler::ApplyDamageImmunity -> 0x005afa80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x1198*} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x30*,0x148*} | Steam {0x7*,0x10*,0x30*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005af980: 16-byte aligned: YES
  005af980  55             PUSH EBP
  005af981  8B EC          MOV EBP,ESP
  005af983  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005af980
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005af980 FUN_005af980 size=255 ret=16 callers=0 callees=2

undefined4 FUN_005af980(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int local_8;
  
  if ((*(int *)(param_3 + 0x1198) != 0) && ((int *)(param_3 + 0x148) != (int *)0x0)) {
    for (local_8 = 0; local_8 < *(int *)(param_3 + 0x14c); local_8 = local_8 + 1) {
      puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_8 * 4);
      if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996068) && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
          (uVar1 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar1)) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
        FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
        return 0;
      }
    }
  }
  return 0;
}
```

### decoy: 0x005af710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af710: 16-byte aligned: YES
  005af710  55             PUSH EBP
  005af711  8B EC          MOV EBP,ESP
  005af713  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af710
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x16A [dec 362]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-11-32

GOG function: CSWSItemPropertyHandler::RemoveImprovedSavingThrow  gog 0x007c5cf0  callconv=__thiscall  param_size_bytes=12  gog_size=180  gog_retn=16
Candidate Steam address: 0x005ae8e0 (size 180, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aea60:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5c30 CSWSItemPropertyHandler::ApplyImprovedSavingThrow -> 0x005ae820  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5db0 CSWSItemPropertyHandler::ApplyReducedSavingThrow -> 0x005ae9a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x007c60c0 CSWSItemPropertyHandler::RemoveChangedSavingThrow -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 2 common): GOG {0x6*,0x130*} | Steam {0x6*,0x130*}
imm sets (* = in both, 2 common): GOG {0x10*,0x18*} | Steam {0x10*,0x18*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005ae8e0: 16-byte aligned: YES
  005ae8e0  55             PUSH EBP
  005ae8e1  8B EC          MOV EBP,ESP
  005ae8e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae8e0
string literals (1):
  0x00992E8C  at 005ae908  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AECB0  CSWSItemPropertyHandler::RemoveChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae8e0 FUN_005ae8e0 size=180 ret=16 callers=0 callees=5

undefined4 FUN_005ae8e0(undefined4 param_1,int param_2,undefined4 param_3)

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aecb0(local_14,param_1,param_2,param_3);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005aea60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aea60: 16-byte aligned: YES
  005aea60  55             PUSH EBP
  005aea61  8B EC          MOV EBP,ESP
  005aea63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aea60
string literals (1):
  0x00992E8C  at 005aea88  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AECB0  CSWSItemPropertyHandler::RemoveChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-33

GOG function: CSWSItemPropertyHandler::ApplyDamageImmunity  gog 0x007c6e90  callconv=__thiscall  param_size_bytes=12  gog_size=399  gog_retn=20
Candidate Steam address: 0x005afa80 (size 399, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af860:STR0/CALLEE0/SIG0.607/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6d90 CSWSItemPropertyHandler::RemoveBonusFeat -> 0x005af980  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7020 CSWSItemPropertyHandler::RemoveDamageImmunity -> 0x005afc10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 7 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x14*,0x40*,0x8c*,0xfffffff8*} | Steam {0x3*,0x4*,0x14*,0x40*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005afa80: 16-byte aligned: YES
  005afa80  55             PUSH EBP
  005afa81  8B EC          MOV EBP,ESP
  005afa83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005afa80
string literals (1):
  0x00992E8C  at 005afae8  'Value'
direct callees (9):
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005afa80 FUN_005afa80 size=399 ret=20 callers=0 callees=9

undefined4
FUN_005afa80(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  int local_50;
  undefined2 local_44;
  undefined1 local_2c [8];
  int local_24;
  undefined4 local_20;
  int local_1c;
  undefined2 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952fa3;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = FUN_006f0190(*(undefined1 *)(param_2 + 4));
  if (local_1c != 0) {
    local_18 = *(undefined2 *)(param_2 + 2);
    FUN_00733570("Value");
    local_8 = 0;
    FUN_0071bcf0(*(undefined2 *)(param_2 + 6),local_2c,&local_20);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
    local_24 = 1 << ((byte)local_18 & 0x1f);
    iVar2 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar2 == 0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_50;
    if (local_50 != 0) {
      local_44 = (undefined2)DAT_00995f5c;
      *(undefined2 *)(local_50 + 8) = local_44;
      *(ushort *)(local_50 + 0xa [dec 10]) = *(ushort *)(local_50 + 0xa [dec 10]) & 0xfff8 | 3;
      FUN_005e4e80(0,local_24);
      FUN_005e4e80(1,local_20);
      FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
      FUN_00544210(local_14,param_5,param_5);
    }
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005af860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af860: 16-byte aligned: YES
  005af860  55             PUSH EBP
  005af861  8B EC          MOV EBP,ESP
  005af863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005af860
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-11-34

GOG function: CSWSItemPropertyHandler::RemoveEnhancementBonus  gog 0x007c2600  callconv=__thiscall  param_size_bytes=16  gog_size=1917  gog_retn=16
Candidate Steam address: 0x005ab1f0 (size 1917, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005aaad0:STR0/CALLEE0/SIG0.586/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c1ee0 CSWSItemPropertyHandler::ApplyEnhancementBonus -> 0x005aaad0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c2d90 CSWSItemPropertyHandler::ApplyEnhancementPenalty -> 0x005ab980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 12 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x10*,0x20*,0x94*,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x10*,0x20*,0x94*,0x148*,0x4000*,0x8000*,0x10000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005ab1f0: 16-byte aligned: YES
  005ab1f0  55             PUSH EBP
  005ab1f1  8B EC          MOV EBP,ESP
  005ab1f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ab1f0
string literals (1):
  0x00992E8C  at 005ab21f  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x2 [dec 2]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x13C [dec 316]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x122 [dec 290]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ab1f0 FUN_005ab1f0 size=1917 ret=16 callers=0 callees=6

undefined4 FUN_005ab1f0(int param_1,short *param_2,int param_3,int param_4)

{
  byte bVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  undefined1 local_28 [8];
  uint local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952c48;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  FUN_0071bcf0(param_2[3],local_28,&local_14);
  local_8 = 0xffffffff;
  FUN_00733780(uVar3);
  local_18 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005ab2a6:
  do {
    if (local_18[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f50 < (int)(uint)*(ushort *)(*(int *)(*local_18 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_1c = *(undefined4 **)(*local_18 + local_20 * 4);
    if ((((*(ushort *)(local_1c + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f44) &&
         ((*(ushort *)((int)local_1c + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_1c[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar4 = FUN_005e4e60(0), iVar4 == local_14))
    {
      if (param_4 == 0x10) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996188) {
          local_20 = local_20 + 1;
          goto LAB_005ab2a6;
        }
      }
      else if (param_4 == 0x20) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_0099618c) {
          local_20 = local_20 + 1;
          goto LAB_005ab2a6;
        }
      }
      else if (param_4 == 0x4000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996190) {
          local_20 = local_20 + 1;
          goto LAB_005ab2a6;
        }
      }
      else if (param_4 == 0x8000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996194) {
          local_20 = local_20 + 1;
          goto LAB_005ab2a6;
        }
      }
      else if (param_4 == 0x10000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996198) {
          local_20 = local_20 + 1;
          goto LAB_005ab2a6;
        }
      }
/* ... 136 more lines truncated by --max-lines */
```

### decoy: 0x005aaad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aaad0: 16-byte aligned: YES
  005aaad0  55             PUSH EBP
  005aaad1  8B EC          MOV EBP,ESP
  005aaad3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aaad0
string literals (1):
  0x00992E8C  at 005aabd9  'Value'
direct callees (11):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x2 [dec 2]  x8  via EAX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x4  via ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x38 [dec 56]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-35

GOG function: CSWSItemPropertyHandler::RemoveDamageReduction  gog 0x007c7a70  callconv=__thiscall  param_size_bytes=12  gog_size=443  gog_retn=16
Candidate Steam address: 0x005b0660 (size 443, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0990:STR0/CALLEE0/SIG0.809/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c7900 CSWSItemPropertyHandler::ApplyDamageReduction -> 0x005b04f0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7c30 CSWSItemPropertyHandler::ApplyDamageResistance -> 0x005b0820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Amount'
disp sets (* = in both, 10 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x13e*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x13e*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x6*,0x7*,0x10*,0x4c*,0x148*} | Steam {0x6*,0x7*,0x10*,0x4c*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b0660: 16-byte aligned: YES
  005b0660  55             PUSH EBP
  005b0661  8B EC          MOV EBP,ESP
  005b0663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0660
string literals (1):
  0x0099626C  at 005b0688  'Amount'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13E [dec 318]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0660 FUN_005b0660 size=443 ret=16 callers=0 callees=6

undefined4 FUN_005b0660(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined4 extraout_var;
  undefined4 uVar7;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097c358;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Amount");
  local_8 = 0;
  piVar6 = &local_1c;
  puVar5 = local_28;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  uVar7 = extraout_var;
  FUN_006f0190(6);
  FUN_0071bcf0(uVar3,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2,uVar7);
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13e);
  while( true ) {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f4c < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) break;
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f4c) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((iVar4 = FUN_005e4e60(0), iVar4 == local_1c &&
        (uVar1 = *(ushort *)(param_2 + 2), iVar4 = FUN_005e4e60(1), iVar4 == uVar1 + 1)))) {
      FUN_00544790(*local_18,local_18[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_20 = local_20 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0990: 16-byte aligned: YES
  005b0990  55             PUSH EBP
  005b0991  8B EC          MOV EBP,ESP
  005b0993  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0990
string literals (1):
  0x0099626C  at 005b09c3  'Amount'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
  +0x140 [dec 320]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-11-36

GOG function: CSWSItemPropertyHandler::RemoveTrueSeeing  gog 0x007c66a0  callconv=__thiscall  param_size_bytes=4  gog_size=194  gog_retn=16
Candidate Steam address: 0x005af290 (size 194, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af020:STR0/CALLEE0/SIG0.613/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c65c0 CSWSItemPropertyHandler::ApplyTrueSeeing -> 0x005af1b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6770 CSWSItemPropertyHandler::ApplyFreedomOfMovement -> 0x005af360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xa*,0x14*,0x18*} | Steam {0x4*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x2c*,0x148*} | Steam {0x7*,0x10*,0x2c*,0x148*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 16)
```

### prologue
```
prologue for 0x005af290: 16-byte aligned: YES
  005af290  55             PUSH EBP
  005af291  8B EC          MOV EBP,ESP
  005af293  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005af290
string literals (0):
direct callees (1):
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005af290 FUN_005af290 size=194 ret=16 callers=0 callees=1

undefined4 FUN_005af290(int param_1,undefined4 param_2,int param_3)

{
  undefined4 *puVar1;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_3 + 0x14c) <= local_c) {
      return 0;
    }
    puVar1 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_c * 4);
    if (((*(ushort *)(puVar1 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_0099603c) && (puVar1[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((*(ushort *)((int)puVar1 + 0xa [dec 10]) & 7) == DAT_009967b0)) break;
    local_c = local_c + 1;
  }
  FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
  return 0;
}
```

### decoy: 0x005af020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af020: 16-byte aligned: YES
  005af020  55             PUSH EBP
  005af021  8B EC          MOV EBP,ESP
  005af023  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af020
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x15C [dec 348]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-11-37

GOG function: CSWSItemPropertyHandler::RemoveDecreaseAbilityScore  gog 0x007c8470  callconv=__thiscall  param_size_bytes=12  gog_size=677  gog_retn=16
Candidate Steam address: 0x005b1060 (size 677, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0cc0:STR0/CALLEE0/SIG0.588/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8270 CSWSItemPropertyHandler::ApplyDecreaseAbilityScore -> 0x005b0e60  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8730 CSWSItemPropertyHandler::ApplyDecreaseAC -> 0x005b1320  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x150*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x150*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x5*,0x7*,0x10*,0x15*,0x50*,0x148*} | Steam {0x5*,0x7*,0x10*,0x15*,0x50*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b1060: 16-byte aligned: YES
  005b1060  55             PUSH EBP
  005b1061  8B EC          MOV EBP,ESP
  005b1063  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b1060
string literals (1):
  0x00992E8C  at 005b1088  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
  +0x150 [dec 336]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1060 FUN_005b1060 size=677 ret=16 callers=0 callees=6

undefined4 FUN_005b1060(int param_1,int param_2,int param_3)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953038;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_1c;
  puVar4 = local_28;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x15);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x150);
LAB_005b1118:
  while( true ) {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995fb0 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995fb0) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (iVar3 = FUN_005e4e60(1), iVar3 == local_1c))
    break;
    local_20 = local_20 + 1;
  }
  switch(*(undefined2 *)(param_2 + 2)) {
  case 0:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960f8) {
      local_20 = local_20 + 1;
      goto LAB_005b1118;
    }
    break;
  case 1:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_009960fc) {
      local_20 = local_20 + 1;
      goto LAB_005b1118;
    }
    break;
  case 2:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996100) {
      local_20 = local_20 + 1;
      goto LAB_005b1118;
    }
    break;
  case 3:
    iVar3 = FUN_005e4e60(0);
    if (iVar3 != DAT_00996104) {
      local_20 = local_20 + 1;
      goto LAB_005b1118;
    }
/* ... 18 more lines truncated by --max-lines */
```

### decoy: 0x005b0cc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0cc0: 16-byte aligned: YES
  005b0cc0  55             PUSH EBP
  005b0cc1  8B EC          MOV EBP,ESP
  005b0cc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0cc0
string literals (1):
  0x00992E8C  at 005b0cf3  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B0-11-38

GOG function: CSWSItemPropertyHandler::ApplyAttackBonus  gog 0x007c4070  callconv=__thiscall  param_size_bytes=20  gog_size=864  gog_retn=20
Candidate Steam address: 0x005acc60 (size 864, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005acfc0:STR0/CALLEE0/SIG0.575/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c3c80 CSWSItemPropertyHandler::RemoveDamageBonus -> 0x005ac870  gate_class=REVIEW
- GOG-order neighbour after:  0x007c43d0 CSWSItemPropertyHandler::RemoveAttackBonus -> 0x005acfc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734c90 CGameEffect::CopyEffect -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x38*,0x122*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x38*,0x122*,0x130*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x8*,0x10*,0x14*,0x20*,0x27*,0x28*,0x68*,0x8c*,0x4000*,0x8000*,0x10000*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x8*,0x10*,0x14*,0x20*,0x27*,0x28*,0x68*,0x8c*,0x4000*,0x8000*,0x10000*,...(+1)}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005acc60: 16-byte aligned: YES
  005acc60  55             PUSH EBP
  005acc61  8B EC          MOV EBP,ESP
  005acc63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005acc60
string literals (1):
  0x00992E8C  at 005accce  'Value'
direct callees (10):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005acc60 FUN_005acc60 size=864 ret=20 callers=0 callees=10

undefined4
FUN_005acc60(int param_1,short *param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  undefined4 local_78;
  int local_74;
  undefined2 local_44;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952d5e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_74 = 0;
  }
  else {
    local_74 = FUN_005e4920(1);
  }
  local_20 = local_74;
  local_8 = 0xffffffff;
  local_14 = local_74;
  FUN_00733570("Value");
  local_8 = 1;
  FUN_0071bcf0(param_2[3],local_2c,&local_18);
  local_8 = 0xffffffff;
  FUN_00733780();
  if (local_18 != 0) {
    local_44 = (undefined2)DAT_00995f44;
    *(undefined2 *)(local_14 + 8) = local_44;
    *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_18);
    if (param_4 == 0x10) {
      FUN_005e4e80(1,DAT_00996188);
    }
    else if (param_4 == 0x20) {
      FUN_005e4e80(1,DAT_0099618c);
    }
    else if (param_4 == 0x4000) {
      FUN_005e4e80(1,DAT_00996190);
    }
    else if (param_4 == 0x8000) {
      FUN_005e4e80(1,DAT_00996194);
    }
    else if (param_4 == 0x10000) {
      FUN_005e4e80(1,DAT_00996198);
    }
    else if (param_4 == 8) {
      FUN_005e4e80(1,DAT_0099619c);
    }
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    if (*param_2 == 0x27) {
      if (param_2[1] == 1) {
        FUN_005e4e80(3,1);
      }
      else if (param_2[1] == 2) {
        FUN_005e4e80(4,2);
      }
      else if (param_2[1] == 3) {
        FUN_005e4e80(4,3);
      }
    }
    else if (*param_2 == 0x28) {
      FUN_005e4e80(2,param_2[1]);
    }
    iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if (*(char *)(iVar1 + 8) == '\x03') {
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x005acfc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005acfc0: 16-byte aligned: YES
  005acfc0  55             PUSH EBP
  005acfc1  8B EC          MOV EBP,ESP
  005acfc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005acfc0
string literals (1):
  0x00992E8C  at 005acfe8  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x38 [dec 56]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-39

GOG function: CSWSItemPropertyHandler::RemoveDamageResistance  gog 0x007c7da0  callconv=__thiscall  param_size_bytes=12  gog_size=442  gog_retn=16
Candidate Steam address: 0x005b0990 (size 442, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0660:STR0/CALLEE0/SIG0.809/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c7c30 CSWSItemPropertyHandler::ApplyDamageResistance -> 0x005b0820  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7f60 CSWSItemPropertyHandler::ApplyDamageVulnerability -> 0x005b0b50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Amount'
disp sets (* = in both, 10 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x140*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*,0x140*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x4c*,0x148*} | Steam {0x7*,0x10*,0x4c*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b0990: 16-byte aligned: YES
  005b0990  55             PUSH EBP
  005b0991  8B EC          MOV EBP,ESP
  005b0993  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0990
string literals (1):
  0x0099626C  at 005b09c3  'Amount'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
  +0x140 [dec 320]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0990 FUN_005b0990 size=442 ret=16 callers=0 callees=6

undefined4 FUN_005b0990(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined4 extraout_var;
  undefined4 uVar6;
  undefined1 local_2c [8];
  uint local_24;
  int local_20;
  undefined2 local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009530c8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = *(undefined2 *)(param_2 + 2);
  FUN_00733570("Amount");
  local_8 = 0;
  piVar5 = &local_20;
  puVar4 = local_2c;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  uVar6 = extraout_var;
  FUN_006f0190(7);
  FUN_0071bcf0(uVar3,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar6);
  local_14 = (int *)(param_3 + 0x148);
  local_24 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x140);
  while( true ) {
    if (local_14[1 = byte +0x4] <= (int)local_24) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f24 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_24 * 4) + 8)) break;
    local_18 = *(undefined4 **)(*local_14 + local_24 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f24) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((iVar2 = FUN_005e4e60(0), iVar2 == 1 << ((byte)local_1c & 0x1f) &&
        (iVar2 = FUN_005e4e60(1), iVar2 == local_20)))) {
      FUN_00544790(*local_18,local_18[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_24 = local_24 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0660: 16-byte aligned: YES
  005b0660  55             PUSH EBP
  005b0661  8B EC          MOV EBP,ESP
  005b0663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0660
string literals (1):
  0x0099626C  at 005b0688  'Amount'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13E [dec 318]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-11-40

GOG function: CSWSItemPropertyHandler::ApplyAttackPenalty  gog 0x007c47d0  callconv=__thiscall  param_size_bytes=20  gog_size=765  gog_retn=20
Candidate Steam address: 0x005ad3c0 (size 765, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ad6c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c43d0 CSWSItemPropertyHandler::RemoveAttackBonus -> 0x005acfc0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4ad0 CSWSItemPropertyHandler::RemoveAttackPenalty -> 0x005ad6c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*} | Steam {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x8*,0x10*,0x14*,0x20*,0x64*,0x8c*,0x4000*,0x8000*,0x10000*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x8*,0x10*,0x14*,0x20*,0x64*,0x8c*,0x4000*,0x8000*,0x10000*,0xfffffff8*}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005ad3c0: 16-byte aligned: YES
  005ad3c0  55             PUSH EBP
  005ad3c1  8B EC          MOV EBP,ESP
  005ad3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ad3c0
string literals (1):
  0x00992E8C  at 005ad42e  'Value'
direct callees (13):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ad3c0 FUN_005ad3c0 size=765 ret=20 callers=0 callees=13

undefined4 FUN_005ad3c0(int param_1,int param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 local_70;
  int local_6c;
  undefined2 local_48;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952dce;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_6c = 0;
  }
  else {
    local_6c = FUN_005e4920(1);
  }
  local_20 = local_6c;
  local_8 = 0xffffffff;
  local_14 = local_6c;
  FUN_00733570("Value");
  local_8 = 1;
  piVar4 = &local_18;
  puVar3 = local_2c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780();
  iVar1 = local_14;
  local_18 = -local_18;
  if (local_18 == 0) {
    if (local_14 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar1);
    }
  }
  else {
    local_48 = (undefined2)DAT_00995f48;
    *(undefined2 *)(local_14 + 8) = local_48;
    *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_18);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    if (param_4 == 0x10) {
      FUN_005e4e80(1,DAT_00996188);
    }
    else if (param_4 == 0x20) {
      FUN_005e4e80(1,DAT_0099618c);
    }
    else if (param_4 == 0x4000) {
      FUN_005e4e80(1,DAT_00996190);
    }
    else if (param_4 == 0x8000) {
      FUN_005e4e80(1,DAT_00996194);
    }
    else if (param_4 == 0x10000) {
      FUN_005e4e80(1,DAT_00996198);
    }
    else if (param_4 == 8) {
      FUN_005e4e80(1,DAT_0099619c);
    }
    iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if (*(char *)(iVar1 + 8) == '\x03') {
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x005ad6c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ad6c0: 16-byte aligned: YES
  005ad6c0  55             PUSH EBP
  005ad6c1  8B EC          MOV EBP,ESP
  005ad6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ad6c0
string literals (1):
  0x00992E8C  at 005ad6e8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13C [dec 316]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-11-41

GOG function: CSWSItemPropertyHandler::ApplyEnhancementPenalty  gog 0x007c2d90  callconv=__thiscall  param_size_bytes=20  gog_size=1391  gog_retn=20
Candidate Steam address: 0x005ab980 (size 1391, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005abef0:STR0/CALLEE0/SIG0.515/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c2600 CSWSItemPropertyHandler::RemoveEnhancementBonus -> 0x005ab1f0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c3300 CSWSItemPropertyHandler::RemoveEnhancementPenalty -> 0x005abef0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*} | Steam {0x6*,0x8*,0xa*,0xc*,0x14*,0x34*,0x122*,0x130*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x10*,0x14*,0x20*,0x8c*,0xa8*,0x4000*,0x8000*,0x10000*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x10*,0x14*,0x20*,0x8c*,0xa8*,0x4000*,0x8000*,0x10000*,0xfffffff8*}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005ab980: 16-byte aligned: YES
  005ab980  55             PUSH EBP
  005ab981  8B EC          MOV EBP,ESP
  005ab983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ab980
string literals (1):
  0x00992E8C  at 005aba70  'Value'
direct callees (12):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 8):
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ab980 FUN_005ab980 size=1391 ret=20 callers=0 callees=12

undefined4 FUN_005ab980(int param_1,int param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  uint uVar1;
  int iVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 local_b8;
  undefined4 local_b4;
  int local_b0;
  int local_ac;
  undefined2 local_88;
  undefined2 local_68;
  undefined1 local_44 [8];
  int local_3c;
  int local_38;
  int local_34;
  int local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ca4;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
  local_34 = FUN_00919723(0x8c,uVar1);
  local_8 = 0;
  if (local_34 == 0) {
    local_ac = 0;
  }
  else {
    local_ac = FUN_005e4920(1);
  }
  local_30 = local_ac;
  local_8 = 0xffffffff;
  local_14 = local_ac;
  local_3c = FUN_00919723(0x8c,uVar1);
  local_8 = 1;
  if (local_3c == 0) {
    local_b0 = 0;
  }
  else {
    local_b0 = FUN_005e4920(1);
  }
  local_38 = local_b0;
  local_8 = 0xffffffff;
  local_1c = local_b0;
  FUN_00733570("Value");
  local_8 = 2;
  piVar4 = &local_18;
  puVar3 = local_44;
  uVar1 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar1,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780();
  local_18 = -local_18;
  if (local_18 != 0) {
    local_68 = (undefined2)DAT_00995f48;
    *(undefined2 *)(local_14 + 8) = local_68;
    *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_18);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    if (param_4 == 0x10) {
      FUN_005e4e80(1,DAT_00996188);
    }
    else if (param_4 == 0x20) {
      FUN_005e4e80(1,DAT_0099618c);
/* ... 77 more lines truncated by --max-lines */
```

### decoy: 0x005abef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005abef0: 16-byte aligned: YES
  005abef0  55             PUSH EBP
  005abef1  8B EC          MOV EBP,ESP
  005abef3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005abef0
string literals (1):
  0x00992E8C  at 005abf19  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x13C [dec 316]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-42

GOG function: CSWSItemPropertyHandler::ApplySniperBonus  gog 0x007c9f20  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2ae0 (size 34, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyDoorSabering
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2b10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9ef0 CSWSItemPropertyHandler::ApplyDoorSabering -> 0x005b2ae0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f50 CSWSItemPropertyHandler::ApplyRapidShotBonus -> 0x005b2b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2e8} | Steam {0x2e0}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2ae0: 16-byte aligned: YES
  005b2ae0  55             PUSH EBP
  005b2ae1  8B EC          MOV EBP,ESP
  005b2ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2ae0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E0 [dec 736]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2ae0 FUN_005b2ae0 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2ae0(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e0) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2b10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b10: 16-byte aligned: YES
  005b2b10  55             PUSH EBP
  005b2b11  8B EC          MOV EBP,ESP
  005b2b13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E8 [dec 744]  x1  via EAX
```

## Card B0-11-43

GOG function: CSWSItemPropertyHandler::RemoveDamageImmunity  gog 0x007c7020  callconv=__thiscall  param_size_bytes=12  gog_size=593  gog_retn=16
Candidate Steam address: 0x005afc10 (size 593, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af980:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6e90 CSWSItemPropertyHandler::ApplyDamageImmunity -> 0x005afa80  gate_class=REVIEW
- GOG-order neighbour after:  0x007c72b0 CSWSItemPropertyHandler::ApplyDamagePenalty -> 0x005afea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*}
imm sets (* = in both, 5 common): GOG {0x7*,0xd*,0x10*,0x50*,0x148*} | Steam {0x7*,0xd*,0x10*,0x50*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005afc10: 16-byte aligned: YES
  005afc10  55             PUSH EBP
  005afc11  8B EC          MOV EBP,ESP
  005afc13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005afc10
string literals (1):
  0x00992E8C  at 005afc8d  'Value'
direct callees (6):
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005afc10 FUN_005afc10 size=593 ret=16 callers=0 callees=6

undefined4 FUN_005afc10(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined1 local_34 [8];
  int local_2c;
  int local_28;
  int local_24;
  undefined2 local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952fc8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = *(undefined2 *)(param_2 + 2);
  local_18 = (int *)(param_3 + 0x148);
  if ((local_18 != (int *)0x0) &&
     (local_24 = FUN_006f0190(*(undefined1 *)(param_2 + 4)), local_24 != 0)) {
    FUN_00733570("Value");
    local_8 = 0;
    FUN_0071bcf0(*(undefined2 *)(param_2 + 6),local_34,&local_28);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
    switch(local_20) {
    case 0:
      local_2c = DAT_00996110;
      break;
    case 1:
      local_2c = DAT_00996114;
      break;
    case 2:
      local_2c = DAT_00996118;
      break;
    case 3:
      local_2c = DAT_0099611c;
      break;
    case 4:
      local_2c = DAT_00996120;
      break;
    case 5:
      local_2c = DAT_00996124;
      break;
    case 6:
      local_2c = DAT_00996128;
      break;
    case 7:
      local_2c = DAT_0099612c;
      break;
    case 8:
      local_2c = DAT_00996130;
      break;
    case 9:
      local_2c = DAT_00996134;
      break;
    case 10:
      local_2c = DAT_00996138;
      break;
    case 0xb:
      local_2c = DAT_0099613c;
      break;
    case 0xc:
      local_2c = DAT_00996140;
      break;
    case 0xd:
      local_2c = DAT_00996144;
      break;
    default:
      goto switchD_005afce4_default;
    }
    for (local_14 = 0; local_14 < local_18[1 = byte +0x4]; local_14 = local_14 + 1) {
      local_1c = *(undefined4 **)(*local_18 + local_14 * 4);
/* ... 15 more lines truncated by --max-lines */
```

### decoy: 0x005af980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af980: 16-byte aligned: YES
  005af980  55             PUSH EBP
  005af981  8B EC          MOV EBP,ESP
  005af983  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af980
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-11-44

GOG function: CSWSItemPropertyHandler::RemoveDamageVulnerability  gog 0x007c80d0  callconv=__thiscall  param_size_bytes=12  gog_size=416  gog_retn=16
Candidate Steam address: 0x005b0cc0 (size 416, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0990:STR0/CALLEE0/SIG0.65/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c7f60 CSWSItemPropertyHandler::ApplyDamageVulnerability -> 0x005b0b50  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8270 CSWSItemPropertyHandler::ApplyDecreaseAbilityScore -> 0x005b0e60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 8 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x130*}
imm sets (* = in both, 5 common): GOG {0x7*,0x10*,0x16*,0x44*,0x148*} | Steam {0x7*,0x10*,0x16*,0x44*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b0cc0: 16-byte aligned: YES
  005b0cc0  55             PUSH EBP
  005b0cc1  8B EC          MOV EBP,ESP
  005b0cc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b0cc0
string literals (1):
  0x00992E8C  at 005b0cf3  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b0cc0 FUN_005b0cc0 size=416 ret=16 callers=0 callees=6

undefined4 FUN_005b0cc0(int param_1,int param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  uint uVar3;
  undefined1 *puVar4;
  int *piVar5;
  undefined4 extraout_var;
  undefined4 uVar6;
  undefined1 local_2c [8];
  int local_24;
  undefined2 local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009530f8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = *(undefined2 *)(param_2 + 2);
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_1c;
  puVar4 = local_2c;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  uVar6 = extraout_var;
  FUN_006f0190(0x16);
  FUN_0071bcf0(uVar3,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar6);
  local_14 = (int *)(param_3 + 0x148);
  local_24 = 0;
  while( true ) {
    if (local_14[1 = byte +0x4] <= local_24) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f60 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_24 * 4) + 8)) break;
    local_18 = *(undefined4 **)(*local_14 + local_24 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f60) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((iVar2 = FUN_005e4e60(0), iVar2 == 1 << ((byte)local_20 & 0x1f) &&
        (iVar2 = FUN_005e4e60(1), iVar2 == local_1c)))) {
      FUN_00544790(*local_18,local_18[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_24 = local_24 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0990: 16-byte aligned: YES
  005b0990  55             PUSH EBP
  005b0991  8B EC          MOV EBP,ESP
  005b0993  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0990
string literals (1):
  0x0099626C  at 005b09c3  'Amount'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
  +0x140 [dec 320]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-11-45

GOG function: CSWSItemPropertyHandler::RemoveSpecialWalk  gog 0x007c6430  callconv=__thiscall  param_size_bytes=12  gog_size=392  gog_retn=16
Candidate Steam address: 0x005af020 (size 392, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af290:STR0/CALLEE0/SIG0.613/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6260 CSWSItemPropertyHandler::ApplySpecialWalk -> 0x005aee50  gate_class=REVIEW
- GOG-order neighbour after:  0x007c65c0 CSWSItemPropertyHandler::ApplyTrueSeeing -> 0x005af1b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x15c*,0x1198*} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x15c*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x4c*,0x148*} | Steam {0x7*,0x10*,0x4c*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005af020: 16-byte aligned: YES
  005af020  55             PUSH EBP
  005af021  8B EC          MOV EBP,ESP
  005af023  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005af020
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x15C [dec 348]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005af020 FUN_005af020 size=392 ret=16 callers=0 callees=2

undefined4 FUN_005af020(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  uint local_10;
  
  local_10 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x15c);
  while( true ) {
    if (*(int *)(param_3 + 0x14c) <= (int)local_10) {
      return 0;
    }
    if ((int)DAT_00996008 <
        (int)(uint)*(ushort *)(*(int *)(*(int *)(param_3 + 0x148) + local_10 * 4) + 8)) break;
    puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_10 * 4);
    if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996004) && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
        (uVar1 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar1)) &&
       ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
      FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
    }
    else if (((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996008) && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14)))
            && ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
      FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
      return 0;
    }
    local_10 = local_10 + 1;
  }
  return 0;
}
```

### decoy: 0x005af290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af290: 16-byte aligned: YES
  005af290  55             PUSH EBP
  005af291  8B EC          MOV EBP,ESP
  005af293  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af290
string literals (0):
direct callees (1):
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-11-46

GOG function: CGameEffect::GetString  gog 0x00734be0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x005e4f00 (size 26, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGameEffect::SetObjectID
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e4f20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00734bc0 CGameEffect::SetObjectID -> 0x005e4f00  gate_class=REVIEW
- GOG-order neighbour after:  0x00734c20 CGameEffect::SetString -> 0x005e4f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x48} | Steam {0x78}
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
prologue for 0x005e4f00: 16-byte aligned: YES
  005e4f00  55             PUSH EBP
  005e4f01  8B EC          MOV EBP,ESP
  005e4f03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e4f00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e4f00 FUN_005e4f00 size=26 ret=8 callers=17 callees=0

void __thiscall FUN_005e4f00(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x78 + param_2 * 4) = param_3;
  return;
}
```

### decoy: 0x005e4f20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e4f20: 16-byte aligned: YES
  005e4f20  55             PUSH EBP
  005e4f21  8B EC          MOV EBP,ESP
  005e4f23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e4f20
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

## Card B0-11-47

GOG function: CSWSItemPropertyHandler::ApplyReducedSavingThrow  gog 0x007c5db0  callconv=__thiscall  param_size_bytes=12  gog_size=184  gog_retn=20
Candidate Steam address: 0x005ae9a0 (size 184, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ae820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5cf0 CSWSItemPropertyHandler::RemoveImprovedSavingThrow -> 0x005ae8e0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5e70 CSWSItemPropertyHandler::RemoveReducedSavingThrow -> 0x005aea60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 2 common): GOG {0x6*,0x130*} | Steam {0x6*,0x130*}
imm sets (* = in both, 2 common): GOG {0x14*,0x18*} | Steam {0x14*,0x18*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005ae9a0: 16-byte aligned: YES
  005ae9a0  55             PUSH EBP
  005ae9a1  8B EC          MOV EBP,ESP
  005ae9a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ae9a0
string literals (1):
  0x00992E8C  at 005ae9c8  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005ae9a0 FUN_005ae9a0 size=184 ret=20 callers=0 callees=5

undefined4
FUN_005ae9a0(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aeb20(param_1,local_14,param_2,param_3,param_5);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae820: 16-byte aligned: YES
  005ae820  55             PUSH EBP
  005ae821  8B EC          MOV EBP,ESP
  005ae823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae820
string literals (1):
  0x00992E8C  at 005ae848  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-48

GOG function: CSWSItemPropertyHandler::ApplyDamageBonus  gog 0x007c3800  callconv=__thiscall  param_size_bytes=20  gog_size=1142  gog_retn=20
Candidate Steam address: 0x005ac3f0 (size 1142, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ac870:STR0/CALLEE0/SIG0.565/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c3300 CSWSItemPropertyHandler::RemoveEnhancementPenalty -> 0x005abef0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c3c80 CSWSItemPropertyHandler::RemoveDamageBonus -> 0x005ac870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734c90 CGameEffect::CopyEffect -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x2*,0x6*,0x8*,0x9*,0xa*,0xc*,0x14*,0x34*,0x122*} | Steam {0x2*,0x6*,0x8*,0x9*,0xa*,0xc*,0x14*,0x34*,0x122*}
imm sets (* = in both, 19 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xb*,0xc*,0xd*,0x10*,0x14*,0x20*,0x80*,0x8c*,0x4000*,...(+5)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xb*,0xc*,0xd*,0x10*,0x14*,0x20*,0x80*,0x8c*,0x4000*,...(+5)}

### abi
```
GOG param bytes: 20; callconv: thiscall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005ac3f0: 16-byte aligned: YES
  005ac3f0  55             PUSH EBP
  005ac3f1  8B EC          MOV EBP,ESP
  005ac3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ac3f0
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 9):
  +0x2 [dec 2]  x5  via EAX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x9 [dec 9]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x34 [dec 52]  x1  via ECX
  +0x122 [dec 290]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ac3f0 FUN_005ac3f0 size=1142 ret=20 callers=0 callees=9

undefined4
FUN_005ac3f0(int param_1,short *param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  undefined4 local_8c;
  int local_80;
  undefined2 local_4c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952d16;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_80 = 0;
  }
  else {
    local_80 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if ((param_4 == 0x40000) || (param_4 == 0x80000)) {
    if (local_80 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_80);
    }
  }
  else if ((param_4 == 0x40000) || (param_4 == 0x80000)) {
    if (local_80 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_80);
    }
  }
  else {
    sVar1 = param_2[3];
    if (sVar1 == 0) {
      if (local_80 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(local_80);
      }
    }
    else {
      local_4c = (undefined2)DAT_00995f50;
      *(undefined2 *)(local_80 + 8) = local_4c;
      *(ushort *)(local_80 + 0xa [dec 10]) = *(ushort *)(local_80 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
      FUN_005e4e80(0,sVar1);
      if (*param_2 == 0xb) {
        FUN_005e4e80(1,1 << ((byte)param_2[1] & 0x1f));
      }
      else {
        FUN_005e4e80(1,1 << (*(byte *)((int)param_2 + 9) & 0x1f));
      }
      if (param_4 == 0x10) {
        FUN_005e4e80(5,DAT_009961a4);
      }
      else if (param_4 == 0x20) {
        FUN_005e4e80(5,DAT_009961a8);
      }
      else if (param_4 == 0x4000) {
        FUN_005e4e80(5,DAT_009961ac);
      }
      else if (param_4 == 0x8000) {
        FUN_005e4e80(5,DAT_009961b0);
      }
      else if (param_4 == 0x10000) {
        FUN_005e4e80(5,DAT_009961b4);
      }
      else if (param_4 == 8) {
        FUN_005e4e80(5,DAT_009961b8);
      }
      FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      if (*param_2 == 0xc) {
        if (param_2[1] == 1) {
/* ... 33 more lines truncated by --max-lines */
```

### decoy: 0x005ac870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ac870: 16-byte aligned: YES
  005ac870  55             PUSH EBP
  005ac871  8B EC          MOV EBP,ESP
  005ac873  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ac870
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x9 [dec 9]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x142 [dec 322]  x1  via EAX
```

## Card B0-11-49

GOG function: CSWSItemPropertyHandler::RemoveFreedomOfMovement  gog 0x007c6b20  callconv=__thiscall  param_size_bytes=4  gog_size=336  gog_retn=16
Candidate Steam address: 0x005af710 (size 336, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af980:STR0/CALLEE0/SIG0.675/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6770 CSWSItemPropertyHandler::ApplyFreedomOfMovement -> 0x005af360  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6c70 CSWSItemPropertyHandler::ApplyBonusFeat -> 0x005af860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xa*,0x14*,0x18*,0x16a*,0x1198*} | Steam {0x4*,0x8*,0xa*,0x14*,0x18*,0x16a*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x34*,0x148*} | Steam {0x7*,0x10*,0x34*,0x148*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 16)
```

### prologue
```
prologue for 0x005af710: 16-byte aligned: YES
  005af710  55             PUSH EBP
  005af711  8B EC          MOV EBP,ESP
  005af713  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005af710
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x16A [dec 362]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005af710 FUN_005af710 size=336 ret=16 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005af710(int param_1,undefined4 param_2,int param_3)

{
  undefined4 *puVar1;
  int iVar2;
  uint local_10;
  
  local_10 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x16a);
  while( true ) {
    if (*(int *)(param_3 + 0x14c) <= (int)local_10) {
      return 0;
    }
    if ((int)DAT_00995f74 <
        (int)(uint)*(ushort *)(*(int *)(*(int *)(param_3 + 0x148) + local_10 * 4) + 8)) break;
    puVar1 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_10 * 4);
    if ((((*(ushort *)(puVar1 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f74) &&
         ((*(ushort *)((int)puVar1 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (puVar1[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       (((iVar2 = FUN_005e4e60(0), iVar2 == _DAT_009961d4 ||
         (iVar2 = FUN_005e4e60(0), iVar2 == DAT_009961e0)) ||
        ((iVar2 = FUN_005e4e60(0), iVar2 == DAT_009961e4 ||
         (iVar2 = FUN_005e4e60(0), iVar2 == DAT_00996214)))))) {
      FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
    }
    local_10 = local_10 + 1;
  }
  return 0;
}
```

### decoy: 0x005af980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af980: 16-byte aligned: YES
  005af980  55             PUSH EBP
  005af981  8B EC          MOV EBP,ESP
  005af983  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005af980
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-11-50

GOG function: CSWSItemPropertyHandler::RemoveDecreaseSkillModifier  gog 0x007c8c50  callconv=__thiscall  param_size_bytes=12  gog_size=485  gog_retn=16
Candidate Steam address: 0x005b1840 (size 485, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b14c0:STR0/CALLEE0/SIG0.774/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8ac0 CSWSItemPropertyHandler::ApplyDecreaseSkillModifier -> 0x005b16b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c8e40 CSWSItemPropertyHandler::ApplyImmunity -> 0x005b1a30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 11 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x164*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x164*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x7*,0x10*,0x15*,0x4c*,0x148*} | Steam {0x2*,0x7*,0x10*,0x15*,0x4c*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b1840: 16-byte aligned: YES
  005b1840  55             PUSH EBP
  005b1841  8B EC          MOV EBP,ESP
  005b1843  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b1840
string literals (1):
  0x00992E8C  at 005b1868  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x164 [dec 356]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1840 FUN_005b1840 size=485 ret=16 callers=0 callees=6

undefined4 FUN_005b1840(int param_1,int param_2,int param_3)

{
  byte bVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined1 *puVar6;
  int *piVar7;
  undefined4 extraout_var;
  undefined4 uVar8;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097c358;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar7 = &local_1c;
  puVar6 = local_28;
  uVar4 = (uint)*(ushort *)(param_2 + 6);
  uVar8 = extraout_var;
  FUN_006f0190(0x15);
  FUN_0071bcf0(uVar4,puVar6,piVar7);
  local_8 = 0xffffffff;
  FUN_00733780(uVar3,uVar8);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x164);
  while( true ) {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995ffc < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) break;
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995ffc) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       (((uVar2 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar2 &&
         (iVar5 = FUN_005e4e60(1), iVar5 == local_1c)) &&
        (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar3 = FUN_005e4e60(2), uVar3 == bVar1)))) {
      FUN_00544790(*local_18,local_18[1 = byte +0x4]);
      ExceptionList = local_10;
      return 0;
    }
    local_20 = local_20 + 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b14c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b14c0: 16-byte aligned: YES
  005b14c0  55             PUSH EBP
  005b14c1  8B EC          MOV EBP,ESP
  005b14c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b14c0
string literals (1):
  0x00992E8C  at 005b14e8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13A [dec 314]  x1  via EDX
```

## Card B0-11-51

GOG function: CSWSItemPropertyHandler::RemoveDamagePenalty  gog 0x007c75f0  callconv=__thiscall  param_size_bytes=16  gog_size=780  gog_retn=16
Candidate Steam address: 0x005b01e0 (size 780, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005afea0:STR0/CALLEE0/SIG0.515/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c72b0 CSWSItemPropertyHandler::ApplyDamagePenalty -> 0x005afea0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7900 CSWSItemPropertyHandler::ApplyDamageReduction -> 0x005b04f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 12 common): GOG {0x2*,0x5*,0x7*,0x8*,0x10*,0x14*,0x20*,0x4c*,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x5*,0x7*,0x8*,0x10*,0x14*,0x20*,0x4c*,0x148*,0x4000*,0x8000*,0x10000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005b01e0: 16-byte aligned: YES
  005b01e0  55             PUSH EBP
  005b01e1  8B EC          MOV EBP,ESP
  005b01e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b01e0
string literals (1):
  0x00992E8C  at 005b0209  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00606BD0  CSWSItem::GetDamageFlags
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13C [dec 316]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b01e0 FUN_005b01e0 size=780 ret=16 callers=0 callees=7

undefined4 FUN_005b01e0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953038;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar6 = &local_1c;
  puVar5 = local_28;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar3,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005b0299:
  while( true ) {
    while( true ) {
      if (local_14[1 = byte +0x4] <= (int)local_20) {
        ExceptionList = local_10;
        return 0;
      }
      if ((int)DAT_00995f54 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) {
        ExceptionList = local_10;
        return 0;
      }
      local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
      if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f54) &&
           ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
          (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((iVar4 = FUN_005e4e60(0), iVar4 == local_1c &&
          (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar2 = FUN_005e4e60(2), uVar2 == bVar1))))
      break;
      local_20 = local_20 + 1;
    }
    if (param_4 != 0x10) break;
    iVar4 = FUN_005e4e60(5);
    if (iVar4 == DAT_009961a4) goto LAB_005b0484;
    local_20 = local_20 + 1;
  }
  if (param_4 == 0x20) {
    iVar4 = FUN_005e4e60(5);
    if (iVar4 != DAT_009961a8) {
      local_20 = local_20 + 1;
      goto LAB_005b0299;
    }
  }
  else if (param_4 == 0x4000) {
    iVar4 = FUN_005e4e60(5);
    if (iVar4 != DAT_009961ac) {
      local_20 = local_20 + 1;
      goto LAB_005b0299;
    }
  }
  else if (param_4 == 0x8000) {
    iVar4 = FUN_005e4e60(5);
    if (iVar4 != DAT_009961b0) {
      local_20 = local_20 + 1;
      goto LAB_005b0299;
/* ... 31 more lines truncated by --max-lines */
```

### decoy: 0x005afea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005afea0: 16-byte aligned: YES
  005afea0  55             PUSH EBP
  005afea1  8B EC          MOV EBP,ESP
  005afea3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005afea0
string literals (1):
  0x00992E8C  at 005aff32  'Value'
direct callees (14):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00606BD0  CSWSItem::GetDamageFlags
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-11-52

GOG function: CSWSItemPropertyHandler::ApplyDecreaseAC  gog 0x007c8730  callconv=__thiscall  param_size_bytes=12  gog_size=414  gog_retn=20
Candidate Steam address: 0x005b1320 (size 414, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b16b0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8470 CSWSItemPropertyHandler::RemoveDecreaseAbilityScore -> 0x005b1060  gate_class=REVIEW
- GOG-order neighbour after:  0x007c88d0 CSWSItemPropertyHandler::RemoveDecreaseAC -> 0x005b14c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Value'
disp sets (* = in both, 7 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*,0x130*}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x5*,0x14*,0x38*,0x8c*,0xfffffff8*} | Steam {0x2*,0x4*,0x5*,0x14*,0x38*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b1320: 16-byte aligned: YES
  005b1320  55             PUSH EBP
  005b1321  8B EC          MOV EBP,ESP
  005b1323  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b1320
string literals (1):
  0x00992E8C  at 005b1348  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1320 FUN_005b1320 size=414 ret=20 callers=0 callees=9

undefined4
FUN_005b1320(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953133;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_18;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  local_18 = -local_18;
  if (local_18 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_48;
    local_34 = (undefined2)DAT_00995fe0;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,local_18);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    FUN_005e4e80(5,DAT_0099614c);
    FUN_00544210(local_14,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b16b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b16b0: 16-byte aligned: YES
  005b16b0  55             PUSH EBP
  005b16b1  8B EC          MOV EBP,ESP
  005b16b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b16b0
string literals (1):
  0x00992E8C  at 005b16d8  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-53

GOG function: CSWSItem::AddSubraceRequirements  gog 0x0066d900  callconv=(blank)  param_size_bytes=4  gog_size=1056  gog_retn=4
Candidate Steam address: 0x00609cb0 (size 1053, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::AddGenderRequirements
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.82 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0060a0d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0066d4e0 CSWSItem::AddGenderRequirements -> 0x00609cb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0066dd20 CSWSItem::AddPCRequirements -> 0x0060a4f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x11*,0x130*,0x28c*,0x2d4*} | Steam {0x2*,0x4*,0x11*,0x14,0x130*,0x28c*,0x2d4*}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3d,0xe0*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c,0xe0*,...(+2)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00609cb0: 16-byte aligned: YES
  00609cb0  55             PUSH EBP
  00609cb1  8B EC          MOV EBP,ESP
  00609cb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00609cb0
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042A0  CSWSItem::GetPassiveProperty
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071BCF0  C2DA::GetINTEntry_2
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via ECX
  +0x2 [dec 2]  x1  via ECX
  +0x11 [dec 17]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00609cb0 FUN_00609cb0 size=1053 ret=4 callers=1 callees=9

void __fastcall FUN_00609cb0(int param_1)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_94 [8];
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined1 local_64 [8];
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  uint local_3c;
  int local_38;
  undefined4 local_2c;
  undefined1 local_28 [8];
  int local_20;
  short *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009555d6;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,2);
  local_38 = 0;
  iVar2 = FUN_00603f60(0x3c,0);
  if (iVar2 != 0) {
    for (local_20 = 0; local_20 < *(int *)(param_1 + 0x28c); local_20 = local_20 + 1) {
      local_14 = (short *)FUN_006042a0(local_20);
      if ((*(byte *)((int)local_14 + 0x11) == 0xff) ||
         ((1 << (*(byte *)((int)local_14 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (*local_14 == 0x3c)) {
        local_3c = (uint)(ushort)local_14[1];
        if (local_38 == 0) {
          FUN_00733570(&DAT_00997608);
          local_8._0_1_ = 3;
          uVar3 = FUN_007408f0(local_44,0xbb7b);
          local_8._0_1_ = 4;
          FUN_00733df0(local_4c,uVar3);
          local_8._0_1_ = 5;
          uVar3 = FUN_00733df0(local_5c,local_54);
          local_8._0_1_ = 6;
          FUN_007337c0(uVar3);
          local_8._0_1_ = 5;
          FUN_00733780();
          local_8._0_1_ = 4;
          FUN_00733780();
          local_8._0_1_ = 3;
          FUN_00733780();
          local_8 = CONCAT31(local_8._1_3_,2);
          FUN_00733780();
          local_38 = 1;
        }
        FUN_00733570(&DAT_00997610);
        local_8._0_1_ = 7;
        FUN_0071bcf0(local_3c,local_64,&local_2c);
        local_8._0_1_ = 2;
        FUN_00733780();
        uVar3 = FUN_007408f0(local_6c,local_2c);
        local_8._0_1_ = 8;
        FUN_007337c0(uVar3);
/* ... 38 more lines truncated by --max-lines */
```

### decoy: 0x0060a0d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0060a0d0: 16-byte aligned: YES
  0060a0d0  55             PUSH EBP
  0060a0d1  8B EC          MOV EBP,ESP
  0060a0d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0060a0d0
string literals (0):
direct callees (9):
  0x00733540  CExoString::CExoString(empty)
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042A0  CSWSItem::GetPassiveProperty
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0071BCF0  C2DA::GetINTEntry_2
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via ECX
  +0x130 [dec 304]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via ECX
  +0x11 [dec 17]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

## Card B0-11-54

GOG function: CSWSItemPropertyHandler::RemoveEnhancementPenalty  gog 0x007c3300  callconv=__thiscall  param_size_bytes=16  gog_size=1261  gog_retn=16
Candidate Steam address: 0x005abef0 (size 1261, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ab980:STR0/CALLEE0/SIG0.515/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c2d90 CSWSItemPropertyHandler::ApplyEnhancementPenalty -> 0x005ab980  gate_class=REVIEW
- GOG-order neighbour after:  0x007c3800 CSWSItemPropertyHandler::ApplyDamageBonus -> 0x005ac3f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x0066a400 CSWSItem::GetDamageFlags -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 10 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 12 common): GOG {0x2*,0x5*,0x7*,0x8*,0x10*,0x14*,0x20*,0x68*,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x5*,0x7*,0x8*,0x10*,0x14*,0x20*,0x68*,0x148*,0x4000*,0x8000*,0x10000*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005abef0: 16-byte aligned: YES
  005abef0  55             PUSH EBP
  005abef1  8B EC          MOV EBP,ESP
  005abef3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005abef0
string literals (1):
  0x00992E8C  at 005abf19  'Value'
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x13C [dec 316]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005abef0 FUN_005abef0 size=1261 ret=16 callers=0 callees=7

undefined4 FUN_005abef0(int param_1,int param_2,int param_3,int param_4)

{
  byte bVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  undefined1 *puVar5;
  int *piVar6;
  undefined1 local_28 [8];
  uint local_20;
  int local_1c;
  undefined4 *local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952cd8;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar6 = &local_1c;
  puVar5 = local_28;
  uVar3 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar3,puVar5,piVar6);
  local_8 = 0xffffffff;
  FUN_00733780(uVar2);
  local_1c = -local_1c;
  local_14 = (int *)(param_3 + 0x148);
  local_20 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
LAB_005abfa9:
  do {
    if (local_14[1 = byte +0x4] <= (int)local_20) {
      ExceptionList = local_10;
      return 0;
    }
    if ((int)DAT_00995f54 < (int)(uint)*(ushort *)(*(int *)(*local_14 + local_20 * 4) + 8)) {
      ExceptionList = local_10;
      return 0;
    }
    local_18 = *(undefined4 **)(*local_14 + local_20 * 4);
    if ((((*(ushort *)(local_18 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f48) &&
         ((*(ushort *)((int)local_18 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (local_18[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((iVar4 = FUN_005e4e60(0), iVar4 == local_1c &&
        (bVar1 = *(byte *)(DAT_00a1b4d0 + 0x122), uVar2 = FUN_005e4e60(2), uVar2 == bVar1)))) {
      if (param_4 == 0x10) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996188) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x20) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_0099618c) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x4000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996190) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
      else if (param_4 == 0x8000) {
        iVar4 = FUN_005e4e60(1);
        if (iVar4 != DAT_00996194) {
          local_20 = local_20 + 1;
          goto LAB_005abfa9;
        }
      }
/* ... 86 more lines truncated by --max-lines */
```

### decoy: 0x005ab980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ab980: 16-byte aligned: YES
  005ab980  55             PUSH EBP
  005ab981  8B EC          MOV EBP,ESP
  005ab983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ab980
string literals (1):
  0x00992E8C  at 005aba70  'Value'
direct callees (12):
  0x006D5200  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 8):
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-55

GOG function: CSWSItemPropertyHandler::ApplyDamageReduction  gog 0x007c7900  callconv=__thiscall  param_size_bytes=12  gog_size=355  gog_retn=20
Candidate Steam address: 0x005b04f0 (size 355, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b0820:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c75f0 CSWSItemPropertyHandler::RemoveDamagePenalty -> 0x005b01e0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c7a70 CSWSItemPropertyHandler::RemoveDamageReduction -> 0x005b0660  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'Amount'
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x130*}
imm sets (* = in both, 6 common): GOG {0x4*,0x6*,0x14*,0x38*,0x8c*,0xfffffff8*} | Steam {0x4*,0x6*,0x14*,0x38*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b04f0: 16-byte aligned: YES
  005b04f0  55             PUSH EBP
  005b04f1  8B EC          MOV EBP,ESP
  005b04f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b04f0
string literals (1):
  0x0099626C  at 005b05a9  'Amount'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b04f0 FUN_005b04f0 size=355 ret=20 callers=0 callees=9

undefined4
FUN_005b04f0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  int local_48;
  undefined2 local_2c;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00953073;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_20 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = FUN_005e4920(1);
  }
  local_1c = local_48;
  local_8 = 0xffffffff;
  local_14 = local_48;
  local_2c = (undefined2)DAT_00995f4c;
  *(undefined2 *)(local_48 + 8) = local_2c;
  *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_00733570("Amount");
  local_8 = 1;
  puVar3 = &local_18;
  puVar2 = local_28;
  uVar1 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(6);
  FUN_0071bcf0(uVar1,puVar2,puVar3);
  local_8 = 0xffffffff;
  FUN_00733780();
  FUN_005e4e80(0,local_18);
  FUN_005e4e80(1,*(ushort *)(param_2 + 2) + 1);
  FUN_00544210(local_14,param_5,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b0820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b0820: 16-byte aligned: YES
  005b0820  55             PUSH EBP
  005b0821  8B EC          MOV EBP,ESP
  005b0823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b0820
string literals (1):
  0x0099626C  at 005b08d9  'Amount'
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
```

## Card B0-11-56

GOG function: CSWSItemPropertyHandler::ApplyLight  gog 0x007c4dc0  callconv=__thiscall  param_size_bytes=4  gog_size=276  gog_retn=20
Candidate Steam address: 0x005ad9b0 (size 276, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005adaf0:STR0/CALLEE0/SIG0.472/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4ad0 CSWSItemPropertyHandler::RemoveAttackPenalty -> 0x005ad6c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4ee0 CSWSItemPropertyHandler::RemoveLight -> 0x005adad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x14*} | Steam {0x8*,0xa*,0x14*}
imm sets (* = in both, 7 common): GOG {0x4*,0x14*,0x1d*,0x24*,0x8c*,0x1388*,0xfffffff8*} | Steam {0x4*,0x14*,0x1d*,0x24*,0x8c*,0x1388*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005ad9b0: 16-byte aligned: YES
  005ad9b0  55             PUSH EBP
  005ad9b1  8B EC          MOV EBP,ESP
  005ad9b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ad9b0
string literals (0):
direct callees (6):
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ad9b0 FUN_005ad9b0 size=276 ret=20 callers=0 callees=6

undefined4 FUN_005ad9b0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 in_stack_00000014;
  int local_34;
  undefined2 local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969c6b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_00603f60(0x1d,0);
  if (iVar2 != 0) {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_34 = 0;
    }
    else {
      local_34 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_24 = (undefined2)DAT_00995ff4;
    *(undefined2 *)(local_34 + 8) = local_24;
    *(ushort *)(local_34 + 0xa [dec 10]) = *(ushort *)(local_34 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,5000);
    FUN_00544210(local_34,in_stack_00000014,in_stack_00000014);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005adaf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adaf0: 16-byte aligned: YES
  005adaf0  55             PUSH EBP
  005adaf1  8B EC          MOV EBP,ESP
  005adaf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adaf0
string literals (1):
  0x00992E8C  at 005adb18  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-57

GOG function: CSWSItemPropertyHandler::ApplyTrueSeeing  gog 0x007c65c0  callconv=__thiscall  param_size_bytes=4  gog_size=222  gog_retn=20
Candidate Steam address: 0x005af1b0 (size 222, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af360:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c6430 CSWSItemPropertyHandler::RemoveSpecialWalk -> 0x005af020  gate_class=REVIEW
- GOG-order neighbour after:  0x007c66a0 CSWSItemPropertyHandler::RemoveTrueSeeing -> 0x005af290  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x14*} | Steam {0x8*,0xa*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x14*,0x20*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x20*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005af1b0: 16-byte aligned: YES
  005af1b0  55             PUSH EBP
  005af1b1  8B EC          MOV EBP,ESP
  005af1b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005af1b0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005af1b0 FUN_005af1b0 size=222 ret=20 callers=0 callees=4

undefined4 FUN_005af1b0(int param_1)

{
  int iVar1;
  undefined4 in_stack_00000014;
  int local_30;
  undefined2 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fa7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_30 = 0;
  }
  else {
    local_30 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_20 = (undefined2)DAT_0099603c;
  *(undefined2 *)(local_30 + 8) = local_20;
  *(ushort *)(local_30 + 0xa [dec 10]) = *(ushort *)(local_30 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_00544210(local_30,in_stack_00000014,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005af360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af360: 16-byte aligned: YES
  005af360  55             PUSH EBP
  005af361  8B EC          MOV EBP,ESP
  005af363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005af360
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x14 [dec 20]  x4  via EAX,ECX,EDX
  +0x122 [dec 290]  x4  via EAX,ECX,EDX
```

## Card B0-11-58

GOG function: CSWSItemPropertyHandler::RemoveDamageBonus  gog 0x007c3c80  callconv=__thiscall  param_size_bytes=16  gog_size=995  gog_retn=16
Candidate Steam address: 0x005ac870 (size 995, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ac3f0:STR0/CALLEE0/SIG0.565/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c3800 CSWSItemPropertyHandler::ApplyDamageBonus -> 0x005ac3f0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4070 CSWSItemPropertyHandler::ApplyAttackBonus -> 0x005acc60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x2*,0x4*,0x6*,0x8*,0x9*,0xa*,0x14*,0x18*,0x122*,0x142*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0x9*,0xa*,0x14*,0x18*,0x122*,0x142*,0x1198*}
imm sets (* = in both, 16 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0xb*,0xc*,0xd*,0x10*,0x20*,0x58*,0x148*,0x4000*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0x8*,0xb*,0xc*,0xd*,0x10*,0x20*,0x58*,0x148*,0x4000*,...(+2)}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005ac870: 16-byte aligned: YES
  005ac870  55             PUSH EBP
  005ac871  8B EC          MOV EBP,ESP
  005ac873  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005ac870
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x2 [dec 2]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x9 [dec 9]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x122 [dec 290]  x1  via ECX
  +0x142 [dec 322]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ac870 FUN_005ac870 size=995 ret=16 callers=0 callees=2

undefined4 FUN_005ac870(int param_1,short *param_2,int param_3,int param_4)

{
  byte bVar1;
  byte bVar2;
  ushort uVar3;
  ushort uVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  uint local_18;
  
  uVar3 = param_2[3];
  if (*param_2 == 0xb) {
    bVar1 = (byte)param_2[1];
  }
  else {
    bVar1 = *(byte *)((int)param_2 + 9);
  }
  local_18 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x142);
LAB_005ac8ee:
  while( true ) {
    while( true ) {
      if (*(int *)(param_3 + 0x14c) <= (int)local_18) {
        return 0;
      }
      if ((int)DAT_00995f50 <
          (int)(uint)*(ushort *)(*(int *)(*(int *)(param_3 + 0x148) + local_18 * 4) + 8)) {
        return 0;
      }
      puVar5 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_18 * 4);
      if ((((*(ushort *)(puVar5 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f50) &&
           ((*(ushort *)((int)puVar5 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
          (puVar5[6 = byte +0x18] == *(int *)(param_1 + 0x14))) && (uVar6 = FUN_005e4e60(0), uVar6 == uVar3))
      break;
      local_18 = local_18 + 1;
    }
    if (param_4 != 0x10) break;
    iVar7 = FUN_005e4e60(5);
    if (iVar7 == DAT_009961a4) goto LAB_005acab4;
    local_18 = local_18 + 1;
  }
  if (param_4 == 0x20) {
    iVar7 = FUN_005e4e60(5);
    if (iVar7 != DAT_009961a8) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
    }
  }
  else if (param_4 == 0x4000) {
    iVar7 = FUN_005e4e60(5);
    if (iVar7 != DAT_009961ac) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
    }
  }
  else if (param_4 == 0x8000) {
    iVar7 = FUN_005e4e60(5);
    if (iVar7 != DAT_009961b0) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
    }
  }
  else if (param_4 == 0x10000) {
    iVar7 = FUN_005e4e60(5);
    if (iVar7 != DAT_009961b4) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
    }
  }
  else {
    if (param_4 != 8) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
    }
    iVar7 = FUN_005e4e60(5);
    if (iVar7 != DAT_009961b8) {
      local_18 = local_18 + 1;
      goto LAB_005ac8ee;
/* ... 40 more lines truncated by --max-lines */
```

### decoy: 0x005ac3f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ac3f0: 16-byte aligned: YES
  005ac3f0  55             PUSH EBP
  005ac3f1  8B EC          MOV EBP,ESP
  005ac3f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ac3f0
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 9):
  +0x2 [dec 2]  x5  via EAX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xA [dec 10]  x2  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x9 [dec 9]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x34 [dec 52]  x1  via ECX
  +0x122 [dec 290]  x1  via EAX
```

## Card B0-11-59

GOG function: CSWSItemPropertyHandler::RemoveReducedSavingThrow  gog 0x007c5e70  callconv=__thiscall  param_size_bytes=12  gog_size=180  gog_retn=16
Candidate Steam address: 0x005aea60 (size 180, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ae9a0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c5db0 CSWSItemPropertyHandler::ApplyReducedSavingThrow -> 0x005ae9a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> 0x005aeb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00489130 CTwoDimArrays::GetIPRPCostTable -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x007c60c0 CSWSItemPropertyHandler::RemoveChangedSavingThrow -> Steam ?  in candidate callees: n/a
GOG strings (1): 'Value'
disp sets (* = in both, 2 common): GOG {0x6*,0x130*} | Steam {0x6*,0x130*}
imm sets (* = in both, 3 common): GOG {0x10*,0x14*,0x18*} | Steam {0x10*,0x14*,0x18*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005aea60: 16-byte aligned: YES
  005aea60  55             PUSH EBP
  005aea61  8B EC          MOV EBP,ESP
  005aea63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aea60
string literals (1):
  0x00992E8C  at 005aea88  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AECB0  CSWSItemPropertyHandler::RemoveChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005aea60 FUN_005aea60 size=180 ret=16 callers=0 callees=5

undefined4 FUN_005aea60(undefined4 param_1,int param_2,undefined4 param_3)

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aecb0(local_14,param_1,param_2,param_3);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005ae9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ae9a0: 16-byte aligned: YES
  005ae9a0  55             PUSH EBP
  005ae9a1  8B EC          MOV EBP,ESP
  005ae9a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ae9a0
string literals (1):
  0x00992E8C  at 005ae9c8  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AEB20  CSWSItemPropertyHandler::ApplyChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-11-60

GOG function: CSWSItemPropertyHandler::ApplyFreedomOfMovement  gog 0x007c6770  callconv=__thiscall  param_size_bytes=4  gog_size=942  gog_retn=20
Candidate Steam address: 0x005af360 (size 942, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005af1b0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c66a0 CSWSItemPropertyHandler::RemoveTrueSeeing -> 0x005af290  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6b20 CSWSItemPropertyHandler::RemoveFreedomOfMovement -> 0x005af710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xa*,0x14*,0x122*} | Steam {0x8*,0xa*,0x14*,0x122*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x14*,0x84*,0x8c*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x14*,0x84*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005af360: 16-byte aligned: YES
  005af360  55             PUSH EBP
  005af361  8B EC          MOV EBP,ESP
  005af363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005af360
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x14 [dec 20]  x4  via EAX,ECX,EDX
  +0x122 [dec 290]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005af360 FUN_005af360 size=942 ret=20 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005af360(int param_1)

{
  int iVar1;
  undefined4 in_stack_00000014;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  undefined2 local_74;
  undefined2 local_64;
  undefined2 local_54;
  undefined2 local_44;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952f6c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_88 = 0;
  }
  else {
    local_88 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_44 = (undefined2)DAT_00995f74;
  *(undefined2 *)(local_88 + 8) = local_44;
  *(ushort *)(local_88 + 0xa [dec 10]) = *(ushort *)(local_88 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_005e4e80(0,_DAT_009961d4);
  FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
  FUN_00544210(local_88,in_stack_00000014,in_stack_00000014);
  iVar1 = FUN_00919723(0x8c);
  local_8 = 1;
  if (iVar1 == 0) {
    local_8c = 0;
  }
  else {
    local_8c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_54 = (undefined2)DAT_00995f74;
  *(undefined2 *)(local_8c + 8) = local_54;
  *(ushort *)(local_8c + 0xa [dec 10]) = *(ushort *)(local_8c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_005e4e80(0,DAT_009961e0);
  FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
  FUN_00544210(local_8c,in_stack_00000014,in_stack_00000014);
  iVar1 = FUN_00919723(0x8c);
  local_8 = 2;
  if (iVar1 == 0) {
    local_90 = 0;
  }
  else {
    local_90 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_64 = (undefined2)DAT_00995f74;
  *(undefined2 *)(local_90 + 8) = local_64;
  *(ushort *)(local_90 + 0xa [dec 10]) = *(ushort *)(local_90 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_005e4e80(0,DAT_009961e4);
  FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
  FUN_00544210(local_90,in_stack_00000014,in_stack_00000014);
  iVar1 = FUN_00919723(0x8c);
  local_8 = 3;
  if (iVar1 == 0) {
    local_94 = 0;
  }
  else {
    local_94 = FUN_005e4920(1);
  }
/* ... 11 more lines truncated by --max-lines */
```

### decoy: 0x005af1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005af1b0: 16-byte aligned: YES
  005af1b0  55             PUSH EBP
  005af1b1  8B EC          MOV EBP,ESP
  005af1b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005af1b0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0xA [dec 10]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```
