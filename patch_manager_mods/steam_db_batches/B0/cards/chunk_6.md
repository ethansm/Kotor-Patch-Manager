# Review cards B0 chunk 6

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-6-01

GOG function: CItemRepository::RemoveItem  gog 0x006696b0  callconv=(blank)  param_size_bytes=4  gog_size=180  gog_retn=4
Candidate Steam address: 0x00605e80 (size 180, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00605cb0:STR0/CALLEE1/SIG0.286/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006695b0 CItemRepository::GetItemInRepository -> 0x00605d80  gate_class=REVIEW
- GOG-order neighbour after:  0x00669770 CItemRepository::ClearNewItems -> 0x00605f40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0xc*,0x10*,0x14*,0x2c8*} | Steam {0x4*,0xc*,0x10*,0x14*,0x2c8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x7*,0xc*,0x10*,0xffffff7f*} | Steam {0x4*,0x7*,0xc*,0x10*,0xffffff7f*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00605e80: 16-byte aligned: YES
  00605e80  55             PUSH EBP
  00605e81  8B EC          MOV EBP,ESP
  00605e83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00605e80
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 5):
  +0x2C8 [dec 712]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00605e80 FUN_00605e80 size=180 ret=4 callers=24 callees=1

undefined4 __thiscall FUN_00605e80(int param_1,int param_2)

{
  int local_8;
  
  if (param_2 != 0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x10); local_8 = local_8 + 1) {
      if (*(int *)(param_2 + 0x14) == *(int *)(*(int *)(param_1 + 0xc) + local_8 * 4)) {
        FUN_0049b840(local_8);
        if ((*(uint *)(param_2 + 0x2c8) >> 7 & 1) != 0) {
          *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
          *(uint *)(param_2 + 0x2c8) = *(uint *)(param_2 + 0x2c8) & 0xffffff7f;
        }
        return 1;
      }
    }
  }
  return 0;
}
```

### decoy: 0x00605cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00605cb0: 16-byte aligned: YES
  00605cb0  55             PUSH EBP
  00605cb1  8B EC          MOV EBP,ESP
  00605cb3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00605cb0
string literals (0):
direct callees (3):
  0x0051C0E0  (unnamed)
  0x007339D0  CExoString::operator==
  0x00605CB0  CItemRepository::FindItemWithTag
struct offsets touched (top 5):
  +0x2AC [dec 684]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-6-02

GOG function: CSWSCreatureStats::GetTotalDEXBonus  gog 0x006f6ec0  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006baa40 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006ba9f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6e70 CSWSCreatureStats::GetTotalSTRBonus -> 0x006ba9f0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f6f10 CSWSCreatureStats::GetTotalCONBonus -> 0x006baa90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0xff*} | Steam {0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006baa40: 16-byte aligned: YES
  006baa40  55             PUSH EBP
  006baa41  8B EC          MOV EBP,ESP
  006baa43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006baa40
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006baa40 FUN_006baa40 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006baa40(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,1,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006ba9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ba9f0: 16-byte aligned: YES
  006ba9f0  55             PUSH EBP
  006ba9f1  8B EC          MOV EBP,ESP
  006ba9f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006ba9f0
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-03

GOG function: CSWSCreatureStats::RemoveFeat  gog 0x006f4a70  callconv=(blank)  param_size_bytes=2  gog_size=172  gog_retn=4
Candidate Steam address: 0x006b85f0 (size 172, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b8370:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f4a00 CSWSCreatureStats::DecrementFeatRemainingUses -> 0x006b8580  gate_class=AUTO
- GOG-order neighbour after:  0x006f4b20 CSWSCreatureStats::AddKnownSpell -> 0x006b86a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004c75e0 unnamed -> Steam ?  in candidate callees: n/a
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x18*} | Steam {0x4*,0xc*,0x18*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006b85f0: 16-byte aligned: YES
  006b85f0  55             PUSH EBP
  006b85f1  8B EC          MOV EBP,ESP
  006b85f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b85f0
string literals (0):
direct callees (3):
  0x005AA100  (unnamed)
  0x0049B840  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006b85f0 FUN_006b85f0 size=172 ret=4 callers=1 callees=3

void __thiscall FUN_006b85f0(int param_1,short param_2)

{
  short *psVar1;
  short local_c;
  
  FUN_005aa100(param_2);
  if (0 < *(int *)(param_1 + 0x10)) {
    local_c = (short)*(undefined4 *)(param_1 + 0x10);
    do {
      local_c = local_c + -1;
      if (local_c < 0) {
        return;
      }
      psVar1 = *(short **)(*(int *)(param_1 + 0xc) + local_c * 4);
    } while ((*psVar1 != param_2) || (FUN_0049b840((int)local_c), psVar1 == (short *)0x0));
    FUN_009196fd(psVar1);
  }
  return;
}
```

### decoy: 0x006b8370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b8370: 16-byte aligned: YES
  006b8370  55             PUSH EBP
  006b8371  8B EC          MOV EBP,ESP
  006b8373  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b8370
string literals (0):
direct callees (3):
  0x005933F0  (unnamed)
  0x0049B840  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x3  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
```

## Card B0-6-04

GOG function: CSWSItem::GetUpgradableType  gog 0x0066af00  callconv=(blank)  param_size_bytes=0  gog_size=181  gog_retn=4
Candidate Steam address: 0x00607790 (size 721, retn 4)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::GetPropertyDescription
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.21 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006076d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0066aec0 CSWSItem::GetClientItem -> 0x00607690  gate_class=AUTO
- GOG-order neighbour after:  0x0066afc0 CSWSItem::GetPropertyDescription -> 0x00607790  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004ecc00 CSWItem::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xac*,0x2f0} | Steam {0x4,0x9,0xc,0xac*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3*,0x4*,0x5*,0xc,0x1f,0x26,0x27,0x28,0x29} | Steam {0x2*,0x3*,0x4*,0x5*,0x6,0x2d,0x2e,0x31,0x7c,0x2b0,0x2b8,0x7dac}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00607790: 16-byte aligned: YES
  00607790  55             PUSH EBP
  00607791  8B EC          MOV EBP,ESP
  00607793  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00607790
string literals (0):
direct callees (26):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x006D6E30  (unnamed)
  0x0060A990  CSWSItem::AddAttributeRequirements
  0x00609110  CSWSItem::AddFeatRequirements
  0x00609CB0  CSWSItem::AddGenderRequirements
  0x0060A0D0  CSWSItem::AddSubraceRequirements
  0x0060A4F0  CSWSItem::AddPCRequirements
  0x0060B360  CSWSItem::AddCNPCRequirements
  0x0060B6B0  CSWSItem::AddDamageProperties
  0x0060D8E0  CSWSItem::AddRangeProperties
  0x0060E6B0  CSWSItem::AddCriticalThreatProperties
  0x0060EA50  CSWSItem::AddOnHitProperties
  0x0060FDD0  CSWSItem::AddWeaponSizeProperties
  0x0060DA50  CSWSItem::AddAttackModifierProperties
  0x0060FEE0  CSWSItem::AddDefenceProperties
  0x00610FC0  CSWSItem::AddMiscellaneousProperties
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x00733A60  CExoString::operator==_2
struct offsets touched (top 4):
  +0xAC [dec 172]  x3  via EAX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x9 [dec 9]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00607790 FUN_00607790 size=721 ret=4 callers=9 callees=26

/* WARNING: Removing unreachable block (ram,0x00607957) */

undefined4 __thiscall FUN_00607790(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 *puVar4;
  undefined4 uVar5;
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [8];
  undefined1 local_24 [20];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00955021;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040def0(0);
  local_8 = 1;
  FUN_00733540(uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  iVar2 = FUN_006d6e30();
  if (((*(char *)(iVar2 + 0xac) != '.') &&
      (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) != '\x06')) &&
     (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) != '1')) {
    FUN_0060a990(local_2c);
    FUN_00609110(local_2c);
    FUN_00609cb0(local_2c);
    FUN_0060a0d0(local_2c);
    FUN_0060a4f0(local_2c);
    FUN_0060b360(local_2c);
    iVar2 = FUN_006d6e30();
    if ((*(char *)(iVar2 + 9) != '\0') || (*(int *)(param_1 + 0xc) == 0x2d)) {
      FUN_0060b6b0(local_2c);
      FUN_0060d8e0(local_2c);
      if (*(int *)(param_1 + 0xc) != 0x2d) {
        FUN_0060e6b0(local_2c);
      }
      FUN_0060ea50(local_2c);
      FUN_0060fdd0(local_2c);
    }
    FUN_0060da50(local_2c);
    FUN_0060fee0(local_2c);
    FUN_00610fc0(local_2c);
  }
  FUN_00733540(uVar1);
  local_8._0_1_ = 3;
  uVar5 = 0;
  puVar4 = local_24;
  uVar3 = FUN_0073f350(puVar4,0);
  FUN_007356b0(uVar3,puVar4,uVar5);
  iVar2 = FUN_00733a60(&DAT_00986925);
  if (iVar2 != 0) {
    uVar3 = FUN_007408f0(local_34,0x7dac);
    local_8._0_1_ = 4;
    FUN_007337c0(uVar3);
    local_8._0_1_ = 3;
    FUN_00733780();
  }
  uVar3 = FUN_00733df0(local_3c,local_24);
  local_8._0_1_ = 5;
  FUN_007337c0(uVar3);
  local_8._0_1_ = 3;
  FUN_00733780();
  FUN_007335f0(local_24);
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0040df30();
  ExceptionList = local_10;
  return param_2;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x006076d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006076d0: 16-byte aligned: YES
  006076d0  55             PUSH EBP
  006076d1  8B EC          MOV EBP,ESP
  006076d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006076d0
string literals (0):
direct callees (1):
  0x006D6E30  (unnamed)
struct offsets touched (top 2):
  +0xAC [dec 172]  x1  via ECX
  +0x2F0 [dec 752]  x1  via EAX
```

## Card B0-6-05

GOG function: CItemRepository::ClearItems  gog 0x00669a30  callconv=(blank)  param_size_bytes=0  gog_size=34  gog_retn=0
Candidate Steam address: 0x00606200 (size 34, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006061a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006699d0 CItemRepository::CalculateContentsWeight -> 0x006061a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00669a60 CItemRepository::SetParentId -> 0x00606230  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005ac200 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00606200: 16-byte aligned: YES
  00606200  55             PUSH EBP
  00606201  8B EC          MOV EBP,ESP
  00606203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00606200
string literals (0):
direct callees (1):
  0x006169E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00606200 FUN_00606200 size=34 ret=0 callers=5 callees=1

void __fastcall FUN_00606200(int param_1)

{
  FUN_006169e0(0);
  *(undefined4 *)(param_1 + 4) = 0;
  return;
}
```

### decoy: 0x006061a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006061a0: 16-byte aligned: YES
  006061a0  55             PUSH EBP
  006061a1  8B EC          MOV EBP,ESP
  006061a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006061a0
string literals (0):
direct callees (2):
  0x00606100  CItemRepository::ItemListGetItem
  0x00606B50  CSWSItem::GetWeight
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

## Card B0-6-06

GOG function: CSWSCreatureStats::GetAttacksPerRound  gog 0x006e35f0  callconv=(blank)  param_size_bytes=0  gog_size=23  gog_retn=0
Candidate Steam address: 0x006a7170 (size 23, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006a6f80:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e3400 CSWSCreatureStats::GetBaseAttackBonus -> 0x006a6f80  gate_class=AUTO
- GOG-order neighbour after:  0x006e3610 CSWSCreatureStats::UpdateCombatInformation -> 0x006a7190  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x006a7170: 16-byte aligned: YES
  006a7170  55             PUSH EBP
  006a7171  8B EC          MOV EBP,ESP
  006a7173  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006a7170
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006a7170 FUN_006a7170 size=23 ret=0 callers=2 callees=0

undefined1 FUN_006a7170(void)

{
  return 1;
}
```

### decoy: 0x006a6f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a6f80: 16-byte aligned: YES
  006a6f80  55             PUSH EBP
  006a6f81  8B EC          MOV EBP,ESP
  006a6f83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006a6f80
string literals (1):
  0x009993EC  at 006a70a9  'ToHitMult'
direct callees (6):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F08F0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
struct offsets touched (top 9):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via EAX,ECX
  +0x106 [dec 262]  x2  via EAX,ECX
  +0x251 [dec 593]  x2  via EAX,EDX
  +0x124 [dec 292]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
  +0x154 [dec 340]  x1  via EDX
  +0x250 [dec 592]  x1  via ECX
  +0x11AC [dec 4524]  x1  via EAX
```

## Card B0-6-07

GOG function: CSWSCreature::GetClientCreature  gog 0x006a3970  callconv=__thiscall  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0058ae90 (size 53, retn 0)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Creature::GetServerObjectVt2c
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (GetClientObject 0x54a840)
- best decoy (neighbour/other candidate) scores: 0x0058ad80:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3860 CSWSCreature::GetForceItemMask  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006a39b0 CSWSCreature::AddPazaakCard  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2c*} | Steam {0x2c*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058ae90: 16-byte aligned: YES
  0058ae90  55             PUSH EBP
  0058ae91  8B EC          MOV EBP,ESP
  0058ae93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058ae90
string literals (0):
direct callees (1):
  0x0054A840  CSWSObject::GetClientObject
struct offsets touched (top 1):
  +0x2C [dec 44]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0058ae90 FUN_0058ae90 size=53 ret=0 callers=34 callees=1

undefined4 FUN_0058ae90(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_0054a840();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840();
    uVar3 = (**(code **)(*piVar2 + 0x2c))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x0058ad80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058ad80: 16-byte aligned: YES
  0058ad80  55             PUSH EBP
  0058ad81  8B EC          MOV EBP,ESP
  0058ad83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058ad80
string literals (0):
direct callees (2):
  0x006D0620  (unnamed)
  0x006D6E30  (unnamed)
struct offsets touched (top 2):
  +0xAC [dec 172]  x1  via ECX
  +0x1150 [dec 4432]  x1  via ECX
```

## Card B0-6-08

GOG function: CSWSItem::PowerItem  gog 0x00679380  callconv=(blank)  param_size_bytes=4  gog_size=50  gog_retn=12
Candidate Steam address: 0x00615b50 (size 50, retn 12)
Candidate sources: branch+name_map:high:gog_item_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00615b90:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00679230 CSWSItem::GetPropertyChanceDurationDC -> 0x00615a00  gate_class=AUTO
- GOG-order neighbour after:  0x006793c0 CSWSItem::GetExistsHigherValuedProperty -> 0x00615b90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0047f970 CSWCItem::PowerItem -> Steam ?  in candidate callees: n/a
  0x0066aec0 CSWSItem::GetClientItem -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x00615b50: 16-byte aligned: YES
  00615b50  55             PUSH EBP
  00615b51  8B EC          MOV EBP,ESP
  00615b53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00615b50
string literals (0):
direct callees (2):
  0x00607690  CSWSItem::GetClientItem
  0x007FE6C0  CSWCItem::PowerItem
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00615b50 FUN_00615b50 size=50 ret=12 callers=1 callees=2

void __thiscall FUN_00615b50(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_00607690(param_1);
  if (iVar1 != 0) {
    FUN_007fe6c0(param_2,0,1,0);
  }
  return;
}
```

### decoy: 0x00615b90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00615b90: 16-byte aligned: YES
  00615b90  55             PUSH EBP
  00615b91  8B EC          MOV EBP,ESP
  00615b93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00615b90
string literals (0):
direct callees (1):
  0x006042A0  CSWSItem::GetPassiveProperty
struct offsets touched (top 5):
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x11 [dec 17]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

## Card B0-6-09

GOG function: CSWSCreatureStats::GetTotalWISBonus  gog 0x006f6fb0  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006bab30 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006baa40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6f60 CSWSCreatureStats::GetTotalINTBonus -> 0x006baae0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f7000 CSWSCreatureStats::GetTotalCHABonus -> 0x006bab80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0xff*} | Steam {0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006bab30: 16-byte aligned: YES
  006bab30  55             PUSH EBP
  006bab31  8B EC          MOV EBP,ESP
  006bab33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bab30
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006bab30 FUN_006bab30 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006bab30(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,4,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006baa40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006baa40: 16-byte aligned: YES
  006baa40  55             PUSH EBP
  006baa41  8B EC          MOV EBP,ESP
  006baa43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006baa40
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-10

GOG function: CSWSCreatureStats::GetCreatureTalentRandom  gog 0x006f5030  callconv=(blank)  param_size_bytes=44  gog_size=487  gog_retn=44
Candidate Steam address: 0x006b8bb0 (size 487, retn 44)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006b8da0:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f4cc0 CSWSCreatureStats::GetCreatureHasTalent -> 0x006b8840  gate_class=AUTO
- GOG-order neighbour after:  0x006f5220 CSWSCreatureStats::DecrementFeatRemainingUses_2 -> 0x006b8da0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006f7280 CSWSCreatureStats::SetArcaneSpellFailure -> Steam ?  in candidate callees: n/a
  0x006f7340 CSWSCreatureStats::GetTotalDamageRange -> Steam ?  in candidate callees: n/a
  0x006f7820 CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem -> Steam ?  in candidate callees: n/a
  0x006f8560 CSWSCreatureStats::GetCreatureTalentRandomFeat -> Steam ?  in candidate callees: n/a
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x20*,0x2c*,0x7f000000*,0x80000001*,0xfffffffe*} | Steam {0x20*,0x2c*,0x7f000000*,0x80000001*,0xfffffffe*}

### abi
```
GOG param bytes: 44; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(44/4)*4 = 44 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=44 ; asm RET = 44
Stack Slot Match: TRUE (GOG 44 bytes -> x86 slots 44; ASM RET 44)
```

### prologue
```
prologue for 0x006b8bb0: 16-byte aligned: YES
  006b8bb0  55             PUSH EBP
  006b8bb1  8B EC          MOV EBP,ESP
  006b8bb3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b8bb0
string literals (0):
direct callees (5):
  0x0091EB8B  (unnamed)
  0x006BAEC0  CSWSCreatureStats::GetTotalDamageRange
  0x006BAE00  CSWSCreatureStats::SetArcaneSpellFailure
  0x006BB3A0  CSWSCreatureStats::GetCreatureTalentRandomPowerFromItem
  0x006BC0E0  CSWSCreatureStats::GetCreatureTalentRandomFeat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006b8bb0 FUN_006b8bb0 size=487 ret=44 callers=1 callees=5

int FUN_006b8bb0(undefined4 param_1,undefined4 param_2,undefined4 *param_3,undefined4 *param_4,
                undefined1 *param_5,undefined4 *param_6,undefined4 *param_7,undefined1 *param_8,
                undefined4 param_9,int param_10,int param_11)

{
  bool bVar1;
  bool bVar2;
  uint uVar3;
  int iVar4;
  int local_24;
  int local_20;
  int local_10;
  
  *param_3 = 0xffffffff;
  *param_4 = 0xffffffff;
  *param_5 = 0xff;
  *param_6 = 0x7f000000;
  *param_7 = 0xffffffff;
  *param_8 = 0xff;
  local_10 = 0;
  bVar2 = false;
  bVar1 = false;
  while( true ) {
    if (local_10 != 0) {
      return local_10;
    }
    if ((bVar1) && (bVar2)) break;
    uVar3 = _rand [GHIDRA-AUTO-LABEL: unreliable]();
    uVar3 = uVar3 & 0x80000001;
    if ((int)uVar3 < 0) {
      uVar3 = (uVar3 - 1 | 0xfffffffe) + 1;
    }
    if (uVar3 == 0) {
      if (!bVar1) {
        if ((param_10 == 0) && (param_11 == -1)) {
          bVar1 = true;
        }
        if (param_10 == 0) {
          local_20 = param_11;
        }
        else {
          local_20 = -1;
        }
        iVar4 = FUN_006baec0(param_1,param_2,param_3,param_4,param_5,param_8,param_9,local_20);
        if (((iVar4 == 0) &&
            (iVar4 = FUN_006bae00(param_1,param_2,param_3,param_4,param_5,param_9,local_20),
            iVar4 == 0)) &&
           (iVar4 = FUN_006bb3a0(param_1,param_2,param_3,param_4,param_5,param_6,param_7,param_9,
                                 local_20), iVar4 == 0)) {
          bVar1 = true;
        }
        else {
          local_10 = 1;
        }
      }
    }
    else if ((uVar3 == 1) && (!bVar2)) {
      if ((param_10 == 1) && (param_11 == -1)) {
        bVar1 = true;
      }
      if (param_10 == 1) {
        local_24 = param_11;
      }
      else {
        local_24 = -1;
      }
      iVar4 = FUN_006bc0e0(param_1,param_2,param_3,param_4,param_5,param_9,local_24);
      if (iVar4 == 0) {
        bVar2 = true;
      }
      else {
        local_10 = 1;
      }
    }
  }
  *param_3 = 0xffffffff;
  *param_4 = 0xffffffff;
  return 0;
}
```

### decoy: 0x006b8da0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b8da0: 16-byte aligned: YES
  006b8da0  55             PUSH EBP
  006b8da1  8B EC          MOV EBP,ESP
  006b8da3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b8da0
string literals (0):
direct callees (4):
  0x006B5DB0  CSWSCreatureStats::GetDEXMod
  0x00615C80  CSWSItem::IsLightsaber
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x006D6E30  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via ECX
  +0xEE [dec 238]  x1  via EAX
```

## Card B0-6-11

GOG function: CSWSObject::GetAcceptableAction  gog 0x006b3380  callconv=(blank)  param_size_bytes=4  gog_size=62  gog_retn=4
Candidate Steam address: 0x005d7190 (size 62, retn 4)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005d7160:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b18a0 CSWSObject::RunActions  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006b3410 CSWSObject::GetAQActionIDByID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x46*} | Steam {0x4*,0x8*,0x46*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005d7190: 16-byte aligned: YES
  005d7190  55             PUSH EBP
  005d7191  8B EC          MOV EBP,ESP
  005d7193  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d7190
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5D71D8 [dec 6123992]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005d7190 FUN_005d7190 size=62 ret=4 callers=2 callees=0

undefined4 FUN_005d7190(undefined4 param_1)

{
  undefined4 uVar1;
  
  switch(param_1) {
  case 1:
  case 7:
  case 9:
  case 0xc:
  case 0xf:
  case 0x14:
  case 0x15:
  case 0x18:
  case 0x19:
  case 0x1a:
  case 0x1b:
  case 0x1c:
  case 0x1d:
  case 0x1e:
  case 0x26:
  case 0x27:
  case 0x28:
  case 0x29:
  case 0x2a:
  case 0x2b:
  case 0x2e:
  case 0x32:
  case 0x36:
  case 0x37:
  case 0x38:
  case 0x3d:
  case 0x3f:
  case 0x43:
  case 0x44:
  case 0x45:
  case 0x46:
  case 0x47:
    uVar1 = 1;
    break;
  default:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x005d7160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d7160: 16-byte aligned: YES
  005d7160  55             PUSH EBP
  005d7161  8B EC          MOV EBP,ESP
  005d7163  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d7160
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-6-12

GOG function: CSWSItem::non_virtual_thunk_to_dtor_CSWSItem  gog 0x0067a210  callconv=(blank)  param_size_bytes=0  gog_size=8  gog_retn=None
Candidate Steam address: 0x00616a60 (size 8, retn 4)  THUNK -> 0x006000b0 (everything below is judged on the target)
Candidate sources: branch+name_map:high:gog_item_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: 0x006000b0 | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006000e0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: retn_meta_mismatch
- GOG-order neighbour before: 0x00679820 CSWSItem::GetPropertiesOnlyDescription -> 0x00615ff0  gate_class=AUTO
- GOG-order neighbour after:  0x0067a220 CSWInventory::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006638e0 CSWSItem::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x10} | Steam {0x4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x006000b0: 16-byte aligned: YES
  006000b0  55             PUSH EBP
  006000b1  8B EC          MOV EBP,ESP
  006000b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006000b0
string literals (0):
direct callees (2):
  0x006000E0  CSWSItem::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006000b0 FUN_006000b0 size=44 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_006000b0(undefined4 param_1,uint param_2)

{
  FUN_006000e0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x006000e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006000e0: 16-byte aligned: YES
  006000e0  55             PUSH EBP
  006000e1  8B EC          MOV EBP,ESP
  006000e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006000e0
string literals (0):
direct callees (15):
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x00605000  CItemRepository::Destructor
  0x009196FD  operator delete
  0x005453C0  CSWSObject::GetArea
  0x006037F0  CSWSItem::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x00735360  CExoLocString::Destructor
  0x0053F050  CSWSObject::Destructor_2
  0x006D6E10  (unnamed)
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via ECX
  +0x290 [dec 656]  x3  via EAX,ECX
  +0x294 [dec 660]  x3  via EAX,EDX
  +0x2AC [dec 684]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
```

## Card B0-6-13

GOG function: CExoResMan::AddEncapsulatedResourceFile  gog 0x0061af20  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x00711270 (size 50, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CRes::GetResRef
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711250:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061ae30 CExoResMan::Destructor -> 0x007114d0  gate_class=AUTO
- GOG-order neighbour after:  0x0061af40 CExoResMan::AddResourceImageFile -> 0x007115e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x14,0x1a}
imm sets (* = in both, 1 common): GOG {0x3,0x8*} | Steam {0x8*,0x14}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00711270: 16-byte aligned: YES
  00711270  55             PUSH EBP
  00711271  8B EC          MOV EBP,ESP
  00711273  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711270
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 2):
  +0x14 [dec 20]  x2  via EAX
  +0x1A [dec 26]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00711270 FUN_00711270 size=50 ret=8 callers=2 callees=1

void __thiscall FUN_00711270(int param_1,undefined4 param_2,undefined2 *param_3)

{
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,*(undefined4 *)(param_1 + 0x14));
  *param_3 = *(undefined2 *)(*(int *)(param_1 + 0x14) + 0x1a);
  return;
}
```

### decoy: 0x00711250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711250: 16-byte aligned: YES
  00711250  55             PUSH EBP
  00711251  8B EC          MOV EBP,ESP
  00711253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711250
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-6-14

GOG function: CSWSCreatureStats::GetTotalSTRBonus  gog 0x006f6e70  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006ba9f0 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006baa40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6c50 CSWSCreatureStats::GetFeat -> 0x006ba7d0  gate_class=AUTO
- GOG-order neighbour after:  0x006f6ec0 CSWSCreatureStats::GetTotalDEXBonus -> 0x006baa40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0xff*} | Steam {0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006ba9f0: 16-byte aligned: YES
  006ba9f0  55             PUSH EBP
  006ba9f1  8B EC          MOV EBP,ESP
  006ba9f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006ba9f0
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006ba9f0 FUN_006ba9f0 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006ba9f0(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,0,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006baa40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006baa40: 16-byte aligned: YES
  006baa40  55             PUSH EBP
  006baa41  8B EC          MOV EBP,ESP
  006baa43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006baa40
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-15

GOG function: CExoResMan::Exists_2  gog 0x0061b890  callconv=(blank)  param_size_bytes=12  gog_size=150  gog_retn=12
Candidate Steam address: 0x00711ed0 (size 83, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoResMan::Exists
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711f30:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061b830 CExoResMan::Exists -> 0x00711ed0  gate_class=AUTO
- GOG-order neighbour after:  0x0061b940 CExoResMan::Free -> 0x00711fe0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061bc20 CExoResMan::GetKeyEntryFromTable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1c}
imm sets (* = in both, 2 common): GOG {0x3,0xc*,0x10*,0x14,0x18,0x1c} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00711ed0: 16-byte aligned: YES
  00711ed0  55             PUSH EBP
  00711ed1  8B EC          MOV EBP,ESP
  00711ed3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711ed0
string literals (0):
direct callees (1):
  0x00712360  CExoResMan::GetKeyEntry
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00711ed0 FUN_00711ed0 size=83 ret=12 callers=33 callees=1

/* WARNING: Type propagation algorithm not settling */

int FUN_00711ed0(undefined4 param_1,undefined2 param_2,undefined4 *param_3)

{
  int iVar1;
  int local_10[3 = byte +0xc];
  
  local_10[0] = 0;
  local_10[1 = byte +0x4] = 0;
  iVar1 = FUN_00712360(param_1,param_2,local_10,local_10 + 0x1 [dec 1] {int[]: byte +0x4});
  if ((iVar1 != 0) && (param_3 != (undefined4 *)0x0)) {
    *param_3 = *(undefined4 *)(local_10[0] + 0x1c);
  }
  return iVar1;
}
```

### decoy: 0x00711f30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711f30: 16-byte aligned: YES
  00711f30  55             PUSH EBP
  00711f31  8B EC          MOV EBP,ESP
  00711f33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711f30
string literals (0):
direct callees (1):
  0x007122C0  CExoResMan::GetKeyEntryFromTable
struct offsets touched (top 0):
```

## Card B0-6-16

GOG function: CSWSCreatureStats::GetCreatureTalentRandomPower  gog 0x006f5a80  callconv=(blank)  param_size_bytes=5  gog_size=62  gog_retn=8
Candidate Steam address: 0x006b9600 (size 62, retn 8)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006b9870:STR0/CALLEE0/SIG0.219/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f5630 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility -> 0x006b91b0  gate_class=AUTO
- GOG-order neighbour after:  0x006f5ac0 CSWSCreatureStats::ValidateLevelUp -> 0x006b9640  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006e0580 CSWSCreatureStats_ClassInfo::GetSpellGain -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8d*,0x90*} | Steam {0x8d*,0x90*}
imm sets (* = in both, 2 common): GOG {0x8*,0x28*} | Steam {0x8*,0x28*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006b9600: 16-byte aligned: YES
  006b9600  55             PUSH EBP
  006b9601  8B EC          MOV EBP,ESP
  006b9603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b9600
string literals (0):
direct callees (1):
  0x006A40D0  (unnamed)
struct offsets touched (top 1):
  +0x8D [dec 141]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b9600 FUN_006b9600 size=62 ret=8 callers=3 callees=1

undefined4 __thiscall FUN_006b9600(int param_1,byte param_2,undefined1 param_3)

{
  undefined4 uVar1;
  
  if (param_2 < *(byte *)(param_1 + 0x8d)) {
    uVar1 = FUN_006a40d0(param_3);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x006b9870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b9870: 16-byte aligned: YES
  006b9870  55             PUSH EBP
  006b9871  8B EC          MOV EBP,ESP
  006b9873  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b9870
string literals (0):
direct callees (18):
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x0083EA60  CExoArrayList<uint32>::Add
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006B6A20  CSWSCreatureStats::SetClassLevel
  0x006B69E0  CSWSCreatureStats::SetClass
  0x006B67D0  CSWSCreatureStats::SetSTRBase
  0x006B6810  CSWSCreatureStats::SetDEXBase
  0x006B6850  CSWSCreatureStats::SetCONBase
  0x006B6920  CSWSCreatureStats::SetINTBase
  0x006B6960  CSWSCreatureStats::SetWISBase
  0x006B69A0  CSWSCreatureStats::SetCHABase
  0x006F5AC0  PendingLevelUp::GetSkillDelta
  0x006B7BB0  CSWSCreatureStats::GetSkillRank
  0x006B81C0  CSWSCreatureStats::SetSkillRank
  0x006B8280  CSWSCreatureStats::AddFeat
  0x006B8760  CSWSCreatureStats::RemoveKnownSpell
  0x006B86A0  CSWSCreatureStats::AddKnownSpell
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 10):
  +0x24 [dec 36]  x7  via EAX,ECX,EDX
  +0x8D [dec 141]  x5  via EAX,ECX,EDX
  +0x130 [dec 304]  x3  via ECX,EDX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EDX
  +0xE4 [dec 228]  x2  via EAX,ECX
  +0x12E [dec 302]  x2  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
```

## Card B0-6-17

GOG function: CSWSCreatureStats::GetNumberKnownSpells  gog 0x006f4c80  callconv=(blank)  param_size_bytes=5  gog_size=62  gog_retn=8
Candidate Steam address: 0x006b8800 (size 62, retn 8)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006b87b0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f4c30 CSWSCreatureStats::GetKnownSpell -> 0x006b87b0  gate_class=AUTO
- GOG-order neighbour after:  0x006f4cc0 CSWSCreatureStats::GetCreatureHasTalent -> 0x006b8840  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006e0640 CSWSCreatureStats_ClassInfo::GetNumberKnownSpells -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8d*,0x90*} | Steam {0x8d*,0x90*}
imm sets (* = in both, 2 common): GOG {0x8*,0x28*} | Steam {0x8*,0x28*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006b8800: 16-byte aligned: YES
  006b8800  55             PUSH EBP
  006b8801  8B EC          MOV EBP,ESP
  006b8803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b8800
string literals (0):
direct callees (1):
  0x006A4190  (unnamed)
struct offsets touched (top 1):
  +0x8D [dec 141]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b8800 FUN_006b8800 size=62 ret=8 callers=4 callees=1

undefined4 __thiscall FUN_006b8800(int param_1,byte param_2,undefined1 param_3)

{
  undefined4 uVar1;
  
  if (param_2 < *(byte *)(param_1 + 0x8d)) {
    uVar1 = FUN_006a4190(param_3);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x006b87b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b87b0: 16-byte aligned: YES
  006b87b0  55             PUSH EBP
  006b87b1  8B EC          MOV EBP,ESP
  006b87b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b87b0
string literals (0):
direct callees (1):
  0x006A4080  (unnamed)
struct offsets touched (top 1):
  +0x8D [dec 141]  x1  via ECX
```

## Card B0-6-18

GOG function: CItemRepository::Destructor  gog 0x00668830  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00605000 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00604fb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006687e0 CItemRepository::Constructor -> 0x00604fb0  gate_class=AUTO
- GOG-order neighbour after:  0x00668850 CItemRepository::InsertItem -> 0x00605020  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00605000: 16-byte aligned: YES
  00605000  55             PUSH EBP
  00605001  8B EC          MOV EBP,ESP
  00605003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00605000
string literals (0):
direct callees (1):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00605000 FUN_00605000 size=22 ret=0 callers=7 callees=1
THUNK -> 0x0064DDB0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00605000(undefined4 param_1)

{
  Free(param_1);
  return;
}
```

### decoy: 0x00604fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00604fb0: 16-byte aligned: YES
  00604fb0  55             PUSH EBP
  00604fb1  8B EC          MOV EBP,ESP
  00604fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00604fb0
string literals (0):
direct callees (1):
  0x0084A640  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-6-19

GOG function: CSWSCreatureStats::GetWeaponSpecialization  gog 0x006e5390  callconv=(blank)  param_size_bytes=0  gog_size=202  gog_retn=0
Candidate Steam address: 0x006a8f10 (size 202, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006a8fe0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e5320 CSWSCreatureStats::GetWeaponFinesse -> 0x006a8ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x006e5460 CSWSCreatureStats::GetUseMonkAttackTables -> 0x006a8fe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068f220 CSWSCreature::GetUseMonkAbilities -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x24*,0x8d*,0x53c*} | Steam {0x24*,0x8d*,0x53c*}
imm sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xc*,0x10*,0x20*} | Steam {0x2*,0x4*,0x8*,0xc*,0x10*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006a8f10: 16-byte aligned: YES
  006a8f10  55             PUSH EBP
  006a8f11  8B EC          MOV EBP,ESP
  006a8f13  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006a8f10
string literals (0):
direct callees (1):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via ECX
  +0x53C [dec 1340]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006a8f10 FUN_006a8f10 size=202 ret=0 callers=4 callees=1

/* WARNING: Removing unreachable block (ram,0x006a8fab) */
/* WARNING: Removing unreachable block (ram,0x006a8fb1) */
/* WARNING: Removing unreachable block (ram,0x006a8f8d) */
/* WARNING: Removing unreachable block (ram,0x006a8f93) */
/* WARNING: Removing unreachable block (ram,0x006a8f9c) */
/* WARNING: Removing unreachable block (ram,0x006a8fa2) */
/* WARNING: Removing unreachable block (ram,0x006a8fba) */

undefined4 __fastcall FUN_006a8f10(int param_1)

{
  int iVar1;
  int local_8;
  
  iVar1 = FUN_005766f0();
  if (iVar1 == 1) {
    for (local_8 = 0; local_8 < (int)(uint)*(byte *)(param_1 + 0x8d); local_8 = local_8 + 1) {
    }
  }
  return 1;
}
```

### decoy: 0x006a8fe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a8fe0: 16-byte aligned: YES
  006a8fe0  55             PUSH EBP
  006a8fe1  8B EC          MOV EBP,ESP
  006a8fe3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006a8fe0
string literals (0):
direct callees (1):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via ECX
  +0x53C [dec 1340]  x1  via EDX
```

## Card B0-6-20

GOG function: CItemRepository::CalculateContentsWeight  gog 0x006699d0  callconv=(blank)  param_size_bytes=0  gog_size=95  gog_retn=0
Candidate Steam address: 0x006061a0 (size 95, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00606160:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00669990 CItemRepository::ItemListGetItemObjectID -> 0x00606160  gate_class=AUTO
- GOG-order neighbour after:  0x00669a30 CItemRepository::ClearItems -> 0x00606200  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00669930 CItemRepository::ItemListGetItem -> Steam ?  in candidate callees: n/a
  0x0066a380 CSWSItem::GetWeight -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x006061a0: 16-byte aligned: YES
  006061a0  55             PUSH EBP
  006061a1  8B EC          MOV EBP,ESP
  006061a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006061a0
string literals (0):
direct callees (2):
  0x00606100  CItemRepository::ItemListGetItem
  0x00606B50  CSWSItem::GetWeight
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006061a0 FUN_006061a0 size=95 ret=0 callers=1 callees=2

int __fastcall FUN_006061a0(int param_1)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_8;
  
  local_10 = 0;
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x10); local_8 = local_8 + 1) {
    iVar1 = FUN_00606100(local_8);
    if (iVar1 != 0) {
      iVar1 = FUN_00606b50();
      local_10 = iVar1 + local_10;
    }
  }
  return local_10;
}
```

### decoy: 0x00606160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00606160: 16-byte aligned: YES
  00606160  55             PUSH EBP
  00606161  8B EC          MOV EBP,ESP
  00606163  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00606160
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
```

## Card B0-6-21

GOG function: CSWSArea::SetTransitionPending  gog 0x007231f0  callconv=(blank)  param_size_bytes=5  gog_size=94  gog_retn=8
Candidate Steam address: 0x0052e290 (size 29, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0052e2b0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00723140 CSWSArea::SetFogColor -> 0x0052e060  gate_class=AUTO
- GOG-order neighbour after:  0x00723250 CSWSArea::SetPlayerRestrictMode -> 0x0052e170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040d320 CClientExoApp::SetInputActive -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x2f8,0x2fd} | Steam {0x300}
imm sets (* = in both, 1 common): GOG {0x8*,0xc} | Steam {0x8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0052e290: 16-byte aligned: YES
  0052e290  55             PUSH EBP
  0052e291  8B EC          MOV EBP,ESP
  0052e293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0052e290
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0052e290 FUN_0052e290 size=29 ret=8 callers=1 callees=0

void __thiscall FUN_0052e290(int param_1,int param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x300 + param_2 * 4) = param_3;
  return;
}
```

### decoy: 0x0052e2b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0052e2b0: 16-byte aligned: YES
  0052e2b0  55             PUSH EBP
  0052e2b1  8B EC          MOV EBP,ESP
  0052e2b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0052e2b0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-6-22

GOG function: CSWSCreatureStats::SetForceResistance  gog 0x006f8850  callconv=(blank)  param_size_bytes=1  gog_size=33  gog_retn=4
Candidate Steam address: 0x006bc3d0 (size 33, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006bc400:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f8560 CSWSCreatureStats::GetCreatureTalentRandomFeat -> 0x006bc0e0  gate_class=AUTO
- GOG-order neighbour after:  0x006f8880 CSWSCreatureStats::SetForceResistancePenalty -> 0x006bc400  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006e3610 CSWSCreatureStats::UpdateCombatInformation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x12b*} | Steam {0x12b*}
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
prologue for 0x006bc3d0: 16-byte aligned: YES
  006bc3d0  55             PUSH EBP
  006bc3d1  8B EC          MOV EBP,ESP
  006bc3d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bc3d0
string literals (0):
direct callees (1):
  0x006A7190  CSWSCreatureStats::UpdateCombatInformation
struct offsets touched (top 1):
  +0x12B [dec 299]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006bc3d0 FUN_006bc3d0 size=33 ret=4 callers=2 callees=1

void __thiscall FUN_006bc3d0(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x12b [dec 299]) = param_2;
  FUN_006a7190();
  return;
}
```

### decoy: 0x006bc400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bc400: 16-byte aligned: YES
  006bc400  55             PUSH EBP
  006bc401  8B EC          MOV EBP,ESP
  006bc403  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bc400
string literals (0):
direct callees (1):
  0x006A7190  CSWSCreatureStats::UpdateCombatInformation
struct offsets touched (top 1):
  +0x12C [dec 300]  x1  via EAX
```

## Card B0-6-23

GOG function: CSWSItemPropertyHandler::RemoveEnhancementPenalty  gog 0x007c3300  callconv=__thiscall  param_size_bytes=16  gog_size=1261  gog_retn=16
Candidate Steam address: 0x005ab1f0 (size 1917, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveEnhancementBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.73 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.73
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
disp sets (* = in both, 10 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x122*,0x130*,0x13c*,0x1198*} | Steam {0x2,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x38,0x122*,0x130*,0x13c*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x5*,0x7*,0x8*,0x10*,0x14,0x20*,0x68,0x148*,0x4000*,0x8000*,0x10000*} | Steam {0x2*,0x3,0x4,0x5*,0x6,0x7*,0x8*,0x10*,0x20*,0x94,0x148*,0x4000*,0x8000*,0x10000*}

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

## Card B0-6-24

GOG function: CSWSCreatureStats::GetTag  gog 0x006f2fb0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x006b6b30 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b6b50:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f2f70 CSWSCreatureStats::CalcStatModifier -> 0x006b6af0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f2fd0 CSWSCreatureStats::SetTag -> 0x006b6b50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b6b30: 16-byte aligned: YES
  006b6b30  55             PUSH EBP
  006b6b31  8B EC          MOV EBP,ESP
  006b6b33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b6b30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006b6b30 FUN_006b6b30 size=20 ret=0 callers=3 callees=0

int __fastcall FUN_006b6b30(int param_1)

{
  return *(int *)(param_1 + 0x24) + 0x18;
}
```

### decoy: 0x006b6b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b6b50: 16-byte aligned: YES
  006b6b50  55             PUSH EBP
  006b6b51  8B EC          MOV EBP,ESP
  006b6b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006b6b50
string literals (0):
direct callees (3):
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-25

GOG function: CSWSObject::SetPortrait  gog 0x00663870  callconv=(blank)  param_size_bytes=4  gog_size=34  gog_retn=4
Candidate Steam address: 0x0053efd0 (size 34, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0053efa0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663840 CSWSObject::GetPortrait -> 0x0053efa0  gate_class=REVIEW
- GOG-order neighbour after:  0x006638a0 CSWSObject::GetPortraitId -> 0x0053f000  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x20*} | Steam {0x4*,0x14*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053efd0 FUN_0053efd0 size=34 ret=4 callers=0 callees=1
THUNK -> 0x00710CA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0053efd0(undefined4 param_1)

{
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,param_1);
  return;
}
```

### decoy: 0x0053efa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efa0: 16-byte aligned: YES
  0053efa0  55             PUSH EBP
  0053efa1  8B EC          MOV EBP,ESP
  0053efa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efa0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

## Card B0-6-26

GOG function: CSWSItem::Destructor  gog 0x006638e0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x006000b0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_item_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006000e0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006638c0 CSWSObject::GetFirstName  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00663910 CSWSItem::Destructor_2 -> 0x006000e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00663910 CSWSItem::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x006000b0: 16-byte aligned: YES
  006000b0  55             PUSH EBP
  006000b1  8B EC          MOV EBP,ESP
  006000b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006000b0
string literals (0):
direct callees (2):
  0x006000E0  CSWSItem::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006000b0 FUN_006000b0 size=44 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_006000b0(undefined4 param_1,uint param_2)

{
  FUN_006000e0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x006000e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006000e0: 16-byte aligned: YES
  006000e0  55             PUSH EBP
  006000e1  8B EC          MOV EBP,ESP
  006000e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006000e0
string literals (0):
direct callees (15):
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x00605000  CItemRepository::Destructor
  0x009196FD  operator delete
  0x005453C0  CSWSObject::GetArea
  0x006037F0  CSWSItem::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x00735360  CExoLocString::Destructor
  0x0053F050  CSWSObject::Destructor_2
  0x006D6E10  (unnamed)
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via ECX
  +0x290 [dec 656]  x3  via EAX,ECX
  +0x294 [dec 660]  x3  via EAX,EDX
  +0x2AC [dec 684]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
```

## Card B0-6-27

GOG function: Gob::DisableFog  gog 0x0084c550  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00459f90 (size 31, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Gob::EnableFog
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459f70:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B0-6-28

GOG function: Gob::AddAnimationFlag  gog 0x008fd240  callconv=(blank)  param_size_bytes=4  gog_size=67  gog_retn=4
Candidate Steam address: 0x004e3160 (size 82, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Gob::RemoveAnimationFlag
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3110:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B0-6-29

GOG function: CSWSCreatureStats::Constructor  gog 0x006e06e0  callconv=__thiscall  param_size_bytes=4  gog_size=1893  gog_retn=4
Candidate Steam address: 0x006a4260 (size 1894, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006a49d0:STR0/CALLEE0/SIG0.34/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e0670 CSWSCreatureStats_ClassInfo::HasKnownSpell  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006e0e50 CSWSCreatureStats::Destructor -> 0x006a49d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0042c210 unnamed -> Steam ?  in candidate callees: n/a
  0x00495270 unnamed -> Steam ?  in candidate callees: n/a
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x005090f0 CCombatInformation::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00607350 CExoLocString::Constructor -> Steam ?  in candidate callees: n/a
  ... 8 more GOG callees
GOG strings (0): 
disp sets (* = in both, 110 common): GOG {0x24*,0x54*,0x60*,0x64*,0x68*,0x6c*,0x70*,0x74*,0x78*,0x84*,0x88*,0x8c*,0x8d*,0xe0*,...(+96)} | Steam {0x24*,0x54*,0x60*,0x64*,0x68*,0x6c*,0x70*,0x74*,0x78*,0x84*,0x88*,0x8c*,0x8d*,0xe0*,...(+96)}
imm sets (* = in both, 25 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x14*,0x18*,0x28*,...(+11)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x14*,0x18*,0x28*,...(+11)}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006a4260: 16-byte aligned: YES
  006a4260  55             PUSH EBP
  006a4261  8B EC          MOV EBP,ESP
  006a4263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006a4260
string literals (0):
direct callees (16):
  0x005933B0  (unnamed)
  0x0084A640  (unnamed)
  0x00735240  (unnamed)
  0x007107F0  (unnamed)
  0x0091D67E  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00919723  operator new
  0x006F58B0  PendingLevelUp::Constructor
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00710EB0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x006B6A20  CSWSCreatureStats::SetClassLevel
  0x006B69E0  CSWSCreatureStats::SetClass
  0x0069CAB0  (unnamed)
  0x0091C630  (unnamed)
  0x0064DD10  (unnamed)
struct offsets touched (top 10):
  +0x8D [dec 141]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x123 [dec 291]  x2  via EAX,ECX
  +0x174 [dec 372]  x2  via EAX,ECX
  +0x54 [dec 84]  x1  via EDX
  +0x60 [dec 96]  x1  via ECX
  +0x64 [dec 100]  x1  via EAX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x70 [dec 112]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006a4260 FUN_006a4260 size=1894 ret=4 callers=1 callees=16

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_006a4260(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined1 local_4c [16];
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  undefined1 local_30 [16];
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095b92f;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_005933b0(0);
  local_8 = 0;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  FUN_005933b0(0);
  local_8._0_1_ = 2;
  *(undefined4 *)(param_1 + 0x24) = 0;
  FUN_0084a640(0);
  local_8._0_1_ = 3;
  FUN_00735240(uVar1);
  local_8._0_1_ = 4;
  FUN_00735240();
  local_8._0_1_ = 5;
  FUN_007107f0();
  FUN_00735240();
  local_8._0_1_ = 6;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0x90),0x28,2,FUN_006a3e80,FUN_006a3f90);
  local_8._0_1_ = 7;
  FUN_00733540();
  local_8._0_1_ = 8;
  FUN_007107f0();
  FUN_00733540();
  local_8._0_1_ = 9;
  local_20 = FUN_00919723(0x30);
  local_8._0_1_ = 10;
  if (local_20 == 0) {
    local_60 = 0;
  }
  else {
    local_60 = FUN_006f58b0();
  }
  local_1c = local_60;
  local_8 = CONCAT31(local_8._1_3_,9);
  local_18 = local_60;
  FUN_0083ea60(local_60);
  *(undefined1 *)(param_1 + 0x8d) = 1;
  *(undefined1 *)(param_1 + 0x8c) = 0;
  FUN_00710eb0(local_30,&DAT_00986925);
  *(undefined4 *)(param_1 + 0x54) = 1;
  *(undefined4 *)(param_1 + 0x6c) = 1;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined1 *)(param_1 + 0x64 [dec 100]) = 0;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined2 *)(param_1 + 0xe0) = DAT_0099a8bc;
  FUN_007338d0(&DAT_00986925);
  *(undefined1 *)(param_1 + 0xec) = 0;
  FUN_006b6a20(0,1);
  FUN_006b69e0(0,0);
  *(undefined1 *)(param_1 + 0x8d) = 2;
  for (local_14 = 1; (int)local_14 < 2; local_14 = local_14 + 1) {
/* ... 127 more lines truncated by --max-lines */
```

### decoy: 0x006a49d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a49d0: 16-byte aligned: YES
  006a49d0  55             PUSH EBP
  006a49d1  8B EC          MOV EBP,ESP
  006a49d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006a49d0
string literals (0):
direct callees (9):
  0x009196FD  operator delete
  0x006B8370  CSWSCreatureStats::ClearFeats
  0x0049B840  (unnamed)
  0x006F59B0  PendingLevelUp::Destructor
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0069CBE0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00735360  CExoLocString::Destructor
struct offsets touched (top 5):
  +0x11C [dec 284]  x3  via EAX,ECX
  +0x134 [dec 308]  x3  via ECX,EDX
  +0x174 [dec 372]  x3  via EAX,ECX
  +0x28 [dec 40]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B0-6-30

GOG function: CSWSObject::SetDialogOwner  gog 0x006ada90  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00546db0 (size 22, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00546dd0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ad8b0 CSWSObject::GetNearestObjectByName -> 0x00546bd0  gate_class=AUTO
- GOG-order neighbour after:  0x006adb20 CSWSObject::SetListenExpression  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x54*} | Steam {0x54*}
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
prologue for 0x00546db0: 16-byte aligned: YES
  00546db0  55             PUSH EBP
  00546db1  8B EC          MOV EBP,ESP
  00546db3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00546db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x54 [dec 84]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00546db0 FUN_00546db0 size=22 ret=4 callers=3 callees=0

void __thiscall FUN_00546db0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  return;
}
```

### decoy: 0x00546dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00546dd0: 16-byte aligned: YES
  00546dd0  55             PUSH EBP
  00546dd1  8B EC          MOV EBP,ESP
  00546dd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00546dd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1B0 [dec 432]  x2  via EAX
  +0x1B4 [dec 436]  x1  via ECX
```

## Card B0-6-31

GOG function: CSWSCreatureStats::GetFullName  gog 0x006e10b0  callconv=(blank)  param_size_bytes=4  gog_size=145  gog_retn=4
Candidate Steam address: 0x006a4c30 (size 145, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006a4cd0:STR0/CALLEE0/SIG0.091/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e0e50 CSWSCreatureStats::Destructor -> 0x006a49d0  gate_class=AUTO
- GOG-order neighbour after:  0x006e1150 CSWSCreatureStats::GetSpellGainWithBonus -> 0x006a4cd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x007f42b0 CSWSMessage::GetLocStringServer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x10*,0x14*,0x34*,0x3c*} | Steam {0x4*,0x10*,0x14*,0x34*,0x3c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006a4c30: 16-byte aligned: YES
  006a4c30  55             PUSH EBP
  006a4c31  8B EC          MOV EBP,ESP
  006a4c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006a4c30
string literals (0):
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00633620  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006a4c30 FUN_006a4c30 size=145 ret=4 callers=7 callees=4

undefined4 __thiscall FUN_006a4c30(int param_1,undefined4 param_2)

{
  uint uVar1;
  uint uVar2;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972a61;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  uVar2 = 0;
  FUN_00733540(uVar1,param_1,0);
  local_8 = 1;
  FUN_00633620(0xffffffff,param_1 + 0x34,param_1 + 0x3c,local_18,0);
  FUN_007335f0(local_18);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780(uVar1,param_1,uVar2 | 1);
  ExceptionList = local_10;
  return param_2;
}
```

### decoy: 0x006a4cd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a4cd0: 16-byte aligned: YES
  006a4cd0  55             PUSH EBP
  006a4cd1  8B EC          MOV EBP,ESP
  006a4cd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006a4cd0
string literals (0):
direct callees (3):
  0x006F2A30  IsForceUserClass(classId) {3,4,5,11..16}
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F09D0  (unnamed)
struct offsets touched (top 3):
  +0x8D [dec 141]  x1  via ECX
  +0xF8 [dec 248]  x1  via ECX
  +0x124 [dec 292]  x1  via EDX
```

## Card B0-6-32

GOG function: CItemRepository::GetItemInRepository  gog 0x006695b0  callconv=(blank)  param_size_bytes=8  gog_size=248  gog_retn=8
Candidate Steam address: 0x00605d80 (size 248, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00605b80:STR0/CALLEE0/SIG0.411/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006694e0 CItemRepository::FindItemWithTag -> 0x00605cb0  gate_class=AUTO
- GOG-order neighbour after:  0x006696b0 CItemRepository::RemoveItem -> 0x00605e80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x006695b0 CItemRepository::GetItemInRepository -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x8*,0xc*,0x10*,0x34*,0x70*,0x2ac*} | Steam {0x8*,0xc*,0x10*,0x34*,0x70*,0x2ac*}
imm sets (* = in both, 2 common): GOG {0x8*,0x30*} | Steam {0x8*,0x30*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00605d80: 16-byte aligned: YES
  00605d80  55             PUSH EBP
  00605d81  8B EC          MOV EBP,ESP
  00605d83  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00605d80
string literals (0):
direct callees (3):
  0x0051C0E0  (unnamed)
  0x006D5200  (unnamed)
  0x00605D80  CItemRepository::GetItemInRepository
struct offsets touched (top 6):
  +0xC [dec 12]  x2  via ECX
  +0x2AC [dec 684]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x34 [dec 52]  x1  via EDX
  +0x70 [dec 112]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00605d80 FUN_00605d80 size=248 ret=8 callers=11 callees=3

undefined4 __thiscall FUN_00605d80(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x10) <= local_c) {
      return 0;
    }
    iVar1 = FUN_0051c0e0(*(undefined4 *)(*(int *)(param_1 + 0xc) + local_c * 4));
    if (param_2 == iVar1) break;
    if ((((param_3 != 0) && (iVar1 != 0)) &&
        (iVar2 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc)), iVar2 != 0)) &&
       (((*(int *)(iVar2 + 0x70) != 0 && (*(int *)(iVar1 + 0x2ac) != 0)) &&
        (iVar1 = FUN_00605d80(param_2,param_3), iVar1 != 0)))) {
      return 1;
    }
    local_c = local_c + 1;
  }
  return 1;
}
```

### decoy: 0x00605b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00605b80: 16-byte aligned: YES
  00605b80  55             PUSH EBP
  00605b81  8B EC          MOV EBP,ESP
  00605b83  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00605b80
string literals (0):
direct callees (3):
  0x0051C0E0  (unnamed)
  0x00605B80  CItemRepository::FindItemWithBaseItemType
  0x006D6E30  (unnamed)
struct offsets touched (top 5):
  +0xAC [dec 172]  x2  via ECX,EDX
  +0x2AC [dec 684]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

## Card B0-6-33

GOG function: Gob::DisableFog  gog 0x0084c550  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00459f00 (size 98, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459f70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c4e0 Global::PartSetFog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084c570 Gob::EnableFog -> 0x00459f90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0084c4e0 Global::PartSetFog -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x98} | Steam {0x2c,0x30,0x34,0x7d}
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
prologue for 0x00459f00: 16-byte aligned: YES
  00459f00  55             PUSH EBP
  00459f01  8B EC          MOV EBP,ESP
  00459f03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459f00
string literals (0):
direct callees (1):
  0x00459F00  (unnamed)
struct offsets touched (top 4):
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
  +0x7D [dec 125]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00459f00 FUN_00459f00 size=98 ret=0 callers=3 callees=1

void FUN_00459f00(int *param_1,undefined1 param_2)

{
  int iVar1;
  int local_8;
  
  iVar1 = (**(code **)(*param_1 + 0x2c))();
  if (iVar1 != 0) {
    *(undefined1 *)(iVar1 + 0x7d) = param_2;
  }
  for (local_8 = 0; local_8 < param_1[0xd = byte +0x34]; local_8 = local_8 + 1) {
    FUN_00459f00(*(undefined4 *)(param_1[0xc = byte +0x30] + local_8 * 4),param_2);
  }
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

## Card B0-6-34

GOG function: CSWSCreatureStats::GetBaseFortSavingThrow  gog 0x006f3820  callconv=(blank)  param_size_bytes=0  gog_size=542  gog_retn=0
Candidate Steam address: 0x006b73a0 (size 542, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b75c0:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f3790 CSWSCreatureStats::GetPotentialLevel -> 0x006b7310  gate_class=AUTO
- GOG-order neighbour after:  0x006f3a40 CSWSCreatureStats::GetBaseWillSavingThrow -> 0x006b75c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0050efd0 Global::GetFortSaveBonus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x006e9770 CSWSCreatureStats::GetClassLevel -> Steam ?  in candidate callees: n/a
  0x006f4870 CSWSCreatureStats::HasFeat -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (1): '?SaveMult'
disp sets (* = in both, 9 common): GOG {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*} | Steam {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*} | Steam {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b73a0: 16-byte aligned: YES
  006b73a0  55             PUSH EBP
  006b73a1  8B EC          MOV EBP,ESP
  006b73a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006b73a0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0920  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b73a0 FUN_006b73a0 size=542 ret=0 callers=1 callees=7

char __fastcall FUN_006b73a0(int param_1)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  char local_19;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_19 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(param_1 + 0x8d); local_14 = local_14 + 1)
  {
    if ((local_14 & 0xff) < (uint)*(byte *)(param_1 + 0x8d)) {
      local_37 = *(byte *)(param_1 + 0xab + (local_14 & 0xff) * 0x28);
    }
    else {
      local_37 = 0xff;
    }
    local_18 = (uint)local_37 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124);
    if (local_18 != 0) {
      uVar1 = FUN_006ad2f0(local_14 & 0xff,0);
      cVar2 = FUN_006f0920(uVar1);
      local_19 = local_19 + cVar2;
    }
  }
  if (*(int *)(*(int *)(param_1 + 0x24) + 0x11ac) == 0) {
    local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x24) + 0x250);
    if (local_20 != 0) {
      local_2c = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x154);
      FUN_00733570("SaveMult");
      local_8 = 0;
      FUN_0071b670(local_20,local_34,local_24);
      local_8 = 0xffffffff;
      FUN_00733780();
      if (*(char *)(*(int *)(param_1 + 0x24) + 0x251) == '\0') {
        local_25 = '\x01';
      }
      else {
        local_25 = *(char *)(*(int *)(param_1 + 0x24) + 0x251);
      }
      cVar2 = FUN_0091c860(uVar3,(int)local_25);
      local_19 = local_19 + cVar2;
    }
  }
  iVar4 = FUN_006b83f0(0x16);
  if (iVar4 == 0) {
    iVar4 = FUN_006b83f0(0x15);
    if (iVar4 == 0) {
      iVar4 = FUN_006b83f0(0xd);
      if (iVar4 != 0) {
        local_19 = local_19 + '\x01';
      }
    }
    else {
      local_19 = local_19 + '\x02';
    }
  }
  else {
    local_19 = local_19 + '\x03';
  }
  ExceptionList = local_10;
  return local_19;
}
```

### decoy: 0x006b75c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b75c0: 16-byte aligned: YES
  006b75c0  55             PUSH EBP
  006b75c1  8B EC          MOV EBP,ESP
  006b75c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006b75c0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0990  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B0-6-35

GOG function: Gob::AddAnimationFlag  gog 0x008fd240  callconv=(blank)  param_size_bytes=4  gog_size=67  gog_retn=4
Candidate Steam address: 0x004e1d40 (size 5071, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x004e3110
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3110:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fcfb7 Gob::Animate_6  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fd290 Gob::RemoveAnimationFlag -> 0x004e3160  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xdc*,0xe0*} | Steam {0x4,0x8*,0xc,0x10,0x14,0x16,0x17,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,...(+28)}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x2,0x4*,0x5,0x8,0xc*,0x10,0x14,0x18,0x20,0x40,0x41,0x44,0xa4,0xc0,...(+5)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004e1d40: 16-byte aligned: YES
  004e1d40  55             PUSH EBP
  004e1d41  8B EC          MOV EBP,ESP
  004e1d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004e1d40
string literals (5):
  0x0099138C  at 004e2726  'start%s'
  0x00991384  at 004e27dd  'done%s'
  0x0099138C  at 004e29e9  'start%s'
  0x0099137C  at 004e2ae7  'loop%s'
  0x0099137C  at 004e2b8f  'loop%s'
direct callees (15):
  0x00461DF0  (unnamed)
  0x005142F0  (unnamed)
  0x00507BB0  (unnamed)
  0x004E3B50  (unnamed)
  0x00473E80  (unnamed)
  0x004DFBA0  (unnamed)
  0x0049B840  (unnamed)
  0x009196FD  operator delete
  0x00459E20  Gob::IsInCutscene
  0x00919671  (unnamed)
  0x00449170  (unnamed)
  0x004DCF00  (unnamed)
  0x004DFED0  (unnamed)
  0x004DD380  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x10 [dec 16]  x22  via EAX,ECX,EDX
  +0x8 [dec 8]  x20  via EAX,ECX,EDX
  +0x50 [dec 80]  x20  via EAX,ECX,EDX
  +0x84 [dec 132]  x15  via EAX,ECX,EDX
  +0x94 [dec 148]  x14  via EAX,ECX,EDX
  +0x14 [dec 20]  x13  via EAX,ECX,EDX
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x1A0 [dec 416]  x10  via ECX,EDX
  +0xDC [dec 220]  x9  via EAX,ECX,EDX
  +0x98 [dec 152]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004e1d40 FUN_004e1d40 size=5071 ret=4 callers=1 callees=15

/* WARNING: Removing unreachable block (ram,0x004e2d25) */
/* WARNING: Removing unreachable block (ram,0x004e2d4b) */
/* WARNING: Removing unreachable block (ram,0x004e2d3f) */
/* WARNING: Removing unreachable block (ram,0x004e2d55) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004e1d40(int *param_1,float param_2)

{
  float fVar1;
  char cVar2;
  int *piVar3;
  int iVar4;
  bool bVar5;
  float10 fVar6;
  uint local_248;
  double local_23c;
  float local_224;
  float local_220;
  float local_21c;
  float local_218;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float local_1a4;
  float local_1a0;
  int *local_19c;
  float local_198;
  int local_194;
  float local_190;
  undefined4 local_18c;
  char local_188 [64];
  char local_148 [64];
  char local_108 [64];
  char local_c8 [64];
  uint local_88;
  bool local_81;
  uint local_80;
  int *local_7c;
  char local_78 [64];
  uint local_38;
  int local_34;
  int local_30;
  float local_2c;
  float local_28;
  int local_24;
  float local_20;
  char local_19;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094c998;
  local_10 = ExceptionList;
  local_38 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00461df0(local_38);
  if ((DAT_00a04a20 != 0) &&
     ((param_1[0x25 = byte +0x94] == 0 || (param_1[0x6e = byte +0x1b8] != *(int *)(param_1[0x25 = byte +0x94] + 0x50))))) {
    if (param_1[0x25 = byte +0x94] != 0) {
      param_1[0x6e = byte +0x1b8] = *(int *)(param_1[0x25 = byte +0x94] + 0x50);
    }
    if (((*(char *)((int)param_1 + 0x182) != '\0') && (param_1[0x25 = byte +0x94] != 0)) &&
       (*(int *)(param_1[0x25 = byte +0x94] + 0x90) != 0)) {
      local_20 = 0.0;
      if ((float)param_1[0x61 = byte +0x184] == (float)_DAT_00986b38) {
        local_24 = _DAT_00991394;
      }
      else {
        local_24 = param_1[0x61 = byte +0x184];
      }
      if ((float)param_1[99 = byte +0x18c] == (float)_DAT_00986b38) {
        local_1a8 = (float)param_1[0x29 = byte +0xa4] - (float)param_1[0x30 = byte +0xc0];
        local_1a4 = (float)param_1[0x2a = byte +0xa8] - (float)param_1[0x31 = byte +0xc4];
        local_1a0 = (float)param_1[0x2b = byte +0xac] - (float)param_1[0x32 = byte +0xc8];
/* ... 358 more lines truncated by --max-lines */
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

## Card B0-6-36

GOG function: CSWSCreatureStats::GetBaseReflexSavingThrow  gog 0x006f3c60  callconv=(blank)  param_size_bytes=0  gog_size=542  gog_retn=0
Candidate Steam address: 0x006b77e0 (size 542, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b75c0:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f3a40 CSWSCreatureStats::GetBaseWillSavingThrow -> 0x006b75c0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f3e80 CSWSCreatureStats::GetCanUseSkill -> 0x006b7a00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0050f000 Global::GetRefSaveBonus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x006e9770 CSWSCreatureStats::GetClassLevel -> Steam ?  in candidate callees: n/a
  0x006f4870 CSWSCreatureStats::HasFeat -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (1): '?SaveMult'
disp sets (* = in both, 9 common): GOG {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*} | Steam {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*} | Steam {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b77e0: 16-byte aligned: YES
  006b77e0  55             PUSH EBP
  006b77e1  8B EC          MOV EBP,ESP
  006b77e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006b77e0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0950  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b77e0 FUN_006b77e0 size=542 ret=0 callers=1 callees=7

char __fastcall FUN_006b77e0(int param_1)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  int local_1c;
  char local_15;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_15 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(param_1 + 0x8d); local_14 = local_14 + 1)
  {
    if ((local_14 & 0xff) < (uint)*(byte *)(param_1 + 0x8d)) {
      local_37 = *(byte *)(param_1 + 0xab + (local_14 & 0xff) * 0x28);
    }
    else {
      local_37 = 0xff;
    }
    local_1c = (uint)local_37 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124);
    if (local_1c != 0) {
      uVar1 = FUN_006ad2f0(local_14 & 0xff,0);
      cVar2 = FUN_006f0950(uVar1);
      local_15 = local_15 + cVar2;
    }
  }
  if (*(int *)(*(int *)(param_1 + 0x24) + 0x11ac) == 0) {
    local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x24) + 0x250);
    if (local_20 != 0) {
      local_2c = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x154);
      FUN_00733570("SaveMult");
      local_8 = 0;
      FUN_0071b670(local_20,local_34,local_24);
      local_8 = 0xffffffff;
      FUN_00733780();
      if (*(char *)(*(int *)(param_1 + 0x24) + 0x251) == '\0') {
        local_25 = '\x01';
      }
      else {
        local_25 = *(char *)(*(int *)(param_1 + 0x24) + 0x251);
      }
      cVar2 = FUN_0091c860(uVar3,(int)local_25);
      local_15 = local_15 + cVar2;
    }
  }
  iVar4 = FUN_006b83f0(0x16);
  if (iVar4 == 0) {
    iVar4 = FUN_006b83f0(0x15);
    if (iVar4 == 0) {
      iVar4 = FUN_006b83f0(0xd);
      if (iVar4 != 0) {
        local_15 = local_15 + '\x01';
      }
    }
    else {
      local_15 = local_15 + '\x02';
    }
  }
  else {
    local_15 = local_15 + '\x03';
  }
  ExceptionList = local_10;
  return local_15;
}
```

### decoy: 0x006b75c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b75c0: 16-byte aligned: YES
  006b75c0  55             PUSH EBP
  006b75c1  8B EC          MOV EBP,ESP
  006b75c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006b75c0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0990  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B0-6-37

GOG function: CSWSItem::RemoveUpgrade  gog 0x006796c0  callconv=(blank)  param_size_bytes=8  gog_size=349  gog_retn=8
Candidate Steam address: 0x00615e90 (size 349, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:gog_item_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00615d60:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00679590 CSWSItem::AddUpgrade -> 0x00615d60  gate_class=REVIEW
- GOG-order neighbour after:  0x00679820 CSWSItem::GetPropertiesOnlyDescription -> 0x00615ff0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00667a00 unnamed -> Steam ?  in candidate callees: n/a
  0x00667ad0 CSWSItem::GetPassiveProperty -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x12*,0x28c*,0x294*} | Steam {0x12*,0x28c*,0x294*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x3c*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x3c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00615e90: 16-byte aligned: YES
  00615e90  55             PUSH EBP
  00615e91  8B EC          MOV EBP,ESP
  00615e93  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00615e90
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x006042A0  CSWSItem::GetPassiveProperty
  0x0091DBA0  _memcpy
  0x006041D0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x28C [dec 652]  x3  via EAX,EDX
  +0x294 [dec 660]  x3  via ECX
  +0x12 [dec 18]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00615e90 FUN_00615e90 size=349 ret=8 callers=1 callees=5

void __thiscall FUN_00615e90(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  bool bVar3;
  void *_Src [GHIDRA-AUTO-LABEL: unreliable];
  void *_Src_00 [GHIDRA-AUTO-LABEL: unreliable];
  uint uVar4;
  undefined4 local_24;
  undefined4 local_1c;
  undefined4 local_10;
  
  iVar1 = *(int *)(param_2 + 0x28c);
  uVar4 = *(int *)(param_1 + 0x28c) - iVar1;
  iVar2 = *(int *)(param_1 + 0x28c);
  _Src [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(-(uint)((int)((ulonglong)uVar4 * 0x1c >> 0x20) != 0) |
                              (uint)((ulonglong)uVar4 * 0x1c));
  local_24 = 0;
  for (local_10 = 0; local_10 < iVar2; local_10 = local_10 + 1) {
    bVar3 = false;
    _Src_00 [GHIDRA-AUTO-LABEL: unreliable] = (void *)(local_10 * 0x1c + *(int *)(param_1 + 0x294));
    for (local_1c = 0; local_1c < iVar1; local_1c = local_1c + 1) {
      FUN_006042a0(local_1c);
      if ((uint)*(ushort *)((int)_Src_00 [GHIDRA-AUTO-LABEL: unreliable] + 0x12) == param_3 + 1U) {
        bVar3 = true;
      }
    }
    if (!bVar3) {
      _memcpy [GHIDRA-AUTO-LABEL: unreliable]((void *)(local_24 * 0x1c + (int)_Src [GHIDRA-AUTO-LABEL: unreliable]),_Src_00 [GHIDRA-AUTO-LABEL: unreliable],0x1c);
      local_24 = local_24 + 1;
    }
  }
  FUN_006041d0(uVar4);
  if (*(int *)(param_1 + 0x294) != 0) {
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x294),_Src [GHIDRA-AUTO-LABEL: unreliable],uVar4 * 0x1c);
  }
  FUN_009196fd(_Src [GHIDRA-AUTO-LABEL: unreliable]);
  return;
}
```

### decoy: 0x00615d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00615d60: 16-byte aligned: YES
  00615d60  55             PUSH EBP
  00615d61  8B EC          MOV EBP,ESP
  00615d63  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00615d60
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x0091DBA0  _memcpy
  0x006041D0  (unnamed)
  0x009196FD  operator delete
  0x006042A0  CSWSItem::GetPassiveProperty
struct offsets touched (top 3):
  +0x294 [dec 660]  x4  via ECX,EDX
  +0x28C [dec 652]  x3  via EAX,EDX
  +0x12 [dec 18]  x1  via ECX
```

## Card B0-6-38

GOG function: CSWSCreatureStats::CalcStatModifier  gog 0x006f2f70  callconv=(blank)  param_size_bytes=1  gog_size=48  gog_retn=4
Candidate Steam address: 0x006b6af0 (size 48, retn 4)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006b69a0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f2f30 CSWSCreatureStats::SetClassNegativeLevels -> 0x006b6ab0  gate_class=AUTO
- GOG-order neighbour after:  0x006f2fb0 CSWSCreatureStats::GetTag -> 0x006b6b30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xa*,0xb*} | Steam {0x4*,0xa*,0xb*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006b6af0: 16-byte aligned: YES
  006b6af0  55             PUSH EBP
  006b6af1  8B EC          MOV EBP,ESP
  006b6af3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b6af0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006b6af0 FUN_006b6af0 size=48 ret=4 callers=8 callees=0

int FUN_006b6af0(byte param_1)

{
  int iVar1;
  
  if (param_1 < 10) {
    iVar1 = param_1 - 0xb;
  }
  else {
    iVar1 = param_1 - 0xa [dec 10];
  }
  return iVar1 / 2;
}
```

### decoy: 0x006b69a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b69a0: 16-byte aligned: YES
  006b69a0  55             PUSH EBP
  006b69a1  8B EC          MOV EBP,ESP
  006b69a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b69a0
string literals (0):
direct callees (3):
  0x006B5C50  CSWSCreatureStats::GetCHAStat
  0x006B6AF0  CSWSCreatureStats::CalcStatModifier
  0x006BCE00  CSWSCreatureStats::AdjustSpellUsesPerDay
struct offsets touched (top 2):
  +0xF7 [dec 247]  x1  via EAX
  +0xF8 [dec 248]  x1  via ECX
```

## Card B0-6-39

GOG function: CSWSCreatureStats::GetTotalINTBonus  gog 0x006f6f60  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006baae0 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006bab30:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6f10 CSWSCreatureStats::GetTotalCONBonus -> 0x006baa90  gate_class=REVIEW
- GOG-order neighbour after:  0x006f6fb0 CSWSCreatureStats::GetTotalWISBonus -> 0x006bab30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x10*,0xff*} | Steam {0x3*,0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006baae0: 16-byte aligned: YES
  006baae0  55             PUSH EBP
  006baae1  8B EC          MOV EBP,ESP
  006baae3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006baae0
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006baae0 FUN_006baae0 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006baae0(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,3,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006bab30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bab30: 16-byte aligned: YES
  006bab30  55             PUSH EBP
  006bab31  8B EC          MOV EBP,ESP
  006bab33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bab30
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-40

GOG function: CSWSObject::GetLastName  gog 0x006a5ce0  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0053ef90 (size 16, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f050:STR0/CALLEE0/SIG0.025/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a55e0 CSWSObject::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006a5d00 CSWSObject::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053ef90: 16-byte aligned: YES
  0053ef90  55             PUSH EBP
  0053ef91  8B EC          MOV EBP,ESP
  0053ef93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053ef90
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053ef90 FUN_0053ef90 size=16 ret=0 callers=0 callees=0

undefined4 FUN_0053ef90(void)

{
  return 0x7f000000;
}
```

### decoy: 0x0053f050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f050: 16-byte aligned: YES
  0053f050  55             PUSH EBP
  0053f051  8B EC          MOV EBP,ESP
  0053f053  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0053f050
string literals (0):
direct callees (16):
  0x006A2710  (unnamed)
  0x009196FD  operator delete
  0x00547040  CSWSObject::ClearMatchedExpressionStrings
  0x0051C080  (unnamed)
  0x0053DEA0  (unnamed)
  0x005E4CA0  CGameEffect::Destructor
  0x00840EF0  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x0053E740  Action-list free params
  0x007F2630  CSWCObject::DetachFromServerObject
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x005E6DE0  CSWSScriptVarTable::Destructor
  0x0072F5F0  (unnamed)
  0x00735360  CExoLocString::Destructor
struct offsets touched (top 8):
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x1D0 [dec 464]  x3  via EAX,ECX
  +0x264 [dec 612]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0x1B0 [dec 432]  x2  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x1B4 [dec 436]  x1  via EDX
```

## Card B0-6-41

GOG function: CSWSItem::RemoveFromArea  gog 0x00667020  callconv=(blank)  param_size_bytes=0  gog_size=102  gog_retn=0
Candidate Steam address: 0x006037f0 (size 102, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_item_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00603860:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00666f70 CSWSItem::SaveContainerItems -> 0x00603740  gate_class=AUTO
- GOG-order neighbour after:  0x00667090 CSWSItem::UpdateUsedActiveProperties -> 0x00603860  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ac100 CSWSObject::SetArea -> Steam ?  in candidate callees: n/a
  0x00718240 CSWSArea::RemoveObjectFromArea -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}
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
prologue for 0x006037f0: 16-byte aligned: YES
  006037f0  55             PUSH EBP
  006037f1  8B EC          MOV EBP,ESP
  006037f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006037f0
string literals (0):
direct callees (4):
  0x005453C0  CSWSObject::GetArea
  0x00523090  CSWSArea::RemoveObjectFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
struct offsets touched (top 1):
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006037f0 FUN_006037f0 size=102 ret=0 callers=6 callees=4

void __fastcall FUN_006037f0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 0x14);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### decoy: 0x00603860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603860: 16-byte aligned: YES
  00603860  55             PUSH EBP
  00603861  8B EC          MOV EBP,ESP
  00603863  83 EC 54       SUB ESP,0x54
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00603860
string literals (0):
direct callees (6):
  0x00604260  CSWSItem::GetActiveProperty
  0x00603B50  CSWSItem::GetUsedActivePropertyUsesLeft
  0x0051C2E0  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0064B4D0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x278 [dec 632]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x279 [dec 633]  x2  via EAX,ECX
  +0x6 [dec 6]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x288 [dec 648]  x1  via EAX
  +0x298 [dec 664]  x1  via ECX
  +0x2A8 [dec 680]  x1  via EDX
```

## Card B0-6-42

GOG function: CSWSCreatureStats::GetWeaponFinesse  gog 0x006e5320  callconv=(blank)  param_size_bytes=4  gog_size=110  gog_retn=4
Candidate Steam address: 0x006a8ea0 (size 110, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006a8f10:STR0/CALLEE0/SIG0.268/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e50a0 CSWSCreatureStats::GetRangedDamageBonus -> 0x006a8c20  gate_class=AUTO
- GOG-order neighbour after:  0x006e5390 CSWSCreatureStats::GetWeaponSpecialization -> 0x006a8f10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam ?  in candidate callees: n/a
  0x0068f220 CSWSCreature::GetUseMonkAbilities -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x24*,0x1150*} | Steam {0x24*,0x1150*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x14*} | Steam {0x4*,0x10*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006a8ea0: 16-byte aligned: YES
  006a8ea0  55             PUSH EBP
  006a8ea1  8B EC          MOV EBP,ESP
  006a8ea3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006a8ea0
string literals (0):
direct callees (2):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
  0x006D0620  (unnamed)
struct offsets touched (top 2):
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x1150 [dec 4432]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006a8ea0 FUN_006a8ea0 size=110 ret=4 callers=3 callees=2

undefined4 FUN_006a8ea0(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_005766f0();
  if (iVar1 != 0) {
    iVar1 = FUN_006d0620(0x10);
    if (param_1 == 0) {
      if (iVar1 == 0) {
        return 1;
      }
    }
    else if (iVar1 == 0) {
      return 1;
    }
  }
  return 0;
}
```

### decoy: 0x006a8f10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a8f10: 16-byte aligned: YES
  006a8f10  55             PUSH EBP
  006a8f11  8B EC          MOV EBP,ESP
  006a8f13  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006a8f10
string literals (0):
direct callees (1):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via ECX
  +0x53C [dec 1340]  x1  via EDX
```

## Card B0-6-43

GOG function: CSWSObject::SetAnimation  gog 0x006a8070  callconv=(blank)  param_size_bytes=4  gog_size=35  gog_retn=4
Candidate Steam address: 0x00541390 (size 35, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005413c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a7d60 CSWSObject::ClearAllActions -> 0x00541080  gate_class=AUTO
- GOG-order neighbour after:  0x006a80a0 CSWSObject::GetCurrentHitPoints  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1110 CSWSObject::SetDirtyFlag -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd8*} | Steam {0xd8*}
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
prologue for 0x00541390: 16-byte aligned: YES
  00541390  55             PUSH EBP
  00541391  8B EC          MOV EBP,ESP
  00541393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00541390
string literals (0):
direct callees (1):
  0x0054A430  CSWSObject::SetDirtyFlag
struct offsets touched (top 1):
  +0xD8 [dec 216]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00541390 FUN_00541390 size=35 ret=4 callers=1 callees=1

void __thiscall FUN_00541390(int param_1,undefined4 param_2)

{
  FUN_0054a430(0);
  *(undefined4 *)(param_1 + 0xd8) = param_2;
  return;
}
```

### decoy: 0x005413c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005413c0: 16-byte aligned: YES
  005413c0  55             PUSH EBP
  005413c1  8B EC          MOV EBP,ESP
  005413c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005413c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xE0 [dec 224]  x2  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B0-6-44

GOG function: CItemRepository::ClearNewItems  gog 0x00669770  callconv=(blank)  param_size_bytes=4  gog_size=162  gog_retn=4
Candidate Steam address: 0x00605f40 (size 162, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_citemrepository_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00605e80:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006696b0 CItemRepository::RemoveItem -> 0x00605e80  gate_class=REVIEW
- GOG-order neighbour after:  0x00669820 CItemRepository::GetMatchingItemCount -> 0x00605ff0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x2c8*} | Steam {0x4*,0x8*,0x2c8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x7*,0x18*,0xffffff7f*} | Steam {0x4*,0x7*,0x18*,0xffffff7f*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00605f40: 16-byte aligned: YES
  00605f40  55             PUSH EBP
  00605f41  8B EC          MOV EBP,ESP
  00605f43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00605f40
string literals (0):
direct callees (2):
  0x0051C0E0  (unnamed)
  0x0049B840  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x2C8 [dec 712]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00605f40 FUN_00605f40 size=162 ret=4 callers=1 callees=2

void __thiscall FUN_00605f40(int param_1,int *param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = param_2[1 = byte +0x4];
  while (local_8 = local_8 + -1, -1 < local_8) {
    iVar1 = FUN_0051c0e0(*(undefined4 *)(*param_2 + local_8 * 4));
    if (iVar1 != 0) {
      *(uint *)(iVar1 + 0x2c8) = *(uint *)(iVar1 + 0x2c8) & 0xffffff7f;
      *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + -1;
      FUN_0049b840(local_8);
    }
  }
  return;
}
```

### decoy: 0x00605e80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00605e80: 16-byte aligned: YES
  00605e80  55             PUSH EBP
  00605e81  8B EC          MOV EBP,ESP
  00605e83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00605e80
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 5):
  +0x2C8 [dec 712]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-6-45

GOG function: CSWSCreatureStats::GetExpNeededForLevelUp  gog 0x006f3760  callconv=(blank)  param_size_bytes=0  gog_size=46  gog_retn=0
Candidate Steam address: 0x006b72e0 (size 46, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b7310:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f34e0 CSWSCreatureStats::AddExperience -> 0x006b7060  gate_class=AUTO
- GOG-order neighbour after:  0x006f3790 CSWSCreatureStats::GetPotentialLevel -> 0x006b7310  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b72e0: 16-byte aligned: YES
  006b72e0  55             PUSH EBP
  006b72e1  8B EC          MOV EBP,ESP
  006b72e3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b72e0
string literals (0):
direct callees (1):
  0x006A4E00  CSWSCreatureStats::GetLevel
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006b72e0 FUN_006b72e0 size=46 ret=0 callers=5 callees=1

undefined4 FUN_006b72e0(void)

{
  byte bVar1;
  
  bVar1 = FUN_006a4e00(0);
  return *(undefined4 *)(DAT_00a1b4d0 + 0x38 + (uint)bVar1 * 4);
}
```

### decoy: 0x006b7310 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b7310: 16-byte aligned: YES
  006b7310  55             PUSH EBP
  006b7311  8B EC          MOV EBP,ESP
  006b7313  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b7310
string literals (0):
direct callees (2):
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x0051BD40  GetServerInfo/JoiningRestrictions accessor (CanLevelUp reads +0x94 MaxLevel)
struct offsets touched (top 4):
  +0x8 [dec 8]  x1  via ECX
  +0x68 [dec 104]  x1  via EDX
  +0x6C [dec 108]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
```

## Card B0-6-46

GOG function: CExoResMan::FileCopy  gog 0x0061f0a0  callconv=(blank)  param_size_bytes=14  gog_size=404  gog_retn=16
Candidate Steam address: 0x00716d80 (size 18, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoResMan::FreeResourceData_2
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00715c00:STR0/CALLEE0/SIG0.045/SRV0/VT0
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
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x10*,0x30,0xffff} | Steam {0x10*}

### abi
```
GOG param bytes: 14; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(14/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 14 bytes -> x86 slots 16; ASM RET 16)
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

### decoy: 0x00715c00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00715c00: 16-byte aligned: YES
  00715c00  55             PUSH EBP
  00715c01  8B EC          MOV EBP,ESP
  00715c03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00715c00
string literals (19):
  0x00994E68  at 00715c7f  'RIMS:'
  0x00994E68  at 00715df6  'RIMS:'
  0x0099C428  at 00715e91  'LIVE%d:RIMSXBOX\\\\'
  0x00994E68  at 00715fe4  'RIMS:'
  0x0099C428  at 0071607f  'LIVE%d:RIMSXBOX\\\\'
  0x00993E50  at 00716113  'MODULES:'
  0x0099C418  at 00716157  'LIVE%d:MODULES'
  0x00993F1C  at 007162b6  '%s\\\\modules\\\\%s'
  0x00993E50  at 00716329  'MODULES:'
  0x00993E50  at 007164e0  'MODULES:'
  0x0099C404  at 0071678e  '%s\\\\modules\\\\%s_dlg'
  0x00993E50  at 00716832  'MODULES:'
direct callees (22):
  0x00739CF0  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00711750  CExoResMan::AddKeyTable
  0x0058A1B0  (unnamed)
  0x00710810  (unnamed)
  0x00711ED0  CExoResMan::Exists
  0x00734270  CExoString::Format
  0x00711690  CExoResMan::AddResourceDirectory
  0x005564B0  (unnamed)
  0x00407800  (unnamed)
  0x00734010  CExoString::CStr
  0x00919671  (unnamed)
  0x00716DA0  CExoResMan::CacheResource
  0x007115C0  CExoResMan::AddEncapsulatedResourceFile
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x00556590  (unnamed)
struct offsets touched (top 4):
  +0x5C [dec 92]  x12  via EAX,ECX,EDX
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
  +0x54 [dec 84]  x1  via EAX
  +0x58 [dec 88]  x1  via EDX
```

## Card B0-6-47

GOG function: CSWSCreatureStats::ClearFeats  gog 0x006f47f0  callconv=__thiscall  param_size_bytes=0  gog_size=123  gog_retn=0
Candidate Steam address: 0x006b8370 (size 123, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b85f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f4700 CSWSCreatureStats::AddFeat -> 0x006b8280  gate_class=AUTO
- GOG-order neighbour after:  0x006f4870 CSWSCreatureStats::HasFeat -> 0x006b83f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005fb530 unnamed -> Steam ?  in candidate callees: n/a
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x18*} | Steam {0x4*,0xc*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b8370: 16-byte aligned: YES
  006b8370  55             PUSH EBP
  006b8371  8B EC          MOV EBP,ESP
  006b8373  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b8370
string literals (0):
direct callees (3):
  0x005933F0  (unnamed)
  0x0049B840  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x3  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b8370 FUN_006b8370 size=123 ret=0 callers=1 callees=3

void __fastcall FUN_006b8370(int param_1)

{
  int iVar1;
  
  FUN_005933f0(0);
  while (*(int *)(param_1 + 0x10) != 0) {
    iVar1 = *(int *)(*(int *)(param_1 + 0xc) + -4 + *(int *)(param_1 + 0x10) * 4);
    FUN_0049b840(*(int *)(param_1 + 0x10) + -1);
    if (iVar1 != 0) {
      FUN_009196fd(iVar1);
    }
  }
  return;
}
```

### decoy: 0x006b85f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b85f0: 16-byte aligned: YES
  006b85f0  55             PUSH EBP
  006b85f1  8B EC          MOV EBP,ESP
  006b85f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b85f0
string literals (0):
direct callees (3):
  0x005AA100  (unnamed)
  0x0049B840  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EAX
```

## Card B0-6-48

GOG function: CSWSEffectListHandler::OnApplyPureEvilPowers  gog 0x0074c480  callconv=(blank)  param_size_bytes=8  gog_size=1090  gog_retn=12
Candidate Steam address: 0x005a9730 (size 1136, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyPureGoodPowers
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a9ba0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074c010 CSWSEffectListHandler::OnApplyPureGoodPowers -> 0x005a9730  gate_class=REVIEW
- GOG-order neighbour after:  0x0074c8d0 CSWSEffectListHandler::IsEffectToIgnoreOnDeath -> 0x005a9ff0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0x30*,0x38*,0x8d*,0xab*,0x122,0x130*,0x132*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x38*,0x8d*,0x98,0x9c,0xab*,0x130*,0x132*,0x1198*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x5,0x8,0xc*,0xd,0x24*,0x28*,0x32,0x3e,0x43*,0x68,0x8c*} | Steam {0x2*,0x3*,0x4*,0xc*,0x24*,0x28*,0x3c,0x43*,0x70,0x8c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a9730: 16-byte aligned: YES
  005a9730  55             PUSH EBP
  005a9731  8B EC          MOV EBP,ESP
  005a9733  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a9730
string literals (0):
direct callees (7):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 10):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x132 [dec 306]  x1  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a9730 FUN_005a9730 size=1136 ret=12 callers=0 callees=7

/* WARNING: Removing unreachable block (ram,0x005a9931) */

undefined4 FUN_005a9730(int *param_1,undefined4 param_2)

{
  bool bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  int *piVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  int local_80;
  int local_7c;
  int local_78;
  int local_70;
  char local_25;
  byte local_15;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952b3c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  piVar5 = (int *)(**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (piVar5 == (int *)0x0) {
    uVar6 = 1;
  }
  else {
    iVar8 = piVar5[0x466 = byte +0x1198];
    if (iVar8 == 0) {
      uVar6 = 1;
    }
    else {
      bVar1 = false;
      iVar7 = FUN_0051c600(piVar5[1 = byte +0x4]);
      if ((iVar7 != 0) && (*(int *)(iVar7 + 0x38) == piVar5[1 = byte +0x4])) {
        bVar1 = true;
      }
      if (bVar1) {
        for (local_15 = 0; local_15 < *(byte *)(iVar8 + 0x8d); local_15 = local_15 + 1) {
          if (local_15 < *(byte *)(iVar8 + 0x8d)) {
            local_25 = *(char *)(iVar8 + 0xab + (uint)local_15 * 0x28);
          }
          else {
            local_25 = -1;
          }
          if (((local_25 == DAT_0099b19c) || (local_25 == DAT_0099b1a4)) ||
             (local_25 == DAT_0099b1a7)) {
            sVar2 = FUN_0057eca0();
            iVar7 = FUN_00919723(0x8c);
            local_8 = 0;
            if (iVar7 == 0) {
              local_70 = 0;
            }
            else {
              local_70 = FUN_005e4ac0(param_2);
            }
            local_8 = 0xffffffff;
            *(undefined2 *)(local_70 + 8) = 0x24;
            FUN_005e4e80(0,4);
            FUN_005e4e80(1,3);
            FUN_00544210(local_70,0,0);
            sVar3 = FUN_0057eca0();
            *(short *)(iVar8 + 0x130) = *(short *)(iVar8 + 0x130) + (sVar3 - sVar2);
          }
          else if (((local_25 == DAT_0099b19b) || (local_25 == DAT_0099b1a3)) ||
                  (local_25 == DAT_0099b1a6)) {
            iVar7 = FUN_00919723(0x8c);
            local_8 = 1;
            if (iVar7 == 0) {
              local_78 = 0;
            }
            else {
              local_78 = FUN_005e4ac0(param_2);
            }
/* ... 46 more lines truncated by --max-lines */
```

### decoy: 0x005a9ba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a9ba0: 16-byte aligned: YES
  005a9ba0  55             PUSH EBP
  005a9ba1  8B EC          MOV EBP,ESP
  005a9ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a9ba0
string literals (0):
direct callees (5):
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 9):
  +0x8 [dec 8]  x5  via EDX
  +0x130 [dec 304]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x122 [dec 290]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
  +0x132 [dec 306]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-6-49

GOG function: CSWSObject::GetPortraitId  gog 0x006638a0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x0053f000 (size 18, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::GetLastName
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053efd0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663870 CSWSObject::SetPortrait -> 0x0053efd0  gate_class=REVIEW
- GOG-order neighbour after:  0x006638c0 CSWSObject::GetFirstName  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
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

## Card B0-6-50

GOG function: CSWSCreatureStats::GetUseMonkAttackTables  gog 0x006e5460  callconv=(blank)  param_size_bytes=0  gog_size=277  gog_retn=0
Candidate Steam address: 0x006a8fe0 (size 277, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006a8f10:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e5390 CSWSCreatureStats::GetWeaponSpecialization -> 0x006a8f10  gate_class=REVIEW
- GOG-order neighbour after:  0x006e5580 CSWSCreatureStats::GetUnarmedDamageRoll -> 0x006a9100  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068f220 CSWSCreature::GetUseMonkAbilities -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x24*,0x8d*,0x53c*} | Steam {0x24*,0x8d*,0x53c*}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0xc*,0x10*,0x14*,0x20*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0xc*,0x10*,0x14*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006a8fe0: 16-byte aligned: YES
  006a8fe0  55             PUSH EBP
  006a8fe1  8B EC          MOV EBP,ESP
  006a8fe3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006a8fe0
string literals (0):
direct callees (1):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via ECX
  +0x53C [dec 1340]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006a8fe0 FUN_006a8fe0 size=277 ret=0 callers=4 callees=1

/* WARNING: Removing unreachable block (ram,0x006a90c0) */
/* WARNING: Removing unreachable block (ram,0x006a90c6) */
/* WARNING: Removing unreachable block (ram,0x006a90a2) */
/* WARNING: Removing unreachable block (ram,0x006a90a8) */
/* WARNING: Removing unreachable block (ram,0x006a905d) */
/* WARNING: Removing unreachable block (ram,0x006a9063) */
/* WARNING: Removing unreachable block (ram,0x006a908a) */
/* WARNING: Removing unreachable block (ram,0x006a906c) */
/* WARNING: Removing unreachable block (ram,0x006a9072) */
/* WARNING: Removing unreachable block (ram,0x006a907b) */
/* WARNING: Removing unreachable block (ram,0x006a9081) */
/* WARNING: Removing unreachable block (ram,0x006a90b1) */
/* WARNING: Removing unreachable block (ram,0x006a90b7) */
/* WARNING: Removing unreachable block (ram,0x006a90cf) */

undefined1 __fastcall FUN_006a8fe0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined1 local_14;
  int local_8;
  
  iVar1 = *(int *)(*(int *)(param_1 + 0x24) + 0x53c);
  iVar2 = FUN_005766f0();
  if (iVar2 == 1) {
    for (local_8 = 0; local_8 < (int)(uint)*(byte *)(param_1 + 0x8d); local_8 = local_8 + 1) {
    }
    if (iVar1 < 3) {
      local_14 = 4;
    }
    else {
      local_14 = 6;
    }
  }
  else if (iVar1 < 3) {
    local_14 = 4;
  }
  else {
    local_14 = 4;
  }
  return local_14;
}
```

### decoy: 0x006a8f10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a8f10: 16-byte aligned: YES
  006a8f10  55             PUSH EBP
  006a8f11  8B EC          MOV EBP,ESP
  006a8f13  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006a8f10
string literals (0):
direct callees (1):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via ECX
  +0x53C [dec 1340]  x1  via EDX
```

## Card B0-6-51

GOG function: CSWSObject::GetPortrait  gog 0x00663840  callconv=__thiscall  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x0053efa0 (size 47, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053efd0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: autolabel:GetRect
- GOG-order neighbour before: 0x00663830 CSWSObject::GetLockOrientationToObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00663870 CSWSObject::SetPortrait -> 0x0053efd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 2 common): GOG {0x4*,0x20*} | Steam {0x4*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053efa0: 16-byte aligned: YES
  0053efa0  55             PUSH EBP
  0053efa1  8B EC          MOV EBP,ESP
  0053efa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053efa0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053efa0 CMFCRibbonCategory::GetRect size=47 ret=4 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: class CRect __thiscall CMFCRibbonCategory::GetRect(void)const 
   
   Library: Visual Studio 2010 Debug */

undefined4 * __thiscall CMFCRibbonCategory::GetRect(CMFCRibbonCategory *this)

{
  undefined4 *in_stack_00000004;
  
  *in_stack_00000004 = *(undefined4 *)(this + 0x20);
  in_stack_00000004[1 = byte +0x4] = *(undefined4 *)(this + 0x24);
  in_stack_00000004[2 = byte +0x8] = *(undefined4 *)(this + 0x28);
  in_stack_00000004[3 = byte +0xc] = *(undefined4 *)(this + 0x2c);
  return in_stack_00000004;
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

## Card B0-6-52

GOG function: CSWSItem::RestoreUsedActiveProperties  gog 0x00667710  callconv=(blank)  param_size_bytes=2  gog_size=63  gog_retn=4
Candidate Steam address: 0x00603ee0 (size 63, retn 4)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00603dd0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667600 CSWSItem::GetPropertyByTypeExists -> 0x00603dd0  gate_class=REVIEW
- GOG-order neighbour after:  0x00667a90 CSWSItem::GetActiveProperty -> 0x00604260  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xa*,0x2b*} | Steam {0x4*,0x8*,0xa*,0x2b*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
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

## Card B0-6-53

GOG function: CSWSItem::GetPropertyByTypeExists  gog 0x00667600  callconv=(blank)  param_size_bytes=4  gog_size=245  gog_retn=4
Candidate Steam address: 0x00603dd0 (size 245, retn 4)
Candidate sources: mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::RestoreUsedActiveProperties
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00603b50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00667380 CSWSItem::GetUsedActivePropertyUsesLeft -> 0x00603b50  gate_class=AUTO
- GOG-order neighbour after:  0x00667710 CSWSItem::RestoreUsedActiveProperties -> 0x00603ee0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00667a90 CSWSItem::GetActiveProperty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x6*,0xc*,0x10*,0x288*} | Steam {0x6*,0xc*,0x10*,0x288*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xa*,0x20*,0xff*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xa*,0x20*,0xff*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
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

### decoy: 0x00603b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00603b50: 16-byte aligned: YES
  00603b50  55             PUSH EBP
  00603b51  8B EC          MOV EBP,ESP
  00603b53  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B0-6-54

GOG function: CSWSItem::AddUpgrade  gog 0x00679590  callconv=(blank)  param_size_bytes=6  gog_size=296  gog_retn=8
Candidate Steam address: 0x00615d60 (size 296, retn 8)
Candidate sources: branch+name_map:high:gog_item_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00615e90:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006794f0 CSWSItem::GetIcon -> 0x00615cc0  gate_class=AUTO
- GOG-order neighbour after:  0x006796c0 CSWSItem::RemoveUpgrade -> 0x00615e90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00667a00 unnamed -> Steam ?  in candidate callees: n/a
  0x00667ad0 CSWSItem::GetPassiveProperty -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x12*,0x28c*,0x294*} | Steam {0x12*,0x28c*,0x294*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x28*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x28*}

### abi
```
GOG param bytes: 6; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(6/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 6 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00615d60: 16-byte aligned: YES
  00615d60  55             PUSH EBP
  00615d61  8B EC          MOV EBP,ESP
  00615d63  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00615d60
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x0091DBA0  _memcpy
  0x006041D0  (unnamed)
  0x009196FD  operator delete
  0x006042A0  CSWSItem::GetPassiveProperty
struct offsets touched (top 3):
  +0x294 [dec 660]  x4  via ECX,EDX
  +0x28C [dec 652]  x3  via EAX,EDX
  +0x12 [dec 18]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00615d60 FUN_00615d60 size=296 ret=8 callers=2 callees=5

void __thiscall FUN_00615d60(int param_1,int param_2,short param_3)

{
  void *_Dst [GHIDRA-AUTO-LABEL: unreliable];
  int iVar1;
  uint uVar2;
  undefined4 local_10;
  
  if (param_2 != 0) {
    iVar1 = *(int *)(param_2 + 0x28c);
    uVar2 = iVar1 + *(int *)(param_1 + 0x28c);
    local_10 = *(int *)(param_1 + 0x28c);
    _Dst [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(-(uint)((int)((ulonglong)uVar2 * 0x1c >> 0x20) != 0) |
                                (uint)((ulonglong)uVar2 * 0x1c));
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](_Dst [GHIDRA-AUTO-LABEL: unreliable],*(void **)(param_1 + 0x294),local_10 * 0x1c);
    FUN_006041d0(uVar2);
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x294),_Dst [GHIDRA-AUTO-LABEL: unreliable],local_10 * 0x1c);
    FUN_009196fd(_Dst [GHIDRA-AUTO-LABEL: unreliable]);
    _memcpy [GHIDRA-AUTO-LABEL: unreliable]((void *)((uVar2 - iVar1) * 0x1c + *(int *)(param_1 + 0x294)),*(void **)(param_2 + 0x294)
            ,iVar1 * 0x1c);
    for (; local_10 < (int)uVar2; local_10 = local_10 + 1) {
      iVar1 = FUN_006042a0(local_10);
      *(short *)(iVar1 + 0x12) = param_3 + 1;
    }
  }
  return;
}
```

### decoy: 0x00615e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00615e90: 16-byte aligned: YES
  00615e90  55             PUSH EBP
  00615e91  8B EC          MOV EBP,ESP
  00615e93  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00615e90
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x006042A0  CSWSItem::GetPassiveProperty
  0x0091DBA0  _memcpy
  0x006041D0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x28C [dec 652]  x3  via EAX,EDX
  +0x294 [dec 660]  x3  via ECX
  +0x12 [dec 18]  x1  via EDX
```

## Card B0-6-55

GOG function: CSWSCreatureStats::GetBaseWillSavingThrow  gog 0x006f3a40  callconv=(blank)  param_size_bytes=0  gog_size=542  gog_retn=0
Candidate Steam address: 0x006b75c0 (size 542, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006b73a0:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f3820 CSWSCreatureStats::GetBaseFortSavingThrow -> 0x006b73a0  gate_class=REVIEW
- GOG-order neighbour after:  0x006f3c60 CSWSCreatureStats::GetBaseReflexSavingThrow -> 0x006b77e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0050f040 Global::GetWillSaveBonus -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x006e9770 CSWSCreatureStats::GetClassLevel -> Steam ?  in candidate callees: n/a
  0x006f4870 CSWSCreatureStats::HasFeat -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (1): '?SaveMult'
disp sets (* = in both, 9 common): GOG {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*} | Steam {0x24*,0x8d*,0xab*,0x124*,0x130*,0x154*,0x250*,0x251*,0x11ac*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*} | Steam {0x2*,0x3*,0xd*,0x15*,0x16*,0x28*,0x54*,0x238*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006b75c0: 16-byte aligned: YES
  006b75c0  55             PUSH EBP
  006b75c1  8B EC          MOV EBP,ESP
  006b75c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006b75c0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0990  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006b75c0 FUN_006b75c0 size=542 ret=0 callers=1 callees=7

char __fastcall FUN_006b75c0(int param_1)

{
  undefined1 uVar1;
  char cVar2;
  uint uVar3;
  int iVar4;
  byte local_37;
  undefined1 local_34 [8];
  undefined4 local_2c;
  char local_25;
  undefined1 local_24 [4];
  uint local_20;
  int local_1c;
  char local_15;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095c1d8;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_15 = '\0';
  for (local_14 = 0; (int)local_14 < (int)(uint)*(byte *)(param_1 + 0x8d); local_14 = local_14 + 1)
  {
    if ((local_14 & 0xff) < (uint)*(byte *)(param_1 + 0x8d)) {
      local_37 = *(byte *)(param_1 + 0xab + (local_14 & 0xff) * 0x28);
    }
    else {
      local_37 = 0xff;
    }
    local_1c = (uint)local_37 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124);
    if (local_1c != 0) {
      uVar1 = FUN_006ad2f0(local_14 & 0xff,0);
      cVar2 = FUN_006f0990(uVar1);
      local_15 = local_15 + cVar2;
    }
  }
  if (*(int *)(*(int *)(param_1 + 0x24) + 0x11ac) == 0) {
    local_20 = (uint)*(byte *)(*(int *)(param_1 + 0x24) + 0x250);
    if (local_20 != 0) {
      local_2c = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x154);
      FUN_00733570("SaveMult");
      local_8 = 0;
      FUN_0071b670(local_20,local_34,local_24);
      local_8 = 0xffffffff;
      FUN_00733780();
      if (*(char *)(*(int *)(param_1 + 0x24) + 0x251) == '\0') {
        local_25 = '\x01';
      }
      else {
        local_25 = *(char *)(*(int *)(param_1 + 0x24) + 0x251);
      }
      cVar2 = FUN_0091c860(uVar3,(int)local_25);
      local_15 = local_15 + cVar2;
    }
  }
  iVar4 = FUN_006b83f0(0x16);
  if (iVar4 == 0) {
    iVar4 = FUN_006b83f0(0x15);
    if (iVar4 == 0) {
      iVar4 = FUN_006b83f0(0xd);
      if (iVar4 != 0) {
        local_15 = local_15 + '\x01';
      }
    }
    else {
      local_15 = local_15 + '\x02';
    }
  }
  else {
    local_15 = local_15 + '\x03';
  }
  ExceptionList = local_10;
  return local_15;
}
```

### decoy: 0x006b73a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b73a0: 16-byte aligned: YES
  006b73a0  55             PUSH EBP
  006b73a1  8B EC          MOV EBP,ESP
  006b73a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006b73a0
string literals (0):
direct callees (7):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F0920  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX,ECX
  +0x124 [dec 292]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x250 [dec 592]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B0-6-56

GOG function: CSWSItem::AddSubraceRequirements  gog 0x0066d900  callconv=(blank)  param_size_bytes=4  gog_size=1056  gog_retn=4
Candidate Steam address: 0x0060a0d0 (size 1056, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_item_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00609cb0:STR0/CALLEE0/SIG0.817/SRV0/VT0
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
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x11*,0x130*,0x28c*,0x2d4*} | Steam {0x2*,0x4*,0x11*,0x130*,0x28c*,0x2d4*}
imm sets (* = in both, 16 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3d*,0xe0*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3d*,0xe0*,...(+2)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0060a0d0: 16-byte aligned: YES
  0060a0d0  55             PUSH EBP
  0060a0d1  8B EC          MOV EBP,ESP
  0060a0d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0060a0d0 FUN_0060a0d0 size=1056 ret=4 callers=1 callees=9

void __fastcall FUN_0060a0d0(int param_1)

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
  undefined4 local_3c;
  int local_38;
  uint local_2c;
  int local_28;
  undefined1 local_1c [8];
  short *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00955676;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,2);
  local_38 = 0;
  iVar2 = FUN_00603f60(0x3d,0);
  if (iVar2 != 0) {
    for (local_28 = 0; local_28 < *(int *)(param_1 + 0x28c); local_28 = local_28 + 1) {
      local_14 = (short *)FUN_006042a0(local_28);
      if ((*(byte *)((int)local_14 + 0x11) == 0xff) ||
         ((1 << (*(byte *)((int)local_14 + 0x11) & 0x1f) & *(uint *)(param_1 + 0x2d4)) != 0)) {
        bVar1 = true;
      }
      else {
        bVar1 = false;
      }
      if ((bVar1) && (*local_14 == 0x3d)) {
        local_2c = (uint)(ushort)local_14[1];
        if (local_38 == 0) {
          FUN_00733570(&DAT_00997608);
          local_8._0_1_ = 3;
          uVar3 = FUN_007408f0(local_44,0xbb8d);
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
        FUN_0071bcf0(local_2c,local_64,&local_3c);
        local_8._0_1_ = 2;
        FUN_00733780();
        uVar3 = FUN_007408f0(local_6c,local_3c);
        local_8._0_1_ = 8;
        FUN_007337c0(uVar3);
/* ... 38 more lines truncated by --max-lines */
```

### decoy: 0x00609cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00609cb0: 16-byte aligned: YES
  00609cb0  55             PUSH EBP
  00609cb1  8B EC          MOV EBP,ESP
  00609cb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B0-6-57

GOG function: CSWSCreatureStats::GetSpellUsesLeft  gog 0x006f53c0  callconv=(blank)  param_size_bytes=5  gog_size=620  gog_retn=16
Candidate Steam address: 0x006b8f40 (size 620, retn 16)
Candidate sources: branch+name_map:high:gog_creaturestats_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006b91b0:STR0/CALLEE0/SIG0.779/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006f5360 CSWSCreatureStats::UnReadySpell -> 0x006b8ee0  gate_class=AUTO
- GOG-order neighbour after:  0x006f5630 CSWSCreatureStats::GetCreatureTalentRandomSpecialAbility -> 0x006b91b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004ed450 CSWSpellArray::GetSpell -> Steam ?  in candidate callees: n/a
  0x004f0120 CSWSpell::GetAdjustedForcePoints -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00705890 CSWSArea::GetRoomForceRating -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x24*,0x8d*,0x9c*,0xf8*,0x104*,0x130*,0x132*,0x18a*,0x1110*,0x1198*,0x11ac*} | Steam {0x4*,0x8*,0x24*,0x8d*,0x9c*,0xf8*,0x104*,0x130*,0x132*,0x18a*,0x1110*,0x1198*,0x11ac*}
imm sets (* = in both, 8 common): GOG {0x2*,0x10*,0x1e*,0x28*,0x32*,0x64*,0x94*,0x148*} | Steam {0x2*,0x10*,0x1e*,0x28*,0x32*,0x64*,0x94*,0x148*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x006b8f40: 16-byte aligned: YES
  006b8f40  55             PUSH EBP
  006b8f41  8B EC          MOV EBP,ESP
  006b8f43  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006b8f40
string literals (0):
direct callees (5):
  0x006C1450  (unnamed)
  0x005453C0  CSWSObject::GetArea
  0x0054B4F0  CSWSArea::GetRoomForceRating
  0x006C4120  (unnamed)
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 10):
  +0x24 [dec 36]  x8  via EAX,ECX,EDX
  +0x130 [dec 304]  x4  via EAX,EDX
  +0x132 [dec 306]  x2  via ECX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x8D [dec 141]  x1  via ECX
  +0x9C [dec 156]  x1  via EDX
  +0xF8 [dec 248]  x1  via ECX
  +0x104 [dec 260]  x1  via EDX
  +0x18A [dec 394]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006b8f40 FUN_006b8f40 size=620 ret=16 callers=6 callees=5

/* WARNING: Removing unreachable block (ram,0x006b910b) */
/* WARNING: Removing unreachable block (ram,0x006b916e) */

undefined1 __thiscall FUN_006b8f40(int param_1,undefined4 param_2,byte param_3)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  short local_64;
  int local_34;
  undefined4 local_24;
  int local_18;
  undefined1 local_12;
  int local_10;
  
  if (param_3 < *(byte *)(param_1 + 0x8d)) {
    iVar2 = FUN_006c1450(param_2);
    if (iVar2 == 0) {
      local_12 = 0;
    }
    else {
      iVar2 = FUN_005453c0();
      local_24 = 0;
      if (iVar2 != 0) {
        local_24 = FUN_0054b4f0(*(int *)(param_1 + 0x24) + 0x94);
      }
      local_12 = 0;
      uVar3 = FUN_006c4120((int)*(short *)(param_1 + 0x18a),
                           *(undefined4 *)(*(int *)(param_1 + 0x24) + 0x11ac),local_24,
                           *(undefined4 *)(*(int *)(param_1 + 0x24) + 0x1110),
                           *(undefined1 *)(*(int *)(*(int *)(param_1 + 0x24) + 0x1198) + 0xf8));
      local_18 = -1;
      for (local_10 = 0; local_10 < *(int *)(*(int *)(param_1 + 0x24) + 0x14c);
          local_10 = local_10 + 1) {
        if (*(ushort *)(*(int *)(*(int *)(*(int *)(param_1 + 0x24) + 0x148) + local_10 * 4) + 8) ==
            DAT_009960d0) {
          local_18 = FUN_005e4e60(0);
          break;
        }
      }
      if (local_18 == -1) {
        if (uVar3 <= (uint)(int)(short)(*(short *)(param_1 + 0x130) + *(short *)(param_1 + 0x132)))
        {
          local_12 = 1;
        }
      }
      else {
        local_34 = 0;
        if (local_18 == 0) {
          local_34 = 0x32;
        }
        else if (local_18 == 1) {
          local_34 = 0x28;
        }
        else if (local_18 == 2) {
          local_34 = 0x1e;
        }
        iVar2 = (int)(uVar3 * local_34) / 100;
        local_64 = *(short *)(param_1 + 0x130) + *(short *)(param_1 + 0x132);
        if ((uVar3 - iVar2 <= (uint)(int)local_64) &&
           (sVar1 = (**(code **)(**(int **)(param_1 + 0x24) + 0x9c))(0), 0 < sVar1 - iVar2)) {
          local_12 = 1;
        }
      }
    }
  }
  else {
    local_12 = 0;
  }
  return local_12;
}
```

### decoy: 0x006b91b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006b91b0: 16-byte aligned: YES
  006b91b0  55             PUSH EBP
  006b91b1  8B EC          MOV EBP,ESP
  006b91b3  81 EC 90 00 00 00 SUB ESP,0x90
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006b91b0
string literals (0):
direct callees (6):
  0x006C1450  (unnamed)
  0x005453C0  CSWSObject::GetArea
  0x0054B4F0  CSWSArea::GetRoomForceRating
  0x005E4E60  CGameEffect::GetInteger
  0x006C4120  (unnamed)
  0x0054A610  CSWSObject::SetCurrentHitPoints
struct offsets touched (top 10):
  +0x24 [dec 36]  x18  via EAX,ECX,EDX
  +0x130 [dec 304]  x6  via EAX,ECX,EDX
  +0x132 [dec 306]  x5  via EAX,ECX,EDX
  +0xE8 [dec 232]  x3  via EAX,ECX
  +0x9C [dec 156]  x2  via EDX
  +0xF8 [dec 248]  x2  via ECX,EDX
  +0x18A [dec 394]  x2  via EAX,ECX
  +0x1110 [dec 4368]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x11AC [dec 4524]  x2  via ECX,EDX
```

## Card B0-6-58

GOG function: CSWSItem::AddGenderRequirements  gog 0x0066d4e0  callconv=(blank)  param_size_bytes=4  gog_size=1053  gog_retn=4
Candidate Steam address: 0x00609cb0 (size 1053, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_item_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0060a0d0:STR0/CALLEE0/SIG0.817/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0066c940 CSWSItem::AddFeatRequirements -> 0x00609110  gate_class=AUTO
- GOG-order neighbour after:  0x0066d900 CSWSItem::AddSubraceRequirements -> 0x0060a0d0  gate_class=REVIEW

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
disp sets (* = in both, 7 common): GOG {0x2*,0x4*,0x11*,0x14*,0x130*,0x28c*,0x2d4*} | Steam {0x2*,0x4*,0x11*,0x14*,0x130*,0x28c*,0x2d4*}
imm sets (* = in both, 16 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0xe0*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0xe0*,...(+2)}

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

## Card B0-6-59

GOG function: CSWSCreatureStats::GetTotalCHABonus  gog 0x006f7000  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006bab80 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006bab30:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6fb0 CSWSCreatureStats::GetTotalWISBonus -> 0x006bab30  gate_class=REVIEW
- GOG-order neighbour after:  0x006f7050 CSWSCreatureStats::SetSpellLikeAbilityReady -> 0x006babd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 4 common): GOG {0x4*,0x5*,0x10*,0xff*} | Steam {0x4*,0x5*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006bab80: 16-byte aligned: YES
  006bab80  55             PUSH EBP
  006bab81  8B EC          MOV EBP,ESP
  006bab83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006bab80
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006bab80 FUN_006bab80 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006bab80(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,5,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006bab30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006bab30: 16-byte aligned: YES
  006bab30  55             PUSH EBP
  006bab31  8B EC          MOV EBP,ESP
  006bab33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006bab30
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

## Card B0-6-60

GOG function: CSWSCreatureStats::GetTotalCONBonus  gog 0x006f6f10  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=0
Candidate Steam address: 0x006baa90 (size 70, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:gog_creaturestats_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006baa40:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006f6ec0 CSWSCreatureStats::GetTotalDEXBonus -> 0x006baa40  gate_class=REVIEW
- GOG-order neighbour after:  0x006f6f60 CSWSCreatureStats::GetTotalINTBonus -> 0x006baae0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x10*,0xff*} | Steam {0x2*,0x4*,0x10*,0xff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006baa90: 16-byte aligned: YES
  006baa90  55             PUSH EBP
  006baa91  8B EC          MOV EBP,ESP
  006baa93  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006baa90
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006baa90 FUN_006baa90 size=70 ret=0 callers=4 callees=1

undefined4 __fastcall FUN_006baa90(int param_1)

{
  undefined4 uVar1;
  undefined4 local_c[2 = byte +0x8];
  
  local_c[0] = 0;
  uVar1 = FUN_00582010(4,0,local_c,0,0,0,0,0xff,2,0,0,param_1,*(undefined4 *)(param_1 + 0x24));
  return uVar1;
}
```

### decoy: 0x006baa40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006baa40: 16-byte aligned: YES
  006baa40  55             PUSH EBP
  006baa41  8B EC          MOV EBP,ESP
  006baa43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006baa40
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via EAX
```
