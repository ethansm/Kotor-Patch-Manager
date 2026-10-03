# Review cards B15 chunk 3

Answer each card with one TSV row in `steam_db_batches/B15/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B15-3-01

GOG function: Global::ExecuteCommandGetGold  gog 0x00793430  callconv=__thiscall  param_size_bytes=8  gog_size=248  gog_retn=8
Candidate Steam address: 0x0067df50 (size 165, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067de50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007933b0 Global::ExecuteCommandGetGoingToBeAttackedBy -> 0x0067ddd0  gate_class=AUTO
- GOG-order neighbour after:  0x00793530 Global::ExecuteCommandGetGoldPieceValue -> 0x0067df50  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [418 GetGold, 771 GetItemComponent] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x006693b0 CItemRepository::FindItemWithBaseItemType -> Steam 0x00605b80  in candidate callees: no
  0x00699e10 CSWSCreature::GetGold -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x2cc,0x3c4} | Steam {0x8*,0x38}
imm sets (* = in both, 3 common): GOG {0x8*,0x17,0x30,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067df50: 16-byte aligned: YES
  0067df50  55             PUSH EBP
  0067df51  8B EC          MOV EBP,ESP
  0067df53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067df50
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00606A80  CSWSItem::GetCost
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x38 [dec 56]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067df50 FUN_0067df50 size=165 ret=8 callers=0 callees=5

undefined4 FUN_0067df50(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int **ppiVar4;
  undefined4 extraout_var;
  undefined4 local_10;
  int *local_c;
  undefined4 local_8;
  
  local_8 = 0;
  iVar2 = FUN_006fdaf0(&local_10);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    ppiVar4 = &local_c;
    FUN_0051c080(local_10,ppiVar4,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
    cVar1 = FUN_0053dfb0(local_10,ppiVar4);
    if (cVar1 == DAT_0099447f) {
      iVar2 = (**(code **)(*local_c + 0x38))();
      if (iVar2 != 0) {
        (**(code **)(*local_c + 0x38))();
        local_8 = FUN_00606a80();
      }
    }
    iVar2 = FUN_006fd9c0(local_8);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x0067de50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067de50: 16-byte aligned: YES
  0067de50  55             PUSH EBP
  0067de51  8B EC          MOV EBP,ESP
  0067de53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067de50
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005812E0  CSWSCreature::GetGold
  0x0051C160  ServerApp::GetPlaceableById
  0x00605B80  CItemRepository::FindItemWithBaseItemType
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x3C4 [dec 964]  x2  via EAX,ECX
  +0x2CC [dec 716]  x1  via EDX
```

## Card B15-3-02

GOG function: Global::ExecuteCommandTestString  gog 0x007aa860  callconv=__thiscall  param_size_bytes=8  gog_size=340  gog_retn=8
Candidate Steam address: 0x00695210 (size 340, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00695370:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aa790 Global::ExecuteCommandTimeConversions -> 0x00695140  gate_class=AUTO
- GOG-order neighbour after:  0x007aa9c0 Global::ExecuteCommandUnequipItem -> 0x00695370  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [177 TestStringAgainstPattern] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x007f12b0 unnamed -> Steam ?  in candidate callees: n/a
  0x007f1330 unnamed -> Steam ?  in candidate callees: n/a
  0x007f1520 CSWSExpression::ParseString -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x4c*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x4c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00695210: 16-byte aligned: YES
  00695210  55             PUSH EBP
  00695211  8B EC          MOV EBP,ESP
  00695213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00695210
string literals (0):
direct callees (9):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006A2690  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x006A2900  (unnamed)
  0x006A34A0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x006A2710  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00695210 FUN_00695210 size=340 ret=8 callers=0 callees=9

undefined4 FUN_00695210(void)

{
  int iVar1;
  undefined1 *puStack_68;
  undefined1 *puStack_64;
  uint uStack_60;
  undefined4 local_5c;
  undefined4 local_54;
  undefined4 local_50;
  undefined1 *local_4c;
  undefined4 local_48;
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  undefined4 local_34;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ad98;
  local_10 = ExceptionList;
  uStack_60 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  puStack_64 = (undefined1 *)0x695240;
  FUN_00733540();
  local_8 = 0;
  puStack_64 = (undefined1 *)0x69524f;
  FUN_00733540();
  local_8._0_1_ = 1;
  puStack_64 = local_3c;
  puStack_68 = (undefined1 *)0x695262;
  iVar1 = FUN_006fda70();
  if (iVar1 != 0) {
    puStack_64 = local_44;
    puStack_68 = (undefined1 *)0x695275;
    iVar1 = FUN_006fda70();
    if (iVar1 != 0) {
      puStack_64 = (undefined1 *)0x6952ab;
      FUN_006a2690();
      local_8._0_1_ = 2;
      local_4c = (undefined1 *)&puStack_68;
      local_5c = FUN_007335f0(local_3c);
      FUN_006a2900();
      puStack_64 = (undefined1 *)0x1;
      puStack_68 = local_44;
      puStack_64 = (undefined1 *)FUN_006a34a0();
      puStack_68 = (undefined1 *)0x6952eb;
      local_34 = puStack_64;
      iVar1 = FUN_006fd9c0();
      if (iVar1 == 0) {
        local_50 = 0xfffff830;
        local_8._0_1_ = 1;
        puStack_64 = (undefined1 *)0x695302;
        FUN_006a2710();
        local_8 = (uint)local_8._1_3_ << 8;
        puStack_64 = (undefined1 *)0x69530e;
        FUN_00733780();
        local_8 = 0xffffffff;
        puStack_64 = (undefined1 *)0x69531d;
        FUN_00733780();
        ExceptionList = local_10;
        return local_50;
      }
      local_54 = 0;
      local_8._0_1_ = 1;
      puStack_64 = (undefined1 *)0x695335;
      FUN_006a2710();
      local_8 = (uint)local_8._1_3_ << 8;
      puStack_64 = (undefined1 *)0x695341;
      FUN_00733780();
      local_8 = 0xffffffff;
      puStack_64 = (undefined1 *)0x695350;
      FUN_00733780();
      ExceptionList = local_10;
      return local_54;
    }
  }
  local_48 = 0xfffff82f;
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x00695370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695370: 16-byte aligned: YES
  00695370  55             PUSH EBP
  00695371  8B EC          MOV EBP,ESP
  00695373  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695370
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00567490  CSWSCreature::AddUnequipActions
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
```

## Card B15-3-03

GOG function: Global::ExecuteCommandGetStringLeft  gog 0x0079e2c0  callconv=__thiscall  param_size_bytes=8  gog_size=354  gog_retn=8
Candidate Steam address: 0x00688c70 (size 354, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00688b60:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e1b0 Global::ExecuteCommandGetStrByStrRef -> 0x00688b60  gate_class=REVIEW
- GOG-order neighbour after:  0x0079e430 Global::ExecuteCommandGetStringLength -> 0x00688de0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [63 GetStringLeft] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606610 CExoString::Left -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00688c70: 16-byte aligned: YES
  00688c70  55             PUSH EBP
  00688c71  8B EC          MOV EBP,ESP
  00688c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00688c70
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00688c70 FUN_00688c70 size=354 ret=8 callers=0 callees=7

undefined4 FUN_00688c70(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_34 [8];
  undefined4 local_2c;
  undefined4 local_28;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a628;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  iVar1 = FUN_006fda70(local_24);
  if (iVar1 == 0) {
    local_28 = 0xfffff82f;
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    iVar1 = FUN_006fd9a0(&local_14);
    if (iVar1 == 0) {
      local_2c = 0xfffff82f;
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_28 = local_2c;
    }
    else {
      uVar2 = FUN_00734500(local_34,local_14);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar2);
      local_8._0_1_ = 1;
      FUN_00733780();
      iVar1 = FUN_006fda90(local_1c);
      if (iVar1 == 0) {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        local_28 = 0xfffff830;
      }
      else {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        local_28 = 0;
      }
    }
  }
  ExceptionList = local_10;
  return local_28;
}
```

### decoy: 0x00688b60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00688b60: 16-byte aligned: YES
  00688b60  55             PUSH EBP
  00688b61  8B EC          MOV EBP,ESP
  00688b63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00688b60
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x0071DF90  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

## Card B15-3-04

GOG function: Global::ExecuteCommandGetLastAttacker  gog 0x00796e60  callconv=__thiscall  param_size_bytes=8  gog_size=168  gog_retn=8
Candidate Steam address: 0x00681890 (size 168, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00681940:STR0/CALLEE0/SIG0.458/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00796bc0 Global::ExecuteCommandGetJournalQuestExperience -> 0x006815e0  gate_class=AUTO
- GOG-order neighbour after:  0x00796f10 Global::ExecuteCommandGetLastHostileTarget -> 0x00681940  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [36 GetLastAttacker] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x10*,0x180*} | Steam {0x8*,0x10*,0x180*}
imm sets (* = in both, 5 common): GOG {0x8*,0x18*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x18*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00681890: 16-byte aligned: YES
  00681890  55             PUSH EBP
  00681891  8B EC          MOV EBP,ESP
  00681893  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00681890
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 3):
  +0x10 [dec 16]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x180 [dec 384]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00681890 FUN_00681890 size=168 ret=8 callers=0 callees=3

undefined4 FUN_00681890(void)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_10;
  int *local_c;
  undefined4 local_8;
  
  uVar4 = 0;
  local_8 = 0x7f000000;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar4 = 0xfffff82f;
  }
  else {
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar2 = extraout_var;
    local_c = (int *)FUN_0051c0a0(local_10);
    if (local_c != (int *)0x0) {
      iVar1 = (**(code **)(*local_c + 0x10))(uVar2,uVar3,uVar4);
      if (iVar1 != 0) {
        iVar1 = (**(code **)(*local_c + 0x10))();
        local_8 = *(undefined4 *)(iVar1 + 0x180);
      }
    }
    iVar1 = FUN_006fdb10(local_8);
    if (iVar1 == 0) {
      uVar4 = 0xfffff830;
    }
    else {
      uVar4 = 0;
    }
  }
  return uVar4;
}
```

### decoy: 0x00681940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00681940: 16-byte aligned: YES
  00681940  55             PUSH EBP
  00681941  8B EC          MOV EBP,ESP
  00681943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00681940
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x580 [dec 1408]  x1  via ECX
```

## Card B15-3-05

GOG function: Global::ExecuteCommandGetPlaceableIllumination  gog 0x0079cd60  callconv=__thiscall  param_size_bytes=8  gog_size=120  gog_retn=8
Candidate Steam address: 0x00687710 (size 120, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetPartyLeader | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: GetLightIsOn 0x5f6570); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006874f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079cce0 Global::ExecuteCommandGetPartyLeader -> 0x0069b7c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0079cde0 Global::ExecuteCommandGetPlotFlag -> 0x00687790  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [545 GetPlaceableIllumination] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x00770910 CSWSPlaceable::GetLightIsOn -> Steam 0x005f6570  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x14*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00687710: 16-byte aligned: YES
  00687710  55             PUSH EBP
  00687711  8B EC          MOV EBP,ESP
  00687713  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00687710
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C160  ServerApp::GetPlaceableById
  0x005F6570  CSWSPlaceable::GetLightIsOn
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00687710 FUN_00687710 size=120 ret=8 callers=0 callees=4

undefined4 FUN_00687710(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 0;
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar3 = extraout_var;
    iVar1 = FUN_0051c160(local_c);
    if (iVar1 != 0) {
      local_8 = FUN_005f6570(uVar3,uVar2);
    }
    iVar1 = FUN_006fd9c0(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006874f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006874f0: 16-byte aligned: YES
  006874f0  55             PUSH EBP
  006874f1  8B EC          MOV EBP,ESP
  006874f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006874f0
string literals (0):
direct callees (5):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-06

GOG function: Global::ExecuteCommandSetSpeedBlurEffect  gog 0x007a7ee0  callconv=__thiscall  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x00692890 (size 117, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006927f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a7e40 Global::ExecuteCommandSetReturnStrref -> 0x006927f0  gate_class=AUTO
- GOG-order neighbour after:  0x007a7f60 Global::ExecuteCommandSetSubType -> 0x00692910  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [563 SWMG_SetSpeedBlurEffect] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x0086ad40 Global::AurEnableSpeedBlur -> Steam ?  in candidate callees: n/a
  0x0086ad60 Global::AurDisableSpeedBlur -> Steam ?  in candidate callees: n/a
  0x0086ae80 Global::AurSetSpeedBlurRatio -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x8*,0xc*,0xfffff82f*} | Steam {0x2*,0x4*,0x8*,0xc*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00692890: 16-byte aligned: YES
  00692890  55             PUSH EBP
  00692891  8B EC          MOV EBP,ESP
  00692893  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00692890
string literals (0):
direct callees (5):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x00478FA0  (unnamed)
  0x00478E60  Global::AurEnableSpeedBlur
  0x00478E80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00692890 FUN_00692890 size=117 ret=8 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00692890(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (1 < param_2) {
      local_c = _DAT_009875b4;
      uVar2 = extraout_var;
      iVar1 = FUN_006fd9e0(&local_c);
      if (iVar1 == 0) {
        return 0xfffff82f;
      }
      FUN_00478fa0(local_c,uVar2);
    }
    if (local_8 == 0) {
      FUN_00478e80();
    }
    else {
      FUN_00478e60();
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x006927f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006927f0: 16-byte aligned: YES
  006927f0  55             PUSH EBP
  006927f1  8B EC          MOV EBP,ESP
  006927f3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006927f0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FEC90  CSWPartyTable::SetReturnStrref
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-07

GOG function: Global::ExecuteCommandGetIsNight  gog 0x007956b0  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x006800d0 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067fdc0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007955e0 Global::ExecuteCommandGetIsInCombat -> 0x00680000  gate_class=AUTO
- GOG-order neighbour after:  0x00795720 Global::ExecuteCommandGetIsOpen -> 0x00680140  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [406 GetIsNight] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x1dc*} | Steam {0x8*,0x1dc*}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0x14*,0xfffff830*} | Steam {0x2*,0x8*,0x14*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006800d0: 16-byte aligned: YES
  006800d0  55             PUSH EBP
  006800d1  8B EC          MOV EBP,ESP
  006800d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006800d0
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006800d0 FUN_006800d0 size=106 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_006800d0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_0051bd90(param_1);
  if ((iVar1 != 0) && (*(char *)(iVar1 + 0x1dc) == '\x02')) {
    local_8 = 1;
  }
  iVar1 = FUN_006fd9c0(local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0067fdc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067fdc0: 16-byte aligned: YES
  0067fdc0  55             PUSH EBP
  0067fdc1  8B EC          MOV EBP,ESP
  0067fdc3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067fdc0
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

## Card B15-3-08

GOG function: Global::ExecuteCommandGetMatchedStringsNum  gog 0x00798d10  callconv=__thiscall  param_size_bytes=8  gog_size=123  gog_retn=8
Candidate Steam address: 0x006837c0 (size 141, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00683740:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00798c90 Global::ExecuteCommandGetMatchedStringsCount -> 0x006836c0  gate_class=AUTO
- GOG-order neighbour after:  0x00798d90 Global::ExecuteCommandGetMetaMagicFeat -> 0x006837c0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [195 GetListenPatternNumber] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x1bc} | Steam {0x8*,0x10,0x1f9}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff830*} | Steam {0x8*,0x14*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006837c0: 16-byte aligned: YES
  006837c0  55             PUSH EBP
  006837c1  8B EC          MOV EBP,ESP
  006837c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006837c0
string literals (0):
direct callees (2):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0x1F9 [dec 505]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006837c0 FUN_006837c0 size=141 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_006837c0(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_d;
  undefined4 local_c;
  
  local_c = 0;
  local_d = 0xff;
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  piVar1 = (int *)FUN_0051c0a0(*(undefined4 *)(param_1 + 8));
  if (piVar1 != (int *)0x0) {
    iVar2 = (**(code **)(*piVar1 + 0x10))(param_1,uVar3);
    if (iVar2 != 0) {
      local_c = (**(code **)(*piVar1 + 0x10))();
    }
  }
  if (local_c != 0) {
    local_d = *(undefined1 *)(local_c + 0x1f9);
  }
  iVar2 = FUN_006fd9c0(local_d);
  if (iVar2 == 0) {
    uVar3 = 0xfffff830;
  }
  else {
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x00683740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00683740: 16-byte aligned: YES
  00683740  55             PUSH EBP
  00683741  8B EC          MOV EBP,ESP
  00683743  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00683740
string literals (0):
direct callees (3):
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX
  +0x1BC [dec 444]  x1  via ECX
```

## Card B15-3-09

GOG function: Global::ExecuteCommandGetTimeSecond  gog 0x0079ee10  callconv=__thiscall  param_size_bytes=8  gog_size=72  gog_retn=8
Candidate Steam address: 0x006897c0 (size 72, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetWorldTimeSecond 0x51b180)
- best decoy (neighbour/other candidate) scores: 0x00689770:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079edc0 Global::ExecuteCommandGetTimeMinute -> 0x00689770  gate_class=REVIEW
- GOG-order neighbour after:  0x0079ee60 Global::ExecuteCommandGetTotalDamageDealt -> 0x00689810  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [18 GetTimeSecond] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064bd90 CServerExoApp::GetWorldTimer -> Steam 0x0051c370  in candidate callees: yes
  0x0080b660 CWorldTimer::GetWorldTimeSecond -> Steam 0x0051b180  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006897c0: 16-byte aligned: YES
  006897c0  55             PUSH EBP
  006897c1  8B EC          MOV EBP,ESP
  006897c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006897c0
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B180  CWorldTimer::GetWorldTimeSecond
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006897c0 FUN_006897c0 size=72 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_006897c0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  FUN_0051c370(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  uVar1 = FUN_0051b180();
  iVar2 = FUN_006fd9c0(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0xfffff830;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00689770 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689770: 16-byte aligned: YES
  00689770  55             PUSH EBP
  00689771  8B EC          MOV EBP,ESP
  00689773  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00689770
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B140  CWorldTimer::GetWorldTimeMinute
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-10

GOG function: Global::ExecuteCommandSoundObjectPlay  gog 0x007a88f0  callconv=__thiscall  param_size_bytes=8  gog_size=84  gog_retn=8
Candidate Steam address: 0x006932a0 (size 84, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00693520:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a8870 Global::ExecuteCommandSoundObjectGetVolume -> 0x00693220  gate_class=AUTO
- GOG-order neighbour after:  0x007a8950 Global::ExecuteCommandSoundObjectSetFixedVariance -> 0x00693300  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [413 SoundObjectPlay] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007f7410 CSWSSoundObject::Play -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0xfffff82f*} | Steam {0x8*,0x10*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006932a0: 16-byte aligned: YES
  006932a0  55             PUSH EBP
  006932a1  8B EC          MOV EBP,ESP
  006932a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006932a0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C220  (unnamed)
  0x0062A620  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006932a0 FUN_006932a0 size=84 ret=8 callers=0 callees=3

undefined4 FUN_006932a0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar3 = extraout_var;
    iVar1 = FUN_0051c220(local_8);
    if (iVar1 != 0) {
      FUN_0062a620(uVar3,uVar2);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00693520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00693520: 16-byte aligned: YES
  00693520  55             PUSH EBP
  00693521  8B EC          MOV EBP,ESP
  00693523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00693520
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C220  (unnamed)
  0x0062A710  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-3-11

GOG function: Global::ExecuteCommandRemoveEffectByExactMatch  gog 0x007a5170  callconv=__thiscall  param_size_bytes=8  gog_size=534  gog_retn=8
Candidate Steam address: 0x0068fb20 (size 534, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0068f7e0:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a5000 Global::ExecuteCommandRemoveEffectByID -> 0x0068f9b0  gate_class=AUTO
- GOG-order neighbour after:  0x007a5390 Global::ExecuteCommandRemoveFromParty -> 0x0068fd40  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [868 RemoveEffectByExactMatch] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x20*} | Steam {0x4*,0x8*,0x10*,0x20*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x48*,0x148*,0xfffff82f*} | Steam {0x4*,0x8*,0x48*,0x148*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068fb20: 16-byte aligned: YES
  0068fb20  55             PUSH EBP
  0068fb21  8B EC          MOV EBP,ESP
  0068fb23  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068fb20
string literals (0):
direct callees (8):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E4E60  CGameEffect::GetInteger
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x20 [dec 32]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0068fb20 FUN_0068fb20 size=534 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_0068fb20(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  uint local_1c;
  int local_14;
  int local_10;
  int *local_c;
  undefined4 local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fdab0(0,&local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      local_c = (int *)FUN_0051c0a0(local_8);
      if ((local_c != (int *)0x0) && (iVar1 = (**(code **)(*local_c + 0x10))(), iVar1 != 0)) {
        iVar1 = (**(code **)(*local_c + 0x10))();
        piVar5 = (int *)(iVar1 + 0x148);
        local_14 = 0;
        if ((piVar5 != (int *)0x0) && (0 < *(int *)(iVar1 + 0x14c))) {
          for (local_1c = 0; local_1c < *(uint *)(iVar1 + 0x14c); local_1c = local_1c + 1) {
            if (*(short *)(*(int *)(*piVar5 + local_1c * 4) + 8) == *(short *)(local_10 + 8)) {
              iVar3 = FUN_005e4e60(0);
              iVar4 = FUN_005e4e60(0);
              if (iVar3 == iVar4) {
                iVar3 = FUN_005e4e60(1);
                iVar4 = FUN_005e4e60(1);
                if (iVar3 == iVar4) {
                  local_14 = *(int *)(*piVar5 + local_1c * 4);
                  *(undefined4 *)(*(int *)(*piVar5 + local_1c * 4) + 0x20) = 0;
                  break;
                }
              }
            }
          }
          if (local_14 != 0) {
            uVar2 = *(undefined4 *)(param_1 + 8);
            uVar7 = 0;
            uVar6 = 0;
            uVar8 = DAT_0099242c;
            FUN_0051c280(0,0,uVar2,local_8,DAT_0099242c,local_14);
            FUN_0051e170(uVar6,uVar7,uVar2,local_8,uVar8,local_14);
          }
        }
      }
      iVar1 = local_10;
      if (local_10 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068f7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f7e0: 16-byte aligned: YES
  0068f7e0  55             PUSH EBP
  0068f7e1  8B EC          MOV EBP,ESP
  0068f7e3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f7e0
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x20 [dec 32]  x1  via ECX
```

## Card B15-3-12

GOG function: Global::ExecuteCommandSoundObjectSetPitchVariation  gog 0x007a89d0  callconv=__thiscall  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x00693380 (size 117, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00693300:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a8950 Global::ExecuteCommandSoundObjectSetFixedVariance -> 0x00693300  gate_class=REVIEW
- GOG-order neighbour after:  0x007a8a50 Global::ExecuteCommandSoundObjectSetPosition -> 0x00693400  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [690 SoundObjectSetPitchVariance] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007f7630 CSWSSoundObject::ChangePitchVariation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00693380: 16-byte aligned: YES
  00693380  55             PUSH EBP
  00693381  8B EC          MOV EBP,ESP
  00693383  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00693380
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C220  (unnamed)
  0x0062A840  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00693380 FUN_00693380 size=117 ret=8 callers=0 callees=4

undefined4 FUN_00693380(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9e0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c220(local_8);
      if (iVar1 != 0) {
        FUN_0062a840(local_c);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00693300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00693300: 16-byte aligned: YES
  00693300  55             PUSH EBP
  00693301  8B EC          MOV EBP,ESP
  00693303  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00693300
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C220  (unnamed)
  0x0062A8A0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-3-13

GOG function: Global::ExecuteCommandGetTimeHour  gog 0x0079ed20  callconv=__thiscall  param_size_bytes=8  gog_size=72  gog_retn=8
Candidate Steam address: 0x006896d0 (size 72, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetWorldTimeHour 0x51b100)
- best decoy (neighbour/other candidate) scores: 0x00689720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079ebb0 Global::ExecuteCommandGetTag -> 0x00689560  gate_class=AUTO
- GOG-order neighbour after:  0x0079ed70 Global::ExecuteCommandGetTimeMillisecond -> 0x00689720  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [16 GetTimeHour] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064bd90 CServerExoApp::GetWorldTimer -> Steam 0x0051c370  in candidate callees: yes
  0x0080b5e0 CWorldTimer::GetWorldTimeHour -> Steam 0x0051b100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006896d0: 16-byte aligned: YES
  006896d0  55             PUSH EBP
  006896d1  8B EC          MOV EBP,ESP
  006896d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006896d0
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B100  CWorldTimer::GetWorldTimeHour
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006896d0 FUN_006896d0 size=72 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_006896d0(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  FUN_0051c370(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  uVar1 = FUN_0051b100();
  iVar2 = FUN_006fd9c0(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0xfffff830;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00689720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689720: 16-byte aligned: YES
  00689720  55             PUSH EBP
  00689721  8B EC          MOV EBP,ESP
  00689723  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00689720
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B1C0  CWorldTimer::GetWorldTimeMillisecond
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-14

GOG function: Global::ExecuteCommandGetRunScriptVar  gog 0x0079d600  callconv=__thiscall  param_size_bytes=8  gog_size=43  gog_retn=8
Candidate Steam address: 0x00687fb0 (size 43, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00687ec0:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079d510 Global::ExecuteCommandGetRestrictMode -> 0x00687ec0  gate_class=AUTO
- GOG-order neighbour after:  0x0079d630 Global::ExecuteCommandGetSkillRank -> 0x00687fe0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [565 GetRunScriptVar] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xfffff830*} | Steam {0x8*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00687fb0: 16-byte aligned: YES
  00687fb0  55             PUSH EBP
  00687fb1  8B EC          MOV EBP,ESP
  00687fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00687fb0
string literals (0):
direct callees (1):
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00687fb0 FUN_00687fb0 size=43 ret=8 callers=0 callees=1

undefined4 FUN_00687fb0(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006fd9c0(DAT_00a7e10c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00687ec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00687ec0: 16-byte aligned: YES
  00687ec0  55             PUSH EBP
  00687ec1  8B EC          MOV EBP,ESP
  00687ec3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00687ec0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00557EF0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x2E4 [dec 740]  x1  via EAX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B15-3-15

GOG function: Global::ExecuteCommandGetTimeMinute  gog 0x0079edc0  callconv=__thiscall  param_size_bytes=8  gog_size=72  gog_retn=8
Candidate Steam address: 0x00689770 (size 72, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetWorldTimeMinute 0x51b140)
- best decoy (neighbour/other candidate) scores: 0x00689720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079ed70 Global::ExecuteCommandGetTimeMillisecond -> 0x00689720  gate_class=REVIEW
- GOG-order neighbour after:  0x0079ee10 Global::ExecuteCommandGetTimeSecond -> 0x006897c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [17 GetTimeMinute] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064bd90 CServerExoApp::GetWorldTimer -> Steam 0x0051c370  in candidate callees: yes
  0x0080b620 CWorldTimer::GetWorldTimeMinute -> Steam 0x0051b140  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00689770: 16-byte aligned: YES
  00689770  55             PUSH EBP
  00689771  8B EC          MOV EBP,ESP
  00689773  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00689770
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B140  CWorldTimer::GetWorldTimeMinute
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00689770 FUN_00689770 size=72 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_00689770(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  FUN_0051c370(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  uVar1 = FUN_0051b140();
  iVar2 = FUN_006fd9c0(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0xfffff830;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00689720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689720: 16-byte aligned: YES
  00689720  55             PUSH EBP
  00689721  8B EC          MOV EBP,ESP
  00689723  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00689720
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B1C0  CWorldTimer::GetWorldTimeMillisecond
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-16

GOG function: Global::ExecuteCommandEffectMovementSpeedIncrease  gog 0x0078b150  callconv=__thiscall  param_size_bytes=8  gog_size=395  gog_retn=8
Candidate Steam address: 0x006759e0 (size 395, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectMovementSpeedDecrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00675b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078afc0 Global::ExecuteCommandEffectMovementSpeedDecrease -> 0x006759e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b2e0 Global::ExecuteCommandEffectPoison -> 0x00675d00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [165 EffectMovementSpeedIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006759e0: 16-byte aligned: YES
  006759e0  55             PUSH EBP
  006759e1  8B EC          MOV EBP,ESP
  006759e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006759e0
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006759e0 FUN_006759e0 size=395 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_006759e0(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int local_40;
  undefined2 local_34;
  undefined4 local_20;
  int local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0bb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_3 == 1) {
    iVar3 = FUN_006fd9a0(&local_20);
    if (iVar3 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar3 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar3 == 0) {
        local_40 = 0;
      }
      else {
        local_40 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_18 = local_40;
      local_34 = (undefined2)DAT_00995f90;
      *(undefined2 *)(local_40 + 8) = local_34;
      *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_20);
      piVar5 = &local_1c;
      uVar2 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar2,piVar5);
      local_11 = FUN_0053dfb0(uVar2,piVar5);
      if ((local_11 == DAT_0099447f) && (local_1c != 0)) {
        FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      }
      iVar4 = FUN_006fdad0(0,local_18);
      iVar3 = local_18;
      if (iVar4 == 0) {
        uVar2 = 0xfffff830;
      }
      else {
        if (local_18 != 0) {
          FUN_005e4ca0();
          FUN_009196fd(iVar3);
        }
        uVar2 = 0;
      }
    }
  }
  else {
    uVar2 = 0xfffff82f;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00675b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675b70: 16-byte aligned: YES
  00675b70  55             PUSH EBP
  00675b71  8B EC          MOV EBP,ESP
  00675b73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675b70
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-3-17

GOG function: Global::ExecuteCommandVectorMagnitude  gog 0x007aacd0  callconv=__thiscall  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x00695680 (size 107, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00695500:STR0/CALLEE2/SIG0.15/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aab50 Global::ExecuteCommandVectorConversions -> 0x00695500  gate_class=AUTO
- GOG-order neighbour after:  0x007aad40 Global::ExecuteCommandVectorNormalize -> 0x006956f0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [104 VectorMagnitude] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640c40 CVirtualMachine::StackPopVector -> Steam 0x006fda20  in candidate callees: yes
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x14*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x14*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00695680: 16-byte aligned: YES
  00695680  55             PUSH EBP
  00695681  8B EC          MOV EBP,ESP
  00695683  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00695680
string literals (0):
direct callees (3):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00695680 FUN_00695680 size=107 ret=8 callers=0 callees=3

undefined4 FUN_00695680(void)

{
  int iVar1;
  float10 fVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  uVar3 = 0;
  iVar1 = FUN_006fda20(&local_10);
  if (iVar1 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    fVar2 = (float10)FUN_005142f0(&local_10,extraout_var,uVar3);
    iVar1 = FUN_006fda00((float)fVar2);
    if (iVar1 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x00695500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695500: 16-byte aligned: YES
  00695500  55             PUSH EBP
  00695501  8B EC          MOV EBP,ESP
  00695503  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695500
string literals (0):
direct callees (8):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x005143C0  (unnamed)
  0x0091E620  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B15-3-18

GOG function: Global::ExecuteCommandVectorNormalize  gog 0x007aad40  callconv=__thiscall  param_size_bytes=8  gog_size=190  gog_retn=8
Candidate Steam address: 0x006956f0 (size 190, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00695500:STR0/CALLEE2/SIG0.722/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aacd0 Global::ExecuteCommandVectorMagnitude -> 0x00695680  gate_class=REVIEW
- GOG-order neighbour after:  0x007aae00 Global::ExecuteCommandVersusEffect -> 0x006957b0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [137 VectorNormalize] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640c40 CVirtualMachine::StackPopVector -> Steam 0x006fda20  in candidate callees: yes
  0x00640c60 CVirtualMachine::StackPushVector -> Steam 0x006fda40  in candidate callees: yes
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 5 common): GOG {0x8*,0xc*,0x34*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0x34*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006956f0: 16-byte aligned: YES
  006956f0  55             PUSH EBP
  006956f1  8B EC          MOV EBP,ESP
  006956f3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006956f0
string literals (0):
direct callees (4):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x00517980  Vec3::Equal
  0x005143C0  (unnamed)
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006956f0 FUN_006956f0 size=190 ret=8 callers=0 callees=4

undefined4 FUN_006956f0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 extraout_var;
  undefined1 local_34 [12];
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_1c = 0;
  local_18 = 0;
  local_14 = 0;
  iVar1 = FUN_006fda20(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_28 = 0;
    local_24 = 0;
    local_20 = 0;
    iVar1 = FUN_00517980(&local_10,&local_28,extraout_var);
    if (iVar1 == 0) {
      puVar3 = (undefined4 *)FUN_005143c0(local_34,&local_10);
      local_1c = *puVar3;
      local_18 = puVar3[1 = byte +0x4];
      local_14 = puVar3[2 = byte +0x8];
    }
    iVar1 = FUN_006fda40(local_1c,local_18,local_14);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00695500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695500: 16-byte aligned: YES
  00695500  55             PUSH EBP
  00695501  8B EC          MOV EBP,ESP
  00695503  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695500
string literals (0):
direct callees (8):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x005143C0  (unnamed)
  0x0091E620  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B15-3-19

GOG function: Global::ExecuteCommandPrintObject  gog 0x007a4850  callconv=__thiscall  param_size_bytes=8  gog_size=84  gog_retn=8
Candidate Steam address: 0x0068f200 (size 84, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068f0c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a4770 Global::ExecuteCommandPrintLogEntry -> 0x0068f120  gate_class=REVIEW
- GOG-order neighbour after:  0x007a48b0 Global::ExecuteCommandPrintString -> 0x0068f260  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [5 PrintObject] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0xc*,0xfffff82f*} | Steam {0x2*,0x8*,0xc*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068f200: 16-byte aligned: YES
  0068f200  55             PUSH EBP
  0068f201  8B EC          MOV EBP,ESP
  0068f203  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f200
string literals (0):
direct callees (2):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068f200 FUN_0068f200 size=84 ret=8 callers=0 callees=2

undefined4 FUN_0068f200(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  iVar1 = FUN_006fdaf0(local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else if ((param_2 < 2) || (iVar1 = FUN_006fd9a0(local_8), iVar1 != 0)) {
    uVar2 = 0;
  }
  else {
    uVar2 = 0xfffff82f;
  }
  return uVar2;
}
```

### decoy: 0x0068f0c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f0c0: 16-byte aligned: YES
  0068f0c0  55             PUSH EBP
  0068f0c1  8B EC          MOV EBP,ESP
  0068f0c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f0c0
string literals (0):
direct callees (1):
  0x006FD9A0  CVirtualMachine::StackPopInteger
struct offsets touched (top 0):
```

## Card B15-3-20

GOG function: Global::ExecuteCommandIsInTotalDefense  gog 0x007a1450  callconv=__thiscall  param_size_bytes=8  gog_size=128  gog_retn=8
Candidate Steam address: 0x0068be00 (size 128, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068bcf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a13c0 Global::ExecuteCommandIsMeditating -> 0x0068bd70  gate_class=AUTO
- GOG-order neighbour after:  0x007a14d0 Global::ExecuteCommandIsFormActive -> 0x0068be80  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [812 IsInTotalDefense] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00656d70 CSWSCreature::IsInTotalDefense -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x14*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068be00: 16-byte aligned: YES
  0068be00  55             PUSH EBP
  0068be01  8B EC          MOV EBP,ESP
  0068be03  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068be00
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DAAC0  CSWSCreature::IsInTotalDefense
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0068be00 FUN_0068be00 size=128 ret=8 callers=0 callees=4

undefined4 FUN_0068be00(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_10;
  undefined4 local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar3 = extraout_var;
    iVar1 = FUN_0051c100(local_8);
    if (iVar1 != 0) {
      iVar1 = FUN_006daac0(uVar3,uVar2);
      if (iVar1 != 0) {
        local_10 = 1;
      }
    }
    iVar1 = FUN_006fd9c0(local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068bcf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068bcf0: 16-byte aligned: YES
  0068bcf0  55             PUSH EBP
  0068bcf1  8B EC          MOV EBP,ESP
  0068bcf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068bcf0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00563D70  CSWSCreature::GetActivity
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-3-21

GOG function: Global::ExecuteCommandGetNearestObject  gog 0x00799310  callconv=__thiscall  param_size_bytes=8  gog_size=4828  gog_retn=8
Candidate Steam address: 0x006839a0 (size 385, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetModuleName | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00683d40:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007991a0 Global::ExecuteCommandGetName -> 0x00683bd0  gate_class=AUTO
- GOG-order neighbour after:  0x0079a710 Global::ExecuteCommandGetNearestMine -> 0x00685140  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [38 GetNearestCreature, 226 GetNearestCreatureToLocation, 227 GetNearestObject, 228 GetNearestObjectToLocation, 229 GetNearestObjectByTag] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (28), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00545d10 CExoArrayList::insert -> Steam ?  in candidate callees: n/a
  0x0054c000 CExoArrayList_float_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  ... 20 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8*,0x10,0x30,0x6c,0x8d,0x94,0x9c,0xab,0xe0,0x122,0x1198,0x11ac} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3,0x4*,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc,0xd,0xe,0x10,...(+24)} | Steam {0x2*,0x4*,0x8*,0x3a,0x4c,0x5c,0xd2,0x231,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006839a0: 16-byte aligned: YES
  006839a0  55             PUSH EBP
  006839a1  8B EC          MOV EBP,ESP
  006839a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006839a0
string literals (0):
direct callees (9):
  0x00733540  CExoString::Constructor
  0x0051BDB0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00734190  CExoString::Find_2
  0x00919F10  _strlen
  0x00734870  CExoString::Right
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006839a0 FUN_006839a0 size=385 ret=8 callers=0 callees=9

undefined4 FUN_006839a0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  size_t local_44;
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  int local_1c;
  char *local_18 [2];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a498;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (param_1 == 0xd2) {
    iVar2 = FUN_0051bd90();
    FUN_007337c0(iVar2 + 0x5c);
    local_1c = FUN_00734190(0x3a,0);
    if (local_1c != -1) {
      if (local_18[0] == (char *)0x0) {
        local_44 = 0;
      }
      else {
        local_44 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_18[0]);
      }
      uVar1 = FUN_00734870(local_2c,(local_44 - local_1c) + -1);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar1);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
  }
  else if (param_1 == 0x231) {
    uVar1 = FUN_0051bdb0(local_24);
    local_8._0_1_ = 1;
    FUN_007337c0(uVar1);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
  }
  iVar2 = FUN_006fda90(local_18);
  if (iVar2 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0xfffff830;
  }
  else {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0;
  }
  ExceptionList = local_10;
  return uVar1;
}
```

### decoy: 0x00683d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00683d40: 16-byte aligned: YES
  00683d40  55             PUSH EBP
  00683d41  8B EC          MOV EBP,ESP
  00683d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00683d40
string literals (0):
direct callees (28):
  0x00733540  CExoString::Constructor
  0x0051C080  CServerExoApp::GetObjectArray
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x006FDA70  CVirtualMachine::StackPopString
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x00522AF0  CSWSArea::GetFirstObjectIndiceByX
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0084A640  (unnamed)
  0x0058C250  CSWSCreature::UseLoreOnItem
  0x0091FBF2  (unnamed)
  0x00476620  (unnamed)
  0x005E13C0  CSWSObject::GetReputation
  0x005476C0  CSWSObject::HasSpellEffectApplied
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x8D [dec 141]  x4  via EAX,ECX,EDX
  +0x94 [dec 148]  x4  via EAX,EDX
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x9C [dec 156]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via ECX
  +0x11AC [dec 4524]  x3  via EAX,ECX,EDX
  +0x122 [dec 290]  x2  via ECX
```

## Card B15-3-22

GOG function: Global::ExecuteCommandSetPlaceableIllumination  gog 0x007a7c90  callconv=__thiscall  param_size_bytes=8  gog_size=114  gog_retn=8
Candidate Steam address: 0x00692640 (size 114, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SetLightIsOn 0x5f6470)
- best decoy (neighbour/other candidate) scores: 0x00692760:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a7c20 Global::ExecuteCommandSetNPCSelectability -> 0x006925d0  gate_class=AUTO
- GOG-order neighbour after:  0x007a7d10 Global::ExecuteCommandSetPlotFlag -> 0x006926c0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [544 SetPlaceableIllumination] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x00770810 CSWSPlaceable::SetLightIsOn -> Steam 0x005f6470  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00692640: 16-byte aligned: YES
  00692640  55             PUSH EBP
  00692641  8B EC          MOV EBP,ESP
  00692643  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00692640
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C160  ServerApp::GetPlaceableById
  0x005F6470  CSWSPlaceable::SetLightIsOn
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00692640 FUN_00692640 size=114 ret=8 callers=0 callees=4

undefined4 FUN_00692640(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c160(local_c);
      if (iVar1 != 0) {
        FUN_005f6470(local_8);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00692760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00692760: 16-byte aligned: YES
  00692760  55             PUSH EBP
  00692761  8B EC          MOV EBP,ESP
  00692763  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00692760
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00557EF0  (unnamed)
  0x0052E170  CSWSArea::SetPlayerRestrictMode
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

## Card B15-3-23

GOG function: Global::ExecuteCommandSoundObjectStop  gog 0x007a8b70  callconv=__thiscall  param_size_bytes=8  gog_size=84  gog_retn=8
Candidate Steam address: 0x00693520 (size 84, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006932a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a8af0 Global::ExecuteCommandSoundObjectSetVolume -> 0x006934a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007a8bd0 Global::ExecuteCommandSoundObjectFadeAndStop -> 0x00693580  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [414 SoundObjectStop] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007f7500 CSWSSoundObject::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0xfffff82f*} | Steam {0x8*,0x10*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00693520: 16-byte aligned: YES
  00693520  55             PUSH EBP
  00693521  8B EC          MOV EBP,ESP
  00693523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00693520
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C220  (unnamed)
  0x0062A710  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00693520 FUN_00693520 size=84 ret=8 callers=0 callees=3

undefined4 FUN_00693520(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar3 = extraout_var;
    iVar1 = FUN_0051c220(local_8);
    if (iVar1 != 0) {
      FUN_0062a710(uVar3,uVar2);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x006932a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006932a0: 16-byte aligned: YES
  006932a0  55             PUSH EBP
  006932a1  8B EC          MOV EBP,ESP
  006932a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006932a0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C220  (unnamed)
  0x0062A620  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-3-24

GOG function: Global::ExecuteCommandSetRestrictMode  gog 0x007a7db0  callconv=__thiscall  param_size_bytes=8  gog_size=135  gog_retn=8
Candidate Steam address: 0x00692760 (size 135, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SetPlayerRestrictMode 0x52e170)
- best decoy (neighbour/other candidate) scores: 0x00692640:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a7d10 Global::ExecuteCommandSetPlotFlag -> 0x006926c0  gate_class=AUTO
- GOG-order neighbour after:  0x007a7e40 Global::ExecuteCommandSetReturnStrref -> 0x006927f0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [58 SetPlayerRestrictMode] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x00723250 CSWSArea::SetPlayerRestrictMode -> Steam 0x0052e170  in candidate callees: yes
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00692760: 16-byte aligned: YES
  00692760  55             PUSH EBP
  00692761  8B EC          MOV EBP,ESP
  00692763  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00692760
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x00557EF0  (unnamed)
  0x0052E170  CSWSArea::SetPlayerRestrictMode
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00692760 FUN_00692760 size=135 ret=8 callers=0 callees=4

undefined4 FUN_00692760(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar3 = extraout_var;
    iVar1 = FUN_0051bd90(extraout_var);
    if (iVar1 != 0) {
      uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      FUN_0051bd90(uVar3);
      iVar1 = FUN_00557ef0();
      if (iVar1 != 0) {
        FUN_0051bd90(local_8,uVar3,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar4,uVar2);
        FUN_00557ef0();
        FUN_0052e170(local_8);
      }
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00692640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00692640: 16-byte aligned: YES
  00692640  55             PUSH EBP
  00692641  8B EC          MOV EBP,ESP
  00692643  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00692640
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C160  ServerApp::GetPlaceableById
  0x005F6470  CSWSPlaceable::SetLightIsOn
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-3-25

GOG function: Global::ExecuteCommandGetLastAttackResult  gog 0x00797150  callconv=__thiscall  param_size_bytes=8  gog_size=128  gog_retn=8
Candidate Steam address: 0x00681b80 (size 128, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00681c00:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00797010 Global::ExecuteCommandGetLastAttackMode -> 0x00681a40  gate_class=REVIEW
- GOG-order neighbour after:  0x007971d0 Global::ExecuteCommandGetWasForcePowerSuccessful -> 0x00681c00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [725 GetLastAttackResult] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x5a0*} | Steam {0x8*,0x5a0*}
imm sets (* = in both, 5 common): GOG {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00681b80: 16-byte aligned: YES
  00681b80  55             PUSH EBP
  00681b81  8B EC          MOV EBP,ESP
  00681b83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00681b80
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5A0 [dec 1440]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00681b80 FUN_00681b80 size=128 ret=8 callers=0 callees=3

undefined4 FUN_00681b80(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0x7f000000;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c100(local_c);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x5a0);
    }
    iVar1 = FUN_006fd9c0(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00681c00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00681c00: 16-byte aligned: YES
  00681c00  55             PUSH EBP
  00681c01  8B EC          MOV EBP,ESP
  00681c03  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00681c00
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5A8 [dec 1448]  x1  via ECX
```

## Card B15-3-26

GOG function: Global::ExecuteCommandGetIsDay  gog 0x00794f90  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067f940 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetIsDawn | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067f9b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00794f20 Global::ExecuteCommandGetIsDawn -> 0x0067f940  gate_class=REVIEW
- GOG-order neighbour after:  0x00795000 Global::ExecuteCommandGetIsDoorActionPossible -> 0x0067fa20  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [405 GetIsDay] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x1dc*} | Steam {0x8*,0x1dc*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff830*} | Steam {0x3,0x8*,0x14*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067f940: 16-byte aligned: YES
  0067f940  55             PUSH EBP
  0067f941  8B EC          MOV EBP,ESP
  0067f943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067f940
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067f940 FUN_0067f940 size=106 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0067f940(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  
  local_c = 0;
  iVar1 = FUN_0051bd90(param_1);
  if ((iVar1 != 0) && (*(char *)(iVar1 + 0x1dc) == '\x03')) {
    local_c = 1;
  }
  iVar1 = FUN_006fd9c0(local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0067f9b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067f9b0: 16-byte aligned: YES
  0067f9b0  55             PUSH EBP
  0067f9b1  8B EC          MOV EBP,ESP
  0067f9b3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067f9b0
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

## Card B15-3-27

GOG function: Global::ExecuteCommandSoundObjectSetFixedVariance  gog 0x007a8950  callconv=__thiscall  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x00693300 (size 117, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00693380:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a88f0 Global::ExecuteCommandSoundObjectPlay -> 0x006932a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007a89d0 Global::ExecuteCommandSoundObjectSetPitchVariation -> 0x00693380  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [124 SoundObjectSetFixedVariance] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007f7690 CSWSSoundObject::ChangeFixedVariance -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00693300: 16-byte aligned: YES
  00693300  55             PUSH EBP
  00693301  8B EC          MOV EBP,ESP
  00693303  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00693300
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C220  (unnamed)
  0x0062A8A0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00693300 FUN_00693300 size=117 ret=8 callers=0 callees=4

undefined4 FUN_00693300(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9e0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c220(local_c);
      if (iVar1 != 0) {
        FUN_0062a8a0(local_8);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00693380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00693380: 16-byte aligned: YES
  00693380  55             PUSH EBP
  00693381  8B EC          MOV EBP,ESP
  00693383  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00693380
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C220  (unnamed)
  0x0062A840  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-3-28

GOG function: Global::ExecuteCommandPutDownItem  gog 0x007a4bb0  callconv=__thiscall  param_size_bytes=8  gog_size=112  gog_retn=8
Candidate Steam address: 0x0068f560 (size 112, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0068f770:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a4a90 Global::ExecuteCommandPlayOverlayAnimation -> 0x0068f440  gate_class=AUTO
- GOG-order neighbour after:  0x007a4c20 CSWVirtualMachineCommands::ExecuteCommandRandom -> 0x0068f5d0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [35 ActionPutDownItem] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00680b30 CSWSCreature::AddHealActions -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x94*,0xfffff82f*} | Steam {0x8*,0x10*,0x94*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068f560: 16-byte aligned: YES
  0068f560  55             PUSH EBP
  0068f561  8B EC          MOV EBP,ESP
  0068f563  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f560
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00567F00  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0068f560 FUN_0068f560 size=112 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_0068f560(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
    if ((local_8 != 0) && (iVar1 = FUN_00567f00(local_c,local_8 + 0x94,0), iVar1 == 1)) {
      return 0;
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0068f770 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f770: 16-byte aligned: YES
  0068f770  55             PUSH EBP
  0068f771  8B EC          MOV EBP,ESP
  0068f773  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f770
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FA920  CSWPartyTable::RemoveNPC
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-3-29

GOG function: Global::ExecuteCommandGetIsDay  gog 0x00794f90  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067fa20 (size 895, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067f9b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00794f20 Global::ExecuteCommandGetIsDawn -> 0x0067f940  gate_class=REVIEW
- GOG-order neighbour after:  0x00795000 Global::ExecuteCommandGetIsDoorActionPossible -> 0x0067fa20  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [405 GetIsDay] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x1dc} | Steam {0x8*,0x8d,0xd8,0xfc,0x2a0,0x2ac,0x304,0x328,0x3f0,0x410,0x1198}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff830*} | Steam {0x4,0x6,0x8*,0x14*,0x5d,0x80,0x151,0x2726,0x275c,0xfffff82f,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067fa20: 16-byte aligned: YES
  0067fa20  55             PUSH EBP
  0067fa21  8B EC          MOV EBP,ESP
  0067fa23  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067fa20
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C1A0  (unnamed)
  0x0051C160  ServerApp::GetPlaceableById
  0x006B7BB0  CSWSCreatureStats::GetSkillRank
  0x0061CD50  CSWSDoor::GetOpenLockDC
  0x005F5C40  CSWSPlaceable::GetOpenLockDC
  0x006B8F40  CSWSCreatureStats::GetSpellUsesLeft
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 10):
  +0x8 [dec 8]  x4  via EAX,EDX
  +0xFC [dec 252]  x4  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x3  via ECX,EDX
  +0xD8 [dec 216]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,ECX
  +0x2AC [dec 684]  x2  via EAX,EDX
  +0x304 [dec 772]  x2  via ECX,EDX
  +0x328 [dec 808]  x2  via EAX,ECX
  +0x8D [dec 141]  x1  via EAX
  +0x3F0 [dec 1008]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067fa20 FUN_0067fa20 size=895 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_0067fa20(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  byte local_1e;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  local_1c = 0;
  local_10 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
  iVar2 = FUN_006fdaf0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_14);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if (param_2 == 0x151) {
        local_8 = FUN_0051c1a0(local_18);
      }
      else {
        local_1c = FUN_0051c160(local_18);
      }
      if ((local_10 != 0) &&
         (((local_8 != 0 && (*(int *)(local_8 + 0x410) == 0)) ||
          ((local_1c != 0 && (*(int *)(local_1c + 0x3f0) == 0)))))) {
        switch(local_14) {
        case 0:
          if ((param_2 == 0x151) && (local_8 != 0)) {
            if ((*(int *)(local_8 + 0xd8) == 0x2726) && (*(int *)(local_8 + 0x304) == 0)) {
              local_c = 1;
            }
          }
          else if ((local_1c != 0) &&
                  ((*(int *)(local_1c + 0xd8) == 0x275c && (*(int *)(local_1c + 0x2a0) == 0)))) {
            local_c = 1;
          }
          break;
        case 1:
          cVar1 = FUN_006b7bb0(6,0,0);
          if ((param_2 == 0x151) && (local_8 != 0)) {
            if (((*(int *)(local_8 + 0x304) != 0) && (*(int *)(local_8 + 0x328) == 0)) &&
               (('\0' < cVar1 && (uVar4 = FUN_0061cd50(), (int)(uVar4 & 0xff) <= cVar1 + 0x14)))) {
              local_c = 1;
            }
          }
          else if ((local_1c != 0) &&
                  ((((*(int *)(local_1c + 0x2a0) != 0 && (*(int *)(local_1c + 0x2ac) == 0)) &&
                    ('\0' < cVar1)) && (uVar4 = FUN_005f5c40(), (int)(uVar4 & 0xff) <= cVar1 + 0x14)
                   ))) {
            local_c = 1;
          }
          break;
        case 2:
          if (((local_8 != 0) && (*(int *)(local_8 + 0xfc) == 0)) ||
             ((local_1c != 0 && (*(int *)(local_1c + 0xfc) == 0)))) {
            local_c = 1;
          }
          break;
        case 3:
          local_c = 1;
          break;
        case 4:
          if ((((local_8 != 0) && (*(int *)(local_8 + 0xfc) == 0)) &&
              (*(int *)(local_8 + 0x328) == 0)) ||
             (((local_1c != 0 && (*(int *)(local_1c + 0xfc) == 0)) &&
              (*(int *)(local_1c + 0x2ac) == 0)))) {
/* ... 24 more lines truncated by --max-lines */
```

### decoy: 0x0067f9b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067f9b0: 16-byte aligned: YES
  0067f9b0  55             PUSH EBP
  0067f9b1  8B EC          MOV EBP,ESP
  0067f9b3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067f9b0
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

## Card B15-3-30

GOG function: Global::ExecuteCommandMoveToPoint  gog 0x007a2530  callconv=__thiscall  param_size_bytes=8  gog_size=637  gog_retn=8
Candidate Steam address: 0x0068cb50 (size 900, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandMoveToObject | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.44 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068cee0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a21a0 Global::ExecuteCommandMoveToObject -> 0x0068cb50  gate_class=AUTO
- GOG-order neighbour after:  0x007a27b0 Global::ExecuteCommandMusicBackground -> 0x0068d160  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [21 ActionMoveToLocation, 382 ActionForceMoveToLocation] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: no
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x0067d3a0 CSWSCreature::AddMoveToPointAction -> Steam 0x00564770  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x30*,0x124*} | Steam {0x4*,0x8*,0x30*,0x48,0xec,0x124*,0x394}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x8*,0x15,0x5c,0x17e,0xffff*,0xfffff82f*} | Steam {0x2*,0x3,0x4*,0x5,0x8*,0x11,0x13,0x90,0x94,0x17f,0xfffe,0xffff*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068cb50: 16-byte aligned: YES
  0068cb50  55             PUSH EBP
  0068cb51  8B EC          MOV EBP,ESP
  0068cb53  81 EC 90 00 00 00 SUB ESP,0x90
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068cb50
string literals (0):
direct callees (11):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00585F10  CSWSCreature::GetUseRange
  0x005453C0  CSWSObject::GetArea
  0x007107F0  (unnamed)
  0x00564770  CSWSCreature::AddMoveToPointAction
  0x0053F7F0  CSWSObject::AddAction
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via EDX
  +0xEC [dec 236]  x1  via ECX
  +0x124 [dec 292]  x1  via EDX
  +0x394 [dec 916]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0068cb50 FUN_0068cb50 size=900 ret=8 callers=0 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0068cb50(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  float fVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 *puVar12;
  float *pfVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  int **ppiVar17;
  int *piVar18;
  undefined4 uVar19;
  undefined4 uVar20;
  float local_8c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  uint local_28;
  int local_24;
  char local_1d;
  undefined4 local_1c;
  float local_18;
  int *local_14;
  undefined4 local_10;
  undefined4 local_c;
  float local_8;
  
  local_28 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_10 = 0;
  local_8 = _DAT_0098c014;
  local_18 = 1.0;
  local_1c = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (((iVar1 != 0) && ((param_3 < 2 || (iVar1 = FUN_006fd9a0(&local_10), iVar1 != 0)))) &&
     ((param_3 < 3 || (iVar1 = FUN_006fd9e0(&local_18), iVar1 != 0)))) {
    if (param_2 == 0x17f) {
      if (param_3 < 4) {
        local_1c = _DAT_009919f0;
      }
      else {
        iVar1 = FUN_006fd9e0(&local_1c);
        if (iVar1 == 0) goto LAB_0068cec4;
      }
    }
    if (*(int *)(param_1 + 4) == 1) {
      ppiVar17 = &local_14;
      uVar4 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar4,ppiVar17);
      local_1d = FUN_0053dfb0(uVar4,ppiVar17);
      if (local_1d == DAT_0099447f) {
        piVar18 = &local_24;
        uVar4 = local_c;
        FUN_0051c080(local_c,piVar18);
        local_1d = FUN_0053dfb0(uVar4,piVar18);
        if (((local_1d == DAT_0099447f) && (iVar1 = (**(code **)(*local_14 + 0x30))(), iVar1 != 0))
           && (iVar1 = (**(code **)(*local_14 + 0x30))(), *(int *)(iVar1 + 0xec) != 0)) {
          local_44 = 0;
          local_40 = 0;
/* ... 63 more lines truncated by --max-lines */
```

### decoy: 0x0068cee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068cee0: 16-byte aligned: YES
  0068cee0  55             PUSH EBP
  0068cee1  8B EC          MOV EBP,ESP
  0068cee3  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068cee0
string literals (0):
direct callees (9):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x00564770  CSWSCreature::AddMoveToPointAction
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0x124 [dec 292]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B15-3-31

GOG function: Global::ExecuteCommandMath  gog 0x007a1af0  callconv=__thiscall  param_size_bytes=8  gog_size=747  gog_retn=8
Candidate Steam address: 0x0068c4a0 (size 747, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0068c380:STR0/CALLEE1/SIG0.147/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a19d0 Global::ExecuteCommandLocationAccess -> 0x0068c380  gate_class=AUTO
- GOG-order neighbour after:  0x007a1e10 Global::ExecuteCommandModuleAccess -> 0x0068c7c0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [67 fabs, 68 cos, 69 sin, 70 tan, 71 acos, 72 asin, 73 atan, 74 log, 75 pow, 76 sqrt, 77 abs] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00924754 Global::sqrt -> Steam ?  in candidate callees: n/a
  0x009259f0 Global::sin -> Steam ?  in candidate callees: n/a
  0x00925b20 Global::cos -> Steam ?  in candidate callees: n/a
  0x00925c49 unnamed -> Steam ?  in candidate callees: n/a
  ... 7 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 11 common): GOG {0x4*,0x8*,0x9*,0x10*,0x41*,0x43*,0x44*,0x4d*,0x50*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x9*,0x10*,0x41*,0x43*,0x44*,0x4d*,0x50*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068c4a0: 16-byte aligned: YES
  0068c4a0  55             PUSH EBP
  0068c4a1  8B EC          MOV EBP,ESP
  0068c4a3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068c4a0
string literals (0):
direct callees (15):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0091FBF2  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091EBAD  (unnamed)
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x0091F820  (unnamed)
  0x0091E620  (unnamed)
  0x0091FA70  (unnamed)
  0x009201D0  (unnamed)
  0x00920080  (unnamed)
  0x0091E390  (unnamed)
  0x0091CB24  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068c4a0 FUN_0068c4a0 size=747 ret=8 callers=0 callees=15

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0068c4a0(int param_1)

{
  int iVar1;
  double dVar2;
  float local_14;
  float local_10;
  
  if (param_1 == 0x4d) {
    iVar1 = FUN_006fd9a0();
    if (iVar1 == 0) {
      return 0xfffff82f;
    }
    FUN_0091fbf2();
    iVar1 = FUN_006fd9c0();
    if (iVar1 == 0) {
      return 0xfffff830;
    }
  }
  else {
    iVar1 = FUN_006fd9e0();
    if (iVar1 == 0) {
      return 0xfffff82f;
    }
    switch(param_1) {
    case 0x43:
      _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)local_10);
      break;
    case 0x44:
      FUN_0091ea50((double)(local_10 * (float)_DAT_0098be40));
      break;
    case 0x45:
      FUN_0091e770((double)(local_10 * (float)_DAT_0098be40));
      break;
    case 0x46:
      FUN_0091f820((double)(local_10 * (float)_DAT_0098be40));
      break;
    case 0x47:
      dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)local_10);
      if ((float)dVar2 < (float)_DAT_00986b50 != ((float)dVar2 == (float)_DAT_00986b50)) {
        FUN_0091e620((double)local_10);
      }
      break;
    case 0x48:
      dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)local_10);
      if ((float)dVar2 < (float)_DAT_00986b50 != ((float)dVar2 == (float)_DAT_00986b50)) {
        FUN_0091fa70((double)local_10);
      }
      break;
    case 0x49:
      FUN_009201d0((double)local_10);
      break;
    case 0x4a:
      if ((float)_DAT_00986b38 < local_10) {
        FUN_00920080((double)local_10);
      }
      break;
    case 0x4b:
      iVar1 = FUN_006fd9e0();
      if (iVar1 == 0) {
        return 0xfffff82f;
      }
      if ((local_10 != (float)_DAT_00986b38) || ((float)_DAT_00986b38 <= local_14)) {
        FUN_0091e390((double)local_10,(double)local_14);
      }
      break;
    case 0x4c:
      if ((float)_DAT_00986b38 < local_10) {
        FUN_0091cb24((double)local_10);
      }
    }
    iVar1 = FUN_006fda00();
    if (iVar1 == 0) {
      return 0xfffff830;
    }
  }
  return 0;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x0068c380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068c380: 16-byte aligned: YES
  0068c380  55             PUSH EBP
  0068c381  8B EC          MOV EBP,ESP
  0068c383  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068c380
string literals (0):
direct callees (6):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FDA40  CVirtualMachine::StackPushVector
  0x0091FC10  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
```

## Card B15-3-32

GOG function: Global::ExecuteCommandEffectMovementSpeedIncrease  gog 0x0078b150  callconv=__thiscall  param_size_bytes=8  gog_size=395  gog_retn=8
Candidate Steam address: 0x00675d00 (size 318, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectPoison | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00675b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078afc0 Global::ExecuteCommandEffectMovementSpeedDecrease -> 0x006759e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b2e0 Global::ExecuteCommandEffectPoison -> 0x00675d00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [165 EffectMovementSpeedIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: no
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x34,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x28,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00675d00: 16-byte aligned: YES
  00675d00  55             PUSH EBP
  00675d01  8B EC          MOV EBP,ESP
  00675d03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675d00
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00675d00 FUN_00675d00 size=318 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00675d00(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_34;
  undefined2 local_2c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f4eb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_34 = 0;
    }
    else {
      local_34 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_34;
    *(ushort *)(local_34 + 0xa [dec 10]) = *(ushort *)(local_34 + 0xa [dec 10]) & 0xffe7 | 8;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
    local_2c = (undefined2)DAT_00995fa8;
    *(undefined2 *)(local_14 + 8) = local_2c;
    FUN_005e4e80(0,local_18);
    iVar4 = FUN_006fdad0(0,local_14);
    iVar2 = local_14;
    if (iVar4 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      if (local_14 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar2);
      }
      uVar3 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00675b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675b70: 16-byte aligned: YES
  00675b70  55             PUSH EBP
  00675b71  8B EC          MOV EBP,ESP
  00675b73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675b70
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-3-33

GOG function: Global::ExecuteCommandGetStringUpperCase  gog 0x0079e7c0  callconv=__thiscall  param_size_bytes=8  gog_size=289  gog_retn=8
Candidate Steam address: 0x00689170 (size 289, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00688ed0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e650 Global::ExecuteCommandGetStringRight -> 0x00689000  gate_class=REVIEW
- GOG-order neighbour after:  0x0079e8f0 Global::ExecuteCommandGetStrRefSoundDuration -> 0x006892a0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [60 GetStringUpperCase] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606c90 CExoString::UpperCase -> Steam ?  in candidate callees: n/a
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x30*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00689170: 16-byte aligned: YES
  00689170  55             PUSH EBP
  00689171  8B EC          MOV EBP,ESP
  00689173  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00689170
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734B80  CExoString::UpperCase
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00689170 FUN_00689170 size=289 ret=8 callers=0 callees=6

undefined4 FUN_00689170(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_2c [8];
  undefined4 local_24;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a668;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  iVar1 = FUN_006fda70(local_20);
  if (iVar1 == 0) {
    local_24 = 0xfffff82f;
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    uVar2 = FUN_00734b80(local_2c);
    local_8._0_1_ = 2;
    FUN_007337c0(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780();
    iVar1 = FUN_006fda90(local_18);
    if (iVar1 == 0) {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_24 = 0xfffff830;
    }
    else {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_24 = 0;
    }
  }
  ExceptionList = local_10;
  return local_24;
}
```

### decoy: 0x00688ed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00688ed0: 16-byte aligned: YES
  00688ed0  55             PUSH EBP
  00688ed1  8B EC          MOV EBP,ESP
  00688ed3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00688ed0
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-34

GOG function: Global::ExecuteCommandSoundObjectSetVolume  gog 0x007a8af0  callconv=__thiscall  param_size_bytes=8  gog_size=114  gog_retn=8
Candidate Steam address: 0x006934a0 (size 114, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00693380:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a8a50 Global::ExecuteCommandSoundObjectSetPosition -> 0x00693400  gate_class=AUTO
- GOG-order neighbour after:  0x007a8b70 Global::ExecuteCommandSoundObjectStop -> 0x00693520  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [415 SoundObjectSetVolume] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bc40 CServerExoApp::GetSoundObjectByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007f76f0 CSWSSoundObject::ChangeVolume -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006934a0: 16-byte aligned: YES
  006934a0  55             PUSH EBP
  006934a1  8B EC          MOV EBP,ESP
  006934a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006934a0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C220  (unnamed)
  0x0062A900  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006934a0 FUN_006934a0 size=114 ret=8 callers=0 callees=4

undefined4 FUN_006934a0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c220(local_c);
      if (iVar1 != 0) {
        FUN_0062a900(local_8);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00693380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00693380: 16-byte aligned: YES
  00693380  55             PUSH EBP
  00693381  8B EC          MOV EBP,ESP
  00693383  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00693380
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0051C220  (unnamed)
  0x0062A840  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-3-35

GOG function: Global::ExecuteCommandVector  gog 0x007aaac0  callconv=(blank)  param_size_bytes=0  gog_size=140  gog_retn=8
Candidate Steam address: 0x00695470 (size 140, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006956f0:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007aa9c0 Global::ExecuteCommandUnequipItem -> 0x00695370  gate_class=AUTO
- GOG-order neighbour after:  0x007aab50 Global::ExecuteCommandVectorConversions -> 0x00695500  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [142 Vector] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640c60 CVirtualMachine::StackPushVector -> Steam 0x006fda40  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 5 common): GOG {0x8*,0xc*,0x10*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0x10*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00695470: 16-byte aligned: YES
  00695470  55             PUSH EBP
  00695471  8B EC          MOV EBP,ESP
  00695473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00695470
string literals (0):
direct callees (2):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00695470 FUN_00695470 size=140 ret=8 callers=0 callees=2

undefined4 FUN_00695470(void)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  iVar1 = FUN_006fd9e0(&local_10);
  if (((iVar1 != 0) && (iVar1 = FUN_006fd9e0(&local_c), iVar1 != 0)) &&
     (iVar1 = FUN_006fd9e0(&local_8), iVar1 != 0)) {
    iVar1 = FUN_006fda40(local_10,local_c,local_8);
    if (iVar1 == 0) {
      return 0xfffff830;
    }
    return 0;
  }
  return 0xfffff82f;
}
```

### decoy: 0x006956f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006956f0: 16-byte aligned: YES
  006956f0  55             PUSH EBP
  006956f1  8B EC          MOV EBP,ESP
  006956f3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006956f0
string literals (0):
direct callees (4):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x00517980  Vec3::Equal
  0x005143C0  (unnamed)
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
```

## Card B15-3-36

GOG function: Global::ExecuteCommandGetGold  gog 0x00793430  callconv=__thiscall  param_size_bytes=8  gog_size=248  gog_retn=8
Candidate Steam address: 0x0067ddd0 (size 121, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetGoingToBeAttackedBy | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.34 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067de50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007933b0 Global::ExecuteCommandGetGoingToBeAttackedBy -> 0x0067ddd0  gate_class=AUTO
- GOG-order neighbour after:  0x00793530 Global::ExecuteCommandGetGoldPieceValue -> 0x0067df50  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [418 GetGold, 771 GetItemComponent] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x006693b0 CItemRepository::FindItemWithBaseItemType -> Steam 0x00605b80  in candidate callees: no
  0x00699e10 CSWSCreature::GetGold -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x2cc,0x3c4} | Steam {0x8*,0x564}
imm sets (* = in both, 3 common): GOG {0x8*,0x17,0x30,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14,0x7f000000,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067ddd0: 16-byte aligned: YES
  0067ddd0  55             PUSH EBP
  0067ddd1  8B EC          MOV EBP,ESP
  0067ddd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ddd0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x564 [dec 1380]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ddd0 FUN_0067ddd0 size=121 ret=8 callers=0 callees=3

undefined4 FUN_0067ddd0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    iVar1 = FUN_0051c100(local_c);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x564);
    }
    iVar1 = FUN_006fdb10(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0067de50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067de50: 16-byte aligned: YES
  0067de50  55             PUSH EBP
  0067de51  8B EC          MOV EBP,ESP
  0067de53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067de50
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005812E0  CSWSCreature::GetGold
  0x0051C160  ServerApp::GetPlaceableById
  0x00605B80  CItemRepository::FindItemWithBaseItemType
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x3C4 [dec 964]  x2  via EAX,ECX
  +0x2CC [dec 716]  x1  via EDX
```

## Card B15-3-37

GOG function: Global::ExecuteCommandPrintInteger  gog 0x007a4710  callconv=__thiscall  param_size_bytes=8  gog_size=84  gog_retn=8
Candidate Steam address: 0x0068f0c0 (size 84, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068f200:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a4540 Global::ExecuteCommandPrintFloat -> 0x0068eef0  gate_class=AUTO
- GOG-order neighbour after:  0x007a4770 Global::ExecuteCommandPrintLogEntry -> 0x0068f120  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [4 PrintInteger] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0xc*,0xfffff82f*} | Steam {0x2*,0x8*,0xc*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068f0c0: 16-byte aligned: YES
  0068f0c0  55             PUSH EBP
  0068f0c1  8B EC          MOV EBP,ESP
  0068f0c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f0c0
string literals (0):
direct callees (1):
  0x006FD9A0  CVirtualMachine::StackPopInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068f0c0 FUN_0068f0c0 size=84 ret=8 callers=0 callees=1

undefined4 FUN_0068f0c0(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  iVar1 = FUN_006fd9a0(local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else if ((param_2 < 2) || (iVar1 = FUN_006fd9a0(local_8), iVar1 != 0)) {
    uVar2 = 0;
  }
  else {
    uVar2 = 0xfffff82f;
  }
  return uVar2;
}
```

### decoy: 0x0068f200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f200: 16-byte aligned: YES
  0068f200  55             PUSH EBP
  0068f201  8B EC          MOV EBP,ESP
  0068f203  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f200
string literals (0):
direct callees (2):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
struct offsets touched (top 0):
```

## Card B15-3-38

GOG function: Global::ExecuteCommandEffectAbilityIncrease  gog 0x00786e00  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x00671660 (size 447, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectAbilityDecrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00786c40 Global::ExecuteCommandEffectAbilityDecrease -> 0x00671660  gate_class=REVIEW
- GOG-order neighbour after:  0x00786fc0 Global::ExecuteCommandEffectACDecrease -> 0x006719e0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [80 EffectAbilityIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x38*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x38*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00671660: 16-byte aligned: YES
  00671660  55             PUSH EBP
  00671661  8B EC          MOV EBP,ESP
  00671663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00671660
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00671660 FUN_00671660 size=447 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00671660(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_44;
  undefined2 local_38;
  int local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959f2b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_1c);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_18);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if ((local_1c < 0) || (DAT_0099610c < local_1c)) {
        local_1c = DAT_009960f8;
      }
      iVar2 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar2 == 0) {
        local_44 = 0;
      }
      else {
        local_44 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_20 = local_44;
      local_38 = (undefined2)DAT_00995fb0;
      *(undefined2 *)(local_44 + 8) = local_38;
      *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_1c);
      FUN_005e4e80(1,local_18);
      piVar5 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_11 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_11 == DAT_0099447f) && (local_24 != 0)) {
        FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      }
      iVar4 = FUN_006fdad0(0,local_20);
      iVar2 = local_20;
      if (iVar4 == 0) {
        uVar3 = 0xfffff830;
      }
      else {
        if (local_20 != 0) {
          FUN_005e4ca0();
          FUN_009196fd(iVar2);
        }
        uVar3 = 0;
      }
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00671820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00671820: 16-byte aligned: YES
  00671820  55             PUSH EBP
  00671821  8B EC          MOV EBP,ESP
  00671823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00671820
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-3-39

GOG function: Global::ExecuteCommandGetLastAttackMode  gog 0x00797010  callconv=__thiscall  param_size_bytes=8  gog_size=283  gog_retn=8
Candidate Steam address: 0x00681a40 (size 283, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006819c0:STR0/CALLEE0/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00796f90 Global::ExecuteCommandGetLastAttackAction -> 0x006819c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00797150 Global::ExecuteCommandGetLastAttackResult -> 0x00681b80  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [318 GetLastAttackMode] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0x30*,0x192*,0x520*} | Steam {0x8*,0x30*,0x192*,0x520*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x24*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x24*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00681a40: 16-byte aligned: YES
  00681a40  55             PUSH EBP
  00681a41  8B EC          MOV EBP,ESP
  00681a43  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00681a40
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 4):
  +0x30 [dec 48]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x192 [dec 402]  x1  via EAX
  +0x520 [dec 1312]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00681a40 FUN_00681a40 size=283 ret=8 callers=0 callees=4

undefined4 FUN_00681a40(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int **ppiVar4;
  int *local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar2 = FUN_006fdaf0(&local_c);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    local_8 = 0x7f000000;
    ppiVar4 = &local_10;
    FUN_0051c080(local_c,ppiVar4);
    cVar1 = FUN_0053dfb0(local_c,ppiVar4);
    if (((cVar1 == DAT_0099447f) && (iVar2 = (**(code **)(*local_10 + 0x30))(), iVar2 != 0)) &&
       (iVar2 = (**(code **)(*local_10 + 0x30))(), *(int *)(iVar2 + 0x520) != 0)) {
      switch(*(undefined1 *)(iVar2 + 0x192)) {
      case 1:
        local_8 = 1;
        break;
      case 2:
        local_8 = 2;
        break;
      case 3:
        local_8 = 3;
        break;
      default:
        local_8 = 0;
        break;
      case 5:
        local_8 = 4;
        break;
      case 6:
        local_8 = 5;
      }
    }
    iVar2 = FUN_006fd9c0(local_8);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x006819c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006819c0: 16-byte aligned: YES
  006819c0  55             PUSH EBP
  006819c1  8B EC          MOV EBP,ESP
  006819c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006819c0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x588 [dec 1416]  x1  via ECX
```

## Card B15-3-40

GOG function: Global::ExecuteCommandGetStrByStrRef  gog 0x0079e1b0  callconv=__thiscall  param_size_bytes=8  gog_size=267  gog_retn=8
Candidate Steam address: 0x00688b60 (size 267, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00688c70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e110 Global::ExecuteCommandGetStat -> 0x00688ac0  gate_class=AUTO
- GOG-order neighbour after:  0x0079e2c0 Global::ExecuteCommandGetStringLeft -> 0x00688c70  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [239 GetStringByStrRef] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00627ef0 CTlkTable::Fetch -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00688b60: 16-byte aligned: YES
  00688b60  55             PUSH EBP
  00688b61  8B EC          MOV EBP,ESP
  00688b63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00688b60
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x0071DF90  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00688b60 FUN_00688b60 size=267 ret=8 callers=0 callees=8

void FUN_00688b60(void)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  undefined4 local_3c;
  undefined1 local_38 [28];
  uint local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a5e8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = uVar1;
  iVar2 = FUN_006fd9a0(&local_3c);
  if (iVar2 != 0) {
    uVar3 = extraout_var;
    FUN_00733540(uVar1,extraout_var);
    local_8 = 0;
    FUN_007107f0();
    local_8 = 1;
    FUN_0071df90(local_3c,local_38,DAT_0099c4a2);
    FUN_007335f0(local_38);
    local_8._0_1_ = 2;
    iVar2 = FUN_006fda90(local_18);
    if (iVar2 == 0) {
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    else {
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780(uVar1,uVar3,0);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_1c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00688c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00688c70: 16-byte aligned: YES
  00688c70  55             PUSH EBP
  00688c71  8B EC          MOV EBP,ESP
  00688c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00688c70
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-41

GOG function: Global::ExecuteCommandGetTimeMillisecond  gog 0x0079ed70  callconv=__thiscall  param_size_bytes=8  gog_size=72  gog_retn=8
Candidate Steam address: 0x00689720 (size 72, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetWorldTimeMillisecond 0x51b1c0)
- best decoy (neighbour/other candidate) scores: 0x006896d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079ed20 Global::ExecuteCommandGetTimeHour -> 0x006896d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0079edc0 Global::ExecuteCommandGetTimeMinute -> 0x00689770  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [19 GetTimeMillisecond] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064bd90 CServerExoApp::GetWorldTimer -> Steam 0x0051c370  in candidate callees: yes
  0x0080b6a0 CWorldTimer::GetWorldTimeMillisecond -> Steam 0x0051b1c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00689720: 16-byte aligned: YES
  00689720  55             PUSH EBP
  00689721  8B EC          MOV EBP,ESP
  00689723  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00689720
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B1C0  CWorldTimer::GetWorldTimeMillisecond
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00689720 FUN_00689720 size=72 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_00689720(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  FUN_0051c370(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  uVar1 = FUN_0051b1c0();
  iVar2 = FUN_006fd9c0(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0xfffff830;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x006896d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006896d0: 16-byte aligned: YES
  006896d0  55             PUSH EBP
  006896d1  8B EC          MOV EBP,ESP
  006896d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006896d0
string literals (0):
direct callees (3):
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B100  CWorldTimer::GetWorldTimeHour
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-42

GOG function: Global::ExecuteCommandEffectSpellLevelAbsorption  gog 0x0078bfe0  callconv=__thiscall  param_size_bytes=8  gog_size=470  gog_retn=8
Candidate Steam address: 0x00676880 (size 376, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectSpellImmunity | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.68 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676a00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078be60 Global::ExecuteCommandEffectSpellImmunity -> 0x00676880  gate_class=AUTO
- GOG-order neighbour after:  0x0078c1c0 Global::ExecuteCommandEffectForceResistanceDecrease -> 0x00676be0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [472 EffectSpellLevelAbsorption] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*,0x104}
imm sets (* = in both, 7 common): GOG {0x2,0x3,0x4*,0x8*,0x9,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676880: 16-byte aligned: YES
  00676880  55             PUSH EBP
  00676881  8B EC          MOV EBP,ESP
  00676883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00676880
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x104 [dec 260]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00676880 FUN_00676880 size=376 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00676880(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_40;
  undefined2 local_38;
  undefined2 local_34;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0eb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_14);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_40 = 0;
    }
    else {
      local_40 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    if ((local_14 < -1) || (**(int **)(DAT_00a1b4d0 + 0x104) < local_14)) {
      local_34 = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_40 + 8) = local_34;
    }
    else {
      local_38 = (undefined2)DAT_00995fe4;
      *(undefined2 *)(local_40 + 8) = local_38;
      *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      FUN_005e4e80(0,local_14);
    }
    iVar2 = FUN_006fdad0(0,local_40);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      if (local_40 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(local_40);
      }
      uVar3 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00676a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676a00: 16-byte aligned: YES
  00676a00  55             PUSH EBP
  00676a01  8B EC          MOV EBP,ESP
  00676a03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00676a00
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
```

## Card B15-3-43

GOG function: Global::ExecuteCommandGetPartyLeader  gog 0x0079cce0  callconv=__thiscall  param_size_bytes=8  gog_size=123  gog_retn=8
Candidate Steam address: 0x0069b7c0 (size 123, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandRemoveNPCFromPartyToBase | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006874f0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079cc80 Global::ExecuteCommandGetPCLevellingUp -> 0x006876b0  gate_class=AUTO
- GOG-order neighbour after:  0x0079cd60 Global::ExecuteCommandGetPlaceableIllumination -> 0x00687710  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [238 GetPCSpeaker, 845 GetPartyLeader] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x18*,0x7f000000*,0xfffff830*} | Steam {0x8*,0x18*,0x7f000000*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069b7c0: 16-byte aligned: YES
  0069b7c0  55             PUSH EBP
  0069b7c1  8B EC          MOV EBP,ESP
  0069b7c3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069b7c0
string literals (0):
direct callees (4):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069b7c0 FUN_0069b7c0 size=123 ret=8 callers=0 callees=4

undefined4 __fastcall FUN_0069b7c0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0x7f000000;
  uVar2 = 0;
  FUN_0073fb90(0,param_1);
  iVar1 = FUN_007e5da0(uVar2);
  if (iVar1 != 0) {
    local_8 = FUN_0051c8b0(*(undefined4 *)(iVar1 + 4));
  }
  iVar1 = FUN_006fdb10(local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x006874f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006874f0: 16-byte aligned: YES
  006874f0  55             PUSH EBP
  006874f1  8B EC          MOV EBP,ESP
  006874f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006874f0
string literals (0):
direct callees (5):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-3-44

GOG function: Global::ExecuteCommandGetStringLowerCase  gog 0x0079e520  callconv=__thiscall  param_size_bytes=8  gog_size=289  gog_retn=8
Candidate Steam address: 0x00688ed0 (size 289, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00689170:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e430 Global::ExecuteCommandGetStringLength -> 0x00688de0  gate_class=REVIEW
- GOG-order neighbour after:  0x0079e650 Global::ExecuteCommandGetStringRight -> 0x00689000  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [61 GetStringLowerCase] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006067a0 CExoString::LowerCase -> Steam ?  in candidate callees: n/a
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x30*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00688ed0: 16-byte aligned: YES
  00688ed0  55             PUSH EBP
  00688ed1  8B EC          MOV EBP,ESP
  00688ed3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00688ed0
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00688ed0 FUN_00688ed0 size=289 ret=8 callers=0 callees=6

undefined4 FUN_00688ed0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_2c [8];
  undefined4 local_24;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a668;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  iVar1 = FUN_006fda70(local_20);
  if (iVar1 == 0) {
    local_24 = 0xfffff82f;
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    uVar2 = FUN_00734690(local_2c);
    local_8._0_1_ = 2;
    FUN_007337c0(uVar2);
    local_8._0_1_ = 1;
    FUN_00733780();
    iVar1 = FUN_006fda90(local_18);
    if (iVar1 == 0) {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_24 = 0xfffff830;
    }
    else {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_24 = 0;
    }
  }
  ExceptionList = local_10;
  return local_24;
}
```

### decoy: 0x00689170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689170: 16-byte aligned: YES
  00689170  55             PUSH EBP
  00689171  8B EC          MOV EBP,ESP
  00689173  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00689170
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734B80  CExoString::UpperCase
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-45

GOG function: Global::ExecuteCommandRemoveEffect  gog 0x007a4e30  callconv=__thiscall  param_size_bytes=8  gog_size=457  gog_retn=8
Candidate Steam address: 0x0068f7e0 (size 457, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0068fb20:STR0/CALLEE1/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a4dc0 Global::ExecuteCommandRemoveAvailableNPC -> 0x0068f770  gate_class=AUTO
- GOG-order neighbour after:  0x007a5000 Global::ExecuteCommandRemoveEffectByID -> 0x0068f9b0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [87 RemoveEffect] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x20*} | Steam {0x4*,0x8*,0x10*,0x20*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x50*,0x148*,0xfffff82f*} | Steam {0x4*,0x8*,0x50*,0x148*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068f7e0: 16-byte aligned: YES
  0068f7e0  55             PUSH EBP
  0068f7e1  8B EC          MOV EBP,ESP
  0068f7e3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f7e0
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x20 [dec 32]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0068f7e0 FUN_0068f7e0 size=457 ret=8 callers=0 callees=7

undefined4 __fastcall FUN_0068f7e0(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  uint local_1c;
  int local_14;
  int *local_10;
  int *local_c;
  undefined4 local_8;
  
  local_10 = (int *)0x0;
  iVar2 = FUN_006fdaf0(&local_8);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fdab0(0,&local_10);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      local_c = (int *)FUN_0051c0a0(local_8);
      if ((local_c != (int *)0x0) && (iVar2 = (**(code **)(*local_c + 0x10))(), iVar2 != 0)) {
        iVar2 = (**(code **)(*local_c + 0x10))();
        piVar4 = (int *)(iVar2 + 0x148);
        local_14 = 0;
        if ((piVar4 != (int *)0x0) && (0 < *(int *)(iVar2 + 0x14c))) {
          for (local_1c = 0; local_1c < *(uint *)(iVar2 + 0x14c); local_1c = local_1c + 1) {
            piVar1 = *(int **)(*piVar4 + local_1c * 4);
            if ((*piVar1 == *local_10) && (piVar1[1 = byte +0x4] == local_10[1 = byte +0x4])) {
              local_14 = *(int *)(*piVar4 + local_1c * 4);
              *(undefined4 *)(*(int *)(*piVar4 + local_1c * 4) + 0x20) = 0;
            }
          }
          if (local_14 != 0) {
            uVar3 = *(undefined4 *)(param_1 + 8);
            uVar6 = 0;
            uVar5 = 0;
            uVar7 = DAT_0099242c;
            FUN_0051c280(0,0,uVar3,local_8,DAT_0099242c,local_14);
            FUN_0051e170(uVar5,uVar6,uVar3,local_8,uVar7,local_14);
          }
        }
      }
      piVar4 = local_10;
      if (local_10 != (int *)0x0) {
        FUN_005e4ca0();
        FUN_009196fd(piVar4);
      }
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x0068fb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068fb20: 16-byte aligned: YES
  0068fb20  55             PUSH EBP
  0068fb21  8B EC          MOV EBP,ESP
  0068fb23  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068fb20
string literals (0):
direct callees (8):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E4E60  CGameEffect::GetInteger
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x20 [dec 32]  x1  via EDX
```

## Card B15-3-46

GOG function: Global::ExecuteCommandGetStringRight  gog 0x0079e650  callconv=__thiscall  param_size_bytes=8  gog_size=354  gog_retn=8
Candidate Steam address: 0x00689000 (size 354, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00688c70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e520 Global::ExecuteCommandGetStringLowerCase -> 0x00688ed0  gate_class=REVIEW
- GOG-order neighbour after:  0x0079e7c0 Global::ExecuteCommandGetStringUpperCase -> 0x00689170  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [62 GetStringRight] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606980 CExoString::Right -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x38*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00689000: 16-byte aligned: YES
  00689000  55             PUSH EBP
  00689001  8B EC          MOV EBP,ESP
  00689003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00689000
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734870  CExoString::Right
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00689000 FUN_00689000 size=354 ret=8 callers=0 callees=7

undefined4 FUN_00689000(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_34 [8];
  undefined4 local_2c;
  undefined4 local_28;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a628;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  iVar1 = FUN_006fda70(local_24);
  if (iVar1 == 0) {
    local_28 = 0xfffff82f;
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    iVar1 = FUN_006fd9a0(&local_14);
    if (iVar1 == 0) {
      local_2c = 0xfffff82f;
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      local_28 = local_2c;
    }
    else {
      uVar2 = FUN_00734870(local_34,local_14);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar2);
      local_8._0_1_ = 1;
      FUN_00733780();
      iVar1 = FUN_006fda90(local_1c);
      if (iVar1 == 0) {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        local_28 = 0xfffff830;
      }
      else {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        local_28 = 0;
      }
    }
  }
  ExceptionList = local_10;
  return local_28;
}
```

### decoy: 0x00688c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00688c70: 16-byte aligned: YES
  00688c70  55             PUSH EBP
  00688c71  8B EC          MOV EBP,ESP
  00688c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00688c70
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-47

GOG function: Global::ExecuteCommandGetLastHostileTarget  gog 0x00796f10  callconv=__thiscall  param_size_bytes=8  gog_size=128  gog_retn=8
Candidate Steam address: 0x00681940 (size 128, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006819c0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00796e60 Global::ExecuteCommandGetLastAttacker -> 0x00681890  gate_class=REVIEW
- GOG-order neighbour after:  0x00796f90 Global::ExecuteCommandGetLastAttackAction -> 0x006819c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [721 GetLastHostileTarget] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x580*} | Steam {0x8*,0x580*}
imm sets (* = in both, 5 common): GOG {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00681940: 16-byte aligned: YES
  00681940  55             PUSH EBP
  00681941  8B EC          MOV EBP,ESP
  00681943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00681940
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x580 [dec 1408]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00681940 FUN_00681940 size=128 ret=8 callers=0 callees=3

undefined4 FUN_00681940(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0x7f000000;
  local_8 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c100(local_c);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x580);
    }
    iVar1 = FUN_006fdb10(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006819c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006819c0: 16-byte aligned: YES
  006819c0  55             PUSH EBP
  006819c1  8B EC          MOV EBP,ESP
  006819c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006819c0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x588 [dec 1416]  x1  via ECX
```

## Card B15-3-48

GOG function: Global::ExecuteCommandIsStealthed  gog 0x007a1340  callconv=(blank)  param_size_bytes=0  gog_size=122  gog_retn=8
Candidate Steam address: 0x0068bcf0 (size 122, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068be00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007a12b0 Global::ExecuteCommandIsPC -> 0x0068bc60  gate_class=AUTO
- GOG-order neighbour after:  0x007a13c0 Global::ExecuteCommandIsMeditating -> 0x0068bd70  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [810 IsStealthed] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0067c9a0 CSWSCreature::GetActivity -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x14*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0068bcf0: 16-byte aligned: YES
  0068bcf0  55             PUSH EBP
  0068bcf1  8B EC          MOV EBP,ESP
  0068bcf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068bcf0
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00563D70  CSWSCreature::GetActivity
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0068bcf0 FUN_0068bcf0 size=122 ret=8 callers=0 callees=4

undefined4 FUN_0068bcf0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c100(local_8);
    if (iVar1 != 0) {
      local_10 = FUN_00563d70(1);
    }
    iVar1 = FUN_006fd9c0(local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068be00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068be00: 16-byte aligned: YES
  0068be00  55             PUSH EBP
  0068be01  8B EC          MOV EBP,ESP
  0068be03  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068be00
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DAAC0  CSWSCreature::IsInTotalDefense
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-3-49

GOG function: Global::ExecuteCommandGetLastAttackAction  gog 0x00796f90  callconv=__thiscall  param_size_bytes=8  gog_size=128  gog_retn=8
Candidate Steam address: 0x006819c0 (size 128, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetLastAttackMode | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00681940:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00796f10 Global::ExecuteCommandGetLastHostileTarget -> 0x00681940  gate_class=REVIEW
- GOG-order neighbour after:  0x00797010 Global::ExecuteCommandGetLastAttackMode -> 0x00681a40  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [722 GetLastAttackAction] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x588*} | Steam {0x8*,0x588*}
imm sets (* = in both, 5 common): GOG {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006819c0: 16-byte aligned: YES
  006819c0  55             PUSH EBP
  006819c1  8B EC          MOV EBP,ESP
  006819c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006819c0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x588 [dec 1416]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006819c0 FUN_006819c0 size=128 ret=8 callers=0 callees=3

undefined4 FUN_006819c0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0x7f000000;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c100(local_c);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x588);
    }
    iVar1 = FUN_006fd9c0(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00681940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00681940: 16-byte aligned: YES
  00681940  55             PUSH EBP
  00681941  8B EC          MOV EBP,ESP
  00681943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00681940
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x580 [dec 1408]  x1  via ECX
```

## Card B15-3-50

GOG function: Global::ExecuteCommandIsAIState  gog 0x007a0b00  callconv=__thiscall  param_size_bytes=8  gog_size=313  gog_retn=8
Candidate Steam address: 0x0068b3c0 (size 229, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandIntToString | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068b4b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a0a10 Global::ExecuteCommandIntToString -> 0x0068b3c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007a0c40 Global::ExecuteCommandIsAvailableNPC -> 0x0068b5f0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [140 GetIsDead] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: no
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x30,0x94,0x9c,0x11ac} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x24*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x24*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068b3c0: 16-byte aligned: YES
  0068b3c0  55             PUSH EBP
  0068b3c1  8B EC          MOV EBP,ESP
  0068b3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0068b3c0
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x00733700  CExoString::Constructor_5
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068b3c0 FUN_0068b3c0 size=229 ret=8 callers=0 callees=6

undefined4 __fastcall FUN_0068b3c0(undefined4 param_1)

{
  int iVar1;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a7d0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  iVar1 = FUN_006fd9a0(&local_1c);
  if (iVar1 == 0) {
    local_20 = 0xfffff82f;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    FUN_00733700(local_1c);
    local_8._0_1_ = 1;
    FUN_007337c0(local_28);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    iVar1 = FUN_006fda90(local_18);
    if (iVar1 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_20 = 0xfffff830;
    }
    else {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_20 = 0;
    }
  }
  ExceptionList = local_10;
  return local_20;
}
```

### decoy: 0x0068b4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068b4b0: 16-byte aligned: YES
  0068b4b0  55             PUSH EBP
  0068b4b1  8B EC          MOV EBP,ESP
  0068b4b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068b4b0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x94 [dec 148]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

## Card B15-3-51

GOG function: Global::ExecuteCommandShipBuild  gog 0x007a8290  callconv=__thiscall  param_size_bytes=8  gog_size=60  gog_retn=8
Candidate Steam address: 0x00692c40 (size 60, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00692c80:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a81d0 Global::ExecuteCommandSetXP -> 0x00692b80  gate_class=AUTO
- GOG-order neighbour after:  0x007a82d0 Global::ExecuteCommandShowLevelUpGUI -> 0x00692c80  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [761 ShipBuild] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xfffff830*} | Steam {0x8*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00692c40: 16-byte aligned: YES
  00692c40  55             PUSH EBP
  00692c41  8B EC          MOV EBP,ESP
  00692c43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00692c40
string literals (0):
direct callees (1):
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00692c40 FUN_00692c40 size=60 ret=8 callers=0 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00692c40(void)

{
  int iVar1;
  undefined4 uVar2;
  
  _DAT_00a7e114 = 1;
  iVar1 = FUN_006fd9c0(1);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00692c80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00692c80: 16-byte aligned: YES
  00692c80  55             PUSH EBP
  00692c81  8B EC          MOV EBP,ESP
  00692c83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00692c80
string literals (0):
direct callees (3):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CB7C0  CGuiInGame::ShowLevelUpGUI
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B15-3-52

GOG function: Global::ExecuteCommandRollDice  gog 0x007a5df0  callconv=__thiscall  param_size_bytes=8  gog_size=253  gog_retn=8
Candidate Steam address: 0x006907a0 (size 253, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006900f0:STR0/CALLEE0/SIG0.194/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a5740 Global::ExecuteCommandResistSpell -> 0x006900f0  gate_class=AUTO
- GOG-order neighbour after:  0x007a5f20 Global::ExecuteCommandSaveNPCState -> 0x006908d0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [95 d2, 96 d3, 97 d4, 98 d6, 99 d8, 100 d10, 101 d12, 102 d20, 103 d100] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0050e640 CSWRules::RollDice -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x6*,0x8*,0xa*,0xc*,0x14*,0x18*,0x5f*,0x64*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x3*,0x4*,0x6*,0x8*,0xa*,0xc*,0x14*,0x18*,0x5f*,0x64*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006907a0: 16-byte aligned: YES
  006907a0  55             PUSH EBP
  006907a1  8B EC          MOV EBP,ESP
  006907a3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006907a0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006A2000  CSWRules::RollDice(n,sides)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006907a0 FUN_006907a0 size=253 ret=8 callers=0 callees=3

undefined4 FUN_006907a0(undefined4 param_1,int param_2)

{
  undefined2 uVar1;
  int iVar2;
  undefined4 uVar3;
  uint local_1c;
  uint local_c;
  uint local_8;
  
  local_c = 1;
  switch(param_1) {
  case 0x5f:
    local_8 = 2;
    break;
  case 0x60:
    local_8 = 3;
    break;
  case 0x61:
    local_8 = 4;
    break;
  case 0x62:
    local_8 = 6;
    break;
  case 99:
    local_8 = 8;
    break;
  case 100:
    local_8 = 10;
    break;
  case 0x65:
    local_8 = 0xc;
    break;
  case 0x66:
    local_8 = 0x14;
    break;
  case 0x67:
    local_8 = 100;
  }
  if ((param_2 == 1) && (iVar2 = FUN_006fd9a0(&local_c), iVar2 == 0)) {
    uVar3 = 0xfffff82f;
  }
  else {
    if ((int)local_c < 2) {
      local_1c = 1;
    }
    else {
      local_1c = local_c;
    }
    local_c = local_1c;
    uVar1 = FUN_006a2000(local_1c & 0xff,local_8 & 0xff);
    iVar2 = FUN_006fd9c0(uVar1);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}
```

### decoy: 0x006900f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006900f0: 16-byte aligned: YES
  006900f0  55             PUSH EBP
  006900f1  8B EC          MOV EBP,ESP
  006900f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006900f0
string literals (0):
direct callees (18):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006C1450  (unnamed)
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005433A0  CSWSObject::DoSpellLevelAbsorption
  0x00543AE0  CSWSObject::DoSpellImmunity
  0x006BAD10  CSWSCreatureStats::GetForceResistance
  0x006A2000  CSWRules::RollDice(n,sides)
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E1BD0  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0051C2E0  (unnamed)
  0x00648CC0  (unnamed)
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 10):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x43 [dec 67]  x2  via EAX,EDX
  +0x50 [dec 80]  x2  via EDX
  +0x14C [dec 332]  x2  via EAX,ECX
  +0x1F8 [dec 504]  x2  via EDX
  +0x1198 [dec 4504]  x2  via ECX,EDX
  +0x104 [dec 260]  x1  via ECX
```

## Card B15-3-53

GOG function: Global::ExecuteCommandEffectSpellLevelAbsorption  gog 0x0078bfe0  callconv=__thiscall  param_size_bytes=8  gog_size=470  gog_retn=8
Candidate Steam address: 0x00676be0 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectForceResistanceDecrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.77 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676a00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078be60 Global::ExecuteCommandEffectSpellImmunity -> 0x00676880  gate_class=AUTO
- GOG-order neighbour after:  0x0078c1c0 Global::ExecuteCommandEffectForceResistanceDecrease -> 0x00676be0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [472 EffectSpellLevelAbsorption] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*}
imm sets (* = in both, 6 common): GOG {0x2,0x3,0x4*,0x8*,0x9,0x34,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x28,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676be0: 16-byte aligned: YES
  00676be0  55             PUSH EBP
  00676be1  8B EC          MOV EBP,ESP
  00676be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00676be0
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00676be0 FUN_00676be0 size=317 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00676be0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_34;
  undefined2 local_2c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f4eb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_14);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_34 = 0;
    }
    else {
      local_34 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_2c = (undefined2)DAT_00995fa4;
    *(undefined2 *)(local_34 + 8) = local_2c;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
    FUN_005e4e80(0,local_14);
    *(ushort *)(local_34 + 0xa [dec 10]) = *(ushort *)(local_34 + 0xa [dec 10]) & 0xffe7 | 8;
    iVar2 = FUN_006fdad0(0,local_34);
    if (iVar2 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      if (local_34 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(local_34);
      }
      uVar3 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00676a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676a00: 16-byte aligned: YES
  00676a00  55             PUSH EBP
  00676a01  8B EC          MOV EBP,ESP
  00676a03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00676a00
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
```

## Card B15-3-54

GOG function: Global::ExecuteCommandGetStringLength  gog 0x0079e430  callconv=__thiscall  param_size_bytes=8  gog_size=226  gog_retn=8
Candidate Steam address: 0x00688de0 (size 226, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00688c70:STR0/CALLEE0/SIG0.214/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0079e2c0 Global::ExecuteCommandGetStringLeft -> 0x00688c70  gate_class=REVIEW
- GOG-order neighbour after:  0x0079e520 Global::ExecuteCommandGetStringLowerCase -> 0x00688ed0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [59 GetStringLength] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00921b40 Global::strlen -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x1c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x1c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00688de0: 16-byte aligned: YES
  00688de0  55             PUSH EBP
  00688de1  8B EC          MOV EBP,ESP
  00688de3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00688de0
string literals (0):
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00688de0 FUN_00688de0 size=226 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_00688de0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  char *local_1c [2];
  size_t local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968348;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  local_14 = 0xffffffff;
  iVar1 = FUN_006fda70(local_1c);
  if (iVar1 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else {
    if (local_1c[0] == (char *)0x0) {
      local_14 = 0;
    }
    else {
      local_14 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_1c[0]);
    }
    iVar1 = FUN_006fd9c0(local_14);
    if (iVar1 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0xfffff830;
    }
    else {
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00688c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00688c70: 16-byte aligned: YES
  00688c70  55             PUSH EBP
  00688c71  8B EC          MOV EBP,ESP
  00688c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00688c70
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-55

GOG function: Global::ExecuteCommandIntToString  gog 0x007a0a10  callconv=__thiscall  param_size_bytes=8  gog_size=229  gog_retn=8
Candidate Steam address: 0x0068b3c0 (size 229, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068b110:STR0/CALLEE1/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a0930 Global::ExecuteCommandIntToHexString -> 0x0068b2e0  gate_class=AUTO
- GOG-order neighbour after:  0x007a0b00 Global::ExecuteCommandIsAIState -> 0x0068b3c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [92 IntToString] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x24*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x24*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068b3c0: 16-byte aligned: YES
  0068b3c0  55             PUSH EBP
  0068b3c1  8B EC          MOV EBP,ESP
  0068b3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0068b3c0
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x00733700  CExoString::Constructor_5
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068b3c0 FUN_0068b3c0 size=229 ret=8 callers=0 callees=6

undefined4 __fastcall FUN_0068b3c0(undefined4 param_1)

{
  int iVar1;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a7d0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  iVar1 = FUN_006fd9a0(&local_1c);
  if (iVar1 == 0) {
    local_20 = 0xfffff82f;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    FUN_00733700(local_1c);
    local_8._0_1_ = 1;
    FUN_007337c0(local_28);
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    iVar1 = FUN_006fda90(local_18);
    if (iVar1 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_20 = 0xfffff830;
    }
    else {
      local_8 = 0xffffffff;
      FUN_00733780();
      local_20 = 0;
    }
  }
  ExceptionList = local_10;
  return local_20;
}
```

### decoy: 0x0068b110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068b110: 16-byte aligned: YES
  0068b110  55             PUSH EBP
  0068b111  8B EC          MOV EBP,ESP
  0068b113  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0068b110
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x007343F0  CExoString::Insert
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-56

GOG function: Global::ExecuteCommandEffectAbilityIncrease  gog 0x00786e00  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x006719e0 (size 534, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectACDecrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.61 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671820:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00786c40 Global::ExecuteCommandEffectAbilityDecrease -> 0x00671660  gate_class=REVIEW
- GOG-order neighbour after:  0x00786fc0 Global::ExecuteCommandEffectACDecrease -> 0x006719e0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [80 EffectAbilityIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*,0x122}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x38,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2,0x3,0x4*,0x5,0x8*,0x40,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006719e0: 16-byte aligned: YES
  006719e0  55             PUSH EBP
  006719e1  8B EC          MOV EBP,ESP
  006719e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006719e0
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006719e0 FUN_006719e0 size=534 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_006719e0(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_4c;
  undefined2 local_3c;
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  undefined4 local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959f5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_20);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      local_28 = DAT_0099614c;
      if ((param_3 < 3) || (iVar2 = FUN_006fd9a0(&local_28), iVar2 != 0)) {
        if ((local_20 < 0) || (DAT_00996180 < local_20)) {
          local_20 = 0;
        }
        iVar2 = FUN_00919723(0x8c,uVar1);
        local_8 = 0;
        if (iVar2 == 0) {
          local_4c = 0;
        }
        else {
          local_4c = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        local_1c = local_4c;
        local_3c = (undefined2)DAT_00995fe0;
        *(undefined2 *)(local_4c + 8) = local_3c;
        *(ushort *)(local_4c + 0xa [dec 10]) = *(ushort *)(local_4c + 0xa [dec 10]) & 0xffe7 | 8;
        FUN_005e4e80(0,local_20);
        FUN_005e4e80(1,local_18);
        FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
        FUN_005e4e80(5,local_28);
        piVar5 = &local_24;
        uVar3 = *(undefined4 *)(param_1 + 8);
        FUN_0051c080(uVar3,piVar5);
        local_11 = FUN_0053dfb0(uVar3,piVar5);
        if ((local_11 == DAT_0099447f) && (local_24 != 0)) {
          FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
        }
        iVar4 = FUN_006fdad0(0,local_1c);
        iVar2 = local_1c;
        if (iVar4 == 0) {
          uVar3 = 0xfffff830;
        }
        else {
          if (local_1c != 0) {
            FUN_005e4ca0();
            FUN_009196fd(iVar2);
          }
          uVar3 = 0;
        }
      }
      else {
        uVar3 = 0xfffff82f;
/* ... 6 more lines truncated by --max-lines */
```

### decoy: 0x00671820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00671820: 16-byte aligned: YES
  00671820  55             PUSH EBP
  00671821  8B EC          MOV EBP,ESP
  00671823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00671820
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-3-57

GOG function: Global::ExecuteCommandInsertString  gog 0x007a0760  callconv=__thiscall  param_size_bytes=8  gog_size=375  gog_retn=8
Candidate Steam address: 0x0068b110 (size 375, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0068b3c0:STR0/CALLEE1/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a05d0 Global::ExecuteCommandGiveItem -> 0x0068af80  gate_class=AUTO
- GOG-order neighbour after:  0x007a08e0 CSWVirtualMachineCommands::ExecuteCommandIntToFloat -> 0x0068b290  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [64 InsertString] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606500 CExoString::Insert -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x2c*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068b110: 16-byte aligned: YES
  0068b110  55             PUSH EBP
  0068b111  8B EC          MOV EBP,ESP
  0068b113  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0068b110
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x007343F0  CExoString::Insert
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068b110 FUN_0068b110 size=375 ret=8 callers=0 callees=6

undefined4 __fastcall FUN_0068b110(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a7a0;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  iVar1 = FUN_006fda70(local_1c);
  if (iVar1 == 0) {
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fda70(local_24);
    if (iVar1 == 0) {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_006fd9a0(&local_14);
      if (iVar1 == 0) {
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_8 = 0xffffffff;
        FUN_00733780();
        uVar2 = 0xfffff82f;
      }
      else {
        FUN_007343f0(local_24,local_14);
        iVar1 = FUN_006fda90(local_1c);
        if (iVar1 == 0) {
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          local_8 = 0xffffffff;
          FUN_00733780();
          uVar2 = 0xfffff830;
        }
        else {
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          local_8 = 0xffffffff;
          FUN_00733780();
          uVar2 = 0;
        }
      }
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x0068b3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068b3c0: 16-byte aligned: YES
  0068b3c0  55             PUSH EBP
  0068b3c1  8B EC          MOV EBP,ESP
  0068b3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0068b3c0
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x00733700  CExoString::Constructor_5
  0x007337C0  CExoString::operator=
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

## Card B15-3-58

GOG function: CSWVirtualMachineCommands::ExecuteCommandRandom  gog 0x007a4c20  callconv=(blank)  param_size_bytes=0  gog_size=109  gog_retn=8
Candidate Steam address: 0x0068f5d0 (size 109, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0068f560:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007a4bb0 Global::ExecuteCommandPutDownItem -> 0x0068f560  gate_class=REVIEW
- GOG-order neighbour after:  0x007a4c90 Global::ExecuteCommandRandomName -> 0x0068f640  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [0 Random] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0068f5d0: 16-byte aligned: YES
  0068f5d0  55             PUSH EBP
  0068f5d1  8B EC          MOV EBP,ESP
  0068f5d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0068f5d0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0091EB8B  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068f5d0 FUN_0068f5d0 size=109 ret=8 callers=0 callees=3

undefined4 FUN_0068f5d0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_c;
  int local_8;
  
  local_c = 0;
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (local_8 != 0) {
      local_c = _rand [GHIDRA-AUTO-LABEL: unreliable]();
      local_c = local_c % local_8;
      if (local_8 < 0) {
        local_c = -local_c;
      }
    }
    iVar1 = FUN_006fd9c0(local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0068f560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f560: 16-byte aligned: YES
  0068f560  55             PUSH EBP
  0068f561  8B EC          MOV EBP,ESP
  0068f563  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0068f560
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00567F00  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B15-3-59

GOG function: Global::ExecuteCommandGetWasForcePowerSuccessful  gog 0x007971d0  callconv=__thiscall  param_size_bytes=8  gog_size=128  gog_retn=8
Candidate Steam address: 0x00681c00 (size 128, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetLastAttackType | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00681b80:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00797150 Global::ExecuteCommandGetLastAttackResult -> 0x00681b80  gate_class=REVIEW
- GOG-order neighbour after:  0x00797250 Global::ExecuteCommandGetLastAttackType -> 0x00681c80  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [726 GetWasForcePowerSuccessful] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x5a8*} | Steam {0x8*,0x5a8*}
imm sets (* = in both, 5 common): GOG {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00681c00: 16-byte aligned: YES
  00681c00  55             PUSH EBP
  00681c01  8B EC          MOV EBP,ESP
  00681c03  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00681c00
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5A8 [dec 1448]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00681c00 FUN_00681c00 size=128 ret=8 callers=0 callees=3

undefined4 FUN_00681c00(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0x7f000000;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c100(local_c);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x5a8);
    }
    iVar1 = FUN_006fd9c0(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00681b80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00681b80: 16-byte aligned: YES
  00681b80  55             PUSH EBP
  00681b81  8B EC          MOV EBP,ESP
  00681b83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00681b80
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5A0 [dec 1440]  x1  via ECX
```

## Card B15-3-60

GOG function: Global::ExecuteCommandPrintLogEntry  gog 0x007a4770  callconv=__thiscall  param_size_bytes=8  gog_size=210  gog_retn=8
Candidate Steam address: 0x0068f120 (size 210, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0068f260:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a4710 Global::ExecuteCommandPrintInteger -> 0x0068f0c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007a4850 Global::ExecuteCommandPrintObject -> 0x0068f200  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [560 WriteTimestampedLogEntry] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0060c500 CExoDebug::GetCurrentTimestamp -> Steam ?  in candidate callees: n/a
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x1c*,0xfffff82f*} | Steam {0x8*,0x1c*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068f120: 16-byte aligned: YES
  0068f120  55             PUSH EBP
  0068f121  8B EC          MOV EBP,ESP
  0068f123  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0068f120
string literals (0):
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x0073A590  (unnamed)
  0x00734010  CExoString::CStr
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0068f120 FUN_0068f120 size=210 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_0068f120(undefined4 param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_20 [8];
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a9e0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1,param_1);
  local_8 = 0;
  iVar2 = FUN_006fda70(local_18);
  if (iVar2 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar3 = 0xfffff82f;
  }
  else {
    FUN_00733540(uVar1,param_1);
    local_8 = CONCAT31(local_8._1_3_,1);
    FUN_0073a590(local_20);
    if (DAT_00a7e110 == 0) {
      FUN_00734010();
      FUN_00734010();
    }
    local_8 = local_8 & 0xffffff00;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x0068f260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0068f260: 16-byte aligned: YES
  0068f260  55             PUSH EBP
  0068f261  8B EC          MOV EBP,ESP
  0068f263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0068f260
string literals (1):
  0x00998D18  at 0068f328  'PRINTSTRING: %s\\n'
direct callees (8):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0073A5C0  (unnamed)
  0x0073A510  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
```
