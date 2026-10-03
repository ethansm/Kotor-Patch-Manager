# Review cards B15 chunk 2

Answer each card with one TSV row in `steam_db_batches/B15/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B15-2-01

GOG function: Global::ExecuteCommandGetDistanceBetween2D  gog 0x00790a80  callconv=__thiscall  param_size_bytes=8  gog_size=274  gog_retn=8
Candidate Steam address: 0x0067b4a0 (size 274, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067b7d0:STR0/CALLEE1/SIG0.778/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00790990 Global::ExecuteCommandGetDistanceBetween -> 0x0067b3b0  gate_class=AUTO
- GOG-order neighbour after:  0x00790ba0 Global::ExecuteCommandGetDistanceBetweenLocations -> 0x0067b5c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [319 GetDistanceBetween2D] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x50*,0x94*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0xc*,0x50*,0x94*,0xfffff82f*,0xfffff830*}

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

### decoy: 0x0067b7d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067b7d0: 16-byte aligned: YES
  0067b7d0  55             PUSH EBP
  0067b7d1  8B EC          MOV EBP,ESP
  0067b7d3  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B15-2-02

GOG function: Global::ExecuteCommandGetFactionWorstAC  gog 0x00792d70  callconv=__thiscall  param_size_bytes=8  gog_size=270  gog_retn=8
Candidate Steam address: 0x0067d790 (size 270, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067d680:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792c60 Global::ExecuteCommandGetFactionWeakestMember -> 0x0067d680  gate_class=REVIEW
- GOG-order neighbour after:  0x00792e80 Global::ExecuteCommandGetFortitudeSavingThrow -> 0x0067d8a0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [192 GetFactionWorstAC] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5a80 CSWSFaction::GetWorstAC -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x34*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x34*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d790: 16-byte aligned: YES
  0067d790  55             PUSH EBP
  0067d791  8B EC          MOV EBP,ESP
  0067d793  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d790
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D4560  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d790 FUN_0067d790 size=270 ret=8 callers=0 callees=6

undefined4 FUN_0067d790(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0;
  local_18 = 0x7f000000;
  local_14 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_14);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_14);
    if (iVar1 != 0) {
      local_10 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_14;
        uVar3 = local_8;
        FUN_00664490(local_10);
        local_18 = FUN_006d4560(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_18);
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

### decoy: 0x0067d680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d680: 16-byte aligned: YES
  0067d680  55             PUSH EBP
  0067d681  8B EC          MOV EBP,ESP
  0067d683  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d680
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3920  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-03

GOG function: Global::ExecuteCommandGetFactionAverageReputation  gog 0x007921b0  callconv=__thiscall  param_size_bytes=8  gog_size=239  gog_retn=8
Candidate Steam address: 0x0067cbd0 (size 239, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067cda0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007920d0 Global::ExecuteCommandGetFactionAverageLevel -> 0x0067caf0  gate_class=REVIEW
- GOG-order neighbour after:  0x007922a0 Global::ExecuteCommandGetFactionAverageXP -> 0x0067ccc0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [186 GetFactionAverageReputation] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5510 CSWSFaction::GetAverageReputation -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x8*,0x30*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x30*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067cbd0: 16-byte aligned: YES
  0067cbd0  55             PUSH EBP
  0067cbd1  8B EC          MOV EBP,ESP
  0067cbd3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067cbd0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3FF0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067cbd0 FUN_0067cbd0 size=239 ret=8 callers=0 callees=5

undefined4 FUN_0067cbd0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = 0xffffffff;
  iVar1 = FUN_006fdaf0(&local_14);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fdaf0(&local_c);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar1 = FUN_0051c100(local_14);
      if (iVar1 != 0) {
        local_8 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
        iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
        if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
          FUN_00664490(local_8);
          local_10 = FUN_006d3ff0(local_c);
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
  }
  return uVar2;
}
```

### decoy: 0x0067cda0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067cda0: 16-byte aligned: YES
  0067cda0  55             PUSH EBP
  0067cda1  8B EC          MOV EBP,ESP
  0067cda3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067cda0
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D46D0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-04

GOG function: Global::ExecuteCommandGetFactionStrongestMember  gog 0x00792b50  callconv=__thiscall  param_size_bytes=8  gog_size=263  gog_retn=8
Candidate Steam address: 0x0067d570 (size 263, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067d680:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792a70 Global::ExecuteCommandGetFactionMostFrequentClass -> 0x0067d490  gate_class=REVIEW
- GOG-order neighbour after:  0x00792c60 Global::ExecuteCommandGetFactionWeakestMember -> 0x0067d680  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [182 GetFactionStrongestMember] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e4fa0 CSWSFaction::GetStrongestMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d570: 16-byte aligned: YES
  0067d570  55             PUSH EBP
  0067d571  8B EC          MOV EBP,ESP
  0067d573  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d570
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3A80  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d570 FUN_0067d570 size=263 ret=8 callers=0 callees=6

undefined4 FUN_0067d570(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_14 = 0x7f000000;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_10;
        uVar3 = local_8;
        FUN_00664490(local_c);
        local_14 = FUN_006d3a80(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_14);
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

### decoy: 0x0067d680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d680: 16-byte aligned: YES
  0067d680  55             PUSH EBP
  0067d681  8B EC          MOV EBP,ESP
  0067d683  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d680
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3920  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-05

GOG function: Global::ExecuteCommandGetDistanceBetweenLocations2D  gog 0x00790db0  callconv=__thiscall  param_size_bytes=8  gog_size=577  gog_retn=8
Candidate Steam address: 0x0067b7d0 (size 577, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067b5c0:STR0/CALLEE2/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00790ba0 Global::ExecuteCommandGetDistanceBetweenLocations -> 0x0067b5c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00791000 Global::ExecuteCommandGetDistanceToObject -> 0x0067ba20  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [334 GetDistanceBetweenLocations2D] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0xc*,0x7c*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x4*,0x8*,0xc*,0x7c*,0xfffff82f*,0xfffff830*}

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

## Card B15-2-06

GOG function: Global::ExecuteCommandGetGold  gog 0x00793430  callconv=__thiscall  param_size_bytes=8  gog_size=248  gog_retn=8
Candidate Steam address: 0x0067de50 (size 248, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067e000:STR0/CALLEE1/SIG1/SRV0/VT0
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
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x006693b0 CItemRepository::FindItemWithBaseItemType -> Steam 0x00605b80  in candidate callees: yes
  0x00699e10 CSWSCreature::GetGold -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x2cc*,0x3c4*} | Steam {0x8*,0x2cc*,0x3c4*}
imm sets (* = in both, 5 common): GOG {0x8*,0x17*,0x30*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x17*,0x30*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067de50: 16-byte aligned: YES
  0067de50  55             PUSH EBP
  0067de51  8B EC          MOV EBP,ESP
  0067de53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0067de50 FUN_0067de50 size=248 ret=8 callers=0 callees=7

undefined4 FUN_0067de50(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_c = 0;
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 == 0) {
      iVar1 = FUN_0051c160(local_10);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x3c4) != 0)) {
        uVar2 = FUN_00605b80(0x17,0);
        local_8 = FUN_0051c0e0(uVar2);
        if (local_8 != 0) {
          local_c = *(undefined4 *)(local_8 + 0x2cc);
        }
      }
    }
    else {
      local_c = FUN_005812e0(uVar2);
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

### decoy: 0x0067e000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067e000: 16-byte aligned: YES
  0067e000  55             PUSH EBP
  0067e001  8B EC          MOV EBP,ESP
  0067e003  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067e000
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005817F0  CSWSCreature::GetChemical
  0x0051C160  ServerApp::GetPlaceableById
  0x00605B80  CItemRepository::FindItemWithBaseItemType
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x3C4 [dec 964]  x2  via EAX,ECX
  +0x2CC [dec 716]  x1  via EDX
```

## Card B15-2-07

GOG function: Global::ExecuteCommandPauseGame  gog 0x007ac770  callconv=__thiscall  param_size_bytes=8  gog_size=71  gog_retn=8
Candidate Steam address: 0x00697170 (size 121, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetItemStackSize | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.20 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697120:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007ac6e0 Global::ExecuteCommandCancelCombat -> 0x00697090  gate_class=REVIEW
- GOG-order neighbour after:  0x007ac7c0 Global::ExecuteCommandGetItemStackSize -> 0x00697170  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [57 PauseGame] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040d810 CClientExoApp::SetPausedByCombat -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x8,0x2cc}
imm sets (* = in both, 2 common): GOG {0x8*,0xc,0xfffff82f*} | Steam {0x8*,0x14,0xfffff82f*,0xfffff830}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00697170: 16-byte aligned: YES
  00697170  55             PUSH EBP
  00697171  8B EC          MOV EBP,ESP
  00697173  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00697170
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x2CC [dec 716]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00697170 FUN_00697170 size=121 ret=8 callers=0 callees=3

undefined4 FUN_00697170(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = FUN_0051c0e0(local_c);
    if (local_8 != 0) {
      local_10 = *(undefined4 *)(local_8 + 0x2cc);
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

## Card B15-2-08

GOG function: Global::ExecuteCommandEffectAbilityDecrease  gog 0x00786c40  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x00671820 (size 447, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectAbilityIncrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671660:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007866b0 Global::ExecuteCommandDoTouchAttack -> 0x006710d0  gate_class=AUTO
- GOG-order neighbour after:  0x00786e00 Global::ExecuteCommandEffectAbilityIncrease -> 0x00671820  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [446 EffectAbilityDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x00671820: 16-byte aligned: YES
  00671820  55             PUSH EBP
  00671821  8B EC          MOV EBP,ESP
  00671823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00671820 FUN_00671820 size=447 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00671820(int param_1)

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
      local_38 = (undefined2)DAT_00995fac;
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

### decoy: 0x00671660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00671660: 16-byte aligned: YES
  00671660  55             PUSH EBP
  00671661  8B EC          MOV EBP,ESP
  00671663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-2-09

GOG function: Global::ExecuteCommandGetEmotions  gog 0x00791a10  callconv=__thiscall  param_size_bytes=8  gog_size=467  gog_retn=8
Candidate Steam address: 0x0067c250 (size 156, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetEffectSpellId | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067c430:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00791970 Global::ExecuteCommandGetEffectType -> 0x0067c390  gate_class=REVIEW
- GOG-order neighbour after:  0x00791bf0 Global::ExecuteCommandGetEncounterData -> 0x0067c610  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [235 GetIsEnemy, 236 GetIsFriend, 237 GetIsNeutral] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: no
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: no
  0x006bd5b0 CSWSObject::GetReputation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x10,0x48,0x398} | Steam {0x1c}
imm sets (* = in both, 3 common): GOG {0x8*,0xa,0x34,0x5a,0xeb,0xec,0xed,0x7f000000,0xfffff82f*,0xfffff830*} | Steam {0x4,0x8*,0x1c,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067c250: 16-byte aligned: YES
  0067c250  55             PUSH EBP
  0067c251  8B EC          MOV EBP,ESP
  0067c253  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067c250
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067c250 FUN_0067c250 size=156 ret=8 callers=0 callees=4

undefined4 FUN_0067c250(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_c;
  undefined4 local_8;
  
  local_c = 0;
  iVar1 = FUN_006fdab0(0,&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = *(undefined4 *)(local_c + 0x1c);
    iVar3 = FUN_006fd9c0(local_8);
    iVar1 = local_c;
    if (iVar3 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      if (local_c != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0067c430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067c430: 16-byte aligned: YES
  0067c430  55             PUSH EBP
  0067c431  8B EC          MOV EBP,ESP
  0067c433  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067c430
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E13C0  CSWSObject::GetReputation
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EDX
  +0x48 [dec 72]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x398 [dec 920]  x1  via ECX
```

## Card B15-2-10

GOG function: Global::ExecuteCommandGetStringRight  gog 0x0079e650  callconv=__thiscall  param_size_bytes=8  gog_size=354  gog_retn=8
Candidate Steam address: 0x00689170 (size 289, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetStringUpperCase | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00689000:STR0/CALLEE1/SIG0.5/SRV0/VT0
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
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0x38,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30,0xfffff82f*,0xfffff830*}

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

### decoy: 0x00689000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689000: 16-byte aligned: YES
  00689000  55             PUSH EBP
  00689001  8B EC          MOV EBP,ESP
  00689003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-2-11

GOG function: Global::ExecuteCommandExportAllCharacters  gog 0x0078e770  callconv=__thiscall  param_size_bytes=8  gog_size=38  gog_retn=8
Candidate Steam address: 0x00679190 (size 38, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006791c0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e630 Global::ExecuteCommandExecuteScript -> 0x00679050  gate_class=AUTO
- GOG-order neighbour after:  0x0078e7a0 Global::ExecuteCommandFindSubString -> 0x006791c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [557 ExportAllCharacters] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0064b8f0 CServerExoApp::SetExportAllCharactersPending -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x00679190: 16-byte aligned: YES
  00679190  55             PUSH EBP
  00679191  8B EC          MOV EBP,ESP
  00679193  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00679190
string literals (0):
direct callees (1):
  0x0051BED0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00679190 FUN_00679190 size=38 ret=8 callers=0 callees=1

undefined4 FUN_00679190(void)

{
  FUN_0051bed0(1);
  return 0;
}
```

### decoy: 0x006791c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006791c0: 16-byte aligned: YES
  006791c0  55             PUSH EBP
  006791c1  8B EC          MOV EBP,ESP
  006791c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006791c0
string literals (0):
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734030  CExoString::Find
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

## Card B15-2-12

GOG function: Global::ExecuteCommandGetFactionAverageLevel  gog 0x007920d0  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067caf0 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ca10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00791ff0 Global::ExecuteCommandGetFactionAverageGoodEvilAlignment -> 0x0067ca10  gate_class=REVIEW
- GOG-order neighbour after:  0x007921b0 Global::ExecuteCommandGetFactionAverageReputation -> 0x0067cbd0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [189 GetFactionAverageLevel] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5740 CSWSFaction::GetAverageLevel -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067caf0: 16-byte aligned: YES
  0067caf0  55             PUSH EBP
  0067caf1  8B EC          MOV EBP,ESP
  0067caf3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067caf0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D4220  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067caf0 FUN_0067caf0 size=212 ret=8 callers=0 callees=5

undefined4 FUN_0067caf0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0xffffffff;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        FUN_00664490(local_8);
        local_c = FUN_006d4220(uVar2);
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

### decoy: 0x0067ca10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ca10: 16-byte aligned: YES
  0067ca10  55             PUSH EBP
  0067ca11  8B EC          MOV EBP,ESP
  0067ca13  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067ca10
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D40A0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B15-2-13

GOG function: Global::ExecuteCommandGetAttemptedSpellTarget  gog 0x0078f920  callconv=__thiscall  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0067a340 (size 97, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetAttemptedMovementTarget | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067a2d0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078f8b0 Global::ExecuteCommandGetAttemptedAttackTarget -> 0x0067a2d0  gate_class=AUTO
- GOG-order neighbour after:  0x0078f990 Global::ExecuteCommandGetAttemptedMovementTarget -> 0x0067a3b0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [375 GetAttemptedSpellTarget] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x568*} | Steam {0x8*,0x568*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x7f000000*,0xfffff830*} | Steam {0x8*,0x10*,0x7f000000*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067a340: 16-byte aligned: YES
  0067a340  55             PUSH EBP
  0067a341  8B EC          MOV EBP,ESP
  0067a343  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067a340
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x568 [dec 1384]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067a340 FUN_0067a340 size=97 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0067a340(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0x7f000000;
  iVar1 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
  if (iVar1 != 0) {
    local_8 = *(undefined4 *)(iVar1 + 0x568);
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

### decoy: 0x0067a2d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067a2d0: 16-byte aligned: YES
  0067a2d0  55             PUSH EBP
  0067a2d1  8B EC          MOV EBP,ESP
  0067a2d3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067a2d0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x550 [dec 1360]  x1  via ECX
```

## Card B15-2-14

GOG function: Global::ExecuteCommandEventUserDefined  gog 0x0078e490  callconv=__thiscall  param_size_bytes=8  gog_size=402  gog_retn=8
Candidate Steam address: 0x00678eb0 (size 402, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00678c40:STR0/CALLEE0/SIG0.278/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e220 Global::ExecuteCommandEventSpellCastAt -> 0x00678c40  gate_class=REVIEW
- GOG-order neighbour after:  0x0078e630 Global::ExecuteCommandExecuteScript -> 0x00679050  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [132 EventUserDefined] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0077a9b0 CSWCCMessageData::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0077aa40 CSWCCMessageData::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xb*,0x34*,0x3c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0xb*,0x34*,0x3c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00678eb0: 16-byte aligned: YES
  00678eb0  55             PUSH EBP
  00678eb1  8B EC          MOV EBP,ESP
  00678eb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00678eb0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
  0x005E1BD0  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00678eb0 FUN_00678eb0 size=402 ret=8 callers=0 callees=7

undefined4 FUN_00678eb0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *local_40;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a20b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x34,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_40 = (undefined2 *)0x0;
  }
  else {
    local_40 = (undefined2 *)FUN_005e1a60();
  }
  local_8 = 0xffffffff;
  iVar1 = FUN_006fd9a0(&local_14);
  if (iVar1 == 0) {
    if (local_40 != (undefined2 *)0x0) {
      FUN_005e1af0();
      FUN_009196fd(local_40);
    }
    uVar2 = 0xfffff82f;
  }
  else {
    *local_40 = 0xb;
    FUN_005e1bd0(0,local_14);
    iVar1 = FUN_006fdad0(1,local_40);
    if (iVar1 == 0) {
      if (local_40 != (undefined2 *)0x0) {
        FUN_005e1af0();
        FUN_009196fd(local_40);
      }
      uVar2 = 0xfffff830;
    }
    else {
      if (local_40 != (undefined2 *)0x0) {
        FUN_005e1af0();
        FUN_009196fd(local_40);
      }
      uVar2 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00678c40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00678c40: 16-byte aligned: YES
  00678c40  55             PUSH EBP
  00678c41  8B EC          MOV EBP,ESP
  00678c43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00678c40
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E1BD0  (unnamed)
  0x005E1C70  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 0):
```

## Card B15-2-15

GOG function: Global::ExecuteCommandEffectBodyFuel  gog 0x0078d8d0  callconv=__thiscall  param_size_bytes=8  gog_size=240  gog_retn=8
Candidate Steam address: 0x006782f0 (size 240, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006783e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d7d0 Global::ExecuteCommandEffectChoke -> 0x006781f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d9c0 Global::ExecuteCommandEffectPsychicStatic -> 0x006783e0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [224 EffectBodyFuel] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x24*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006782f0: 16-byte aligned: YES
  006782f0  55             PUSH EBP
  006782f1  8B EC          MOV EBP,ESP
  006782f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006782f0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006782f0 FUN_006782f0 size=240 ret=8 callers=0 callees=5

undefined4 FUN_006782f0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_30;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a17b;
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
  local_28 = (undefined2)DAT_009960a0;
  *(undefined2 *)(local_30 + 8) = local_28;
  iVar1 = FUN_006fdad0(0,local_30);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if (local_30 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_30);
    }
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x006783e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006783e0: 16-byte aligned: YES
  006783e0  55             PUSH EBP
  006783e1  8B EC          MOV EBP,ESP
  006783e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006783e0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-2-16

GOG function: Global::ExecuteCommandGetHitDice  gog 0x00793d10  callconv=__thiscall  param_size_bytes=8  gog_size=222  gog_retn=8
Candidate Steam address: 0x0067e730 (size 222, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067e4a0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00793c50 Global::ExecuteCommandGetHasSpellEffect -> 0x0067e670  gate_class=REVIEW
- GOG-order neighbour after:  0x00793df0 Global::ExecuteCommandGetHitpoints -> 0x0067e810  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [166 GetHitDice] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam 0x006a4e00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x30*,0x1198*} | Steam {0x8*,0x30*,0x1198*}
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
prologue for 0x0067e730: 16-byte aligned: YES
  0067e730  55             PUSH EBP
  0067e731  8B EC          MOV EBP,ESP
  0067e733  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067e730
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067e730 FUN_0067e730 size=222 ret=8 callers=0 callees=5

undefined4 FUN_0067e730(void)

{
  int iVar1;
  undefined4 uVar2;
  int **ppiVar3;
  undefined4 extraout_var;
  int *local_14;
  uint local_10;
  undefined4 local_c;
  char local_5;
  
  local_10 = 0;
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    ppiVar3 = &local_14;
    FUN_0051c080(local_c,ppiVar3,extraout_var);
    local_5 = FUN_0053dfb0(local_c,ppiVar3);
    if (((local_5 == DAT_0099447f) && (DAT_00994473 < *(byte *)(local_14 + 0x2 [dec 2] {int*: byte +0x8}))) &&
       (iVar1 = (**(code **)(*local_14 + 0x30))(), iVar1 != 0)) {
      local_10 = FUN_006a4e00(0);
      local_10 = local_10 & 0xff;
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

### decoy: 0x0067e4a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067e4a0: 16-byte aligned: YES
  0067e4a0  55             PUSH EBP
  0067e4a1  8B EC          MOV EBP,ESP
  0067e4a3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067e4a0
string literals (0):
direct callees (6):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006B7A00  CSWSCreatureStats::GetCanUseSkill
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x30 [dec 48]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B15-2-17

GOG function: Global::ExecuteCommandGetDistanceToObject2D  gog 0x007911a0  callconv=__thiscall  param_size_bytes=8  gog_size=426  gog_retn=8
Candidate Steam address: 0x0067bbc0 (size 426, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ba20:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00791000 Global::ExecuteCommandGetDistanceToObject -> 0x0067ba20  gate_class=REVIEW
- GOG-order neighbour after:  0x00791350 Global::ExecuteCommandGetEffect -> 0x0067bd70  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [335 GetDistanceToObject2D] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x60*,0x94*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0xc*,0x60*,0x94*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067bbc0: 16-byte aligned: YES
  0067bbc0  55             PUSH EBP
  0067bbc1  8B EC          MOV EBP,ESP
  0067bbc3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067bbc0
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067bbc0 FUN_0067bbc0 size=426 ret=8 callers=0 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0067bbc0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  int *piVar4;
  undefined1 local_48 [12];
  int local_3c;
  char local_36;
  char local_35;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  float local_18;
  int local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_18 = _DAT_00987474;
  local_34 = 0;
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  local_c = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (*(int *)(param_1 + 4) == 1) {
      piVar4 = &local_14;
      uVar2 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar2,piVar4);
      local_35 = FUN_0053dfb0(uVar2,piVar4);
      if (local_35 == DAT_0099447f) {
        local_28 = *(undefined4 *)(local_14 + 0x94);
        local_24 = *(undefined4 *)(local_14 + 0x98);
        local_20 = *(undefined4 *)(local_14 + 0x9c);
        local_10 = FUN_005453c0();
        local_1c = 1;
      }
    }
    if (local_1c == 1) {
      piVar4 = &local_3c;
      FUN_0051c080(local_8,piVar4);
      local_36 = FUN_0053dfb0(local_8,piVar4);
      if (((local_36 == DAT_0099447f) && (local_3c != 0)) &&
         (DAT_00994473 < *(byte *)(local_3c + 8))) {
        local_34 = *(undefined4 *)(local_3c + 0x94);
        local_30 = *(undefined4 *)(local_3c + 0x98);
        local_2c = *(undefined4 *)(local_3c + 0x9c);
        iVar1 = FUN_005453c0();
        if (iVar1 == local_10) {
          local_2c = 0;
          local_20 = 0;
          uVar2 = FUN_004452b0(local_48,&local_34,&local_28);
          fVar3 = (float10)FUN_005142f0(uVar2);
          local_18 = (float)fVar3;
        }
      }
    }
    iVar1 = FUN_006fda00(local_18);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
/* ... 3 more lines truncated by --max-lines */
```

### decoy: 0x0067ba20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ba20: 16-byte aligned: YES
  0067ba20  55             PUSH EBP
  0067ba21  8B EC          MOV EBP,ESP
  0067ba23  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067ba20
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
```

## Card B15-2-18

GOG function: Global::ExecuteCommandGetIsDay  gog 0x00794f90  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067f9b0 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetIsDoorActionPossible | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067f940:STR0/CALLEE0/SIG0.875/SRV0/VT0
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
prologue for 0x0067f9b0: 16-byte aligned: YES
  0067f9b0  55             PUSH EBP
  0067f9b1  8B EC          MOV EBP,ESP
  0067f9b3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067f9b0
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
// FN 0067f9b0 FUN_0067f9b0 size=106 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0067f9b0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  
  local_c = 0;
  iVar1 = FUN_0051bd90(param_1);
  if ((iVar1 != 0) && (*(char *)(iVar1 + 0x1dc) == '\x01')) {
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

### decoy: 0x0067f940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067f940: 16-byte aligned: YES
  0067f940  55             PUSH EBP
  0067f941  8B EC          MOV EBP,ESP
  0067f943  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067f940
string literals (0):
direct callees (2):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x1DC [dec 476]  x1  via EDX
```

## Card B15-2-19

GOG function: Global::ExecuteCommandGetMaxForcePoints  gog 0x00794030  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=8
Candidate Steam address: 0x0067ea50 (size 123, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ead0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00793f80 Global::ExecuteCommandGetCurrentForcePoints -> 0x0067e9a0  gate_class=AUTO
- GOG-order neighbour after:  0x007940b0 Global::ExecuteCommandGetIdentified -> 0x0067ead0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [56 GetMaxForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006977d0 CSWSCreature::GetMaxForcePoints -> Steam ?  in candidate callees: n/a
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
prologue for 0x0067ea50: 16-byte aligned: YES
  0067ea50  55             PUSH EBP
  0067ea51  8B EC          MOV EBP,ESP
  0067ea53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ea50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ea50 FUN_0067ea50 size=123 ret=8 callers=0 callees=4

undefined4 FUN_0067ea50(void)

{
  short sVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  iVar2 = FUN_006fdaf0(&local_10);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar4 = extraout_var;
    local_8 = FUN_0051c100(local_10);
    local_c = 0;
    if (local_8 != 0) {
      sVar1 = FUN_0057eca0(uVar4,uVar3);
      local_c = (int)sVar1;
    }
    iVar2 = FUN_006fd9c0(local_c);
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

### decoy: 0x0067ead0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ead0: 16-byte aligned: YES
  0067ead0  55             PUSH EBP
  0067ead1  8B EC          MOV EBP,ESP
  0067ead3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067ead0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-20

GOG function: Global::ExecuteCommandGetFactionMostDamagedMember  gog 0x00792960  callconv=__thiscall  param_size_bytes=8  gog_size=263  gog_retn=8
Candidate Steam address: 0x0067d380 (size 263, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067d570:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007927d0 Global::ExecuteCommandGetFactionMember -> 0x0067d1f0  gate_class=AUTO
- GOG-order neighbour after:  0x00792a70 Global::ExecuteCommandGetFactionMostFrequentClass -> 0x0067d490  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [183 GetFactionMostDamagedMember] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5100 CSWSFaction::GetMostDamagedMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d380: 16-byte aligned: YES
  0067d380  55             PUSH EBP
  0067d381  8B EC          MOV EBP,ESP
  0067d383  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d380
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3BE0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d380 FUN_0067d380 size=263 ret=8 callers=0 callees=6

undefined4 FUN_0067d380(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_18;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_18 = 0x7f000000;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_10;
        uVar3 = local_8;
        FUN_00664490(local_c);
        local_18 = FUN_006d3be0(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_18);
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

### decoy: 0x0067d570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d570: 16-byte aligned: YES
  0067d570  55             PUSH EBP
  0067d571  8B EC          MOV EBP,ESP
  0067d573  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d570
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3A80  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-21

GOG function: Global::ExecuteCommandGetAttemptedMovementTarget  gog 0x0078f990  callconv=__thiscall  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0067a3b0 (size 97, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetSpellTarget | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067a340:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078f920 Global::ExecuteCommandGetAttemptedSpellTarget -> 0x0067a340  gate_class=REVIEW
- GOG-order neighbour after:  0x0078fa00 Global::ExecuteCommandGetSpellTarget -> 0x0067a420  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [489 GetAttemptedMovementTarget] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x54c*} | Steam {0x8*,0x54c*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x7f000000*,0xfffff830*} | Steam {0x8*,0x10*,0x7f000000*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067a3b0: 16-byte aligned: YES
  0067a3b0  55             PUSH EBP
  0067a3b1  8B EC          MOV EBP,ESP
  0067a3b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067a3b0
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x54C [dec 1356]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067a3b0 FUN_0067a3b0 size=97 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0067a3b0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0x7f000000;
  iVar1 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
  if (iVar1 != 0) {
    local_8 = *(undefined4 *)(iVar1 + 0x54c);
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

### decoy: 0x0067a340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067a340: 16-byte aligned: YES
  0067a340  55             PUSH EBP
  0067a341  8B EC          MOV EBP,ESP
  0067a343  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067a340
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x568 [dec 1384]  x1  via ECX
```

## Card B15-2-22

GOG function: Global::ExecuteCommandFindSubString  gog 0x0078e7a0  callconv=__thiscall  param_size_bytes=8  gog_size=322  gog_retn=8
Candidate Steam address: 0x006791c0 (size 322, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00679050:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e770 Global::ExecuteCommandExportAllCharacters -> 0x00679190  gate_class=REVIEW
- GOG-order neighbour after:  0x0078e8f0 Global::ExecuteCommandFloatingText -> 0x00679310  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [66 FindSubString] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606140 CExoString::Find -> Steam ?  in candidate callees: n/a
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x28*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x28*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006791c0: 16-byte aligned: YES
  006791c0  55             PUSH EBP
  006791c1  8B EC          MOV EBP,ESP
  006791c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006791c0
string literals (0):
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x00734030  CExoString::Find
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006791c0 FUN_006791c0 size=322 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_006791c0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a270;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  FUN_00733540();
  local_8._0_1_ = 1;
  local_1c = 0xffffffff;
  iVar1 = FUN_006fda70(local_24);
  if (iVar1 == 0) {
    local_8 = (uint)local_8._1_3_ << 8;
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fda70(local_18);
    if (iVar1 == 0) {
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar2 = 0xfffff82f;
    }
    else {
      local_1c = FUN_00734030(local_18,0);
      iVar1 = FUN_006fd9c0(local_1c);
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
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00679050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00679050: 16-byte aligned: YES
  00679050  55             PUSH EBP
  00679051  8B EC          MOV EBP,ESP
  00679053  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00679050
string literals (0):
direct callees (6):
  0x00733540  CExoString::Constructor
  0x006FDA70  CVirtualMachine::StackPopString
  0x00733780  CExoString::~CExoString
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FD8D0  CVirtualMachine::RunScript
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B15-2-23

GOG function: Global::ExecuteCommandGetIsDusk  gog 0x007953a0  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067fdc0 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067f9b0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00795000 Global::ExecuteCommandGetIsDoorActionPossible -> 0x0067fa20  gate_class=AUTO
- GOG-order neighbour after:  0x00795410 Global::ExecuteCommandGetIsEncounterCreature -> 0x0067fe30  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [408 GetIsDusk] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x1dc*} | Steam {0x8*,0x1dc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14*,0xfffff830*} | Steam {0x4*,0x8*,0x14*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067fdc0: 16-byte aligned: YES
  0067fdc0  55             PUSH EBP
  0067fdc1  8B EC          MOV EBP,ESP
  0067fdc3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067fdc0
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
// FN 0067fdc0 FUN_0067fdc0 size=106 ret=8 callers=0 callees=2

undefined4 __fastcall FUN_0067fdc0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_c;
  
  local_c = 0;
  iVar1 = FUN_0051bd90(param_1);
  if ((iVar1 != 0) && (*(char *)(iVar1 + 0x1dc) == '\x04')) {
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

## Card B15-2-24

GOG function: Global::ExecuteCommandIntToString  gog 0x007a0a10  callconv=__thiscall  param_size_bytes=8  gog_size=229  gog_retn=8
Candidate Steam address: 0x0068b4b0 (size 313, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::ExecuteCommandGetIsDead | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068b3c0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007a0930 Global::ExecuteCommandIntToHexString -> 0x0068b2e0  gate_class=AUTO
- GOG-order neighbour after:  0x007a0b00 Global::ExecuteCommandIsAIState -> 0x0068b3c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [92 IntToString] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0x30,0x94,0x9c,0x11ac}
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
prologue for 0x0068b4b0: 16-byte aligned: YES
  0068b4b0  55             PUSH EBP
  0068b4b1  8B EC          MOV EBP,ESP
  0068b4b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0068b4b0 FUN_0068b4b0 size=313 ret=8 callers=0 callees=3

undefined4 FUN_0068b4b0(void)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  undefined4 uVar6;
  undefined4 local_28;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 1;
  iVar3 = FUN_006fdaf0(&local_8);
  if (iVar3 == 0) {
    return 0xfffff82f;
  }
  piVar4 = (int *)FUN_0051c0a0(local_8);
  if ((piVar4 == (int *)0x0) || (iVar3 = (**(code **)(*piVar4 + 0x30))(), iVar3 == 0)) {
    if (piVar4 != (int *)0x0) {
      local_c = (**(code **)(*piVar4 + 0x94))();
    }
    goto LAB_0068b5c9;
  }
  piVar5 = (int *)(**(code **)(*piVar4 + 0x30))();
  iVar3 = (**(code **)(*piVar5 + 0x94))();
  if (iVar3 == 0) {
    (**(code **)(*piVar4 + 0x30))();
    piVar4 = (int *)(**(code **)(*piVar4 + 0x30))();
    if (piVar4[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*piVar4 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    if (bVar1) goto LAB_0068b5a1;
    local_28 = 0;
  }
  else {
LAB_0068b5a1:
    local_28 = 1;
  }
  local_c = local_28;
LAB_0068b5c9:
  iVar3 = FUN_006fd9c0(local_c);
  if (iVar3 == 0) {
    uVar6 = 0xfffff830;
  }
  else {
    uVar6 = 0;
  }
  return uVar6;
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

## Card B15-2-25

GOG function: Global::ExecuteCommandGetFactionBestAC  gog 0x00792380  callconv=__thiscall  param_size_bytes=8  gog_size=263  gog_retn=8
Candidate Steam address: 0x0067cda0 (size 263, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067d050:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007922a0 Global::ExecuteCommandGetFactionAverageXP -> 0x0067ccc0  gate_class=REVIEW
- GOG-order neighbour after:  0x00792490 Global::ExecuteCommandGetFactionEqual -> 0x0067ceb0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [193 GetFactionBestAC] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5bf0 CSWSFaction::GetBestAC -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067cda0: 16-byte aligned: YES
  0067cda0  55             PUSH EBP
  0067cda1  8B EC          MOV EBP,ESP
  0067cda3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067cda0
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D46D0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067cda0 FUN_0067cda0 size=263 ret=8 callers=0 callees=6

undefined4 FUN_0067cda0(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_14 = 0x7f000000;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_10;
        uVar3 = local_8;
        FUN_00664490(local_c);
        local_14 = FUN_006d46d0(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_14);
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

### decoy: 0x0067d050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d050: 16-byte aligned: YES
  0067d050  55             PUSH EBP
  0067d051  8B EC          MOV EBP,ESP
  0067d053  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d050
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3DB0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-26

GOG function: Global::ExecuteCommandEffectPsychicStatic  gog 0x0078d9c0  callconv=__thiscall  param_size_bytes=8  gog_size=240  gog_retn=8
Candidate Steam address: 0x006783e0 (size 240, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006782f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d8d0 Global::ExecuteCommandEffectBodyFuel -> 0x006782f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078dab0 Global::ExecuteCommandEffectLightsaberThrow -> 0x006784d0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [676 EffectPsychicStatic] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x24*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006783e0: 16-byte aligned: YES
  006783e0  55             PUSH EBP
  006783e1  8B EC          MOV EBP,ESP
  006783e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006783e0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006783e0 FUN_006783e0 size=240 ret=8 callers=0 callees=5

undefined4 FUN_006783e0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_30;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a17b;
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
  local_28 = (undefined2)DAT_009960a4;
  *(undefined2 *)(local_30 + 8) = local_28;
  iVar1 = FUN_006fdad0(0,local_30);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if (local_30 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_30);
    }
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x006782f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006782f0: 16-byte aligned: YES
  006782f0  55             PUSH EBP
  006782f1  8B EC          MOV EBP,ESP
  006782f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006782f0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-2-27

GOG function: Global::ExecuteCommandGetFactionWeakestMember  gog 0x00792c60  callconv=__thiscall  param_size_bytes=8  gog_size=263  gog_retn=8
Candidate Steam address: 0x0067d680 (size 263, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067d570:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792b50 Global::ExecuteCommandGetFactionStrongestMember -> 0x0067d570  gate_class=REVIEW
- GOG-order neighbour after:  0x00792d70 Global::ExecuteCommandGetFactionWorstAC -> 0x0067d790  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [181 GetFactionWeakestMember] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e4e40 CSWSFaction::GetWeakestMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d680: 16-byte aligned: YES
  0067d680  55             PUSH EBP
  0067d681  8B EC          MOV EBP,ESP
  0067d683  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d680
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3920  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d680 FUN_0067d680 size=263 ret=8 callers=0 callees=6

undefined4 FUN_0067d680(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_14 = 0x7f000000;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_10;
        uVar3 = local_8;
        FUN_00664490(local_c);
        local_14 = FUN_006d3920(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_14);
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

### decoy: 0x0067d570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d570: 16-byte aligned: YES
  0067d570  55             PUSH EBP
  0067d571  8B EC          MOV EBP,ESP
  0067d573  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d570
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3A80  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-28

GOG function: Global::ExecuteCommandGetIsDawn  gog 0x00794f20  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067f940 (size 106, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067f9b0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00794ea0 Global::ExecuteCommandGetInventoryDisturbType -> 0x0067f8c0  gate_class=AUTO
- GOG-order neighbour after:  0x00794f90 Global::ExecuteCommandGetIsDay -> 0x0067f9b0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [407 GetIsDawn] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x1dc*} | Steam {0x8*,0x1dc*}
imm sets (* = in both, 4 common): GOG {0x3*,0x8*,0x14*,0xfffff830*} | Steam {0x3*,0x8*,0x14*,0xfffff830*}

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

## Card B15-2-29

GOG function: Global::ExecuteCommandGetFactionLeastDamagedMember  gog 0x00792630  callconv=__thiscall  param_size_bytes=8  gog_size=263  gog_retn=8
Candidate Steam address: 0x0067d050 (size 263, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067cda0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792550 Global::ExecuteCommandGetFactionGold -> 0x0067cf70  gate_class=REVIEW
- GOG-order neighbour after:  0x00792740 Global::ExecuteCommandGetFactionLeader -> 0x0067d160  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [184 GetFactionLeastDamagedMember] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x00640d30 CVirtualMachine::StackPushObject -> Steam 0x006fdb10  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e52d0 CSWSFaction::GetLeastDamagedMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 6 common): GOG {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d050: 16-byte aligned: YES
  0067d050  55             PUSH EBP
  0067d051  8B EC          MOV EBP,ESP
  0067d053  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d050
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3DB0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d050 FUN_0067d050 size=263 ret=8 callers=0 callees=6

undefined4 FUN_0067d050(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0x7f000000;
  local_14 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_14);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = 1;
    if ((param_2 == 2) && (iVar1 = FUN_006fd9a0(&local_8), iVar1 == 0)) {
      return 0xfffff82f;
    }
    iVar1 = FUN_0051c100(local_14);
    if (iVar1 != 0) {
      local_10 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        uVar2 = local_14;
        uVar3 = local_8;
        FUN_00664490(local_10);
        local_c = FUN_006d3db0(uVar2,uVar3);
      }
    }
    iVar1 = FUN_006fdb10(local_c);
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

### decoy: 0x0067cda0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067cda0: 16-byte aligned: YES
  0067cda0  55             PUSH EBP
  0067cda1  8B EC          MOV EBP,ESP
  0067cda3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067cda0
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D46D0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-30

GOG function: Global::ExecuteCommandGetFactionAverageXP  gog 0x007922a0  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067ccc0 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067caf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007921b0 Global::ExecuteCommandGetFactionAverageReputation -> 0x0067cbd0  gate_class=REVIEW
- GOG-order neighbour after:  0x00792380 Global::ExecuteCommandGetFactionBestAC -> 0x0067cda0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [190 GetFactionAverageXP] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e57e0 CSWSFaction::GetAverageXP -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067ccc0: 16-byte aligned: YES
  0067ccc0  55             PUSH EBP
  0067ccc1  8B EC          MOV EBP,ESP
  0067ccc3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ccc0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D42C0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ccc0 FUN_0067ccc0 size=212 ret=8 callers=0 callees=5

undefined4 FUN_0067ccc0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        FUN_00664490(local_c);
        local_8 = FUN_006d42c0(uVar2);
      }
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

### decoy: 0x0067caf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067caf0: 16-byte aligned: YES
  0067caf0  55             PUSH EBP
  0067caf1  8B EC          MOV EBP,ESP
  0067caf3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067caf0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D4220  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B15-2-31

GOG function: Global::ExecuteCommandEffectHorrified  gog 0x0078d370  callconv=__thiscall  param_size_bytes=8  gog_size=252  gog_retn=8
Candidate Steam address: 0x00677d90 (size 252, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00677a50:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d250 Global::ExecuteCommandEffectBlasterDeflectionDecrease -> 0x00677c70  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d470 Global::ExecuteCommandEffectDamageForcePoints -> 0x00677e90  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [471 EffectHorrified] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x24*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00677d90: 16-byte aligned: YES
  00677d90  55             PUSH EBP
  00677d91  8B EC          MOV EBP,ESP
  00677d93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677d90
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00677d90 FUN_00677d90 size=252 ret=8 callers=0 callees=6

undefined4 FUN_00677d90(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_30;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a17b;
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
  local_28 = (undefined2)DAT_00995f3c;
  *(undefined2 *)(local_30 + 8) = local_28;
  FUN_005e4e80(0,8);
  iVar1 = FUN_006fdad0(0,local_30);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if (local_30 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_30);
    }
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00677a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677a50: 16-byte aligned: YES
  00677a50  55             PUSH EBP
  00677a51  8B EC          MOV EBP,ESP
  00677a53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677a50
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-2-32

GOG function: Global::ExecuteCommandGetFactionGold  gog 0x00792550  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067cf70 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ccc0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792490 Global::ExecuteCommandGetFactionEqual -> 0x0067ceb0  gate_class=AUTO
- GOG-order neighbour after:  0x00792630 Global::ExecuteCommandGetFactionLeastDamagedMember -> 0x0067d050  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [185 GetFactionGold] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e54a0 CSWSFaction::GetGold -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067cf70: 16-byte aligned: YES
  0067cf70  55             PUSH EBP
  0067cf71  8B EC          MOV EBP,ESP
  0067cf73  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067cf70
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3F80  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067cf70 FUN_0067cf70 size=212 ret=8 callers=0 callees=5

undefined4 FUN_0067cf70(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0xffffffff;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        FUN_00664490(local_8);
        local_c = FUN_006d3f80(uVar2);
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

### decoy: 0x0067ccc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ccc0: 16-byte aligned: YES
  0067ccc0  55             PUSH EBP
  0067ccc1  8B EC          MOV EBP,ESP
  0067ccc3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067ccc0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D42C0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B15-2-33

GOG function: Global::ExecuteCommandGetCommandable  gog 0x00790200  callconv=__thiscall  param_size_bytes=8  gog_size=155  gog_retn=8
Candidate Steam address: 0x0067ac20 (size 155, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067adc0:STR0/CALLEE0/SIG0.467/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078ff50 Global::ExecuteCommandGetClassInformation -> 0x0067a970  gate_class=AUTO
- GOG-order neighbour after:  0x007902a0 Global::ExecuteCommandGetCreatureHasTalent -> 0x0067acc0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [163 GetCommandable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xec*} | Steam {0x8*,0xec*}
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
prologue for 0x0067ac20: 16-byte aligned: YES
  0067ac20  55             PUSH EBP
  0067ac21  8B EC          MOV EBP,ESP
  0067ac23  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ac20
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ac20 FUN_0067ac20 size=155 ret=8 callers=0 callees=4

undefined4 __thiscall FUN_0067ac20(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  int local_14;
  undefined4 local_10;
  char local_9;
  undefined4 local_8;
  
  local_10 = 0;
  local_8 = *(undefined4 *)(param_1 + 8);
  if ((0 < param_3) && (iVar1 = FUN_006fdaf0(&local_8), iVar1 == 0)) {
    return 0xfffff82f;
  }
  piVar3 = &local_14;
  uVar2 = local_8;
  FUN_0051c080(local_8,piVar3,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  local_9 = FUN_0053dfb0(uVar2,piVar3);
  if (local_9 == DAT_0099447f) {
    local_10 = *(undefined4 *)(local_14 + 0xec);
  }
  iVar1 = FUN_006fd9c0(local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0067adc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067adc0: 16-byte aligned: YES
  0067adc0  55             PUSH EBP
  0067adc1  8B EC          MOV EBP,ESP
  0067adc3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067adc0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x53C [dec 1340]  x1  via ECX
```

## Card B15-2-34

GOG function: Global::ExecuteCommandGetGoldPieceValue  gog 0x00793530  callconv=__thiscall  param_size_bytes=8  gog_size=165  gog_retn=8
Candidate Steam address: 0x0067df50 (size 165, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067e100:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00793430 Global::ExecuteCommandGetGold -> 0x0067de50  gate_class=REVIEW
- GOG-order neighbour after:  0x007935e0 Global::ExecuteCommandGetChemical -> 0x0067e000  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [311 GetGoldPieceValue, 772 GetItemComponentPieceValue] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x0066a2b0 CSWSItem::GetCost -> Steam 0x00606a80  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x38*} | Steam {0x8*,0x38*}
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

### decoy: 0x0067e100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067e100: 16-byte aligned: YES
  0067e100  55             PUSH EBP
  0067e101  8B EC          MOV EBP,ESP
  0067e103  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067e100
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

## Card B15-2-35

GOG function: Global::ExecuteCommandEffectHealForcePoints  gog 0x0078d590  callconv=__thiscall  param_size_bytes=8  gog_size=282  gog_retn=8
Candidate Steam address: 0x00677fb0 (size 282, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677e90:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d470 Global::ExecuteCommandEffectDamageForcePoints -> 0x00677e90  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d6b0 Global::ExecuteCommandEffectTemporaryFP -> 0x006780d0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [373 EffectHealForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00677fb0: 16-byte aligned: YES
  00677fb0  55             PUSH EBP
  00677fb1  8B EC          MOV EBP,ESP
  00677fb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677fb0
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00677fb0 FUN_00677fb0 size=282 ret=8 callers=0 callees=7

undefined4 FUN_00677fb0(void)

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
    local_2c = (undefined2)DAT_00996098;
    *(undefined2 *)(local_34 + 8) = local_2c;
    FUN_005e4e80(0,local_14);
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

### decoy: 0x00677e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677e90: 16-byte aligned: YES
  00677e90  55             PUSH EBP
  00677e91  8B EC          MOV EBP,ESP
  00677e93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677e90
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-36

GOG function: Global::ExecuteCommandIntToString  gog 0x007a0a10  callconv=__thiscall  param_size_bytes=8  gog_size=229  gog_retn=8
Candidate Steam address: 0x0068b2e0 (size 210, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandIntToHexString | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0068b3c0:STR0/CALLEE1/SIG0.5/SRV0/VT0
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
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0x24,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc,0x1c,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0068b2e0: 16-byte aligned: YES
  0068b2e0  55             PUSH EBP
  0068b2e1  8B EC          MOV EBP,ESP
  0068b2e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0068b2e0
string literals (1):
  0x00998CD8  at 0068b349  '0x%08x'
direct callees (5):
  0x00733540  CExoString::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733780  CExoString::~CExoString
  0x00734270  CExoString::Format
  0x006FDA90  CVirtualMachine::StackPushString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0068b2e0 FUN_0068b2e0 size=210 ret=8 callers=0 callees=5

undefined4 __fastcall FUN_0068b2e0(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00968348;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  iVar1 = FUN_006fd9a0(&local_14);
  if (iVar1 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_00734270(local_1c,"0x%08x",local_14);
    iVar1 = FUN_006fda90(local_1c);
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

## Card B15-2-37

GOG function: Global::ExecuteCommandGetChemicalPieceValue  gog 0x007936e0  callconv=__thiscall  param_size_bytes=8  gog_size=174  gog_retn=8
Candidate Steam address: 0x0067e100 (size 174, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067df50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007935e0 Global::ExecuteCommandGetChemical -> 0x0067e000  gate_class=REVIEW
- GOG-order neighbour after:  0x00793790 Global::ExecuteCommandGetHasFeat -> 0x0067e1b0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [775 GetChemicalPieceValue] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x0066a2b0 CSWSItem::GetCost -> Steam 0x00606a80  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x38*} | Steam {0x8*,0x38*}
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
prologue for 0x0067e100: 16-byte aligned: YES
  0067e100  55             PUSH EBP
  0067e101  8B EC          MOV EBP,ESP
  0067e103  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067e100
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
// FN 0067e100 FUN_0067e100 size=174 ret=8 callers=0 callees=5

undefined4 FUN_0067e100(void)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int **ppiVar4;
  undefined4 extraout_var;
  undefined4 local_10;
  int *local_c;
  uint local_8;
  
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
        local_8 = local_8 / 0x14;
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

### decoy: 0x0067df50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067df50: 16-byte aligned: YES
  0067df50  55             PUSH EBP
  0067df51  8B EC          MOV EBP,ESP
  0067df53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B15-2-38

GOG function: Global::ExecuteCommandEffectAbilityDecrease  gog 0x00786c40  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x006710d0 (size 1417, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandDoTouchAttack | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671660:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007866b0 Global::ExecuteCommandDoTouchAttack -> 0x006710d0  gate_class=AUTO
- GOG-order neighbour after:  0x00786e00 Global::ExecuteCommandEffectAbilityIncrease -> 0x00671820  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [446 EffectAbilityDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: no
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: no
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: no
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: no
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0xa} | Steam {0x4,0x8*,0x10,0x30,0x3c,0x5c,0xabc,0x10dc,0x1198}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x38,0x8c,0xfffff82f*,0xfffff830*,0xffffffe7} | Steam {0x2,0x3,0x4*,0x8*,0xd,0x14,0x34,0x92,0x93,0xac,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006710d0: 16-byte aligned: YES
  006710d0  55             PUSH EBP
  006710d1  8B EC          MOV EBP,ESP
  006710d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006710d0
string literals (0):
direct callees (20):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A7DB0  CSWSCreatureStats::GetMeleeAttackBonus
  0x006A8670  (unnamed)
  0x006A5110  CSWSCreatureStats::GetArmorClassVersus
  0x006A2000  CSWRules::RollDice(n,sides)
  0x006E1C60  CSWSCreature::ResolveDefensiveEffects
  0x005908C0  CSWSCombatRound::GetAttack
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
struct offsets touched (top 9):
  +0x8 [dec 8]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x9  via EDX
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x1198 [dec 4504]  x3  via ECX
  +0x10DC [dec 4316]  x2  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x3C [dec 60]  x1  via EAX
  +0x5C [dec 92]  x1  via ECX
  +0xABC [dec 2748]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006710d0 FUN_006710d0 size=1417 ret=8 callers=0 callees=20

undefined4 __thiscall FUN_006710d0(int param_1,int param_2,int param_3)

{
  char cVar1;
  short sVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_b8;
  int local_44;
  char local_3d;
  int *local_3c;
  uint local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  int *local_24;
  undefined1 local_1d;
  uint local_1c;
  int local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959efb;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_2c = 0;
  local_44 = 1;
  local_3d = '\0';
  local_1d = 0;
  iVar4 = FUN_006fdaf0(&local_30);
  if (iVar4 == 0) {
    uVar5 = 0xfffff82f;
  }
  else if ((param_3 < 2) || (iVar4 = FUN_006fd9a0(&local_44), iVar4 != 0)) {
    local_34 = FUN_0051c080(uVar3);
    cVar1 = FUN_0053dfb0(local_30,&local_3c);
    if ((cVar1 == DAT_0099447f) &&
       (cVar1 = FUN_0053dfb0(*(undefined4 *)(param_1 + 8),&local_24), cVar1 == DAT_0099447f)) {
      iVar4 = (**(code **)(*local_24 + 0x30))();
      if (iVar4 != 0) {
        iVar4 = (**(code **)(*local_3c + 0x30))();
        if (iVar4 == 0) {
          if (((char)local_3c[2 = byte +0x8] == DAT_00994479) || ((char)local_3c[2 = byte +0x8] == DAT_00994478)) {
            local_2c = 1;
          }
        }
        else {
          if (param_2 == 0x92) {
            (**(code **)(*local_24 + 0x30))();
            local_18 = FUN_006a7db0(0,1,1,0);
          }
          else {
            (**(code **)(*local_24 + 0x30))();
            local_18 = FUN_006a8670(0,1,1);
          }
          (**(code **)(*local_3c + 0x30))();
          uVar6 = 1;
          uVar5 = (**(code **)(*local_24 + 0x30))(1);
          sVar2 = FUN_006a5110(uVar5,uVar6);
          local_28 = (int)sVar2;
          local_1c = FUN_006a2000(1,0x14);
          local_1c = local_1c & 0xffff;
          uVar6 = 1;
          uVar5 = (**(code **)(*local_3c + 0x10))(1);
          (**(code **)(*local_24 + 0x30))(uVar5);
          iVar4 = FUN_006e1c60(uVar5,uVar6);
          if (iVar4 == 0) {
            if (local_1c == 1) {
              local_2c = 0;
            }
            else if (((int)(local_1c + local_18) < local_28) && (local_1c != 0x14)) {
              local_2c = 0;
/* ... 91 more lines truncated by --max-lines */
```

### decoy: 0x00671660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00671660: 16-byte aligned: YES
  00671660  55             PUSH EBP
  00671661  8B EC          MOV EBP,ESP
  00671663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-2-39

GOG function: Global::ExecuteCommandEffectForceResistanceDecrease  gog 0x0078c1c0  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676d20 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectForceResistanceIncrease | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676be0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078bfe0 Global::ExecuteCommandEffectSpellLevelAbsorption -> 0x00676a00  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c300 Global::ExecuteCommandEffectForceResistanceIncrease -> 0x00676d20  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [454 EffectForceResistanceDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x00676d20: 16-byte aligned: YES
  00676d20  55             PUSH EBP
  00676d21  8B EC          MOV EBP,ESP
  00676d23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00676d20 FUN_00676d20 size=317 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00676d20(int param_1)

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
    local_2c = (undefined2)DAT_00995fa0;
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

### decoy: 0x00676be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676be0: 16-byte aligned: YES
  00676be0  55             PUSH EBP
  00676be1  8B EC          MOV EBP,ESP
  00676be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-2-40

GOG function: Global::ExecuteCommandEventSpellCastAt  gog 0x0078e220  callconv=__thiscall  param_size_bytes=8  gog_size=620  gog_retn=8
Candidate Steam address: 0x00678c40 (size 620, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00678eb0:STR0/CALLEE0/SIG0.278/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e130 Global::ExecuteCommandEventConversation -> 0x00678b50  gate_class=REVIEW
- GOG-order neighbour after:  0x0078e490 Global::ExecuteCommandEventUserDefined -> 0x00678eb0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [244 EventSpellCastAt] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0077a9b0 CSWCCMessageData::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0077aa40 CSWCCMessageData::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0x34*,0x5c*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x4*,0x8*,0x34*,0x5c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00678c40: 16-byte aligned: YES
  00678c40  55             PUSH EBP
  00678c41  8B EC          MOV EBP,ESP
  00678c43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00678c40
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E1BD0  (unnamed)
  0x005E1C70  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00678c40 FUN_00678c40 size=620 ret=8 callers=0 callees=9

undefined4 FUN_00678c40(undefined4 param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *local_58;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a1db;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x34,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_58 = (undefined2 *)0x0;
  }
  else {
    local_58 = (undefined2 *)FUN_005e1a60();
  }
  local_8 = 0xffffffff;
  local_14 = 1;
  iVar1 = FUN_006fdaf0(&local_1c);
  if (iVar1 == 0) {
    if (local_58 != (undefined2 *)0x0) {
      FUN_005e1af0();
      FUN_009196fd(local_58);
    }
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_18);
    if (iVar1 == 0) {
      if (local_58 != (undefined2 *)0x0) {
        FUN_005e1af0();
        FUN_009196fd(local_58);
      }
      uVar2 = 0xfffff82f;
    }
    else if ((param_2 < 3) || (iVar1 = FUN_006fd9a0(&local_14), iVar1 != 0)) {
      *local_58 = 2;
      FUN_005e1bd0(0,local_18);
      FUN_005e1bd0(1,local_14);
      FUN_005e1c70(0,local_1c);
      iVar1 = FUN_006fdad0(1,local_58);
      if (iVar1 == 0) {
        if (local_58 != (undefined2 *)0x0) {
          FUN_005e1af0();
          FUN_009196fd(local_58);
        }
        uVar2 = 0xfffff830;
      }
      else {
        if (local_58 != (undefined2 *)0x0) {
          FUN_005e1af0();
          FUN_009196fd(local_58);
        }
        uVar2 = 0;
      }
    }
    else {
      if (local_58 != (undefined2 *)0x0) {
        FUN_005e1af0();
        FUN_009196fd(local_58);
      }
      uVar2 = 0xfffff82f;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00678eb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00678eb0: 16-byte aligned: YES
  00678eb0  55             PUSH EBP
  00678eb1  8B EC          MOV EBP,ESP
  00678eb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00678eb0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
  0x005E1BD0  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 0):
```

## Card B15-2-41

GOG function: Global::ExecuteCommandGetFactionMostFrequentClass  gog 0x00792a70  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067d490 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067d380:STR0/CALLEE0/SIG0.786/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00792960 Global::ExecuteCommandGetFactionMostDamagedMember -> 0x0067d380  gate_class=REVIEW
- GOG-order neighbour after:  0x00792b50 Global::ExecuteCommandGetFactionStrongestMember -> 0x0067d570  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [191 GetFactionMostFrequentClass] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e5880 CSWSFaction::GetMostFrequentClass -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067d490: 16-byte aligned: YES
  0067d490  55             PUSH EBP
  0067d491  8B EC          MOV EBP,ESP
  0067d493  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067d490
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D4360  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067d490 FUN_0067d490 size=212 ret=8 callers=0 callees=5

undefined4 FUN_0067d490(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_c = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        FUN_00664490(local_c);
        local_8 = FUN_006d4360(uVar2);
      }
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

### decoy: 0x0067d380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067d380: 16-byte aligned: YES
  0067d380  55             PUSH EBP
  0067d381  8B EC          MOV EBP,ESP
  0067d383  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067d380
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D3BE0  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B15-2-42

GOG function: Global::ExecuteCommandEffectTemporaryFP  gog 0x0078d6b0  callconv=__thiscall  param_size_bytes=8  gog_size=282  gog_retn=8
Candidate Steam address: 0x006780d0 (size 282, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677fb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d590 Global::ExecuteCommandEffectHealForcePoints -> 0x00677fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d7d0 Global::ExecuteCommandEffectChoke -> 0x006781f0  gate_class=REVIEW
- GOG-side fact: the GOG command table stores this GOG function at slot 156 (EffectTemporaryForcePoints) and then overwrites that slot with another handler (dead code)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006780d0: 16-byte aligned: YES
  006780d0  55             PUSH EBP
  006780d1  8B EC          MOV EBP,ESP
  006780d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006780d0
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006780d0 FUN_006780d0 size=282 ret=8 callers=0 callees=7

undefined4 FUN_006780d0(void)

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
    local_2c = (undefined2)DAT_00996084;
    *(undefined2 *)(local_34 + 8) = local_2c;
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

### decoy: 0x00677fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677fb0: 16-byte aligned: YES
  00677fb0  55             PUSH EBP
  00677fb1  8B EC          MOV EBP,ESP
  00677fb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677fb0
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-43

GOG function: Global::ExecuteCommandEffectBlasterDeflectionDecrease  gog 0x0078d250  callconv=__thiscall  param_size_bytes=8  gog_size=282  gog_retn=8
Candidate Steam address: 0x00677c70 (size 282, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677b50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d130 Global::ExecuteCommandEffectBlasterDeflectionIncrease -> 0x00677b50  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d370 Global::ExecuteCommandEffectHorrified -> 0x00677d90  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [470 EffectBlasterDeflectionDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00677c70: 16-byte aligned: YES
  00677c70  55             PUSH EBP
  00677c71  8B EC          MOV EBP,ESP
  00677c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677c70
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00677c70 FUN_00677c70 size=282 ret=8 callers=0 callees=7

undefined4 FUN_00677c70(void)

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
    local_2c = (undefined2)DAT_0099608c;
    *(undefined2 *)(local_34 + 8) = local_2c;
    FUN_005e4e80(0,local_14);
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

### decoy: 0x00677b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677b50: 16-byte aligned: YES
  00677b50  55             PUSH EBP
  00677b51  8B EC          MOV EBP,ESP
  00677b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677b50
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-44

GOG function: Global::ExecuteCommandGetDistanceToObject  gog 0x00791000  callconv=__thiscall  param_size_bytes=8  gog_size=416  gog_retn=8
Candidate Steam address: 0x0067ba20 (size 416, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067bbc0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00790db0 Global::ExecuteCommandGetDistanceBetweenLocations2D -> 0x0067b7d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007911a0 Global::ExecuteCommandGetDistanceToObject2D -> 0x0067bbc0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [41 GetDistanceToObject] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x60*,0x94*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0xc*,0x60*,0x94*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067ba20: 16-byte aligned: YES
  0067ba20  55             PUSH EBP
  0067ba21  8B EC          MOV EBP,ESP
  0067ba23  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ba20
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ba20 FUN_0067ba20 size=416 ret=8 callers=0 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0067ba20(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  int *piVar4;
  undefined1 local_48 [12];
  int local_3c;
  char local_36;
  char local_35;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  float local_18;
  int local_14;
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_18 = _DAT_00987474;
  local_34 = 0;
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  local_c = 0;
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (*(int *)(param_1 + 4) == 1) {
      piVar4 = &local_14;
      uVar2 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar2,piVar4);
      local_35 = FUN_0053dfb0(uVar2,piVar4);
      if (local_35 == DAT_0099447f) {
        local_28 = *(undefined4 *)(local_14 + 0x94);
        local_24 = *(undefined4 *)(local_14 + 0x98);
        local_20 = *(undefined4 *)(local_14 + 0x9c);
        local_10 = FUN_005453c0();
        local_1c = 1;
      }
    }
    if (local_1c == 1) {
      piVar4 = &local_3c;
      FUN_0051c080(local_8,piVar4);
      local_36 = FUN_0053dfb0(local_8,piVar4);
      if (((local_36 == DAT_0099447f) && (local_3c != 0)) &&
         (DAT_00994473 < *(byte *)(local_3c + 8))) {
        local_34 = *(undefined4 *)(local_3c + 0x94);
        local_30 = *(undefined4 *)(local_3c + 0x98);
        local_2c = *(undefined4 *)(local_3c + 0x9c);
        iVar1 = FUN_005453c0();
        if (iVar1 == local_10) {
          uVar2 = FUN_004452b0(local_48,&local_34,&local_28);
          fVar3 = (float10)FUN_005142f0(uVar2);
          local_18 = (float)fVar3;
        }
      }
    }
    iVar1 = FUN_006fda00(local_18);
    if (iVar1 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x0067bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067bbc0: 16-byte aligned: YES
  0067bbc0  55             PUSH EBP
  0067bbc1  8B EC          MOV EBP,ESP
  0067bbc3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067bbc0
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 2):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
```

## Card B15-2-45

GOG function: Global::ExecuteCommandGetFactionAverageGoodEvilAlignment  gog 0x00791ff0  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067ca10 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067caf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00791e90 Global::ExecuteCommandGetFacing -> 0x0067c8b0  gate_class=AUTO
- GOG-order neighbour after:  0x007920d0 Global::ExecuteCommandGetFactionAverageLevel -> 0x0067caf0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [187 GetFactionAverageGoodEvilAlignment] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x007e55c0 CSWSFaction::GetAverageGoodEvilAlignment -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x2c*,0x7f000000*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067ca10: 16-byte aligned: YES
  0067ca10  55             PUSH EBP
  0067ca11  8B EC          MOV EBP,ESP
  0067ca13  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ca10
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D40A0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ca10 FUN_0067ca10 size=212 ret=8 callers=0 callees=5

undefined4 FUN_0067ca10(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0xffffffff;
  local_10 = 0x7f000000;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 != 0) {
      local_8 = *(undefined4 *)(*(int *)(iVar1 + 0x1198) + 0x78);
      iVar1 = *(int *)(*(int *)(DAT_00a1b4a4 + 8) + 4);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x10054) != 0)) {
        FUN_00664490(local_8);
        local_c = FUN_006d40a0(uVar2);
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

### decoy: 0x0067caf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067caf0: 16-byte aligned: YES
  0067caf0  55             PUSH EBP
  0067caf1  8B EC          MOV EBP,ESP
  0067caf3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067caf0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664490  (unnamed)
  0x006D4220  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x78 [dec 120]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B15-2-46

GOG function: Global::ExecuteCommandGetEffectDurationType  gog 0x00791790  callconv=__thiscall  param_size_bytes=8  gog_size=156  gog_retn=8
Candidate Steam address: 0x0067c1b0 (size 156, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067c2f0:STR0/CALLEE1/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007916e0 Global::ExecuteCommandGetEffectCreator -> 0x0067c100  gate_class=AUTO
- GOG-order neighbour after:  0x00791830 Global::ExecuteCommandGetEffectSpellId -> 0x0067c250  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [89 GetEffectDurationType] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xa*} | Steam {0xa*}
imm sets (* = in both, 6 common): GOG {0x4*,0x7*,0x8*,0x18*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x7*,0x8*,0x18*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067c1b0: 16-byte aligned: YES
  0067c1b0  55             PUSH EBP
  0067c1b1  8B EC          MOV EBP,ESP
  0067c1b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067c1b0
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xA [dec 10]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067c1b0 FUN_0067c1b0 size=156 ret=8 callers=0 callees=4

undefined4 FUN_0067c1b0(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  iVar1 = FUN_006fdab0(0,&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar3 = FUN_006fd9c0(*(ushort *)(local_8 + 0xa [dec 10]) & 7);
    iVar1 = local_8;
    if (iVar3 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      if (local_8 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0067c2f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067c2f0: 16-byte aligned: YES
  0067c2f0  55             PUSH EBP
  0067c2f1  8B EC          MOV EBP,ESP
  0067c2f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067c2f0
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xA [dec 10]  x1  via ECX
```

## Card B15-2-47

GOG function: Global::ExecuteCommandGetDistanceBetweenLocations  gog 0x00790ba0  callconv=__thiscall  param_size_bytes=8  gog_size=527  gog_retn=8
Candidate Steam address: 0x0067b5c0 (size 527, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0067b7d0:STR0/CALLEE2/SIG0.375/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x8*,0xc*,0x64*,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x4*,0x8*,0xc*,0x64*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067b5c0: 16-byte aligned: YES
  0067b5c0  55             PUSH EBP
  0067b5c1  8B EC          MOV EBP,ESP
  0067b5c3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0067b5c0 FUN_0067b5c0 size=527 ret=8 callers=0 callees=6

undefined4 FUN_0067b5c0(void)

{
  int iVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_24 [12];
  int local_18;
  int local_14;
  float local_10;
  int local_c;
  int local_8;
  
  iVar1 = FUN_006fdab0(2,&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fdab0(2,&local_8);
    if (iVar1 == 0) {
      if (local_c != 0) {
        local_18 = local_c;
        local_14 = local_c;
        if (local_c != 0) {
          FUN_00623320();
          FUN_009196fd(local_14);
        }
      }
      uVar2 = 0xfffff82f;
    }
    else {
      uVar2 = FUN_004452b0(local_24,local_c,local_8);
      fVar3 = (float10)FUN_005142f0(uVar2);
      local_10 = (float)fVar3;
      iVar1 = FUN_006fda00(local_10);
      if (iVar1 == 0) {
        if (local_c != 0) {
          if (local_c != 0) {
            FUN_00623320();
            FUN_009196fd(local_c);
          }
          local_c = 0;
        }
        if ((local_8 != 0) && (local_8 != 0)) {
          FUN_00623320();
          FUN_009196fd(local_8);
        }
        uVar2 = 0xfffff830;
      }
      else {
        if (local_c != 0) {
          if (local_c != 0) {
            FUN_00623320();
            FUN_009196fd(local_c);
          }
          local_c = 0;
        }
        if ((local_8 != 0) && (local_8 != 0)) {
          FUN_00623320();
          FUN_009196fd(local_8);
        }
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0067b7d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067b7d0: 16-byte aligned: YES
  0067b7d0  55             PUSH EBP
  0067b7d1  8B EC          MOV EBP,ESP
  0067b7d3  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B15-2-48

GOG function: CSWVirtualMachineCommands::ExecuteCommandFloatToInt  gog 0x0078ebd0  callconv=__thiscall  param_size_bytes=8  gog_size=80  gog_retn=8
Candidate Steam address: 0x006795f0 (size 80, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00679640:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078e8f0 Global::ExecuteCommandFloatingText -> 0x00679310  gate_class=AUTO
- GOG-order neighbour after:  0x0078ec20 Global::ExecuteCommandFloatToString -> 0x00679640  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [231 FloatToInt] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0xc*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006795f0: 16-byte aligned: YES
  006795f0  55             PUSH EBP
  006795f1  8B EC          MOV EBP,ESP
  006795f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006795f0
string literals (0):
direct callees (3):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x0091C860  _ftol2_sse
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006795f0 FUN_006795f0 size=80 ret=8 callers=0 callees=3

undefined4 FUN_006795f0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined1 local_8 [4];
  
  iVar1 = FUN_006fd9e0(local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = FUN_0091c860(extraout_var);
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

### decoy: 0x00679640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00679640: 16-byte aligned: YES
  00679640  55             PUSH EBP
  00679641  8B EC          MOV EBP,ESP
  00679643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00679640
string literals (2):
  0x00998CA0  at 00679710  '%%%2d.%02df\\n'
  0x00998C90  at 0067972b  '%%%1d.%02df\\n'
direct callees (8):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B15-2-49

GOG function: Global::ExecuteCommandGetEffectType  gog 0x00791970  callconv=__thiscall  param_size_bytes=8  gog_size=159  gog_retn=8
Candidate Steam address: 0x0067c390 (size 159, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: StackPopEngineStructure 0x6fdab0, GetEffectTypeScriptConstant 0x5e5f60)
- best decoy (neighbour/other candidate) scores: 0x0067c2f0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007918d0 Global::ExecuteCommandGetEffectSubType -> 0x0067c2f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00791a10 Global::ExecuteCommandGetEmotions -> 0x0067c250  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [170 GetEffectType] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00735c20 CGameEffect::GetEffectTypeScriptConstant -> Steam 0x005e5f60  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x18*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x18*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067c390: 16-byte aligned: YES
  0067c390  55             PUSH EBP
  0067c391  8B EC          MOV EBP,ESP
  0067c393  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067c390
string literals (0):
direct callees (5):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x005E5F60  CGameEffect::GetEffectTypeScriptConstant
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0067c390 FUN_0067c390 size=159 ret=8 callers=0 callees=5

undefined4 FUN_0067c390(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_c;
  undefined4 local_8;
  
  local_c = 0;
  iVar1 = FUN_006fdab0(0,&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = FUN_005e5f60();
    iVar3 = FUN_006fd9c0(local_8);
    iVar1 = local_c;
    if (iVar3 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      if (local_c != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0067c2f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067c2f0: 16-byte aligned: YES
  0067c2f0  55             PUSH EBP
  0067c2f1  8B EC          MOV EBP,ESP
  0067c2f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067c2f0
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xA [dec 10]  x1  via ECX
```

## Card B15-2-50

GOG function: Global::ExecuteCommandGetHasSpellEffect  gog 0x00793c50  callconv=__thiscall  param_size_bytes=8  gog_size=190  gog_retn=8
Candidate Steam address: 0x0067e670 (size 190, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: HasSpellEffectApplied 0x5476c0)
- best decoy (neighbour/other candidate) scores: 0x0067e2d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00793b70 Global::ExecuteCommandGetHasSpell -> 0x0067e590  gate_class=AUTO
- GOG-order neighbour after:  0x00793d10 Global::ExecuteCommandGetHitDice -> 0x0067e730  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [304 GetHasSpellEffect] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x006ae3a0 CSWSObject::HasSpellEffectApplied -> Steam 0x005476c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x10*} | Steam {0x8*,0x10*}
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
prologue for 0x0067e670: 16-byte aligned: YES
  0067e670  55             PUSH EBP
  0067e671  8B EC          MOV EBP,ESP
  0067e673  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067e670
string literals (0):
direct callees (6):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005476C0  CSWSObject::HasSpellEffectApplied
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0067e670 FUN_0067e670 size=190 ret=8 callers=0 callees=6

undefined4 FUN_0067e670(void)

{
  int iVar1;
  undefined4 uVar2;
  int **ppiVar3;
  undefined4 extraout_var;
  undefined4 local_18;
  undefined4 local_14;
  int *local_10;
  undefined4 local_c;
  char local_5;
  
  local_14 = 0;
  iVar1 = FUN_006fd9a0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = extraout_var;
    iVar1 = FUN_006fdaf0(&local_18);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      ppiVar3 = &local_10;
      FUN_0051c080(local_18,ppiVar3,uVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
      local_5 = FUN_0053dfb0(local_18,ppiVar3);
      if (local_5 == DAT_0099447f) {
        iVar1 = (**(code **)(*local_10 + 0x10))();
        if (iVar1 != 0) {
          local_14 = FUN_005476c0(local_c);
        }
      }
      iVar1 = FUN_006fd9c0(local_14);
      if (iVar1 == 0) {
        uVar2 = 0xfffff830;
      }
      else {
        uVar2 = 0;
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x0067e2d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067e2d0: 16-byte aligned: YES
  0067e2d0  55             PUSH EBP
  0067e2d1  8B EC          MOV EBP,ESP
  0067e2d3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067e2d0
string literals (0):
direct callees (6):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x00549ED0  CSWSObject::GetHasFeatEffectApplied
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x10 [dec 16]  x1  via EAX
```

## Card B15-2-51

GOG function: Global::ExecuteCommandEffectChoke  gog 0x0078d7d0  callconv=__thiscall  param_size_bytes=8  gog_size=252  gog_retn=8
Candidate Steam address: 0x006781f0 (size 252, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006782f0:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d6b0 Global::ExecuteCommandEffectTemporaryFP -> 0x006780d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d8d0 Global::ExecuteCommandEffectBodyFuel -> 0x006782f0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [159 EffectChoke] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x7*,0x8*,0x24*,0x8c*,0xfffff830*} | Steam {0x4*,0x7*,0x8*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006781f0: 16-byte aligned: YES
  006781f0  55             PUSH EBP
  006781f1  8B EC          MOV EBP,ESP
  006781f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006781f0
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006781f0 FUN_006781f0 size=252 ret=8 callers=0 callees=6

undefined4 FUN_006781f0(void)

{
  int iVar1;
  undefined4 uVar2;
  int local_30;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a17b;
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
  local_28 = (undefined2)DAT_00995f3c;
  *(undefined2 *)(local_30 + 8) = local_28;
  FUN_005e4e80(0,7);
  iVar1 = FUN_006fdad0(0,local_30);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if (local_30 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_30);
    }
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x006782f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006782f0: 16-byte aligned: YES
  006782f0  55             PUSH EBP
  006782f1  8B EC          MOV EBP,ESP
  006782f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006782f0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-2-52

GOG function: Global::ExecuteCommandEventConversation  gog 0x0078e130  callconv=__thiscall  param_size_bytes=8  gog_size=226  gog_retn=8
Candidate Steam address: 0x00678b50 (size 226, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00678c40:STR0/CALLEE0/SIG0.222/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078de70 Global::ExecuteCommandEventActivateItem -> 0x00678890  gate_class=AUTO
- GOG-order neighbour after:  0x0078e220 Global::ExecuteCommandEventSpellCastAt -> 0x00678c40  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [295 EventConversation] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0077a9b0 CSWCCMessageData::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0077aa40 CSWCCMessageData::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x4*,0x7*,0x8*,0x20*,0x34*,0xfffff830*} | Steam {0x4*,0x7*,0x8*,0x20*,0x34*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00678b50: 16-byte aligned: YES
  00678b50  55             PUSH EBP
  00678b51  8B EC          MOV EBP,ESP
  00678b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00678b50
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00678b50 FUN_00678b50 size=226 ret=8 callers=0 callees=5

undefined4 FUN_00678b50(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined2 *local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fa7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x34,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_2c = (undefined2 *)0x0;
  }
  else {
    local_2c = (undefined2 *)FUN_005e1a60();
  }
  local_8 = 0xffffffff;
  *local_2c = 7;
  iVar1 = FUN_006fdad0(1,local_2c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    if (local_2c != (undefined2 *)0x0) {
      FUN_005e1af0();
      FUN_009196fd(local_2c);
    }
    uVar2 = 0;
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00678c40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00678c40: 16-byte aligned: YES
  00678c40  55             PUSH EBP
  00678c41  8B EC          MOV EBP,ESP
  00678c43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00678c40
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x005E1AF0  (unnamed)
  0x009196FD  operator delete
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E1BD0  (unnamed)
  0x005E1C70  (unnamed)
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 0):
```

## Card B15-2-53

GOG function: Global::ExecuteCommandPauseGame  gog 0x007ac770  callconv=__thiscall  param_size_bytes=8  gog_size=71  gog_retn=8
Candidate Steam address: 0x00697090 (size 134, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandCancelCombat | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697120:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x8}
imm sets (* = in both, 2 common): GOG {0x8*,0xc,0xfffff82f*} | Steam {0x2,0x8*,0x14,0x7f000000,0xfffff82f*}

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

## Card B15-2-54

GOG function: Global::ExecuteCommandGetEffectSubType  gog 0x007918d0  callconv=__thiscall  param_size_bytes=8  gog_size=156  gog_retn=8
Candidate Steam address: 0x0067c2f0 (size 156, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0067c1b0:STR0/CALLEE1/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00791830 Global::ExecuteCommandGetEffectSpellId -> 0x0067c250  gate_class=AUTO
- GOG-order neighbour after:  0x00791970 Global::ExecuteCommandGetEffectType -> 0x0067c390  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [90 GetEffectSubType] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> Steam 0x006fdab0  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xa*} | Steam {0xa*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x18*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x18*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067c2f0: 16-byte aligned: YES
  0067c2f0  55             PUSH EBP
  0067c2f1  8B EC          MOV EBP,ESP
  0067c2f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067c2f0
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xA [dec 10]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067c2f0 FUN_0067c2f0 size=156 ret=8 callers=0 callees=4

undefined4 FUN_0067c2f0(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  iVar1 = FUN_006fdab0(0,&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar3 = FUN_006fd9c0(*(ushort *)(local_8 + 0xa [dec 10]) & 0x18);
    iVar1 = local_8;
    if (iVar3 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      if (local_8 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0067c1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067c1b0: 16-byte aligned: YES
  0067c1b0  55             PUSH EBP
  0067c1b1  8B EC          MOV EBP,ESP
  0067c1b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067c1b0
string literals (0):
direct callees (4):
  0x006FDAB0  CVirtualMachine::StackPopEngineStructure
  0x006FD9C0  CVirtualMachine::StackPushInteger
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0xA [dec 10]  x1  via ECX
```

## Card B15-2-55

GOG function: Global::ExecuteCommandGetStringRight  gog 0x0079e650  callconv=__thiscall  param_size_bytes=8  gog_size=354  gog_retn=8
Candidate Steam address: 0x00688ed0 (size 289, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetStringLowerCase | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00689000:STR0/CALLEE1/SIG0.5/SRV0/VT0
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
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640c90 CVirtualMachine::StackPopString -> Steam 0x006fda70  in candidate callees: yes
  0x00640cb0 CVirtualMachine::StackPushString -> Steam 0x006fda90  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0x38,0xfffff82f*,0xfffff830*} | Steam {0x2*,0x8*,0x30,0xfffff82f*,0xfffff830*}

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

### decoy: 0x00689000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00689000: 16-byte aligned: YES
  00689000  55             PUSH EBP
  00689001  8B EC          MOV EBP,ESP
  00689003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-2-56

GOG function: Global::ExecuteCommandEffectDamageForcePoints  gog 0x0078d470  callconv=__thiscall  param_size_bytes=8  gog_size=282  gog_retn=8
Candidate Steam address: 0x00677e90 (size 282, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677fb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d370 Global::ExecuteCommandEffectHorrified -> 0x00677d90  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d590 Global::ExecuteCommandEffectHealForcePoints -> 0x00677fb0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [372 EffectDamageForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x28*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00677e90: 16-byte aligned: YES
  00677e90  55             PUSH EBP
  00677e91  8B EC          MOV EBP,ESP
  00677e93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677e90
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00677e90 FUN_00677e90 size=282 ret=8 callers=0 callees=7

undefined4 FUN_00677e90(void)

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
    local_2c = (undefined2)DAT_00996094;
    *(undefined2 *)(local_34 + 8) = local_2c;
    FUN_005e4e80(0,local_14);
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

### decoy: 0x00677fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677fb0: 16-byte aligned: YES
  00677fb0  55             PUSH EBP
  00677fb1  8B EC          MOV EBP,ESP
  00677fb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677fb0
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-57

GOG function: Global::ExecuteCommandGetIdentified  gog 0x007940b0  callconv=__thiscall  param_size_bytes=8  gog_size=116  gog_retn=8
Candidate Steam address: 0x0067ead0 (size 116, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ea50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00794030 Global::ExecuteCommandGetMaxForcePoints -> 0x0067ea50  gate_class=REVIEW
- GOG-order neighbour after:  0x00794130 Global::ExecuteCommandGetInPersistentObject -> 0x0067eb50  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [332 GetIdentified] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
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
prologue for 0x0067ead0: 16-byte aligned: YES
  0067ead0  55             PUSH EBP
  0067ead1  8B EC          MOV EBP,ESP
  0067ead3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ead0
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ead0 FUN_0067ead0 size=116 ret=8 callers=0 callees=3

undefined4 FUN_0067ead0(void)

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
    local_8 = FUN_0051c0e0(local_c);
    iVar1 = FUN_006fd9c0(local_8 != 0);
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

### decoy: 0x0067ea50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ea50: 16-byte aligned: YES
  0067ea50  55             PUSH EBP
  0067ea51  8B EC          MOV EBP,ESP
  0067ea53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0067ea50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B15-2-58

GOG function: Global::ExecuteCommandGetChemical  gog 0x007935e0  callconv=__thiscall  param_size_bytes=8  gog_size=248  gog_retn=8
Candidate Steam address: 0x0067e000 (size 248, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067de50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00793530 Global::ExecuteCommandGetGoldPieceValue -> 0x0067df50  gate_class=REVIEW
- GOG-order neighbour after:  0x007936e0 Global::ExecuteCommandGetChemicalPieceValue -> 0x0067e100  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [774 GetChemicals] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00640be0 CVirtualMachine::StackPushInteger -> Steam 0x006fd9c0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bb80 CServerExoApp::GetPlaceableByGameObjectID -> Steam ?  in candidate callees: n/a
  0x006693b0 CItemRepository::FindItemWithBaseItemType -> Steam 0x00605b80  in candidate callees: yes
  0x0069a320 CSWSCreature::GetChemical -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0x2cc*,0x3c4*} | Steam {0x8*,0x2cc*,0x3c4*}
imm sets (* = in both, 5 common): GOG {0x8*,0x17*,0x30*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x17*,0x30*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067e000: 16-byte aligned: YES
  0067e000  55             PUSH EBP
  0067e001  8B EC          MOV EBP,ESP
  0067e003  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067e000
string literals (0):
direct callees (7):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005817F0  CSWSCreature::GetChemical
  0x0051C160  ServerApp::GetPlaceableById
  0x00605B80  CItemRepository::FindItemWithBaseItemType
  0x0051C0E0  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x3C4 [dec 964]  x2  via EAX,ECX
  +0x2CC [dec 716]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0067e000 FUN_0067e000 size=248 ret=8 callers=0 callees=7

undefined4 FUN_0067e000(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_c = 0;
    uVar2 = extraout_var;
    iVar1 = FUN_0051c100(local_10);
    if (iVar1 == 0) {
      iVar1 = FUN_0051c160(local_10);
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0x3c4) != 0)) {
        uVar2 = FUN_00605b80(0x17,0);
        local_8 = FUN_0051c0e0(uVar2);
        if (local_8 != 0) {
          local_c = *(undefined4 *)(local_8 + 0x2cc);
        }
      }
    }
    else {
      local_c = FUN_005817f0(uVar2);
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

## Card B15-2-59

GOG function: Global::ExecuteCommandGetCreatureTalent  gog 0x00790420  callconv=__thiscall  param_size_bytes=8  gog_size=696  gog_retn=8
Candidate Steam address: 0x0067ac20 (size 155, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_FindPath | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.29 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067ae40:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007903a0 Global::ExecuteCommandGetCreatureSize -> 0x0067adc0  gate_class=AUTO
- GOG-order neighbour after:  0x007906e0 Global::ExecuteCommandGetCurrentAction -> 0x0067b100  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [307 GetCreatureTalentRandom, 308 GetCreatureTalentBest] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x006a51b0 CScriptTalent::Constructor -> Steam ?  in candidate callees: n/a
  0x006f5030 CSWSCreatureStats::GetCreatureTalentRandom -> Steam 0x006b8bb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x1198} | Steam {0x8*,0xec}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x4,0x5,0x8*,0x68,0x133,0x134,0x7f000000,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x18,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067ac20: 16-byte aligned: YES
  0067ac20  55             PUSH EBP
  0067ac21  8B EC          MOV EBP,ESP
  0067ac23  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067ac20
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0067ac20 FUN_0067ac20 size=155 ret=8 callers=0 callees=4

undefined4 __thiscall FUN_0067ac20(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  int local_14;
  undefined4 local_10;
  char local_9;
  undefined4 local_8;
  
  local_10 = 0;
  local_8 = *(undefined4 *)(param_1 + 8);
  if ((0 < param_3) && (iVar1 = FUN_006fdaf0(&local_8), iVar1 == 0)) {
    return 0xfffff82f;
  }
  piVar3 = &local_14;
  uVar2 = local_8;
  FUN_0051c080(local_8,piVar3,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  local_9 = FUN_0053dfb0(uVar2,piVar3);
  if (local_9 == DAT_0099447f) {
    local_10 = *(undefined4 *)(local_14 + 0xec);
  }
  iVar1 = FUN_006fd9c0(local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff830;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0067ae40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0067ae40: 16-byte aligned: YES
  0067ae40  55             PUSH EBP
  0067ae41  8B EC          MOV EBP,ESP
  0067ae43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0067ae40
string literals (0):
direct callees (7):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0053E400  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006B8BB0  CSWSCreatureStats::GetCreatureTalentRandom
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 2):
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-2-60

GOG function: Global::ExecuteCommandEffectForceResistanceDecrease  gog 0x0078c1c0  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676a00 (size 470, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandEffectSpellLevelAbsorption | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.77 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676be0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078bfe0 Global::ExecuteCommandEffectSpellLevelAbsorption -> 0x00676a00  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c300 Global::ExecuteCommandEffectForceResistanceIncrease -> 0x00676d20  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [454 EffectForceResistanceDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x28,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2,0x3,0x4*,0x8*,0x9,0x34,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676a00: 16-byte aligned: YES
  00676a00  55             PUSH EBP
  00676a01  8B EC          MOV EBP,ESP
  00676a03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00676a00 FUN_00676a00 size=470 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00676a00(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_40;
  undefined2 local_38;
  undefined2 local_34;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0bb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_1c);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_14);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      iVar2 = FUN_006fd9a0(&local_20);
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
        local_18 = local_40;
        if ((local_1c < 10) && (-2 < local_1c)) {
          local_38 = (undefined2)DAT_00996020;
          *(undefined2 *)(local_40 + 8) = local_38;
          *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xffe7 | 8;
          FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
          FUN_005e4e80(0,local_1c);
          FUN_005e4e80(1,local_14);
          FUN_005e4e80(2,local_20);
        }
        else {
          local_34 = (undefined2)DAT_00995f1c;
          *(undefined2 *)(local_40 + 8) = local_34;
        }
        if (local_14 == 0) {
          FUN_005e4e80(3,1);
        }
        else {
          FUN_005e4e80(3,0);
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
/* ... 6 more lines truncated by --max-lines */
```

### decoy: 0x00676be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676be0: 16-byte aligned: YES
  00676be0  55             PUSH EBP
  00676be1  8B EC          MOV EBP,ESP
  00676be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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
