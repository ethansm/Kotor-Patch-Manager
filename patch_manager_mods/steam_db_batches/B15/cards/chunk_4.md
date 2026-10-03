# Review cards B15 chunk 4

Answer each card with one TSV row in `steam_db_batches/B15/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B15-4-01

GOG function: Global::ExecuteCommandGetIsDebilitated  gog 0x007ac3d0  callconv=__thiscall  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x00696d80 (size 129, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00696bf0:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac310 Global::ExecuteCommandSetFormation -> 0x00696cc0  gate_class=AUTO
- GOG-order neighbour after:  0x007ac460 Global::ExecuteCommandChangeByFaction -> 0x00696e10  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [732 GetIsDebilitated] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00656cb0 CSWSCreature::IsDebilitated -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x00696d80: 16-byte aligned: YES
  00696d80  55             PUSH EBP
  00696d81  8B EC          MOV EBP,ESP
  00696d83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00696d80
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DAA00  CSWSCreature::IsDebilitated
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00696d80 FUN_00696d80 size=129 ret=8 callers=0 callees=4

undefined4 FUN_00696d80(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_c = 0;
    local_8 = FUN_0051c100(local_10);
    if (local_8 != 0) {
      local_c = FUN_006daa00(0);
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

### decoy: 0x00696bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00696bf0: 16-byte aligned: YES
  00696bf0  55             PUSH EBP
  00696bf1  8B EC          MOV EBP,ESP
  00696bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00696bf0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006E6880  CSWSCreature::ApplyOnHitInstantDeath
  0x006E6A40  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B15-4-02

GOG function: Global::ExecuteCommandGetHealTarget  gog 0x007afc50  callconv=__thiscall  param_size_bytes=8  gog_size=127  gog_retn=8
Candidate Steam address: 0x0069a600 (size 127, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069a580:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007afbd0 Global::ExecuteCommandSetHealTarget -> 0x0069a580  gate_class=REVIEW
- GOG-order neighbour after:  0x007afcd0 Global::ExecuteCommandGetRandomDestination -> 0x0069a680  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [814 GetHealTarget] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x5ac*} | Steam {0x8*,0x5ac*}
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
prologue for 0x0069a600: 16-byte aligned: YES
  0069a600  55             PUSH EBP
  0069a601  8B EC          MOV EBP,ESP
  0069a603  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069a600
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5AC [dec 1452]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069a600 FUN_0069a600 size=127 ret=8 callers=0 callees=3

undefined4 FUN_0069a600(void)

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
    iVar1 = FUN_0051c100(local_c);
    local_8 = 0x7f000000;
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(iVar1 + 0x5ac);
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

### decoy: 0x0069a580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069a580: 16-byte aligned: YES
  0069a580  55             PUSH EBP
  0069a581  8B EC          MOV EBP,ESP
  0069a583  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069a580
string literals (0):
direct callees (2):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x5AC [dec 1452]  x1  via EAX
```

## Card B15-4-03

GOG function: Global::ExecuteCommandShowGalaxyMapGui  gog 0x007acab0  callconv=__thiscall  param_size_bytes=8  gog_size=74  gog_retn=8
Candidate Steam address: 0x00697460 (size 74, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: ShowGalaxyMapGui 0x7ca730)
- best decoy (neighbour/other candidate) scores: 0x00697120:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac840 Global::ExecuteCommandSetItemStackSize -> 0x006971f0  gate_class=AUTO
- GOG-order neighbour after:  0x007acb00 Global::ExecuteCommandSetGalaxyMapPlanetAvailable -> 0x006974b0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [739 ShowGalaxyMap] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff82f*} | Steam {0x8*,0xc*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697460: 16-byte aligned: YES
  00697460  55             PUSH EBP
  00697461  8B EC          MOV EBP,ESP
  00697463  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697460
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F750  ClientApp::GetGuiInGame
  0x007CA730  CGuiInGame::ShowGalaxyMapGui
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00697460 FUN_00697460 size=74 ret=8 callers=0 callees=3

undefined4 FUN_00697460(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_0073f750(local_8,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 4));
    FUN_007ca730(local_8);
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00697120 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697120: 16-byte aligned: YES
  00697120  55             PUSH EBP
  00697121  8B EC          MOV EBP,ESP
  00697123  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697120
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B15-4-04

GOG function: Global::ExecuteCommandEffectForceResistanceIncrease  gog 0x0078c300  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676e60 (size 418, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.71 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676d20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c1c0 Global::ExecuteCommandEffectForceResistanceDecrease -> 0x00676be0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c440 CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints -> 0x00676e60  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [212 EffectForceResistanceIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 2 common): GOG {0x8*,0xa*} | Steam {0x8*,0xa*,0x30}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x38,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676e60: 16-byte aligned: YES
  00676e60  55             PUSH EBP
  00676e61  8B EC          MOV EBP,ESP
  00676e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00676e60
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
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x30 [dec 48]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00676e60 FUN_00676e60 size=418 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00676e60(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int **ppiVar5;
  int local_44;
  undefined2 local_38;
  undefined2 local_34;
  int local_20;
  int *local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954f5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_20);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 0;
    if (iVar2 == 0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_44;
    *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xffe7 | 8;
    if (local_20 < 1) {
      local_38 = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_44 + 8) = local_38;
    }
    else {
      local_34 = (undefined2)DAT_00996084;
      *(undefined2 *)(local_44 + 8) = local_34;
      FUN_005e4e80(0,local_20);
    }
    ppiVar5 = &local_1c;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,ppiVar5);
    local_11 = FUN_0053dfb0(uVar3,ppiVar5);
    if ((local_11 == DAT_0099447f) && (iVar2 = (**(code **)(*local_1c + 0x30))(), iVar2 != 0)) {
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
    }
    iVar4 = FUN_006fdad0(0,local_18);
    iVar2 = local_18;
    if (iVar4 == 0) {
      uVar3 = 0xfffff830;
    }
    else {
      if (local_18 != 0) {
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

### decoy: 0x00676d20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676d20: 16-byte aligned: YES
  00676d20  55             PUSH EBP
  00676d21  8B EC          MOV EBP,ESP
  00676d23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00676d20
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

## Card B15-4-05

GOG function: Global::ExecuteCommandSetMaxStealthXP  gog 0x007acf90  callconv=__thiscall  param_size_bytes=8  gog_size=109  gog_retn=8
Candidate Steam address: 0x00697940 (size 109, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697a30:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acf10 Global::ExecuteCommandGetMaxStealthXP -> 0x006978c0  gate_class=AUTO
- GOG-order neighbour after:  0x007ad000 Global::ExecuteCommandGetCurrentStealthXP -> 0x006979b0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [468 SetMaxStealthXP] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00722ee0 CSWSArea::GetClientArea_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x00697940: 16-byte aligned: YES
  00697940  55             PUSH EBP
  00697941  8B EC          MOV EBP,ESP
  00697943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697940
string literals (0):
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005453C0  CSWSObject::GetArea
  0x0052DE00  CSWSArea::SetMaxStealthXP
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00697940 FUN_00697940 size=109 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_00697940(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_8;
  
  uVar1 = FUN_0051c8f0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  iVar2 = FUN_0051c100(uVar1);
  iVar3 = FUN_006fd9a0(&local_8);
  if (iVar3 == 0) {
    uVar1 = 0xfffff82f;
  }
  else {
    if (iVar2 != 0) {
      iVar2 = FUN_005453c0();
      if (iVar2 != 0) {
        FUN_0052de00(local_8);
      }
    }
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00697a30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697a30: 16-byte aligned: YES
  00697a30  55             PUSH EBP
  00697a31  8B EC          MOV EBP,ESP
  00697a33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697a30
string literals (0):
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005453C0  CSWSObject::GetArea
  0x0052DE40  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-4-06

GOG function: Global::ExecuteCommandActionSwitchWeapons  gog 0x007b0f50  callconv=__thiscall  param_size_bytes=8  gog_size=119  gog_retn=8
Candidate Steam address: 0x0069b980 (size 119, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069b840:STR0/CALLEE0/SIG0.321/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0f10 Global::ExecuteCommandGetIsXBox -> 0x0069b940  gate_class=REVIEW
- GOG-order neighbour after:  0x007b0fd0 Global::ExecuteCommandUnlockAllMusic -> 0x0069ba00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [853 ActionSwitchWeapons] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006a64d0 CSWSObject::AddAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x47*,0xffff*} | Steam {0x8*,0xc*,0x47*,0xffff*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069b980: 16-byte aligned: YES
  0069b980  55             PUSH EBP
  0069b981  8B EC          MOV EBP,ESP
  0069b983  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069b980
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0069b980 FUN_0069b980 size=119 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0069b980(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
  if (iVar1 != 0) {
    FUN_0053f7f0(0x47,0xffff,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
  }
  return 0;
}
```

### decoy: 0x0069b840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069b840: 16-byte aligned: YES
  0069b840  55             PUSH EBP
  0069b841  8B EC          MOV EBP,ESP
  0069b843  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069b840
string literals (0):
direct callees (11):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FAAF0  CSWPartyTable::GetNPCObject
  0x005F8B00  CSWPartyTable::RemoveMember
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00775860  (unnamed)
  0x0058A330  CSWSCreature::SetPartyMemberFlag
  0x005FD9A0  CSWPartyTable::KillNPCObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-07

GOG function: Global::ExecuteCommandSetBonusForcePoints  gog 0x007af710  callconv=(blank)  param_size_bytes=0  gog_size=291  gog_retn=8
Candidate Steam address: 0x0069a0c0 (size 291, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::NodeHasProjection | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069a1f0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007af640 Global::ExecuteCommandGetRacialSubType -> 0x00699ff0  gate_class=AUTO
- GOG-order neighbour after:  0x007af840 Global::ExecuteCommandAddBonusForcePoints -> 0x0069a1f0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [801 SetBonusForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004df5f0 CGuiInGame::UpdateStatus -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x11f4*,0x1200*} | Steam {0x4*,0x8*,0x30*,0x11f4*,0x1200*}
imm sets (* = in both, 5 common): GOG {0x8*,0xb*,0xc*,0x34*,0xfffff830*} | Steam {0x8*,0xb*,0xc*,0x34*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0069a0c0: 16-byte aligned: YES
  0069a0c0  55             PUSH EBP
  0069a0c1  8B EC          MOV EBP,ESP
  0069a0c3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069a0c0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 5):
  +0x11F4 [dec 4596]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x1200 [dec 4608]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069a0c0 FUN_0069a0c0 size=291 ret=8 callers=0 callees=5

undefined4 FUN_0069a0c0(void)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 extraout_var;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      piVar3 = (int *)FUN_0051c0a0(local_c);
      if (piVar3 == (int *)0x0) {
        iVar1 = 0;
      }
      else {
        iVar1 = (**(code **)(*piVar3 + 0x30))();
      }
      if (iVar1 != 0) {
        if ((*(int *)(iVar1 + 0x1200) != 0) && (*(int *)(iVar1 + 0x11f4) != local_8)) {
          if (*(int *)(iVar1 + 0x11f4) < local_8) {
            uVar4 = 0xb;
            iVar5 = local_8;
            FUN_0073f750(0xb,local_8,iVar1,uVar2);
            FUN_007ceef0(uVar4,iVar5);
          }
          else {
            uVar4 = 0xc;
            iVar5 = local_8;
            FUN_0073f750(0xc,local_8,iVar1,uVar2);
            FUN_007ceef0(uVar4,iVar5);
          }
        }
        *(int *)(iVar1 + 0x11f4) = local_8;
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0069a1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069a1f0: 16-byte aligned: YES
  0069a1f0  55             PUSH EBP
  0069a1f1  8B EC          MOV EBP,ESP
  0069a1f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069a1f0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EDX
  +0x11F4 [dec 4596]  x2  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x1200 [dec 4608]  x1  via EAX
```

## Card B15-4-08

GOG function: Global::ExecuteCommandSetMusicVolume  gog 0x007aee50  callconv=__thiscall  param_size_bytes=8  gog_size=156  gog_retn=8
Candidate Steam address: 0x00699800 (size 156, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006997b0:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aee00 Global::ExecuteCommandGetCheatCode -> 0x006997b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007aeef0 Global::ExecuteCommandCreateItemOnFloor -> 0x006998a0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [765 SetMusicVolume] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0063a5a0 CExoSound::SetMusicVolume -> Steam ?  in candidate callees: n/a
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x5*,0x8*,0x14*,0xfffff830*} | Steam {0x5*,0x8*,0x14*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00699800: 16-byte aligned: YES
  00699800  55             PUSH EBP
  00699801  8B EC          MOV EBP,ESP
  00699803  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00699800
string literals (0):
direct callees (2):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0070B600  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00699800 FUN_00699800 size=156 ret=8 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00699800(void)

{
  int iVar1;
  undefined4 uVar2;
  float local_18;
  float local_14;
  float local_10;
  float local_8;
  
  iVar1 = FUN_006fd9e0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if ((float)_DAT_00986b38 <= local_8) {
      local_10 = local_8;
    }
    else {
      local_10 = 0.0;
    }
    if ((float)_DAT_00986b50 <= local_10) {
      local_18 = 1.0;
    }
    else {
      if ((float)_DAT_00986b38 <= local_8) {
        local_14 = local_8;
      }
      else {
        local_14 = 0.0;
      }
      local_18 = local_14;
    }
    local_8 = local_18;
    FUN_0070b600(local_18,1);
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x006997b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006997b0: 16-byte aligned: YES
  006997b0  55             PUSH EBP
  006997b1  8B EC          MOV EBP,ESP
  006997b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006997b0
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

## Card B15-4-09

GOG function: Global::ExecuteCommandSetMinOneHP  gog 0x007abf80  callconv=__thiscall  param_size_bytes=8  gog_size=172  gog_retn=8
Candidate Steam address: 0x00696930 (size 172, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00696890:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007abee0 Global::ExecuteCommandGetMinOneHP -> 0x00696890  gate_class=REVIEW
- GOG-order neighbour after:  0x007ac030 Global::ExecuteCommandSetFadeIn -> 0x006969e0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [716 SetMinOneHP] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x10*,0x22c*} | Steam {0x8*,0x10*,0x22c*}
imm sets (* = in both, 3 common): GOG {0x8*,0x18*,0xfffff82f*} | Steam {0x8*,0x18*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00696930: 16-byte aligned: YES
  00696930  55             PUSH EBP
  00696931  8B EC          MOV EBP,ESP
  00696933  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00696930
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
struct offsets touched (top 3):
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x22C [dec 556]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00696930 FUN_00696930 size=172 ret=8 callers=0 callees=3

undefined4 FUN_00696930(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  int local_10;
  int *local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      local_c = (int *)FUN_0051c0a0(local_8);
      if (local_c != (int *)0x0) {
        iVar1 = (**(code **)(*local_c + 0x10))(uVar2,uVar3);
        if (iVar1 != 0) {
          iVar1 = (**(code **)(*local_c + 0x10))();
          if (local_10 == 0) {
            *(undefined4 *)(iVar1 + 0x22c) = 0;
          }
          else {
            *(undefined4 *)(iVar1 + 0x22c) = 1;
          }
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00696890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00696890: 16-byte aligned: YES
  00696890  55             PUSH EBP
  00696891  8B EC          MOV EBP,ESP
  00696893  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00696890
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x10 [dec 16]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x22C [dec 556]  x1  via ECX
```

## Card B15-4-10

GOG function: Global::ExecuteCommandGetCheatCode  gog 0x007aee00  callconv=__thiscall  param_size_bytes=8  gog_size=76  gog_retn=8
Candidate Steam address: 0x006997b0 (size 76, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00699760:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aedb0 Global::ExecuteCommandSuppressStatusSummaryEntry -> 0x00699760  gate_class=AUTO
- GOG-order neighbour after:  0x007aee50 Global::ExecuteCommandSetMusicVolume -> 0x00699800  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [764 GetCheatCode] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x006997b0: 16-byte aligned: YES
  006997b0  55             PUSH EBP
  006997b1  8B EC          MOV EBP,ESP
  006997b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006997b0
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006997b0 FUN_006997b0 size=76 ret=8 callers=0 callees=2

undefined4 FUN_006997b0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_c [4];
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_006fd9a0(local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
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

### decoy: 0x00699760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00699760: 16-byte aligned: YES
  00699760  55             PUSH EBP
  00699761  8B EC          MOV EBP,ESP
  00699763  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00699760
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF470  CGuiInGame::SuppressStatusSummary
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B15-4-11

GOG function: Global::ExecuteCommandAurPostString  gog 0x007f97c0  callconv=__thiscall  param_size_bytes=8  gog_size=283  gog_retn=8
Candidate Steam address: 0x006f6200 (size 284, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006f6320:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f9140 CSWVirtualMachineCommands::InitializeSWMGCommands -> 0x006f5b80  trusted
- GOG-order neighbour after:  0x007f98e0 CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEvent -> 0x006f6320  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [582 AurPostString] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0x2c*,0xfffff82f*} | Steam {0x8*,0x2c*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006f6200: 16-byte aligned: YES
  006f6200  55             PUSH EBP
  006f6201  8B EC          MOV EBP,ESP
  006f6203  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006f6200
string literals (0):
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9E0  CVirtualMachine::StackPopFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006f6200 FUN_006f6200 size=284 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_006f6200(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec08;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  iVar1 = FUN_006fda70(local_24);
  if (iVar1 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(local_14);
    if (iVar1 == 0) {
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_006fd9a0(local_18);
      if (iVar1 == 0) {
        local_8 = 0xffffffff;
        FUN_00733780();
        uVar2 = 0xfffff82f;
      }
      else {
        iVar1 = FUN_006fd9e0(local_1c);
        if (iVar1 == 0) {
          local_8 = 0xffffffff;
          FUN_00733780();
          uVar2 = 0xfffff82f;
        }
        else {
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

### decoy: 0x006f6320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f6320: 16-byte aligned: YES
  006f6320  55             PUSH EBP
  006f6321  8B EC          MOV EBP,ESP
  006f6323  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006f6320
string literals (0):
direct callees (8):
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00733540  CExoString::Constructor
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-12

GOG function: Global::ExecuteCommandSetGalaxyMapPlanetAvailable  gog 0x007acb00  callconv=__thiscall  param_size_bytes=8  gog_size=104  gog_retn=8
Candidate Steam address: 0x006974b0 (size 104, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SetPlanetAvailable 0x5fed30)
- best decoy (neighbour/other candidate) scores: 0x00697590:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acab0 Global::ExecuteCommandShowGalaxyMapGui -> 0x00697460  gate_class=REVIEW
- GOG-order neighbour after:  0x007acb70 Global::ExecuteCommandGetGalaxyMapPlanetAvailable -> 0x00697520  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [742 SetPlanetAvailable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x007046e0 CSWPartyTable::SetPlanetAvailable -> Steam 0x005fed30  in candidate callees: yes
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
prologue for 0x006974b0: 16-byte aligned: YES
  006974b0  55             PUSH EBP
  006974b1  8B EC          MOV EBP,ESP
  006974b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006974b0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED30  CSWPartyTable::SetPlanetAvailable
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006974b0 FUN_006974b0 size=104 ret=8 callers=0 callees=3

undefined4 FUN_006974b0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      FUN_0051c8d0(local_c,local_8,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_005fed30(local_c,local_8);
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00697590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697590: 16-byte aligned: YES
  00697590  55             PUSH EBP
  00697591  8B EC          MOV EBP,ESP
  00697593  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697590
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FECD0  CSWPartyTable::SetPlanetSelectable
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-13

GOG function: Global::ExecuteCommandDisableMap  gog 0x007b10d0  callconv=__thiscall  param_size_bytes=8  gog_size=91  gog_retn=8
Candidate Steam address: 0x0069bb00 (size 91, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069bc10:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0fd0 Global::ExecuteCommandUnlockAllMusic -> 0x0069ba00  gate_class=REVIEW
- GOG-order neighbour after:  0x007b1130 Global::ExecuteCommandDetonateMine -> 0x0069bb60  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [856 DisableMap] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x258*} | Steam {0x8*,0x258*}
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
prologue for 0x0069bb00: 16-byte aligned: YES
  0069bb00  55             PUSH EBP
  0069bb01  8B EC          MOV EBP,ESP
  0069bb03  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069bb00
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x258 [dec 600]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069bb00 FUN_0069bb00 size=91 ret=8 callers=0 callees=2

undefined4 FUN_0069bb00(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_006fd9a0(&local_8);
  uVar2 = local_8;
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_0051c8d0(extraout_var);
    *(undefined4 *)(iVar1 + 0x258 [dec 600]) = uVar2;
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0069bc10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069bc10: 16-byte aligned: YES
  0069bc10  55             PUSH EBP
  0069bc11  8B EC          MOV EBP,ESP
  0069bc13  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069bc10
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x25C [dec 604]  x1  via ECX
```

## Card B15-4-14

GOG function: Global::ExecuteCommandGetMinOneHP  gog 0x007abee0  callconv=__thiscall  param_size_bytes=8  gog_size=154  gog_retn=8
Candidate Steam address: 0x00696890 (size 154, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00696930:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007abdf0 Global::ExecuteCommandGivePlotXP -> 0x006967a0  gate_class=AUTO
- GOG-order neighbour after:  0x007abf80 Global::ExecuteCommandSetMinOneHP -> 0x00696930  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [715 GetMinOneHP] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x10*,0x22c*} | Steam {0x8*,0x10*,0x22c*}
imm sets (* = in both, 4 common): GOG {0x8*,0x18*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x18*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00696890: 16-byte aligned: YES
  00696890  55             PUSH EBP
  00696891  8B EC          MOV EBP,ESP
  00696893  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00696890
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x10 [dec 16]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
  +0x22C [dec 556]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00696890 FUN_00696890 size=154 ret=8 callers=0 callees=3

undefined4 FUN_00696890(void)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_10;
  undefined4 local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar4 = extraout_var;
    piVar3 = (int *)FUN_0051c0a0(local_8);
    if (piVar3 != (int *)0x0) {
      iVar1 = (**(code **)(*piVar3 + 0x10))(uVar4,uVar2);
      if (iVar1 != 0) {
        iVar1 = (**(code **)(*piVar3 + 0x10))();
        local_10 = *(undefined4 *)(iVar1 + 0x22c);
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

### decoy: 0x00696930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00696930: 16-byte aligned: YES
  00696930  55             PUSH EBP
  00696931  8B EC          MOV EBP,ESP
  00696933  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00696930
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
struct offsets touched (top 3):
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x22C [dec 556]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-15

GOG function: Global::ExecuteCommandGetIsPartyLeader  gog 0x007b0d70  callconv=__thiscall  param_size_bytes=8  gog_size=159  gog_retn=8
Candidate Steam address: 0x0069b720 (size 159, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069b530:STR0/CALLEE0/SIG0.9/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0cb0 Global::ExecuteCommandActionFollowOwner -> 0x0069b660  gate_class=AUTO
- GOG-order neighbour after:  0x007b0e10 Global::ExecuteCommandRemoveNPCFromPartyToBase -> 0x0069b840  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [844 GetIsPartyLeader] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x1c*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x1c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069b720: 16-byte aligned: YES
  0069b720  55             PUSH EBP
  0069b721  8B EC          MOV EBP,ESP
  0069b723  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069b720
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0069b720 FUN_0069b720 size=159 ret=8 callers=0 callees=5

undefined4 FUN_0069b720(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = 0;
    FUN_0073fb90(0,extraout_var);
    iVar1 = FUN_007e5da0(uVar2);
    if (iVar1 != 0) {
      iVar1 = FUN_0051c8b0(*(undefined4 *)(iVar1 + 4));
      if (local_8 == iVar1) {
        local_c = 1;
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

### decoy: 0x0069b530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069b530: 16-byte aligned: YES
  0069b530  55             PUSH EBP
  0069b531  8B EC          MOV EBP,ESP
  0069b533  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069b530
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FDC40  CSWPartyTable::GetPUPID
  0x005FF2C0  CSWPartyTable::GetPUPOwner
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via ECX
```

## Card B15-4-16

GOG function: Global::ExecuteCommandGetScriptParameter  gog 0x007af2f0  callconv=(blank)  param_size_bytes=0  gog_size=106  gog_retn=8
Candidate Steam address: 0x00699ca0 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00699c30:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007af280 Global::ExecuteCommandSetAvailableNPCID -> 0x00699c30  gate_class=AUTO
- GOG-order neighbour after:  0x007af360 Global::ExecuteCommandGetScriptStringParameter -> 0x00699d10  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [768 GetScriptParameter] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x5*,0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x5*,0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00699ca0: 16-byte aligned: YES
  00699ca0  55             PUSH EBP
  00699ca1  8B EC          MOV EBP,ESP
  00699ca3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00699ca0
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00699ca0 FUN_00699ca0 size=106 ret=8 callers=0 callees=2

undefined4 FUN_00699ca0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if ((local_c < 1) || (5 < local_c)) {
      local_8 = 0;
    }
    else {
      local_8 = *(undefined4 *)(DAT_00a1b4a8 + -4 + local_c * 4);
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

### decoy: 0x00699c30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00699c30: 16-byte aligned: YES
  00699c30  55             PUSH EBP
  00699c31  8B EC          MOV EBP,ESP
  00699c33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00699c30
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FAA80  CSWPartyTable::SetNPCObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-17

GOG function: Global::ExecuteCommandUnlockAllMusic  gog 0x007b0fd0  callconv=__thiscall  param_size_bytes=8  gog_size=252  gog_retn=8
Candidate Steam address: 0x0069ba00 (size 252, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0069b940:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0f50 Global::ExecuteCommandActionSwitchWeapons -> 0x0069b980  gate_class=REVIEW
- GOG-order neighbour after:  0x007b10d0 Global::ExecuteCommandDisableMap -> 0x0069bb00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [855 UnlockAllSongs] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00608370 CExoIni::Constructor -> Steam ?  in candidate callees: n/a
  0x006083f0 CExoIni::Destructor -> Steam ?  in candidate callees: n/a
  0x00608480 CExoIni::WriteIniEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (3): '.\\\\swkotor2.ini'; 'Game Options'; 'UnlockedPlanetSongs'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x4*,0x8*,0x38*,0x7ff*} | Steam {0x2*,0x3*,0x4*,0x8*,0x38*,0x7ff*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069ba00: 16-byte aligned: YES
  0069ba00  55             PUSH EBP
  0069ba01  8B EC          MOV EBP,ESP
  0069ba03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0069ba00
string literals (3):
  0x00998D58  at 0069ba3e  'UnlockedPlanetSongs'
  0x00986CD4  at 0069ba4f  'Game Options'
  0x00986B98  at 0069ba60  '.\\\\swkotor2.ini'
direct callees (6):
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733700  CExoString::Constructor_5
  0x00736350  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007362C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0069ba00 FUN_0069ba00 size=252 ret=8 callers=0 callees=6

undefined4 FUN_0069ba00(void)

{
  undefined4 uVar1;
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  undefined1 local_20 [12];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095aff8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00736240(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  local_14 = 0x7ff;
  FUN_00733570("UnlockedPlanetSongs");
  local_8._0_1_ = 1;
  FUN_00733570("Game Options");
  local_8._0_1_ = 2;
  FUN_00733570(".\\swkotor2.ini");
  local_8._0_1_ = 3;
  uVar1 = FUN_00733700(local_14);
  local_8._0_1_ = 4;
  FUN_00736350(uVar1,local_30,local_28,local_20);
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_00733780();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_007362c0();
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x0069b940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069b940: 16-byte aligned: YES
  0069b940  55             PUSH EBP
  0069b941  8B EC          MOV EBP,ESP
  0069b943  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069b940
string literals (0):
direct callees (1):
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

## Card B15-4-18

GOG function: Global::ExecuteCommandRemoveNPCFromPartyToBase  gog 0x007b0e10  callconv=__thiscall  param_size_bytes=8  gog_size=244  gog_retn=8
Candidate Steam address: 0x0069b840 (size 244, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: RemoveMember 0x5f8b00, GetNPCObject 0x5faaf0, KillNPCObject 0x5fd9a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069b720:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0d70 Global::ExecuteCommandGetIsPartyLeader -> 0x0069b720  gate_class=REVIEW
- GOG-order neighbour after:  0x007b0f10 Global::ExecuteCommandGetIsXBox -> 0x0069b940  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [846 RemoveNPCFromPartyToBase] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: yes
  0x0040d3a0 CClientExoApp::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
  0x00421750 CSWCCreature::SetInParty -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006a2e10 CSWSCreature::SetInParty -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x0069b840: 16-byte aligned: YES
  0069b840  55             PUSH EBP
  0069b841  8B EC          MOV EBP,ESP
  0069b843  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069b840
string literals (0):
direct callees (11):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FAAF0  CSWPartyTable::GetNPCObject
  0x005F8B00  CSWPartyTable::RemoveMember
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00775860  (unnamed)
  0x0058A330  CSWSCreature::SetPartyMemberFlag
  0x005FD9A0  CSWPartyTable::KillNPCObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069b840 FUN_0069b840 size=244 ret=8 callers=0 callees=11

undefined4 FUN_0069b840(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    FUN_0051c8d0(extraout_var);
    uVar2 = FUN_005faaf0(local_c,0,1);
    iVar1 = FUN_005f8b00(local_c);
    if (iVar1 != 0) {
      uVar3 = FUN_0073fec0(uVar2);
      iVar1 = FUN_0073f550(uVar3);
      iVar4 = FUN_0051c100(uVar2);
      if (iVar1 != 0) {
        FUN_00775860(0);
      }
      if (iVar4 != 0) {
        FUN_0058a330(0,1);
      }
    }
    uVar2 = FUN_005fd9a0(local_c,1);
    iVar1 = FUN_006fd9c0(uVar2);
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

### decoy: 0x0069b720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069b720: 16-byte aligned: YES
  0069b720  55             PUSH EBP
  0069b721  8B EC          MOV EBP,ESP
  0069b723  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069b720
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-19

GOG function: Global::ExecuteCommandSetCurrentStealthXP  gog 0x007ad080  callconv=__thiscall  param_size_bytes=8  gog_size=109  gog_retn=8
Candidate Steam address: 0x00697a30 (size 109, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697940:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ad000 Global::ExecuteCommandGetCurrentStealthXP -> 0x006979b0  gate_class=AUTO
- GOG-order neighbour after:  0x007ad0f0 Global::ExecuteCommandAwardStealthXP -> 0x00697aa0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [478 SetCurrentStealthXP] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00722f20 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x00697a30: 16-byte aligned: YES
  00697a30  55             PUSH EBP
  00697a31  8B EC          MOV EBP,ESP
  00697a33  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697a30
string literals (0):
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005453C0  CSWSObject::GetArea
  0x0052DE40  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00697a30 FUN_00697a30 size=109 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_00697a30(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_8;
  
  uVar1 = FUN_0051c8f0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  iVar2 = FUN_0051c100(uVar1);
  iVar3 = FUN_006fd9a0(&local_8);
  if (iVar3 == 0) {
    uVar1 = 0xfffff82f;
  }
  else {
    if (iVar2 != 0) {
      iVar2 = FUN_005453c0();
      if (iVar2 != 0) {
        FUN_0052de40(local_8);
      }
    }
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00697940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697940: 16-byte aligned: YES
  00697940  55             PUSH EBP
  00697941  8B EC          MOV EBP,ESP
  00697943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697940
string literals (0):
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005453C0  CSWSObject::GetArea
  0x0052DE00  CSWSArea::SetMaxStealthXP
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-4-20

GOG function: Global::ExecuteCommandResetDialogState  gog 0x007ae9d0  callconv=__thiscall  param_size_bytes=8  gog_size=45  gog_retn=8
Candidate Steam address: 0x00699380 (size 45, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006993b0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ae890 Global::ExecuteCommandGetIsLiveContentAvailable -> 0x00699240  gate_class=AUTO
- GOG-order neighbour after:  0x007aea00 Global::ExecuteCommandSetGoodEvilValue -> 0x006993b0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [749 ResetDialogState] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004df280 CGuiInGame::SetGlobalDialogState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00699380: 16-byte aligned: YES
  00699380  55             PUSH EBP
  00699381  8B EC          MOV EBP,ESP
  00699383  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00699380
string literals (0):
direct callees (2):
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEB80  CGuiInGame::SetGlobalDialogState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00699380 FUN_00699380 size=45 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_00699380(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = 0;
  FUN_0073f750(0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  FUN_007ceb80(uVar1);
  return 0;
}
```

### decoy: 0x006993b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006993b0: 16-byte aligned: YES
  006993b0  55             PUSH EBP
  006993b1  8B EC          MOV EBP,ESP
  006993b3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006993b0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via EDX
  +0x18A [dec 394]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B15-4-21

GOG function: Global::ExecuteCommandSaveNPCByObject  gog 0x007b1890  callconv=__thiscall  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x0069c2c0 (size 118, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SaveMemberByObject 0x5f8130)
- best decoy (neighbour/other candidate) scores: 0x0069c340:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b17d0 Global::ExecuteCommandGetCombatActionPending -> 0x0069c200  gate_class=AUTO
- GOG-order neighbour after:  0x007b1910 Global::ExecuteCommandSavePUPByObject -> 0x0069c340  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [873 SaveNPCByObject] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006fdca0 CSWPartyTable::SaveMemberByObject -> Steam 0x005f8130  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x7f000000*,0xfffff82f*} | Steam {0x8*,0x10*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069c2c0: 16-byte aligned: YES
  0069c2c0  55             PUSH EBP
  0069c2c1  8B EC          MOV EBP,ESP
  0069c2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069c2c0
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F8130  CSWPartyTable::SaveMemberByObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0069c2c0 FUN_0069c2c0 size=118 ret=8 callers=0 callees=4

undefined4 FUN_0069c2c0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0xffffffff;
  local_8 = 0x7f000000;
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fdaf0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      uVar3 = local_c;
      uVar4 = local_8;
      FUN_0051c8d0(local_c,local_8,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_005f8130(uVar3,uVar4);
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0069c340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069c340: 16-byte aligned: YES
  0069c340  55             PUSH EBP
  0069c341  8B EC          MOV EBP,ESP
  0069c343  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069c340
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F8520  CSWPartyTable::SavePuppetByObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-22

GOG function: Global::ExecuteCommandVectorMagnitude  gog 0x007aacd0  callconv=__thiscall  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x006956f0 (size 190, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandVectorNormalize | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.21 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00695680:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007aab50 Global::ExecuteCommandVectorConversions -> 0x00695500  gate_class=AUTO
- GOG-order neighbour after:  0x007aad40 Global::ExecuteCommandVectorNormalize -> 0x006956f0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [104 VectorMagnitude] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: no
  0x00640c40 CVirtualMachine::StackPopVector -> Steam 0x006fda20  in candidate callees: yes
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 3 common): GOG {0x4,0x8*,0x14,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc,0x34,0xfffff82f*,0xfffff830*}

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

### decoy: 0x00695680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695680: 16-byte aligned: YES
  00695680  55             PUSH EBP
  00695681  8B EC          MOV EBP,ESP
  00695683  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695680
string literals (0):
direct callees (3):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

## Card B15-4-23

GOG function: Global::ExecuteCommandGetIsXBox  gog 0x007b0f10  callconv=__thiscall  param_size_bytes=8  gog_size=50  gog_retn=8
Candidate Steam address: 0x0069b940 (size 50, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0069b840:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b0e10 Global::ExecuteCommandRemoveNPCFromPartyToBase -> 0x0069b840  gate_class=REVIEW
- GOG-order neighbour after:  0x007b0f50 Global::ExecuteCommandActionSwitchWeapons -> 0x0069b980  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [851 GetIsXBox] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x0069b940: 16-byte aligned: YES
  0069b940  55             PUSH EBP
  0069b941  8B EC          MOV EBP,ESP
  0069b943  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069b940
string literals (0):
direct callees (1):
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0069b940 FUN_0069b940 size=50 ret=8 callers=0 callees=1

undefined4 FUN_0069b940(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006fd9c0(0);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0069b840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069b840: 16-byte aligned: YES
  0069b840  55             PUSH EBP
  0069b841  8B EC          MOV EBP,ESP
  0069b843  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069b840
string literals (0):
direct callees (11):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FAAF0  CSWPartyTable::GetNPCObject
  0x005F8B00  CSWPartyTable::RemoveMember
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00775860  (unnamed)
  0x0058A330  CSWSCreature::SetPartyMemberFlag
  0x005FD9A0  CSWPartyTable::KillNPCObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-24

GOG function: Global::ExecuteCommandGetAttacker  gog 0x007ac240  callconv=__thiscall  param_size_bytes=8  gog_size=197  gog_retn=8
Candidate Steam address: 0x00696bf0 (size 197, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00696d80:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac150 Global::ExecuteCommandSetFadeOut -> 0x00696b00  gate_class=AUTO
- GOG-order neighbour after:  0x007ac310 Global::ExecuteCommandSetFormation -> 0x00696cc0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [727 GetFirstAttacker, 728 GetNextAttacker] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00662b80 CServerExoAppInternal::DebugValidatePath -> Steam ?  in candidate callees: n/a
  0x00662d40 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 7 common): GOG {0x8*,0x14*,0x2d7*,0x2d8*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x14*,0x2d7*,0x2d8*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00696bf0: 16-byte aligned: YES
  00696bf0  55             PUSH EBP
  00696bf1  8B EC          MOV EBP,ESP
  00696bf3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00696bf0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006E6880  CSWSCreature::ApplyOnHitInstantDeath
  0x006E6A40  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00696bf0 FUN_00696bf0 size=197 ret=8 callers=0 callees=5

undefined4 __thiscall FUN_00696bf0(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  local_10 = *(undefined4 *)(param_1 + 8);
  local_8 = DAT_00994484;
  if ((param_3 < 1) || (iVar1 = FUN_006fdaf0(&local_10), iVar1 != 0)) {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    local_c = FUN_0051c100(local_10);
    if (local_c != 0) {
      if (param_2 == 0x2d7) {
        iVar1 = FUN_006e6880();
        if (iVar1 == 0x7f000000) {
          local_8 = DAT_00994484;
        }
      }
      else if ((param_2 == 0x2d8) && (iVar1 = FUN_006e6a40(param_1,uVar2), iVar1 == 0x7f000000)) {
        local_8 = DAT_00994484;
      }
    }
    iVar1 = FUN_006fdb10(local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  else {
    uVar2 = 0xfffff82f;
  }
  return uVar2;
}
```

### decoy: 0x00696d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00696d80: 16-byte aligned: YES
  00696d80  55             PUSH EBP
  00696d81  8B EC          MOV EBP,ESP
  00696d83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00696d80
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DAA00  CSWSCreature::IsDebilitated
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-25

GOG function: Global::ExecuteCommandSetLockHeadFollowInDialog  gog 0x007ad750  callconv=__thiscall  param_size_bytes=8  gog_size=140  gog_retn=8
Candidate Steam address: 0x00698100 (size 140, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00698070:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ad6c0 Global::ExecuteCommandSetLockOrientationInDialog -> 0x00698070  gate_class=REVIEW
- GOG-order neighbour after:  0x007ad7e0 Global::ExecuteCommandEnableRendering -> 0x00698190  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [506 SetLockHeadFollowInDialog] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d3a0 CClientExoApp::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
  0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 4 common): GOG {0x8*,0x18*,0x7f000000*,0xfffff82f*} | Steam {0x8*,0x18*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00698100: 16-byte aligned: YES
  00698100  55             PUSH EBP
  00698101  8B EC          MOV EBP,ESP
  00698103  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00698100
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF780  CGuiInGame::SetLockHeadFollowInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00698100 FUN_00698100 size=140 ret=8 callers=0 callees=5

undefined4 FUN_00698100(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      if (local_8 != 0x7f000000) {
        uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
        uVar3 = FUN_0073fec0(local_8);
        FUN_0073f750(uVar3,local_c,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar4);
        FUN_007cf780(uVar3,local_c);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00698070 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00698070: 16-byte aligned: YES
  00698070  55             PUSH EBP
  00698071  8B EC          MOV EBP,ESP
  00698073  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00698070
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF5D0  CGuiInGame::SetLockObjectOrientationInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EDX
```

## Card B15-4-26

GOG function: Global::ExecuteCommandSetLockOrientationInDialog  gog 0x007ad6c0  callconv=__thiscall  param_size_bytes=8  gog_size=140  gog_retn=8
Candidate Steam address: 0x00698070 (size 140, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00698100:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ad530 Global::ExecuteCommandDuplicateHeadAppearance -> 0x00697ee0  gate_class=AUTO
- GOG-order neighbour after:  0x007ad750 Global::ExecuteCommandSetLockHeadFollowInDialog -> 0x00698100  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [505 SetLockOrientationInDialog] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0040d3a0 CClientExoApp::ServerToClientObjectId -> Steam ?  in candidate callees: n/a
  0x004dfcd0 CGuiInGame::SetLockObjectOrientationInDialog -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 4 common): GOG {0x8*,0x18*,0x7f000000*,0xfffff82f*} | Steam {0x8*,0x18*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00698070: 16-byte aligned: YES
  00698070  55             PUSH EBP
  00698071  8B EC          MOV EBP,ESP
  00698073  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00698070
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF5D0  CGuiInGame::SetLockObjectOrientationInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00698070 FUN_00698070 size=140 ret=8 callers=0 callees=5

undefined4 FUN_00698070(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      if (local_8 != 0x7f000000) {
        uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
        uVar3 = FUN_0073fec0(local_8);
        FUN_0073f750(uVar3,local_c,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar4);
        FUN_007cf5d0(uVar3,local_c);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00698100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00698100: 16-byte aligned: YES
  00698100  55             PUSH EBP
  00698101  8B EC          MOV EBP,ESP
  00698103  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00698100
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F750  ClientApp::GetGuiInGame
  0x007CF780  CGuiInGame::SetLockHeadFollowInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EDX
```

## Card B15-4-27

GOG function: Global::ExecuteCommandGetDistanceBetweenLocations  gog 0x00790ba0  callconv=__thiscall  param_size_bytes=8  gog_size=527  gog_retn=8
Candidate Steam address: 0x0067b4a0 (size 274, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetDistanceBetween2D | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.28 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067b5c0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00790a80 Global::ExecuteCommandGetDistanceBetween2D -> 0x0067b4a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00790db0 Global::ExecuteCommandGetDistanceBetweenLocations2D -> 0x0067b7d0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [298 GetDistanceBetweenLocations] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: no
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 5 common): GOG {0x2,0x4*,0x8*,0xc*,0x64,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0xc*,0x50,0x94,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067b4a0: 16-byte aligned: YES
  0067b4a0  55             PUSH EBP
  0067b4a1  8B EC          MOV EBP,ESP
  0067b4a3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067b4a0
string literals (0):
direct callees (6):
  0x0051C080  CServerExoApp::GetObjectArray
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067b4a0 FUN_0067b4a0 size=274 ret=8 callers=0 callees=6

undefined4 __fastcall FUN_0067b4a0(undefined4 param_1)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 local_40 [12];
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  undefined4 local_10;
  float local_c;
  undefined4 local_8;
  
  local_c = 0.0;
  local_14 = FUN_0051c080(param_1);
  iVar2 = FUN_006fdaf0(&local_10);
  if ((iVar2 == 0) || (iVar2 = FUN_006fdaf0(&local_8), iVar2 == 0)) {
    return 0xfffff82f;
  }
  cVar1 = FUN_0053dfb0(local_10,&local_18);
  if ((cVar1 == DAT_0099447f) && (cVar1 = FUN_0053dfb0(local_8,&local_1c), cVar1 == DAT_0099447f)) {
    local_34 = *(undefined4 *)(local_18 + 0x94);
    local_30 = *(undefined4 *)(local_18 + 0x98);
    local_2c = 0;
    local_28 = *(undefined4 *)(local_1c + 0x94);
    local_24 = *(undefined4 *)(local_1c + 0x98);
    local_20 = 0;
    uVar3 = FUN_004452b0(local_40,&local_34,&local_28);
    fVar4 = (float10)FUN_005142f0(uVar3);
    local_c = (float)fVar4;
  }
  iVar2 = FUN_006fda00(local_c);
  if (iVar2 == 0) {
    return 0xfffff830;
  }
  return 0;
}
```

### decoy: 0x0067b5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067b5c0: 16-byte aligned: YES
  0067b5c0  55             PUSH EBP
  0067b5c1  8B EC          MOV EBP,ESP
  0067b5c3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067b5c0
string literals (0):
direct callees (6):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

## Card B15-4-28

GOG function: Global::ExecuteCommandGetGalaxyMapPlanetAvailable  gog 0x007acb70  callconv=__thiscall  param_size_bytes=8  gog_size=103  gog_retn=8
Candidate Steam address: 0x00697520 (size 103, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetPlanetAvailable 0x5fed60)
- best decoy (neighbour/other candidate) scores: 0x00697600:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acb00 Global::ExecuteCommandSetGalaxyMapPlanetAvailable -> 0x006974b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007acbe0 Global::ExecuteCommandSetGalaxyMapPlanetSelectable -> 0x00697590  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [743 GetPlanetAvailable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x00704710 CSWPartyTable::GetPlanetAvailable -> Steam 0x005fed60  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x10*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697520: 16-byte aligned: YES
  00697520  55             PUSH EBP
  00697521  8B EC          MOV EBP,ESP
  00697523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697520
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED60  CSWPartyTable::GetPlanetAvailable
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00697520 FUN_00697520 size=103 ret=8 callers=0 callees=4

undefined4 FUN_00697520(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_0051c8d0(local_c,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
    local_8 = FUN_005fed60(local_c);
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

### decoy: 0x00697600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697600: 16-byte aligned: YES
  00697600  55             PUSH EBP
  00697601  8B EC          MOV EBP,ESP
  00697603  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697600
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED00  CSWPartyTable::GetPlanetSelectable
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-29

GOG function: Global::ExecuteCommandSavePUPByObject  gog 0x007b1910  callconv=__thiscall  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x0069c340 (size 118, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetIsPlayerMadeCharacter | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SavePuppetByObject 0x5f8520)
- best decoy (neighbour/other candidate) scores: 0x0069c2c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b1890 Global::ExecuteCommandSaveNPCByObject -> 0x0069c2c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007b1990 Global::ExecuteCommandGetIsPlayerMadeCharacter -> 0x0069c3c0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [874 SavePUPByObject] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006fe090 CSWPartyTable::SavePuppetByObject -> Steam 0x005f8520  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x7f000000*,0xfffff82f*} | Steam {0x8*,0x10*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069c340: 16-byte aligned: YES
  0069c340  55             PUSH EBP
  0069c341  8B EC          MOV EBP,ESP
  0069c343  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069c340
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F8520  CSWPartyTable::SavePuppetByObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0069c340 FUN_0069c340 size=118 ret=8 callers=0 callees=4

undefined4 FUN_0069c340(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0xffffffff;
  local_8 = 0x7f000000;
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fdaf0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      uVar3 = local_c;
      uVar4 = local_8;
      FUN_0051c8d0(local_c,local_8,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_005f8520(uVar3,uVar4);
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0069c2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069c2c0: 16-byte aligned: YES
  0069c2c0  55             PUSH EBP
  0069c2c1  8B EC          MOV EBP,ESP
  0069c2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069c2c0
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F8130  CSWPartyTable::SaveMemberByObject
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-30

GOG function: Global::ExecuteCommandGetNearestObject  gog 0x00799310  callconv=__thiscall  param_size_bytes=8  gog_size=4828  gog_retn=8
Candidate Steam address: 0x00683b30 (size 151, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetMovementRate | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00683bd0:STR0/CALLEE0/SIG0.104/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007991a0 Global::ExecuteCommandGetName -> 0x00683bd0  gate_class=AUTO
- GOG-order neighbour after:  0x0079a710 Global::ExecuteCommandGetNearestMine -> 0x00685140  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [38 GetNearestCreature, 226 GetNearestCreatureToLocation, 227 GetNearestObject, 228 GetNearestObjectToLocation, 229 GetNearestObjectByTag] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (28), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00545d10 CExoArrayList::insert -> Steam ?  in candidate callees: n/a
  0x0054c000 CExoArrayList_float_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  ... 20 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4,0x8*,0x10,0x30,0x6c,0x8d,0x94,0x9c,0xab,0xe0,0x122,0x1198*,0x11ac} | Steam {0x8*,0x1a4,0x1198*}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc,0xd,0xe,0x10,...(+24)} | Steam {0x8*,0x1c,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00683b30: 16-byte aligned: YES
  00683b30  55             PUSH EBP
  00683b31  8B EC          MOV EBP,ESP
  00683b33  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00683b30
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x1198 [dec 4504]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x1A4 [dec 420]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00683b30 FUN_00683b30 size=151 ret=8 callers=0 callees=3

undefined4 FUN_00683b30(void)

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
    iVar1 = FUN_0051c100(local_8);
    local_c = 0;
    if ((iVar1 != 0) && (*(int *)(iVar1 + 0x1198) != 0)) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x1a4);
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

### decoy: 0x00683bd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00683bd0: 16-byte aligned: YES
  00683bd0  55             PUSH EBP
  00683bd1  8B EC          MOV EBP,ESP
  00683bd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00683bd0
string literals (0):
direct callees (7):
  0x00733540  CExoString::Constructor
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x00733780  CExoString::~CExoString
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00633620  (unnamed)
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 4):
  +0x8 [dec 8]  x2  via ECX
  +0x10 [dec 16]  x2  via EDX
  +0x8C [dec 140]  x1  via EDX
  +0x90 [dec 144]  x1  via EAX
```

## Card B15-4-31

GOG function: Global::ExecuteCommandGetGalaxyMapPlanetSelectable  gog 0x007acc50  callconv=__thiscall  param_size_bytes=8  gog_size=103  gog_retn=8
Candidate Steam address: 0x00697600 (size 103, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetPlanetSelectable 0x5fed00)
- best decoy (neighbour/other candidate) scores: 0x00697520:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acbe0 Global::ExecuteCommandSetGalaxyMapPlanetSelectable -> 0x00697590  gate_class=REVIEW
- GOG-order neighbour after:  0x007accc0 Global::ExecuteCommandGetGalaxyMapSelectedPlanet -> 0x00697670  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [741 GetPlanetSelectable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x007046b0 CSWPartyTable::GetPlanetSelectable -> Steam 0x005fed00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x10*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697600: 16-byte aligned: YES
  00697600  55             PUSH EBP
  00697601  8B EC          MOV EBP,ESP
  00697603  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697600
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED00  CSWPartyTable::GetPlanetSelectable
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00697600 FUN_00697600 size=103 ret=8 callers=0 callees=4

undefined4 FUN_00697600(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_0051c8d0(local_8,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
    uVar2 = FUN_005fed00(local_8);
    iVar1 = FUN_006fd9c0(uVar2);
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

### decoy: 0x00697520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697520: 16-byte aligned: YES
  00697520  55             PUSH EBP
  00697521  8B EC          MOV EBP,ESP
  00697523  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697520
string literals (0):
direct callees (4):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED60  CSWPartyTable::GetPlanetAvailable
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-32

GOG function: Global::ExecuteCommandGetDistanceBetweenLocations  gog 0x00790ba0  callconv=__thiscall  param_size_bytes=8  gog_size=527  gog_retn=8
Candidate Steam address: 0x0067b7d0 (size 577, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetDistanceBetweenLocations2D | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067b5c0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00790a80 Global::ExecuteCommandGetDistanceBetween2D -> 0x0067b4a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00790db0 Global::ExecuteCommandGetDistanceBetweenLocations2D -> 0x0067b7d0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [298 GetDistanceBetweenLocations] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xc*,0x64,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x4*,0x8*,0xc*,0x7c,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067b7d0: 16-byte aligned: YES
  0067b7d0  55             PUSH EBP
  0067b7d1  8B EC          MOV EBP,ESP
  0067b7d3  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067b7d0
string literals (0):
direct callees (6):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067b7d0 FUN_0067b7d0 size=577 ret=8 callers=0 callees=6

undefined4 FUN_0067b7d0(void)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_3c [12];
  undefined4 *local_30;
  undefined4 *local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 *local_18;
  undefined4 *local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdab0(2,&local_18);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fdab0(2,&local_14);
    if (iVar1 == 0) {
      if (local_18 != (undefined4 *)0x0) {
        local_30 = local_18;
        local_2c = local_18;
        if (local_18 != (undefined4 *)0x0) {
          FUN_00623320();
          FUN_009196fd(local_2c);
        }
      }
      uVar2 = 0xfffff82f;
    }
    else {
      local_24 = *local_18;
      local_20 = local_18[1 = byte +0x4];
      local_1c = 0;
      local_10 = *local_14;
      local_c = local_14[1 = byte +0x4];
      local_8 = 0;
      uVar2 = FUN_004452b0(local_3c,&local_24,&local_10);
      fVar3 = (float10)FUN_005142f0(uVar2);
      local_28 = (float)fVar3;
      iVar1 = FUN_006fda00(local_28);
      if (iVar1 == 0) {
        if (local_18 != (undefined4 *)0x0) {
          if (local_18 != (undefined4 *)0x0) {
            FUN_00623320();
            FUN_009196fd(local_18);
          }
          local_18 = (undefined4 *)0x0;
        }
        if ((local_14 != (undefined4 *)0x0) && (local_14 != (undefined4 *)0x0)) {
          FUN_00623320();
          FUN_009196fd(local_14);
        }
        uVar2 = 0xfffff830;
      }
      else {
        if (local_18 != (undefined4 *)0x0) {
          if (local_18 != (undefined4 *)0x0) {
            FUN_00623320();
            FUN_009196fd(local_18);
          }
          local_18 = (undefined4 *)0x0;
        }
        if ((local_14 != (undefined4 *)0x0) && (local_14 != (undefined4 *)0x0)) {
          FUN_00623320();
          FUN_009196fd(local_14);
        }
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0067b5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067b5c0: 16-byte aligned: YES
  0067b5c0  55             PUSH EBP
  0067b5c1  8B EC          MOV EBP,ESP
  0067b5c3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067b5c0
string literals (0):
direct callees (6):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

## Card B15-4-33

GOG function: Global::ExecuteCommandEffectForceResistanceIncrease  gog 0x0078c300  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676be0 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectForceResistanceDecrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676d20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c1c0 Global::ExecuteCommandEffectForceResistanceDecrease -> 0x00676be0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c440 CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints -> 0x00676e60  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [212 EffectForceResistanceIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

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

### decoy: 0x00676d20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676d20: 16-byte aligned: YES
  00676d20  55             PUSH EBP
  00676d21  8B EC          MOV EBP,ESP
  00676d23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00676d20
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

## Card B15-4-34

GOG function: Global::ExecuteCommandSetHealTarget  gog 0x007afbd0  callconv=__thiscall  param_size_bytes=8  gog_size=120  gog_retn=8
Candidate Steam address: 0x0069a580 (size 120, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069a600:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007afb70 Global::ExecuteCommandYavinHackCloseDoor -> 0x0069a520  gate_class=AUTO
- GOG-order neighbour after:  0x007afc50 Global::ExecuteCommandGetHealTarget -> 0x0069a600  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [813 SetHealTarget] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x5ac*} | Steam {0x8*,0x5ac*}
imm sets (* = in both, 3 common): GOG {0x8*,0x18*,0xfffff82f*} | Steam {0x8*,0x18*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069a580: 16-byte aligned: YES
  0069a580  55             PUSH EBP
  0069a581  8B EC          MOV EBP,ESP
  0069a583  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069a580
string literals (0):
direct callees (2):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x5AC [dec 1452]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0069a580 FUN_0069a580 size=120 ret=8 callers=0 callees=2

undefined4 FUN_0069a580(void)

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
    iVar1 = FUN_006fdaf0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c100(local_8);
      if (iVar1 != 0) {
        *(undefined4 *)(iVar1 + 0x5ac) = local_c;
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0069a600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069a600: 16-byte aligned: YES
  0069a600  55             PUSH EBP
  0069a601  8B EC          MOV EBP,ESP
  0069a603  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069a600
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x5AC [dec 1452]  x1  via ECX
```

## Card B15-4-35

GOG function: Global::ExecuteCommandCancelCombat  gog 0x007ac6e0  callconv=__thiscall  param_size_bytes=8  gog_size=134  gog_retn=8
Candidate Steam address: 0x00697090 (size 134, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00696d80:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac670 Global::ExecuteCommandGetSubScreenID -> 0x00697020  gate_class=AUTO
- GOG-order neighbour after:  0x007ac770 Global::ExecuteCommandPauseGame -> 0x00697120  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [54 CancelCombat] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069f230 CSWSCreature::CancelCombat -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x14*,0x7f000000*,0xfffff82f*} | Steam {0x2*,0x8*,0x14*,0x7f000000*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697090: 16-byte aligned: YES
  00697090  55             PUSH EBP
  00697091  8B EC          MOV EBP,ESP
  00697093  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697090
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00586750  CSWSCreature::CancelCombat
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00697090 FUN_00697090 size=134 ret=8 callers=0 callees=4

undefined4 FUN_00697090(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  local_10 = 0x7f000000;
  local_8 = 0;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if ((1 < param_2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    local_c = FUN_0051c100(local_10);
    if (local_c != 0) {
      FUN_00586750(local_8);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00696d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00696d80: 16-byte aligned: YES
  00696d80  55             PUSH EBP
  00696d81  8B EC          MOV EBP,ESP
  00696d83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00696d80
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DAA00  CSWSCreature::IsDebilitated
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-4-36

GOG function: Global::ExecuteCommandVectorMagnitude  gog 0x007aacd0  callconv=__thiscall  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x00695500 (size 378, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandVectorConversions | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00695680:STR0/CALLEE2/SIG0.5/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 3 common): GOG {0x4,0x8*,0x14,0xfffff82f*,0xfffff830*} | Steam {0x5,0x8*,0xc,0x48,0x90,0x91,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00695500: 16-byte aligned: YES
  00695500  55             PUSH EBP
  00695501  8B EC          MOV EBP,ESP
  00695503  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00695500 FUN_00695500 size=378 ret=8 callers=0 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00695500(int param_1)

{
  int iVar1;
  float *pfVar2;
  float10 fVar3;
  undefined1 local_30 [12];
  float local_24;
  float local_20;
  undefined4 local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  
  if (param_1 == 0x91) {
    local_10 = 0.0;
    local_c = 0.0;
    local_8 = 0.0;
    iVar1 = FUN_006fda20();
    if (iVar1 == 0) {
      return 0xfffff82f;
    }
    pfVar2 = (float *)FUN_005143c0(local_30,&local_10);
    local_10 = *pfVar2;
    local_c = pfVar2[1 = byte +0x4];
    local_8 = pfVar2[2 = byte +0x8];
    fVar3 = (float10)FUN_0091e620((double)local_10);
    local_14 = (float)fVar3;
    if (local_c < (float)_DAT_00986b38) {
      local_14 = (float)_DAT_0098c260 - local_14;
    }
    iVar1 = FUN_006fda00();
    if (iVar1 == 0) {
      return 0xfffff830;
    }
  }
  if (param_1 == 0x90) {
    local_24 = 0.0;
    local_20 = 0.0;
    local_1c = 0;
    iVar1 = FUN_006fd9e0();
    if (iVar1 == 0) {
      return 0xfffff82f;
    }
    fVar3 = (float10)FUN_0091ea50((double)(local_18 * (float)_DAT_0098be40));
    local_24 = (float)fVar3;
    fVar3 = (float10)FUN_0091e770((double)(local_18 * (float)_DAT_0098be40));
    local_20 = (float)fVar3;
    local_1c = 0;
    iVar1 = FUN_006fda40(local_24,local_20,0);
    if (iVar1 == 0) {
      return 0xfffff830;
    }
  }
  return 0;
}
```

### decoy: 0x00695680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695680: 16-byte aligned: YES
  00695680  55             PUSH EBP
  00695681  8B EC          MOV EBP,ESP
  00695683  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695680
string literals (0):
direct callees (3):
  0x006FDA20  CVirtualMachine::StackPopVector
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

## Card B15-4-37

GOG function: Global::ExecuteCommandAddBonusForcePoints  gog 0x007af840  callconv=__thiscall  param_size_bytes=8  gog_size=270  gog_retn=8
Candidate Steam address: 0x0069a1f0 (size 270, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0069a0c0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007af710 Global::ExecuteCommandSetBonusForcePoints -> 0x0069a0c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007af950 Global::ExecuteCommandGetBonusForcePoints -> 0x0069a300  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [802 AddBonusForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x004df5f0 CGuiInGame::UpdateStatus -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x11f4*,0x1200*} | Steam {0x4*,0x8*,0x30*,0x11f4*,0x1200*}
imm sets (* = in both, 5 common): GOG {0x8*,0xb*,0xc*,0x2c*,0xfffff830*} | Steam {0x8*,0xb*,0xc*,0x2c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069a1f0: 16-byte aligned: YES
  0069a1f0  55             PUSH EBP
  0069a1f1  8B EC          MOV EBP,ESP
  0069a1f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069a1f0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EDX
  +0x11F4 [dec 4596]  x2  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x1200 [dec 4608]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0069a1f0 FUN_0069a1f0 size=270 ret=8 callers=0 callees=5

undefined4 FUN_0069a1f0(void)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_8);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      piVar3 = (int *)FUN_0051c0a0(local_c);
      if (piVar3 == (int *)0x0) {
        iVar1 = 0;
      }
      else {
        iVar1 = (**(code **)(*piVar3 + 0x30))();
      }
      if (iVar1 != 0) {
        *(int *)(iVar1 + 0x11f4) = *(int *)(iVar1 + 0x11f4) + local_8;
        if (*(int *)(iVar1 + 0x1200) != 0) {
          if (local_8 < 0) {
            uVar4 = 0xc;
            FUN_0073f750(0xc,local_8,iVar1,uVar2);
            FUN_007ceef0(uVar4,local_8);
          }
          else {
            uVar4 = 0xb;
            FUN_0073f750(0xb,local_8,iVar1,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 4));
            FUN_007ceef0(uVar4,local_8);
          }
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0069a0c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0069a0c0: 16-byte aligned: YES
  0069a0c0  55             PUSH EBP
  0069a0c1  8B EC          MOV EBP,ESP
  0069a0c3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0069a0c0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0073F750  ClientApp::GetGuiInGame
  0x007CEEF0  CGuiInGame::UpdateStatus
struct offsets touched (top 5):
  +0x11F4 [dec 4596]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x1200 [dec 4608]  x1  via ECX
```

## Card B15-4-38

GOG function: Global::ExecuteCommandChangeFaction  gog 0x00784680  callconv=(blank)  param_size_bytes=4  gog_size=650  gog_retn=8
Candidate Steam address: 0x0066f0a0 (size 650, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::ExecuteCommandChangeFaction/ChangeToStandardFaction | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066f330:STR0/CALLEE0/SIG0.208/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00783dc0 Global::ExecuteCommandCastSpell -> 0x0066e7e0  gate_class=AUTO
- GOG-order neighbour after:  0x00784910 Global::ExecuteCommandClearAllActions -> 0x0066f330  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [173 ChangeFaction, 412 ChangeToStandardFaction] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c0b0 CServerExoApp::GetFactionOfObject -> Steam ?  in candidate callees: n/a
  0x007e4850 CSWSFaction::AddMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
  0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x40*,0x48*,0x6c*,0x27c*,0x2f8*,0x1198*} | Steam {0x4*,0x8*,0x40*,0x48*,0x6c*,0x27c*,0x2f8*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x64*,0xad*,0x19c*,0xfffff82f*} | Steam {0x8*,0x64*,0xad*,0x19c*,0xfffff82f*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0066f0a0: 16-byte aligned: YES
  0066f0a0  55             PUSH EBP
  0066f0a1  8B EC          MOV EBP,ESP
  0066f0a3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066f0a0
string literals (0):
direct callees (8):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0051C690  (unnamed)
  0x00664650  CFactionManager::GetIsNPCFaction(id) (1..N)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x00664490  (unnamed)
  0x006D3330  CSWSFaction::AddMember(objId, bSuppress)
struct offsets touched (top 9):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x6C [dec 108]  x3  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x40 [dec 64]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x27C [dec 636]  x1  via ECX
  +0x2F8 [dec 760]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0066f0a0 FUN_0066f0a0 size=650 ret=8 callers=0 callees=8

undefined4 FUN_0066f0a0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined4 local_20;
  int *local_1c;
  int local_18;
  undefined4 local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_1c = (int *)0x0;
  iVar2 = FUN_006fdaf0(&local_c);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    uVar3 = extraout_var;
    if (param_1 == 0xad) {
      iVar2 = FUN_006fdaf0(&local_8);
      if (iVar2 == 0) {
        return 0xfffff82f;
      }
    }
    else if ((param_1 == 0x19c) && (iVar2 = FUN_006fd9a0(&local_14), iVar2 == 0)) {
      return 0xfffff82f;
    }
    local_10 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
    if (local_10 == 0) {
      uVar3 = 0;
    }
    else if (*(int *)(local_10 + 0x10054) == 0) {
      uVar3 = 0;
    }
    else {
      if (param_1 == 0xad) {
        local_18 = FUN_0051c100(local_c);
        if ((local_18 == 0) || (*(int *)(*(int *)(local_18 + 0x1198) + 0x6c) != 0)) {
          return 0;
        }
        iVar2 = FUN_0051c690(local_8,&local_20);
        if (iVar2 == 0) {
          return 0;
        }
        local_18 = FUN_0051c100(local_8);
        if ((local_18 == 0) || (*(int *)(*(int *)(local_18 + 0x1198) + 0x6c) != 0)) {
          return 0;
        }
      }
      else if (param_1 == 0x19c) {
        iVar2 = FUN_00664650(local_14);
        if (iVar2 == 0) {
          return 0;
        }
        local_18 = FUN_0051c100(local_c);
        if ((local_18 != 0) && (*(int *)(*(int *)(local_18 + 0x1198) + 0x6c) != 0)) {
          return 0;
        }
        local_20 = local_14;
      }
      local_1c = (int *)FUN_0051c0a0(local_c);
      uVar1 = local_20;
      if (local_1c != (int *)0x0) {
        if ((char)local_1c[2 = byte +0x8] == DAT_00994478) {
          iVar2 = (**(code **)(*local_1c + 0x48))();
          *(undefined4 *)(iVar2 + 0x27c) = uVar1;
        }
        else if ((char)local_1c[2 = byte +0x8] == DAT_00994476) {
          iVar2 = (**(code **)(*local_1c + 0x40))(uVar3);
          *(undefined4 *)(iVar2 + 0x2f8) = uVar1;
        }
        else {
          uVar3 = 0;
          FUN_00664490(local_20);
          FUN_006d3330(local_c,uVar3);
/* ... 7 more lines truncated by --max-lines */
```

### decoy: 0x0066f330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066f330: 16-byte aligned: YES
  0066f330  55             PUSH EBP
  0066f331  8B EC          MOV EBP,ESP
  0066f333  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066f330
string literals (0):
direct callees (3):
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00541080  CSWSObject::ClearAllActions
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
```

## Card B15-4-39

GOG function: Global::ExecuteCommandGetNearestObject  gog 0x00799310  callconv=__thiscall  param_size_bytes=8  gog_size=4828  gog_retn=8
Candidate Steam address: 0x006838d0 (size 172, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006839a0:STR0/CALLEE0/SIG0.085/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007991a0 Global::ExecuteCommandGetName -> 0x00683bd0  gate_class=AUTO
- GOG-order neighbour after:  0x0079a710 Global::ExecuteCommandGetNearestMine -> 0x00685140  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [38 GetNearestCreature, 226 GetNearestCreatureToLocation, 227 GetNearestObject, 228 GetNearestObjectToLocation, 229 GetNearestObjectByTag] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (28), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00545d10 CExoArrayList::insert -> Steam ?  in candidate callees: n/a
  0x0054c000 CExoArrayList_float_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  ... 20 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8*,0x10,0x30,0x6c,0x8d,0x94,0x9c,0xab,0xe0,0x122,0x1198,0x11ac} | Steam {0x8*,0x174,0x178,0x17c,0x180}
imm sets (* = in both, 4 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x9,0xa,0xb*,0xc,0xd,0xe,0x10,...(+24)} | Steam {0x8*,0xb*,0x14,0x11a,0x7f000000*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006838d0: 16-byte aligned: YES
  006838d0  55             PUSH EBP
  006838d1  8B EC          MOV EBP,ESP
  006838d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006838d0
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 6):
  +0x8 [dec 8]  x1  via EAX
  +0x174 [dec 372]  x1  via EAX
  +0x178 [dec 376]  x1  via EDX
  +0x17C [dec 380]  x1  via ECX
  +0x180 [dec 384]  x1  via EAX
  +0x683990 [dec 6830480]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006838d0 FUN_006838d0 size=172 ret=8 callers=0 callees=2

undefined4 FUN_006838d0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  
  local_c = 0x7f000000;
  iVar1 = FUN_0051bd90();
  if (iVar1 != 0) {
    switch(param_1) {
    case 0x11a:
      local_c = *(undefined4 *)(iVar1 + 0x174);
      break;
    case 0x11b:
      local_c = *(undefined4 *)(iVar1 + 0x178);
      break;
    case 0x124:
      local_c = *(undefined4 *)(iVar1 + 0x17c);
      break;
    case 0x125:
      local_c = *(undefined4 *)(iVar1 + 0x180);
    }
  }
  iVar1 = FUN_006fdb10(local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x006839a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006839a0: 16-byte aligned: YES
  006839a0  55             PUSH EBP
  006839a1  8B EC          MOV EBP,ESP
  006839a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-4-40

GOG function: Global::ExecuteCommandPauseGame  gog 0x007ac770  callconv=__thiscall  param_size_bytes=8  gog_size=71  gog_retn=8
Candidate Steam address: 0x00697120 (size 71, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697460:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac6e0 Global::ExecuteCommandCancelCombat -> 0x00697090  gate_class=REVIEW
- GOG-order neighbour after:  0x007ac7c0 Global::ExecuteCommandGetItemStackSize -> 0x00697170  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [57 PauseGame] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0xfffff82f*} | Steam {0x8*,0xc*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697120: 16-byte aligned: YES
  00697120  55             PUSH EBP
  00697121  8B EC          MOV EBP,ESP
  00697123  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697120
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00740350  CClientExoApp::SetPausedByCombat
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00697120 FUN_00697120 size=71 ret=8 callers=0 callees=2

undefined4 FUN_00697120(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_00740350(local_8,0,0);
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00697460 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697460: 16-byte aligned: YES
  00697460  55             PUSH EBP
  00697461  8B EC          MOV EBP,ESP
  00697463  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697460
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F750  ClientApp::GetGuiInGame
  0x007CA730  CGuiInGame::ShowGalaxyMapGui
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B15-4-41

GOG function: Global::ExecuteCommandSetGalaxyMapPlanetSelectable  gog 0x007acbe0  callconv=__thiscall  param_size_bytes=8  gog_size=104  gog_retn=8
Candidate Steam address: 0x00697590 (size 104, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: SetPlanetSelectable 0x5fecd0)
- best decoy (neighbour/other candidate) scores: 0x006974b0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acb70 Global::ExecuteCommandGetGalaxyMapPlanetAvailable -> 0x00697520  gate_class=REVIEW
- GOG-order neighbour after:  0x007acc50 Global::ExecuteCommandGetGalaxyMapPlanetSelectable -> 0x00697600  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [740 SetPlanetSelectable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x00704680 CSWPartyTable::SetPlanetSelectable -> Steam 0x005fecd0  in candidate callees: yes
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
prologue for 0x00697590: 16-byte aligned: YES
  00697590  55             PUSH EBP
  00697591  8B EC          MOV EBP,ESP
  00697593  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697590
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FECD0  CSWPartyTable::SetPlanetSelectable
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00697590 FUN_00697590 size=104 ret=8 callers=0 callees=3

undefined4 FUN_00697590(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fd9a0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      FUN_0051c8d0(local_8,local_c,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_005fecd0(local_8,local_c);
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006974b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006974b0: 16-byte aligned: YES
  006974b0  55             PUSH EBP
  006974b1  8B EC          MOV EBP,ESP
  006974b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006974b0
string literals (0):
direct callees (3):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FED30  CSWPartyTable::SetPlanetAvailable
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-4-42

GOG function: Global::ExecuteCommandChangeFaction  gog 0x00784680  callconv=(blank)  param_size_bytes=4  gog_size=650  gog_retn=8
Candidate Steam address: 0x0066f3b0 (size 199, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandCloseDoor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.28 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066f330:STR0/CALLEE0/SIG0.208/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00783dc0 Global::ExecuteCommandCastSpell -> 0x0066e7e0  gate_class=AUTO
- GOG-order neighbour after:  0x00784910 Global::ExecuteCommandClearAllActions -> 0x0066f330  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [173 ChangeFaction, 412 ChangeToStandardFaction] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: no
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064c0b0 CServerExoApp::GetFactionOfObject -> Steam ?  in candidate callees: n/a
  0x007e4850 CSWSFaction::AddMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
  0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x40,0x48,0x6c,0x27c,0x2f8,0x1198} | Steam {0x4*,0x8*,0x10}
imm sets (* = in both, 2 common): GOG {0x8*,0x64,0xad,0x19c,0xfffff82f*} | Steam {0x8*,0x34,0xfffff82f*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0066f3b0: 16-byte aligned: YES
  0066f3b0  55             PUSH EBP
  0066f3b1  8B EC          MOV EBP,ESP
  0066f3b3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066f3b0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x00547940  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EDX
  +0x10 [dec 16]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0066f3b0 FUN_0066f3b0 size=199 ret=8 callers=0 callees=5

undefined4 FUN_0066f3b0(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int **ppiVar4;
  int extraout_var;
  int *local_14;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  iVar2 = FUN_006fdaf0(&local_8);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    local_10 = 0;
    if (*(int *)(extraout_var + 4) == 1) {
      ppiVar4 = &local_14;
      uVar3 = *(undefined4 *)(extraout_var + 8);
      FUN_0051c080(uVar3,ppiVar4,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
      cVar1 = FUN_0053dfb0(uVar3,ppiVar4);
      if (cVar1 == DAT_0099447f) {
        local_c = FUN_005453c0();
        if (local_c != 0) {
          uVar3 = 0;
          (**(code **)(*local_14 + 0x10))(local_8,0);
          FUN_00547940(local_8,uVar3);
        }
      }
    }
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x0066f330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066f330: 16-byte aligned: YES
  0066f330  55             PUSH EBP
  0066f331  8B EC          MOV EBP,ESP
  0066f333  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066f330
string literals (0):
direct callees (3):
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00541080  CSWSObject::ClearAllActions
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
```
