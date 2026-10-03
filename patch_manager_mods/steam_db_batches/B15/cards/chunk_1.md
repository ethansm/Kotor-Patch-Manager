# Review cards B15 chunk 1

Answer each card with one TSV row in `steam_db_batches/B15/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B15-1-01

GOG function: Global::ExecuteCommandEffectForceSight  gog 0x00789df0  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00674810 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006746a0:STR0/CALLEE0/SIG0.786/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789c80 Global::ExecuteCommandEffectForceBody -> 0x006746a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00789f30 Global::ExecuteCommandEffectFPRegenModifier -> 0x00674950  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [823 EffectForceSight] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0x30*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674810: 16-byte aligned: YES
  00674810  55             PUSH EBP
  00674811  8B EC          MOV EBP,ESP
  00674813  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674810
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674810 FUN_00674810 size=317 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00674810(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  int local_3c;
  undefined2 local_30;
  int local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097f5db;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_3c = 0;
  }
  else {
    local_3c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_18 = local_3c;
  local_30 = (undefined2)DAT_009960e0;
  *(undefined2 *)(local_3c + 8) = local_30;
  piVar4 = &local_1c;
  uVar3 = *(undefined4 *)(param_1 + 8);
  FUN_0051c080(uVar3,piVar4);
  local_11 = FUN_0053dfb0(uVar3,piVar4);
  if ((local_11 == DAT_0099447f) && (local_1c != 0)) {
    FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
  }
  iVar2 = FUN_006fdad0(0,local_18);
  iVar1 = local_18;
  if (iVar2 == 0) {
    uVar3 = 0xfffff830;
  }
  else {
    if (local_18 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar1);
    }
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x006746a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006746a0: 16-byte aligned: YES
  006746a0  55             PUSH EBP
  006746a1  8B EC          MOV EBP,ESP
  006746a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006746a0
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-02

GOG function: Global::ExecuteCommandEffectImmunity  gog 0x0078a840  callconv=__thiscall  param_size_bytes=8  gog_size=429  gog_retn=8
Candidate Steam address: 0x00675260 (size 429, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00675410:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078a6c0 Global::ExecuteCommandEffectHitPointChangeWhenDying -> 0x006750e0  gate_class=AUTO
- GOG-order neighbour after:  0x0078a9f0 Global::ExecuteCommandEffectInvisibility -> 0x00675410  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [273 EffectImmunity] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
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
prologue for 0x00675260: 16-byte aligned: YES
  00675260  55             PUSH EBP
  00675261  8B EC          MOV EBP,ESP
  00675263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675260
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00675260 FUN_00675260 size=429 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00675260(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  undefined2 local_2c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969b2b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_14 = local_38;
  local_2c = (undefined2)DAT_00995f74;
  *(undefined2 *)(local_38 + 8) = local_2c;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
  *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
  iVar1 = FUN_006fd9a0(&local_18);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (DAT_0099623c < local_18) {
      local_3c = DAT_0099623c;
    }
    else {
      local_3c = local_18;
    }
    if (local_3c < 0) {
      local_40 = 0;
    }
    else {
      if (DAT_0099623c < local_18) {
        local_44 = DAT_0099623c;
      }
      else {
        local_44 = local_18;
      }
      local_40 = local_44;
    }
    local_18 = local_40;
    FUN_005e4e80(0,local_40);
    FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    iVar3 = FUN_006fdad0(0,local_14);
    iVar1 = local_14;
    if (iVar3 == 0) {
      uVar2 = 0xfffff830;
    }
    else {
      if (local_14 != 0) {
        FUN_005e4ca0();
        FUN_009196fd(iVar1);
      }
      uVar2 = 0;
    }
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00675410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675410: 16-byte aligned: YES
  00675410  55             PUSH EBP
  00675411  8B EC          MOV EBP,ESP
  00675413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675410
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EAX
```

## Card B15-1-03

GOG function: Global::ExecuteCommandEffectDamageImmunityDecrease  gog 0x00788e60  callconv=__thiscall  param_size_bytes=8  gog_size=455  gog_retn=8
Candidate Steam address: 0x00673880 (size 455, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00673a50:STR0/CALLEE0/SIG0.944/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00788c70 Global::ExecuteCommandEffectDamageDecrease -> 0x00673690  gate_class=REVIEW
- GOG-order neighbour after:  0x00789030 Global::ExecuteCommandEffectDamageImmunityIncrease -> 0x00673a50  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [449 EffectDamageImmunityDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 9 common): GOG {0x4*,0x8*,0x38*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffff9c*,0xffffffe7*} | Steam {0x4*,0x8*,0x38*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffff9c*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00673880: 16-byte aligned: YES
  00673880  55             PUSH EBP
  00673881  8B EC          MOV EBP,ESP
  00673883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00673880
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00673880 FUN_00673880 size=455 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00673880(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  undefined2 local_30;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00957b5b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_14 = local_38;
  local_30 = (undefined2)DAT_00995f60;
  *(undefined2 *)(local_38 + 8) = local_30;
  iVar1 = FUN_006fd9a0(&local_1c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_18);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      if (local_18 < 0x65) {
        local_3c = local_18;
      }
      else {
        local_3c = 100;
      }
      if (local_3c < -100) {
        local_40 = -100;
      }
      else {
        if (local_18 < 0x65) {
          local_44 = local_18;
        }
        else {
          local_44 = 100;
        }
        local_40 = local_44;
      }
      local_18 = local_40;
      if ((local_1c < 0) || (DAT_00996144 < local_1c)) {
        local_1c = DAT_0099611c;
      }
      *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      FUN_005e4e80(0,local_1c);
      FUN_005e4e80(1,local_18);
      iVar3 = FUN_006fdad0(0,local_14);
      iVar1 = local_14;
      if (iVar3 == 0) {
        uVar2 = 0xfffff830;
      }
      else {
        if (local_14 != 0) {
          FUN_005e4ca0();
          FUN_009196fd(iVar1);
/* ... 8 more lines truncated by --max-lines */
```

### decoy: 0x00673a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00673a50: 16-byte aligned: YES
  00673a50  55             PUSH EBP
  00673a51  8B EC          MOV EBP,ESP
  00673a53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00673a50
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
```

## Card B15-1-04

GOG function: Global::ExecuteCommandSetGalaxyMapPlanetAvailable  gog 0x007acb00  callconv=__thiscall  param_size_bytes=8  gog_size=104  gog_retn=8
Candidate Steam address: 0x00697460 (size 74, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandShowGalaxyMapGui | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006974b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acab0 Global::ExecuteCommandShowGalaxyMapGui -> 0x00697460  gate_class=REVIEW
- GOG-order neighbour after:  0x007acb70 Global::ExecuteCommandGetGalaxyMapPlanetAvailable -> 0x00697520  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [742 SetPlanetAvailable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x007046e0 CSWPartyTable::SetPlanetAvailable -> Steam 0x005fed30  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {0x4}
imm sets (* = in both, 2 common): GOG {0x8*,0x10,0xfffff82f*} | Steam {0x8*,0xc,0xfffff82f*}

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

## Card B15-1-05

GOG function: Global::ExecuteCommandSaveNPCByObject  gog 0x007b1890  callconv=__thiscall  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x0069c340 (size 118, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetIsPlayerMadeCharacter | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0069c2c0:STR0/CALLEE1/SIG1/SRV0/VT0
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
  0x006fdca0 CSWPartyTable::SaveMemberByObject -> Steam 0x005f8130  in candidate callees: no
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

## Card B15-1-06

GOG function: Global::ExecuteCommandGetFactionAverageLevel  gog 0x007920d0  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067ca10 (size 212, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetFactionAverageGoodEvilAlignment | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067caf0:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B15-1-07

GOG function: Global::ExecuteCommandEffectDamageResistance  gog 0x00789600  callconv=__thiscall  param_size_bytes=8  gog_size=492  gog_retn=8
Candidate Steam address: 0x00674020 (size 492, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00673e10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007893f0 Global::ExecuteCommandEffectDamageReduction -> 0x00673e10  gate_class=REVIEW
- GOG-order neighbour after:  0x007897f0 Global::ExecuteCommandEffectForceShield -> 0x00674210  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [81 EffectDamageResistance] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674020: 16-byte aligned: YES
  00674020  55             PUSH EBP
  00674021  8B EC          MOV EBP,ESP
  00674023  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674020
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
  +0xA [dec 10]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00674020 FUN_00674020 size=492 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00674020(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_3c;
  int local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a05b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_28);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_20);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      iVar2 = FUN_006fd9a0(&local_18);
      if (iVar2 == 0) {
        uVar3 = 0xfffff82f;
      }
      else {
        if ((local_28 < 0) || (DAT_00996144 < local_28)) {
          local_28 = DAT_0099611c;
        }
        iVar2 = FUN_00919723(0x8c,uVar1);
        local_8 = 0;
        if (iVar2 == 0) {
          local_48 = 0;
        }
        else {
          local_48 = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        local_1c = local_48;
        local_3c = (undefined2)DAT_00995f24;
        *(undefined2 *)(local_48 + 8) = local_3c;
        *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
        FUN_005e4e80(0,local_28);
        FUN_005e4e80(1,local_20);
        FUN_005e4e80(2,local_18);
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
/* ... 5 more lines truncated by --max-lines */
```

### decoy: 0x00673e10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00673e10: 16-byte aligned: YES
  00673e10  55             PUSH EBP
  00673e11  8B EC          MOV EBP,ESP
  00673e13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00673e10
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
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-1-08

GOG function: Global::ExecuteCommandEffectModifyAttacks  gog 0x0078ae60  callconv=__thiscall  param_size_bytes=8  gog_size=344  gog_retn=8
Candidate Steam address: 0x00675880 (size 344, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00675710:STR0/CALLEE0/SIG0.889/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078acf0 Global::ExecuteCommandEffectMissChance -> 0x00675710  gate_class=REVIEW
- GOG-order neighbour after:  0x0078afc0 Global::ExecuteCommandEffectMovementSpeedDecrease -> 0x006759e0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [485 EffectModifyAttacks] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x8*,0x2c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x5*,0x8*,0x2c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00675880: 16-byte aligned: YES
  00675880  55             PUSH EBP
  00675881  8B EC          MOV EBP,ESP
  00675883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675880
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00675880 FUN_00675880 size=344 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00675880(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_38;
  undefined2 local_30;
  undefined2 local_2c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
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
      local_38 = 0;
    }
    else {
      local_38 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_38;
    if (local_18 < 6) {
      local_30 = (undefined2)DAT_00995fcc;
      *(undefined2 *)(local_38 + 8) = local_30;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_18);
    }
    else {
      local_2c = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_38 + 8) = local_2c;
    }
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

### decoy: 0x00675710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675710: 16-byte aligned: YES
  00675710  55             PUSH EBP
  00675711  8B EC          MOV EBP,ESP
  00675713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675710
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-1-09

GOG function: Global::ExecuteCommandEffectForceResistanceDecrease  gog 0x0078c1c0  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676be0 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676d20:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B15-1-10

GOG function: Global::ExecuteCommandEffectForceResistanceIncrease  gog 0x0078c300  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00676d20 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676be0:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B15-1-11

GOG function: Global::ExecuteCommandEffectDamageShield  gog 0x00789930  callconv=__thiscall  param_size_bytes=8  gog_size=471  gog_retn=8
Candidate Steam address: 0x00674350 (size 471, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00674020:STR0/CALLEE0/SIG0.818/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007897f0 Global::ExecuteCommandEffectForceShield -> 0x00674210  gate_class=REVIEW
- GOG-order neighbour after:  0x00789b10 Global::ExecuteCommandEffectFactionModifier -> 0x00674530  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [487 EffectDamageShield] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 10 common): GOG {0x2*,0x4*,0x8*,0xa*,0x30*,0x8c*,0x2710*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0xa*,0x30*,0x8c*,0x2710*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674350: 16-byte aligned: YES
  00674350  55             PUSH EBP
  00674351  8B EC          MOV EBP,ESP
  00674353  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674350
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
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674350 FUN_00674350 size=471 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00674350(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_3c;
  undefined2 local_34;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a08b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_1c);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      iVar2 = FUN_006fd9a0(&local_20);
      if (iVar2 == 0) {
        uVar3 = 0xfffff82f;
      }
      else {
        if ((local_18 < 0) || (10000 < local_18)) {
          local_18 = 1;
        }
        if ((local_1c < 1) || (10 < local_1c)) {
          local_1c = 0;
        }
        if ((local_20 < 0) || (DAT_00996144 < local_20)) {
          local_20 = DAT_0099611c;
        }
        iVar2 = FUN_00919723(0x8c,uVar1);
        local_8 = 0;
        if (iVar2 == 0) {
          local_3c = 0;
        }
        else {
          local_3c = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        local_14 = local_3c;
        local_34 = (undefined2)DAT_00996010;
        *(undefined2 *)(local_3c + 8) = local_34;
        FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
        *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
        FUN_005e4e80(0,local_18);
        FUN_005e4e80(1,local_1c);
        FUN_005e4e80(2,local_20);
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
    }
  }
  ExceptionList = local_10;
/* ... 2 more lines truncated by --max-lines */
```

### decoy: 0x00674020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674020: 16-byte aligned: YES
  00674020  55             PUSH EBP
  00674021  8B EC          MOV EBP,ESP
  00674023  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674020
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
  +0xA [dec 10]  x2  via EAX,ECX
```

## Card B15-1-12

GOG function: Global::ExecuteCommandDeleteJournalWorldEntryStrref  gog 0x00785c50  callconv=__thiscall  param_size_bytes=8  gog_size=230  gog_retn=8
Candidate Steam address: 0x00670670 (size 230, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00670580:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00785b60 Global::ExecuteCommandDeleteJournalWorldEntry -> 0x00670580  gate_class=REVIEW
- GOG-order neighbour after:  0x00785d40 Global::ExecuteCommandDestroyObject -> 0x00670760  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [674 DeleteJournalWorldEntryStrref] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0064bcc0 CServerExoApp::GetPlayerList -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x00731970 CSWSModule::DeleteWorldJournalEntryStrref -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x28*,0xfffff82f*} | Steam {0x8*,0x28*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00670670: 16-byte aligned: YES
  00670670  55             PUSH EBP
  00670671  8B EC          MOV EBP,ESP
  00670673  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00670670
string literals (0):
direct callees (10):
  0x0051C2E0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0055F2E0  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BBA0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00670670 FUN_00670670 size=230 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00670670(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_18;
  int local_14;
  undefined4 local_10;
  int local_c;
  int local_8;
  
  local_14 = 0;
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  iVar1 = FUN_0051c2e0(param_1);
  uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  local_8 = FUN_0051bd90();
  iVar2 = FUN_006fd9a0(&local_10);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    if (local_8 != 0) {
      FUN_0055f2e0(local_10);
    }
    local_c = FUN_0051c2a0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar4,uVar3);
    if (local_c != 0) {
      local_18 = FUN_007a1720();
      while (local_18 != 0) {
        local_14 = FUN_00561430(local_18);
        if (local_14 != 0) {
          iVar2 = FUN_00632a20();
          if ((iVar2 != 0) && (iVar1 != 0)) {
            FUN_0064bba0(local_14,local_10);
          }
        }
        FUN_0058c370(&local_18);
      }
    }
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x00670580 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00670580: 16-byte aligned: YES
  00670580  55             PUSH EBP
  00670581  8B EC          MOV EBP,ESP
  00670583  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00670580
string literals (0):
direct callees (10):
  0x0051C2E0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0055F1D0  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BB30  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,EDX
```

## Card B15-1-13

GOG function: Global::ExecuteCommandSaveNPCByObject  gog 0x007b1890  callconv=__thiscall  param_size_bytes=8  gog_size=118  gog_retn=8
Candidate Steam address: 0x0069c200 (size 185, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetCombatActionPending | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0069c2c0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007b17d0 Global::ExecuteCommandGetCombatActionPending -> 0x0069c200  gate_class=AUTO
- GOG-order neighbour after:  0x007b1910 Global::ExecuteCommandSavePUPByObject -> 0x0069c340  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [873 SaveNPCByObject] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006fdca0 CSWPartyTable::SaveMemberByObject -> Steam 0x005f8130  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0x30}
imm sets (* = in both, 2 common): GOG {0x8*,0x10,0x7f000000,0xfffff82f*} | Steam {0x8*,0x18,0x100,0xfffff82f*,0xfffff830}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0069c200: 16-byte aligned: YES
  0069c200  55             PUSH EBP
  0069c201  8B EC          MOV EBP,ESP
  0069c203  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0069c200
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x00521060  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0069c200 FUN_0069c200 size=185 ret=8 callers=0 callees=4

undefined4 FUN_0069c200(void)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  bool bVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar5 = extraout_var;
    piVar3 = (int *)FUN_0051c0a0(local_8);
    bVar4 = false;
    if (piVar3 != (int *)0x0) {
      iVar1 = (**(code **)(*piVar3 + 0x30))(uVar5,uVar2);
      if (iVar1 != 0) {
        iVar1 = (**(code **)(*piVar3 + 0x30))();
        if ((iVar1 != 0) && (iVar1 != -0x100)) {
          iVar1 = FUN_00521060();
          bVar4 = iVar1 != 0;
        }
      }
    }
    iVar1 = FUN_006fd9c0(bVar4);
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

## Card B15-1-14

GOG function: Global::ExecuteCommandDoSinglePlayerAutoSave  gog 0x00786680  callconv=__thiscall  param_size_bytes=8  gog_size=38  gog_retn=8
Candidate Steam address: 0x006710a0 (size 38, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006710d0:STR0/CALLEE0/SIG0.097/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007862f0 Global::ExecuteCommandDoDoorAction -> 0x00670d10  gate_class=AUTO
- GOG-order neighbour after:  0x007866b0 Global::ExecuteCommandDoTouchAttack -> 0x006710d0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [512 DoSinglePlayerAutoSave] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0064b970 CServerExoApp::SetAutoSavePending -> Steam ?  in candidate callees: n/a
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
prologue for 0x006710a0: 16-byte aligned: YES
  006710a0  55             PUSH EBP
  006710a1  8B EC          MOV EBP,ESP
  006710a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006710a0
string literals (0):
direct callees (1):
  0x0051BF50  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006710a0 FUN_006710a0 size=38 ret=8 callers=0 callees=1

undefined4 FUN_006710a0(void)

{
  FUN_0051bf50(1);
  return 0;
}
```

### decoy: 0x006710d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006710d0: 16-byte aligned: YES
  006710d0  55             PUSH EBP
  006710d1  8B EC          MOV EBP,ESP
  006710d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-1-15

GOG function: Global::ExecuteCommandEffectSavingThrowDecrease  gog 0x0078b5a0  callconv=__thiscall  param_size_bytes=8  gog_size=528  gog_retn=8
Candidate Steam address: 0x00675fc0 (size 528, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006761d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078b420 Global::ExecuteCommandEffectResurrection -> 0x00675e40  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b7b0 Global::ExecuteCommandEffectSavingThrowIncrease -> 0x006761d0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [452 EffectSavingThrowDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x3*,0x4*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00675fc0: 16-byte aligned: YES
  00675fc0  55             PUSH EBP
  00675fc1  8B EC          MOV EBP,ESP
  00675fc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675fc0
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
  +0x8 [dec 8]  x4  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00675fc0 FUN_00675fc0 size=528 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_00675fc0(int param_1,undefined4 param_2,int param_3)

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
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959f5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = 0;
  iVar2 = FUN_006fd9a0(&local_20);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    if ((local_20 < 0) || (3 < local_20)) {
      local_20 = 0;
    }
    iVar2 = FUN_006fd9a0(&local_14);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else if ((param_3 < 3) || (iVar2 = FUN_006fd9a0(&local_28), iVar2 != 0)) {
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
      local_3c = (undefined2)DAT_00995f88;
      *(undefined2 *)(local_4c + 8) = local_3c;
      *(ushort *)(local_4c + 0xa [dec 10]) = *(ushort *)(local_4c + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_14);
      FUN_005e4e80(1,local_20);
      FUN_005e4e80(2,local_28);
      FUN_005e4e80(3,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_15 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
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
    }
/* ... 4 more lines truncated by --max-lines */
```

### decoy: 0x006761d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006761d0: 16-byte aligned: YES
  006761d0  55             PUSH EBP
  006761d1  8B EC          MOV EBP,ESP
  006761d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006761d0
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
  +0x8 [dec 8]  x4  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EDX
```

## Card B15-1-16

GOG function: Global::ExecuteCommandGetFactionAverageLevel  gog 0x007920d0  callconv=__thiscall  param_size_bytes=8  gog_size=212  gog_retn=8
Candidate Steam address: 0x0067cbd0 (size 239, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetFactionAverageReputation | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067caf0:STR0/CALLEE0/SIG1/SRV0/VT0
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
imm sets (* = in both, 3 common): GOG {0x8*,0x2c,0x7f000000,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x30,0xfffff82f*,0xfffff830*}

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

## Card B15-1-17

GOG function: Global::ExecuteCommandEffectFPRegenModifier  gog 0x00789f30  callconv=__thiscall  param_size_bytes=8  gog_size=359  gog_retn=8
Candidate Steam address: 0x00674950 (size 359, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00674ac0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789df0 Global::ExecuteCommandEffectForceSight -> 0x00674810  gate_class=REVIEW
- GOG-order neighbour after:  0x0078a0a0 Global::ExecuteCommandEffectVPRegenModifier -> 0x00674ac0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [779 EffectFPRegenModifier] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674950: 16-byte aligned: YES
  00674950  55             PUSH EBP
  00674951  8B EC          MOV EBP,ESP
  00674953  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674950
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674950 FUN_00674950 size=359 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00674950(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_40;
  undefined2 local_34;
  int local_20;
  int local_1c;
  undefined4 local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0bb;
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
      local_40 = 0;
    }
    else {
      local_40 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_1c = local_40;
    local_34 = (undefined2)DAT_009960d8;
    *(undefined2 *)(local_40 + 8) = local_34;
    FUN_005e4e80(0,local_18);
    piVar5 = &local_20;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,piVar5);
    local_11 = FUN_0053dfb0(uVar3,piVar5);
    if ((local_11 == DAT_0099447f) && (local_20 != 0)) {
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
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00674ac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674ac0: 16-byte aligned: YES
  00674ac0  55             PUSH EBP
  00674ac1  8B EC          MOV EBP,ESP
  00674ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674ac0
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-18

GOG function: Global::ExecuteCommandChangeFaction  gog 0x00784680  callconv=(blank)  param_size_bytes=4  gog_size=650  gog_retn=8
Candidate Steam address: 0x0066f330 (size 113, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandClearAllActions | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.21 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066f0a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00783dc0 Global::ExecuteCommandCastSpell -> 0x0066e7e0  gate_class=AUTO
- GOG-order neighbour after:  0x00784910 Global::ExecuteCommandClearAllActions -> 0x0066f330  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [173 ChangeFaction, 412 ChangeToStandardFaction] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: no
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: no
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: no
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064c0b0 CServerExoApp::GetFactionOfObject -> Steam ?  in candidate callees: n/a
  0x007e4850 CSWSFaction::AddMember -> Steam ?  in candidate callees: n/a
  0x007ef020 CFactionManager::GetFaction -> Steam ?  in candidate callees: n/a
  0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x40,0x48,0x6c,0x27c,0x2f8,0x1198} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x8*,0x64,0xad,0x19c,0xfffff82f} | Steam {0x8*,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0066f330: 16-byte aligned: YES
  0066f330  55             PUSH EBP
  0066f331  8B EC          MOV EBP,ESP
  0066f333  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0066f330 FUN_0066f330 size=113 ret=8 callers=0 callees=3

undefined4 __fastcall FUN_0066f330(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = 0;
  local_8 = 0;
  if (*(int *)(param_1 + 4) != 0) {
    puVar3 = &local_c;
    uVar2 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar2,puVar3,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
    cVar1 = FUN_0053dfb0(uVar2,puVar3);
    if (cVar1 == DAT_0099447f) {
      FUN_00541080(1);
    }
  }
  return 0;
}
```

### decoy: 0x0066f0a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066f0a0: 16-byte aligned: YES
  0066f0a0  55             PUSH EBP
  0066f0a1  8B EC          MOV EBP,ESP
  0066f0a3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B15-1-19

GOG function: Global::ExecuteCommandEffectAttackIncrease  gog 0x007878e0  callconv=__thiscall  param_size_bytes=8  gog_size=477  gog_retn=8
Candidate Steam address: 0x00672300 (size 477, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00672120:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00787700 Global::ExecuteCommandEffectAttackDecrease -> 0x00672120  gate_class=REVIEW
- GOG-order neighbour after:  0x00787ac0 Global::ExecuteCommandEffectBeam -> 0x006724e0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [118 EffectAttackIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00672300: 16-byte aligned: YES
  00672300  55             PUSH EBP
  00672301  8B EC          MOV EBP,ESP
  00672303  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00672300
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00672300 FUN_00672300 size=477 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00672300(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_38;
  int local_24;
  int local_20;
  int local_1c;
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959fcb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_14);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_1c);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if ((local_1c < 0) || (DAT_0099619c < local_1c)) {
        local_1c = DAT_00996184;
      }
      iVar2 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar2 == 0) {
        local_48 = 0;
      }
      else {
        local_48 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_20 = local_48;
      local_38 = (undefined2)DAT_00995f44;
      *(undefined2 *)(local_48 + 8) = local_38;
      *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_14);
      FUN_005e4e80(1,local_1c);
      FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_15 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
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

### decoy: 0x00672120 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00672120: 16-byte aligned: YES
  00672120  55             PUSH EBP
  00672121  8B EC          MOV EBP,ESP
  00672123  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00672120
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

## Card B15-1-20

GOG function: CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee  gog 0x0077f030  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=8
Candidate Steam address: 0x00669a50 (size 123, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006699d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0077efb0 CSWVirtualMachineCommands::ExecuteCommandActionDoCommand -> 0x006699d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0077f0b0 CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingRanged -> 0x00669ad0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [399 ActionEquipMostDamagingMelee] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00698e60 CSWSCreature::EquipMostDamagingMeleeWeapon -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00669a50: 16-byte aligned: YES
  00669a50  55             PUSH EBP
  00669a51  8B EC          MOV EBP,ESP
  00669a53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00669a50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00580330  CSWSCreature::EquipMostDamagingMeleeWeapon
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00669a50 FUN_00669a50 size=123 ret=8 callers=0 callees=4

undefined4 __fastcall FUN_00669a50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_10);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      local_8 = FUN_0051c100(*(undefined4 *)(param_1 + 8));
      if (local_8 != 0) {
        FUN_00580330(local_c,local_10,0);
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006699d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006699d0: 16-byte aligned: YES
  006699d0  55             PUSH EBP
  006699d1  8B EC          MOV EBP,ESP
  006699d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006699d0
string literals (0):
direct callees (4):
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E1340  CSWSObject::AddDoCommandAction
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX
```

## Card B15-1-21

GOG function: Global::ExecuteCommandEffectTemporaryHP  gog 0x0078c5f0  callconv=__thiscall  param_size_bytes=8  gog_size=418  gog_retn=8
Candidate Steam address: 0x00677010 (size 418, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676e60:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c440 CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints -> 0x00676e60  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c7a0 Global::ExecuteCommandEffectTrueSeeing -> 0x006771c0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [314 EffectTemporaryHitpoints] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x30*} | Steam {0x8*,0xa*,0x30*}
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
prologue for 0x00677010: 16-byte aligned: YES
  00677010  55             PUSH EBP
  00677011  8B EC          MOV EBP,ESP
  00677013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677010
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
// FN 00677010 FUN_00677010 size=418 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00677010(int param_1)

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
      local_34 = (undefined2)DAT_00995f58;
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

### decoy: 0x00676e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676e60: 16-byte aligned: YES
  00676e60  55             PUSH EBP
  00676e61  8B EC          MOV EBP,ESP
  00676e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-1-22

GOG function: Global::ExecuteCommandEffectSpellLevelAbsorption  gog 0x0078bfe0  callconv=__thiscall  param_size_bytes=8  gog_size=470  gog_retn=8
Candidate Steam address: 0x00676a00 (size 470, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00676be0:STR0/CALLEE0/SIG0.773/SRV0/VT0
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
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x8*,0x9*,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x3*,0x4*,0x8*,0x9*,0x34*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

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

## Card B15-1-23

GOG function: Global::ExecuteCommandEffectMovementSpeedIncrease  gog 0x0078b150  callconv=__thiscall  param_size_bytes=8  gog_size=395  gog_retn=8
Candidate Steam address: 0x00675b70 (size 395, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006759e0:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x00675b70: 16-byte aligned: YES
  00675b70  55             PUSH EBP
  00675b71  8B EC          MOV EBP,ESP
  00675b73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00675b70 FUN_00675b70 size=395 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_00675b70(int param_1,undefined4 param_2,int param_3)

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
      local_34 = (undefined2)DAT_00995f8c;
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

### decoy: 0x006759e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006759e0: 16-byte aligned: YES
  006759e0  55             PUSH EBP
  006759e1  8B EC          MOV EBP,ESP
  006759e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-1-24

GOG function: Global::ExecuteCommandEffectSkillIncrease  gog 0x0078bca0  callconv=__thiscall  param_size_bytes=8  gog_size=448  gog_retn=8
Candidate Steam address: 0x006766c0 (size 448, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00676500:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078bae0 Global::ExecuteCommandEffectSkillDecrease -> 0x00676500  gate_class=REVIEW
- GOG-order neighbour after:  0x0078be60 Global::ExecuteCommandEffectSpellImmunity -> 0x00676880  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [351 EffectSkillIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xa*,0x122*,0x123*} | Steam {0x8*,0xa*,0x122*,0x123*}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x8*,0x34*,0x8c*,0xff*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x34*,0x8c*,0xff*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006766c0: 16-byte aligned: YES
  006766c0  55             PUSH EBP
  006766c1  8B EC          MOV EBP,ESP
  006766c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006766c0
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EAX
  +0x123 [dec 291]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006766c0 FUN_006766c0 size=448 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_006766c0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  int local_44;
  undefined2 local_38;
  undefined2 local_30;
  int local_24;
  int local_20;
  int local_1c;
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a1ab;
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
      local_30 = (undefined2)DAT_00995ff8;
      *(undefined2 *)(local_44 + 8) = local_30;
      *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xffe7 | 8;
      if (((local_1c < 1) || ((int)(uint)*(byte *)(DAT_00a1b4d0 + 0x123) <= local_1c)) &&
         (local_1c != 0xff)) {
        local_38 = (undefined2)DAT_00995f1c;
        *(undefined2 *)(local_44 + 8) = local_38;
      }
      else {
        FUN_005e4e80(0,local_1c);
        FUN_005e4e80(1,local_14);
        FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      }
      piVar4 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar4);
      local_15 = FUN_0053dfb0(uVar3,piVar4);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
        FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      }
      iVar2 = FUN_006fdad0(0,local_20);
      if (iVar2 == 0) {
        uVar3 = 0xfffff830;
      }
      else {
        uVar3 = 0;
      }
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00676500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00676500: 16-byte aligned: YES
  00676500  55             PUSH EBP
  00676501  8B EC          MOV EBP,ESP
  00676503  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00676500
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EAX
  +0x123 [dec 291]  x1  via ECX
```

## Card B15-1-25

GOG function: Global::ExecuteCommandEffectDamageImmunityIncrease  gog 0x00789030  callconv=__thiscall  param_size_bytes=8  gog_size=455  gog_retn=8
Candidate Steam address: 0x00673a50 (size 455, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00673880:STR0/CALLEE0/SIG0.944/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00788e60 Global::ExecuteCommandEffectDamageImmunityDecrease -> 0x00673880  gate_class=REVIEW
- GOG-order neighbour after:  0x00789200 Global::ExecuteCommandEffectDamageIncrease -> 0x00673c20  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [275 EffectDamageImmunityIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 8 common): GOG {0x4*,0x8*,0x38*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x38*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00673a50: 16-byte aligned: YES
  00673a50  55             PUSH EBP
  00673a51  8B EC          MOV EBP,ESP
  00673a53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00673a50
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00673a50 FUN_00673a50 size=455 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00673a50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  undefined2 local_30;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00957b5b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_14 = local_38;
  local_30 = (undefined2)DAT_00995f5c;
  *(undefined2 *)(local_38 + 8) = local_30;
  iVar1 = FUN_006fd9a0(&local_1c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    iVar1 = FUN_006fd9a0(&local_18);
    if (iVar1 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      if (local_18 < 0x65) {
        local_3c = local_18;
      }
      else {
        local_3c = 100;
      }
      if (local_3c < 0) {
        local_40 = 0;
      }
      else {
        if (local_18 < 0x65) {
          local_44 = local_18;
        }
        else {
          local_44 = 100;
        }
        local_40 = local_44;
      }
      local_18 = local_40;
      if ((local_1c < 0) || (DAT_00996144 < local_1c)) {
        local_1c = DAT_0099611c;
      }
      *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      FUN_005e4e80(0,local_1c);
      FUN_005e4e80(1,local_18);
      iVar3 = FUN_006fdad0(0,local_14);
      iVar1 = local_14;
      if (iVar3 == 0) {
        uVar2 = 0xfffff830;
      }
      else {
        if (local_14 != 0) {
          FUN_005e4ca0();
          FUN_009196fd(iVar1);
/* ... 8 more lines truncated by --max-lines */
```

### decoy: 0x00673880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00673880: 16-byte aligned: YES
  00673880  55             PUSH EBP
  00673881  8B EC          MOV EBP,ESP
  00673883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00673880
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
```

## Card B15-1-26

GOG function: Global::ExecuteCommandEffectDamageIncrease  gog 0x00789200  callconv=__thiscall  param_size_bytes=8  gog_size=496  gog_retn=8
Candidate Steam address: 0x00673c20 (size 496, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00673e10:STR0/CALLEE0/SIG0.778/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789030 Global::ExecuteCommandEffectDamageImmunityIncrease -> 0x00673a50  gate_class=REVIEW
- GOG-order neighbour after:  0x007893f0 Global::ExecuteCommandEffectDamageReduction -> 0x00673e10  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [120 EffectDamageIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x8*,0xa*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0xa*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00673c20: 16-byte aligned: YES
  00673c20  55             PUSH EBP
  00673c21  8B EC          MOV EBP,ESP
  00673c23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00673c20
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00673c20 FUN_00673c20 size=496 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00673c20(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_38;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959fcb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_24);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if ((local_18 < 1) || (10 < local_18)) {
        local_18 = 0;
      }
      if ((local_24 < 0) || (DAT_00996144 < local_24)) {
        local_24 = DAT_0099611c;
      }
      iVar2 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar2 == 0) {
        local_48 = 0;
      }
      else {
        local_48 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_1c = local_48;
      local_38 = (undefined2)DAT_00995f50;
      *(undefined2 *)(local_48 + 8) = local_38;
      *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_18);
      FUN_005e4e80(1,local_24);
      FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_20;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_11 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_11 == DAT_0099447f) && (local_20 != 0)) {
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
  }
  ExceptionList = local_10;
  return uVar3;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x00673e10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00673e10: 16-byte aligned: YES
  00673e10  55             PUSH EBP
  00673e11  8B EC          MOV EBP,ESP
  00673e13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00673e10
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
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-1-27

GOG function: Global::ExecuteCommandEffectDroidStun  gog 0x0078cac0  callconv=__thiscall  param_size_bytes=8  gog_size=252  gog_retn=8
Candidate Steam address: 0x006774e0 (size 252, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00677750:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c8c0 Global::ExecuteCommandEffectVisualEffect -> 0x006772e0  gate_class=AUTO
- GOG-order neighbour after:  0x0078cbc0 Global::ExecuteCommandEffectForceJump -> 0x006775e0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [391 EffectDroidStun] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x8*,0x24*,0x8c*,0xfffff830*} | Steam {0x3*,0x4*,0x8*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006774e0: 16-byte aligned: YES
  006774e0  55             PUSH EBP
  006774e1  8B EC          MOV EBP,ESP
  006774e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006774e0
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
// FN 006774e0 FUN_006774e0 size=252 ret=8 callers=0 callees=6

undefined4 FUN_006774e0(void)

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
  FUN_005e4e80(0,3);
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

### decoy: 0x00677750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677750: 16-byte aligned: YES
  00677750  55             PUSH EBP
  00677751  8B EC          MOV EBP,ESP
  00677753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677750
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B15-1-28

GOG function: Global::ExecuteCommandEffectForcePushed  gog 0x0078cd30  callconv=__thiscall  param_size_bytes=8  gog_size=262  gog_retn=8
Candidate Steam address: 0x00677750 (size 262, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006774e0:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078cbc0 Global::ExecuteCommandEffectForceJump -> 0x006775e0  gate_class=AUTO
- GOG-order neighbour after:  0x0078ce40 Global::ExecuteCommandEffectForcePushTargeted -> 0x00677860  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [392 EffectForcePushed] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
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
prologue for 0x00677750: 16-byte aligned: YES
  00677750  55             PUSH EBP
  00677751  8B EC          MOV EBP,ESP
  00677753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00677750
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00677750 FUN_00677750 size=262 ret=8 callers=0 callees=6

undefined4 __fastcall FUN_00677750(int param_1)

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
  local_28 = (undefined2)DAT_0099600c;
  *(undefined2 *)(local_30 + 8) = local_28;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
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

### decoy: 0x006774e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006774e0: 16-byte aligned: YES
  006774e0  55             PUSH EBP
  006774e1  8B EC          MOV EBP,ESP
  006774e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006774e0
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

## Card B15-1-29

GOG function: Global::ExecuteCommandVector  gog 0x007aaac0  callconv=(blank)  param_size_bytes=0  gog_size=140  gog_retn=8
Candidate Steam address: 0x00695370 (size 254, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandUnequipItem | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00695470:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007aa9c0 Global::ExecuteCommandUnequipItem -> 0x00695370  gate_class=AUTO
- GOG-order neighbour after:  0x007aab50 Global::ExecuteCommandVectorConversions -> 0x00695500  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [142 Vector] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: no
  0x00640c60 CVirtualMachine::StackPushVector -> Steam 0x006fda40  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x30,0x38}
imm sets (* = in both, 2 common): GOG {0x8*,0xc,0x10,0xfffff82f*,0xfffff830} | Steam {0x2,0x8*,0x1c,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00695370: 16-byte aligned: YES
  00695370  55             PUSH EBP
  00695371  8B EC          MOV EBP,ESP
  00695373  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00695370 FUN_00695370 size=254 ret=8 callers=0 callees=5

undefined4 FUN_00695370(undefined4 param_1,int param_2)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int **ppiVar10;
  undefined4 uVar11;
  int extraout_var;
  int *local_14;
  undefined4 local_10;
  int *local_c;
  undefined4 local_8;
  
  local_8 = 0;
  iVar2 = FUN_006fdaf0(&local_10);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = extraout_var;
    if ((1 < param_2) && (iVar4 = FUN_006fd9a0(&local_8), iVar4 == 0)) {
      return 0xfffff82f;
    }
    if (*(int *)(iVar2 + 4) == 1) {
      uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      ppiVar10 = &local_14;
      uVar8 = *(undefined4 *)(iVar2 + 8);
      FUN_0051c080(uVar8,ppiVar10);
      cVar1 = FUN_0053dfb0(uVar8,ppiVar10);
      if (cVar1 == DAT_0099447f) {
        ppiVar10 = &local_c;
        FUN_0051c080(local_10,ppiVar10,iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar3);
        cVar1 = FUN_0053dfb0(local_10,ppiVar10);
        if (cVar1 == DAT_0099447f) {
          uVar11 = 0;
          uVar9 = 0;
          uVar7 = 0;
          uVar6 = 0;
          uVar3 = DAT_00994484;
          uVar8 = local_8;
          uVar5 = (**(code **)(*local_c + 0x38))(DAT_00994484,0,0,local_8,0,0);
          (**(code **)(*local_14 + 0x30))(uVar5);
          FUN_00567490(uVar5,uVar3,uVar6,uVar7,uVar8,uVar9,uVar11);
        }
      }
    }
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x00695470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695470: 16-byte aligned: YES
  00695470  55             PUSH EBP
  00695471  8B EC          MOV EBP,ESP
  00695473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695470
string literals (0):
direct callees (2):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-1-30

GOG function: Global::ExecuteCommandDeleteJournalWorldEntry  gog 0x00785b60  callconv=__thiscall  param_size_bytes=8  gog_size=230  gog_retn=8
Candidate Steam address: 0x00670580 (size 230, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00670670:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00785a90 Global::ExecuteCommandDeleteJournalWorldAllEntries -> 0x006704b0  gate_class=AUTO
- GOG-order neighbour after:  0x00785c50 Global::ExecuteCommandDeleteJournalWorldEntryStrref -> 0x00670670  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [673 DeleteJournalWorldEntry] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0064bcc0 CServerExoApp::GetPlayerList -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x00731860 CSWSModule::DeleteWorldJournalEntry -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x28*,0xfffff82f*} | Steam {0x8*,0x28*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00670580: 16-byte aligned: YES
  00670580  55             PUSH EBP
  00670581  8B EC          MOV EBP,ESP
  00670583  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00670580
string literals (0):
direct callees (10):
  0x0051C2E0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0055F1D0  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BB30  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00670580 FUN_00670580 size=230 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00670580(undefined4 param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = 0;
  uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  local_18 = FUN_0051c2e0(param_1);
  uVar3 = *(undefined4 *)(DAT_00a1b4a4 + 8);
  local_8 = FUN_0051bd90();
  iVar1 = FUN_006fd9a0(&local_1c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (local_8 != 0) {
      FUN_0055f1d0(local_1c);
    }
    local_c = FUN_0051c2a0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar3,uVar2);
    if (local_c != 0) {
      local_14 = FUN_007a1720();
      while (local_14 != 0) {
        local_10 = FUN_00561430(local_14);
        if (local_10 != 0) {
          iVar1 = FUN_00632a20();
          if ((iVar1 != 0) && (local_18 != 0)) {
            FUN_0064bb30(local_10,local_1c);
          }
        }
        FUN_0058c370(&local_14);
      }
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00670670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00670670: 16-byte aligned: YES
  00670670  55             PUSH EBP
  00670671  8B EC          MOV EBP,ESP
  00670673  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00670670
string literals (0):
direct callees (10):
  0x0051C2E0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0055F2E0  (unnamed)
  0x0051C2A0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00632A20  (unnamed)
  0x0064BBA0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,EDX
```

## Card B15-1-31

GOG function: Global::ExecuteCommandEffectFury  gog 0x0078a210  callconv=__thiscall  param_size_bytes=8  gog_size=317  gog_retn=8
Candidate Steam address: 0x00674c30 (size 317, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00674810:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078a0a0 Global::ExecuteCommandEffectVPRegenModifier -> 0x00674ac0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078a350 Global::ExecuteCommandEffectForceDrain -> 0x00674d70  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [777 EffectFury] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734960 CGameEffect::Destructor -> Steam 0x005e4ca0  in candidate callees: yes
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0x30*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674c30: 16-byte aligned: YES
  00674c30  55             PUSH EBP
  00674c31  8B EC          MOV EBP,ESP
  00674c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674c30
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674c30 FUN_00674c30 size=317 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00674c30(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  int local_3c;
  undefined2 local_30;
  int local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097f5db;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_3c = 0;
  }
  else {
    local_3c = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_18 = local_3c;
  local_30 = (undefined2)DAT_009960d4;
  *(undefined2 *)(local_3c + 8) = local_30;
  piVar4 = &local_1c;
  uVar3 = *(undefined4 *)(param_1 + 8);
  FUN_0051c080(uVar3,piVar4);
  local_11 = FUN_0053dfb0(uVar3,piVar4);
  if ((local_11 == DAT_0099447f) && (local_1c != 0)) {
    FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
  }
  iVar2 = FUN_006fdad0(0,local_18);
  iVar1 = local_18;
  if (iVar2 == 0) {
    uVar3 = 0xfffff830;
  }
  else {
    if (local_18 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar1);
    }
    uVar3 = 0;
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00674810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674810: 16-byte aligned: YES
  00674810  55             PUSH EBP
  00674811  8B EC          MOV EBP,ESP
  00674813  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674810
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via EAX,EDX
```

## Card B15-1-32

GOG function: Global::ExecuteCommandEffectPoison  gog 0x0078b2e0  callconv=__thiscall  param_size_bytes=8  gog_size=318  gog_retn=8
Candidate Steam address: 0x00675d00 (size 318, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00675b70:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078b150 Global::ExecuteCommandEffectMovementSpeedIncrease -> 0x00675b70  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b420 Global::ExecuteCommandEffectResurrection -> 0x00675e40  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [250 EffectPoison] to this GOG function (InitCommands slot = nwscript declaration index)

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

## Card B15-1-33

GOG function: Global::ExecuteCommandEffectAbilityDecrease  gog 0x00786c40  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x00671660 (size 447, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671820:STR0/CALLEE0/SIG1/SRV0/VT0
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

## Card B15-1-34

GOG function: Global::ExecuteCommandEffectSkillDecrease  gog 0x0078bae0  callconv=__thiscall  param_size_bytes=8  gog_size=448  gog_retn=8
Candidate Steam address: 0x00676500 (size 448, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006766c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078b9c0 Global::ExecuteCommandEffectSeeInvisible -> 0x006763e0  gate_class=AUTO
- GOG-order neighbour after:  0x0078bca0 Global::ExecuteCommandEffectSkillIncrease -> 0x006766c0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [453 EffectSkillDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> Steam 0x006fdad0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0xa*,0x122*,0x123*} | Steam {0x8*,0xa*,0x122*,0x123*}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x8*,0x34*,0x8c*,0xff*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x34*,0x8c*,0xff*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00676500: 16-byte aligned: YES
  00676500  55             PUSH EBP
  00676501  8B EC          MOV EBP,ESP
  00676503  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00676500
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EAX
  +0x123 [dec 291]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00676500 FUN_00676500 size=448 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00676500(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int *piVar4;
  int local_44;
  undefined2 local_38;
  undefined2 local_30;
  int local_24;
  int local_20;
  int local_1c;
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a1ab;
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
      local_30 = (undefined2)DAT_00995ffc;
      *(undefined2 *)(local_44 + 8) = local_30;
      *(ushort *)(local_44 + 0xa [dec 10]) = *(ushort *)(local_44 + 0xa [dec 10]) & 0xffe7 | 8;
      if (((local_1c < 1) || ((int)(uint)*(byte *)(DAT_00a1b4d0 + 0x123) <= local_1c)) &&
         (local_1c != 0xff)) {
        local_38 = (undefined2)DAT_00995f1c;
        *(undefined2 *)(local_44 + 8) = local_38;
      }
      else {
        FUN_005e4e80(0,local_1c);
        FUN_005e4e80(1,local_14);
        FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      }
      piVar4 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar4);
      local_15 = FUN_0053dfb0(uVar3,piVar4);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
        FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      }
      iVar2 = FUN_006fdad0(0,local_20);
      if (iVar2 == 0) {
        uVar3 = 0xfffff830;
      }
      else {
        uVar3 = 0;
      }
    }
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x006766c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006766c0: 16-byte aligned: YES
  006766c0  55             PUSH EBP
  006766c1  8B EC          MOV EBP,ESP
  006766c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006766c0
string literals (0):
direct callees (8):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EAX
  +0x123 [dec 291]  x1  via ECX
```

## Card B15-1-35

GOG function: Global::ExecuteCommandEffectAttackDecrease  gog 0x00787700  callconv=__thiscall  param_size_bytes=8  gog_size=477  gog_retn=8
Candidate Steam address: 0x00672120 (size 477, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00672300:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00787400 Global::ExecuteCommandEffectAreaEffect -> 0x00671e20  gate_class=AUTO
- GOG-order neighbour after:  0x007878e0 Global::ExecuteCommandEffectAttackIncrease -> 0x00672300  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [447 EffectAttackDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00672120: 16-byte aligned: YES
  00672120  55             PUSH EBP
  00672121  8B EC          MOV EBP,ESP
  00672123  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00672120
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00672120 FUN_00672120 size=477 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00672120(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_38;
  int local_24;
  int local_20;
  int local_1c;
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959fcb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_14);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_1c);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if ((local_1c < 0) || (DAT_0099619c < local_1c)) {
        local_1c = DAT_00996184;
      }
      iVar2 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar2 == 0) {
        local_48 = 0;
      }
      else {
        local_48 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_20 = local_48;
      local_38 = (undefined2)DAT_00995f48;
      *(undefined2 *)(local_48 + 8) = local_38;
      *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_14);
      FUN_005e4e80(1,local_1c);
      FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_15 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
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

### decoy: 0x00672300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00672300: 16-byte aligned: YES
  00672300  55             PUSH EBP
  00672301  8B EC          MOV EBP,ESP
  00672303  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00672300
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

## Card B15-1-36

GOG function: Global::ExecuteCommandEffectWhirlwind  gog 0x0078d030  callconv=__thiscall  param_size_bytes=8  gog_size=252  gog_retn=8
Candidate Steam address: 0x00677a50 (size 252, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00677750:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078ce40 Global::ExecuteCommandEffectForcePushTargeted -> 0x00677860  gate_class=AUTO
- GOG-order neighbour after:  0x0078d130 Global::ExecuteCommandEffectBlasterDeflectionIncrease -> 0x00677b50  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [703 EffectWhirlWind] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xa*,0x24*,0x8c*,0xfffff830*} | Steam {0x4*,0x8*,0xa*,0x24*,0x8c*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00677a50: 16-byte aligned: YES
  00677a50  55             PUSH EBP
  00677a51  8B EC          MOV EBP,ESP
  00677a53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00677a50 FUN_00677a50 size=252 ret=8 callers=0 callees=6

undefined4 FUN_00677a50(void)

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
  FUN_005e4e80(0,10);
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

### decoy: 0x00677750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677750: 16-byte aligned: YES
  00677750  55             PUSH EBP
  00677751  8B EC          MOV EBP,ESP
  00677753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677750
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card B15-1-37

GOG function: Global::ExecuteCommandSetGalaxyMapPlanetAvailable  gog 0x007acb00  callconv=__thiscall  param_size_bytes=8  gog_size=104  gog_retn=8
Candidate Steam address: 0x00697520 (size 103, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandGetGalaxyMapPlanetAvailable | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006974b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007acab0 Global::ExecuteCommandShowGalaxyMapGui -> 0x00697460  gate_class=REVIEW
- GOG-order neighbour after:  0x007acb70 Global::ExecuteCommandGetGalaxyMapPlanetAvailable -> 0x00697520  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [742 SetPlanetAvailable] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x007046e0 CSWPartyTable::SetPlanetAvailable -> Steam 0x005fed30  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0xfffff82f*} | Steam {0x8*,0x10*,0xfffff82f*,0xfffff830}

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

## Card B15-1-38

GOG function: Global::ExecuteCommandDistanceConversions  gog 0x00786270  callconv=__thiscall  param_size_bytes=8  gog_size=124  gog_retn=8
Candidate Steam address: 0x00670c90 (size 124, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::CanEquipWeapon2 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: StackPushFloat 0x6fda00); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00670ae0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007860c0 Global::ExecuteCommandDisplayFeedBackText -> 0x00670ae0  gate_class=AUTO
- GOG-order neighbour after:  0x007862f0 Global::ExecuteCommandDoDoorAction -> 0x00670d10  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [218 FeetToMeters, 219 YardsToMeters] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00640c00 CVirtualMachine::StackPopFloat -> Steam 0x006fd9e0  in candidate callees: yes
  0x00640c20 CVirtualMachine::StackPushFloat -> Steam 0x006fda00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x8*,0x10*,0xda*,0xdb*,0xfffff82f*,0xfffff830*} | Steam {0x8*,0x10*,0xda*,0xdb*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00670c90: 16-byte aligned: YES
  00670c90  55             PUSH EBP
  00670c91  8B EC          MOV EBP,ESP
  00670c93  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00670c90
string literals (0):
direct callees (2):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FDA00  CVirtualMachine::StackPushFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00670c90 FUN_00670c90 size=124 ret=8 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00670c90(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  float local_c;
  float local_8;
  
  iVar1 = FUN_006fd9e0(&local_c);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    if (param_1 == 0xda) {
      local_8 = local_c * (float)_DAT_00998c88;
    }
    else if (param_1 == 0xdb) {
      local_8 = local_c * (float)_DAT_00998c80;
    }
    iVar1 = FUN_006fda00(local_8);
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

### decoy: 0x00670ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00670ae0: 16-byte aligned: YES
  00670ae0  55             PUSH EBP
  00670ae1  8B EC          MOV EBP,ESP
  00670ae3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00670ae0
string literals (2):
  0x00996E34  at 00670b9e  'StrRef'
  0x00998C70  at 00670c38  'BAD STRREF'
direct callees (9):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0073F4D0  CClientExoApp::GetGameObject
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0071DEE0  (unnamed)
  0x007F2200  CSWCObject::SetFeedbackInfo
struct offsets touched (top 4):
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x10C [dec 268]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

## Card B15-1-39

GOG function: Global::ExecuteCommandEffectResurrection  gog 0x0078b420  callconv=__thiscall  param_size_bytes=8  gog_size=379  gog_retn=8
Candidate Steam address: 0x00675e40 (size 379, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00675b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078b2e0 Global::ExecuteCommandEffectPoison -> 0x00675d00  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b5a0 Global::ExecuteCommandEffectSavingThrowDecrease -> 0x00675fc0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [82 EffectResurrection] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x00675e40: 16-byte aligned: YES
  00675e40  55             PUSH EBP
  00675e41  8B EC          MOV EBP,ESP
  00675e43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675e40
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
// FN 00675e40 FUN_00675e40 size=379 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00675e40(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
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
    local_34 = (undefined2)DAT_00995f2c;
    *(undefined2 *)(local_40 + 8) = local_34;
    *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xffe7 | 8;
    FUN_005e4e80(0,local_20);
    piVar5 = &local_1c;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,piVar5);
    local_11 = FUN_0053dfb0(uVar3,piVar5);
    if ((local_11 == DAT_0099447f) && (local_1c != 0)) {
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

## Card B15-1-40

GOG function: Global::ExecuteCommandEffectBlasterDeflectionIncrease  gog 0x0078d130  callconv=__thiscall  param_size_bytes=8  gog_size=282  gog_retn=8
Candidate Steam address: 0x00677b50 (size 282, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677c70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078d030 Global::ExecuteCommandEffectWhirlwind -> 0x00677a50  gate_class=REVIEW
- GOG-order neighbour after:  0x0078d250 Global::ExecuteCommandEffectBlasterDeflectionDecrease -> 0x00677c70  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [469 EffectBlasterDeflectionIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x00677b50: 16-byte aligned: YES
  00677b50  55             PUSH EBP
  00677b51  8B EC          MOV EBP,ESP
  00677b53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00677b50 FUN_00677b50 size=282 ret=8 callers=0 callees=7

undefined4 FUN_00677b50(void)

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
    local_2c = (undefined2)DAT_00996088;
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

### decoy: 0x00677c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677c70: 16-byte aligned: YES
  00677c70  55             PUSH EBP
  00677c71  8B EC          MOV EBP,ESP
  00677c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-1-41

GOG function: Global::ExecuteCommandAddMultiClass  gog 0x00781d80  callconv=__thiscall  param_size_bytes=8  gog_size=483  gog_retn=8
Candidate Steam address: 0x0066c7a0 (size 483, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandLevelUpHenchman | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 6 (callees: AddExperience 0x6b7060, GetExpNeededForLevelUp 0x6b72e0, CanLevelUp 0x6b9790); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0066c990:STR0/CALLEE0/SIG0.367/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00781c40 Global::ExecuteCommandAddJournalWorldEntryStrref -> 0x0066c660  gate_class=AUTO
- GOG-order neighbour after:  0x00781f70 Global::ExecuteCommandAddPartyMember -> 0x0066c990  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [389 AddMultiClass] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam 0x006fd9a0  in candidate callees: yes
  0x00640d10 CVirtualMachine::StackPopObject -> Steam 0x006fdaf0  in candidate callees: yes
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006f34e0 CSWSCreatureStats::AddExperience -> Steam 0x006b7060  in candidate callees: yes
  0x006f3760 CSWSCreatureStats::GetExpNeededForLevelUp -> Steam 0x006b72e0  in candidate callees: yes
  0x006f5c10 CSWSCreatureStats::CanLevelUp -> Steam 0x006b9790  in candidate callees: yes
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x68*,0x26c*,0x1198*,0x1200*} | Steam {0x4*,0x8*,0x68*,0x26c*,0x1198*,0x1200*}
imm sets (* = in both, 3 common): GOG {0x8*,0x6c*,0xfffff82f*} | Steam {0x8*,0x6c*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0066c7a0: 16-byte aligned: YES
  0066c7a0  55             PUSH EBP
  0066c7a1  8B EC          MOV EBP,ESP
  0066c7a3  83 EC 6C       SUB ESP,0x6c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0066c7a0
string literals (0):
direct callees (11):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006B72E0  CSWSCreatureStats::GetExpNeededForLevelUp
  0x006BDCD0  CSWSCreatureStats::AddClass
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FE0C0  CSWPartyTable::DistributeExperience
  0x006B7060  CSWSCreatureStats::AddExperience
  0x0072FB00  CClientExoApp::GetClientOptions
  0x006B9790  CSWSCreatureStats::CanLevelUp
  0x006BEDF0  CSWSCreatureStats::AutoLevelup
struct offsets touched (top 6):
  +0x1198 [dec 4504]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0x68 [dec 104]  x2  via EAX
  +0x26C [dec 620]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0x1200 [dec 4608]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0066c7a0 FUN_0066c7a0 size=483 ret=8 callers=0 callees=11

undefined4 FUN_0066c7a0(void)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 extraout_var;
  undefined4 local_18;
  undefined1 local_14 [4];
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fd9a0(local_14);
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
      local_8 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      local_c = FUN_0051c100(local_18);
      local_10 = 0;
      if (*(int *)(local_c + 0x1200) == 0) {
        iVar1 = *(int *)(*(int *)(local_c + 0x1198) + 0x68);
        iVar3 = FUN_006b72e0(uVar2);
        iVar3 = iVar3 - iVar1;
        FUN_006bdcd0(local_14[0],0);
        FUN_006b7060(iVar3);
        *(int *)(local_c + 0x26c) = *(int *)(local_c + 0x26c) + iVar3;
        iVar1 = FUN_0072fb00();
        if (((*(uint *)(iVar1 + 8) & 1) != 0) && (iVar1 = FUN_006b9790(), iVar1 != 0)) {
          FUN_006bedf0(1);
        }
      }
      else {
        iVar1 = *(int *)(*(int *)(local_c + 0x1198) + 0x68);
        iVar3 = FUN_006b72e0(uVar2);
        iVar3 = iVar3 - iVar1;
        FUN_006bdcd0(local_14[0],0);
        iVar1 = FUN_0051c8d0();
        if (iVar1 != 0) {
          uVar2 = 1;
          FUN_0051c8d0(iVar3,1);
          FUN_005fe0c0(iVar3,uVar2);
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x0066c990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066c990: 16-byte aligned: YES
  0066c990  55             PUSH EBP
  0066c991  8B EC          MOV EBP,ESP
  0066c993  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066c990
string literals (0):
direct callees (10):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F8D80  CSWPartyTable::AddMember
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00775860  (unnamed)
  0x0058A330  CSWSCreature::SetPartyMemberFlag
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

## Card B15-1-42

GOG function: Global::ExecuteCommandEffectInvisibility  gog 0x0078a9f0  callconv=__thiscall  param_size_bytes=8  gog_size=386  gog_retn=8
Candidate Steam address: 0x00675410 (size 386, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00675260:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078a840 Global::ExecuteCommandEffectImmunity -> 0x00675260  gate_class=REVIEW
- GOG-order neighbour after:  0x0078ab80 Global::ExecuteCommandEffectLinkEffects -> 0x006755a0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [457 EffectInvisibility] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0x30*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x30*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00675410: 16-byte aligned: YES
  00675410  55             PUSH EBP
  00675411  8B EC          MOV EBP,ESP
  00675413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675410
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00675410 FUN_00675410 size=386 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00675410(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_3c;
  undefined2 local_30;
  undefined2 local_2c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952f1b;
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
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_3c;
    if (((local_18 == 1) || (local_18 == 2)) || (local_18 == 4)) {
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      local_30 = (undefined2)DAT_00995fd8;
      *(undefined2 *)(local_14 + 8) = local_30;
      *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_18);
      FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    }
    else {
      local_2c = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_3c + 8) = local_2c;
    }
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

### decoy: 0x00675260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675260: 16-byte aligned: YES
  00675260  55             PUSH EBP
  00675261  8B EC          MOV EBP,ESP
  00675263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675260
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x005E4E80  CGameEffect::SetInteger
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EAX,ECX
  +0x122 [dec 290]  x1  via EDX
```

## Card B15-1-43

GOG function: Global::ExecuteCommandEffectSavingThrowIncrease  gog 0x0078b7b0  callconv=__thiscall  param_size_bytes=8  gog_size=528  gog_retn=8
Candidate Steam address: 0x006761d0 (size 528, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00675fc0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078b5a0 Global::ExecuteCommandEffectSavingThrowDecrease -> 0x00675fc0  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b9c0 Global::ExecuteCommandEffectSeeInvisible -> 0x006763e0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [117 EffectSavingThrowIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x3*,0x4*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006761d0: 16-byte aligned: YES
  006761d0  55             PUSH EBP
  006761d1  8B EC          MOV EBP,ESP
  006761d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006761d0
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
  +0x8 [dec 8]  x4  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006761d0 FUN_006761d0 size=528 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_006761d0(int param_1,undefined4 param_2,int param_3)

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
  char local_15;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959f5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = 0;
  iVar2 = FUN_006fd9a0(&local_20);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    if ((local_20 < 0) || (3 < local_20)) {
      local_20 = 0;
    }
    iVar2 = FUN_006fd9a0(&local_14);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else if ((param_3 < 3) || (iVar2 = FUN_006fd9a0(&local_28), iVar2 != 0)) {
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
      local_3c = (undefined2)DAT_00995f84;
      *(undefined2 *)(local_4c + 8) = local_3c;
      *(ushort *)(local_4c + 0xa [dec 10]) = *(ushort *)(local_4c + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_14);
      FUN_005e4e80(1,local_20);
      FUN_005e4e80(2,local_28);
      FUN_005e4e80(3,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_24;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_15 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_15 == DAT_0099447f) && (local_24 != 0)) {
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
    }
/* ... 4 more lines truncated by --max-lines */
```

### decoy: 0x00675fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675fc0: 16-byte aligned: YES
  00675fc0  55             PUSH EBP
  00675fc1  8B EC          MOV EBP,ESP
  00675fc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675fc0
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
  +0x8 [dec 8]  x4  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EDX
```

## Card B15-1-44

GOG function: Global::ExecuteCommandEffectForceShield  gog 0x007897f0  callconv=__thiscall  param_size_bytes=8  gog_size=312  gog_retn=8
Candidate Steam address: 0x00674210 (size 312, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00674530:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789600 Global::ExecuteCommandEffectDamageResistance -> 0x00674020  gate_class=REVIEW
- GOG-order neighbour after:  0x00789930 Global::ExecuteCommandEffectDamageShield -> 0x00674350  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [459 EffectForceShield] to this GOG function (InitCommands slot = nwscript declaration index)

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
prologue for 0x00674210: 16-byte aligned: YES
  00674210  55             PUSH EBP
  00674211  8B EC          MOV EBP,ESP
  00674213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674210
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00674210 FUN_00674210 size=312 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00674210(int param_1)

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
  local_18 = 0;
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
    FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
    local_2c = (undefined2)DAT_009960c4;
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

### decoy: 0x00674530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674530: 16-byte aligned: YES
  00674530  55             PUSH EBP
  00674531  8B EC          MOV EBP,ESP
  00674533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674530
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-45

GOG function: Global::ExecuteCommandEffectHeal  gog 0x0078a500  callconv=__thiscall  param_size_bytes=8  gog_size=434  gog_retn=8
Candidate Steam address: 0x00674f20 (size 434, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00674d70:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078a350 Global::ExecuteCommandEffectForceDrain -> 0x00674d70  gate_class=AUTO
- GOG-order neighbour after:  0x0078a6c0 Global::ExecuteCommandEffectHitPointChangeWhenDying -> 0x006750e0  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [78 EffectHeal] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x30*} | Steam {0x8*,0xa*,0x30*}
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
prologue for 0x00674f20: 16-byte aligned: YES
  00674f20  55             PUSH EBP
  00674f21  8B EC          MOV EBP,ESP
  00674f23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674f20
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
// FN 00674f20 FUN_00674f20 size=434 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_00674f20(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
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
  if (param_3 == 1) {
    iVar3 = FUN_006fd9a0(&local_20);
    if (iVar3 == 0) {
      uVar2 = 0xfffff82f;
    }
    else {
      iVar3 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar3 == 0) {
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
        local_34 = (undefined2)DAT_00995fb8;
        *(undefined2 *)(local_44 + 8) = local_34;
        FUN_005e4e80(0,local_20);
      }
      ppiVar5 = &local_1c;
      uVar2 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar2,ppiVar5);
      local_11 = FUN_0053dfb0(uVar2,ppiVar5);
      if ((local_11 == DAT_0099447f) && (iVar3 = (**(code **)(*local_1c + 0x30))(), iVar3 != 0)) {
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

### decoy: 0x00674d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674d70: 16-byte aligned: YES
  00674d70  55             PUSH EBP
  00674d71  8B EC          MOV EBP,ESP
  00674d73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674d70
string literals (0):
direct callees (11):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x005E4E80  CGameEffect::SetInteger
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E5EA0  CGameEffect::SetCreator
  0x006FDAD0  CVirtualMachine::StackPushEngineStructure
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
```

## Card B15-1-46

GOG function: CSWVirtualMachineCommands::ExecuteCommandEffectTemporaryForcePoints  gog 0x0078c440  callconv=__thiscall  param_size_bytes=8  gog_size=418  gog_retn=8
Candidate Steam address: 0x00676e60 (size 418, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00677010:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078c300 Global::ExecuteCommandEffectForceResistanceIncrease -> 0x00676d20  gate_class=REVIEW
- GOG-order neighbour after:  0x0078c5f0 Global::ExecuteCommandEffectTemporaryHP -> 0x00677010  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [156 EffectTemporaryForcePoints] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x30*} | Steam {0x8*,0xa*,0x30*}
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

### decoy: 0x00677010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00677010: 16-byte aligned: YES
  00677010  55             PUSH EBP
  00677011  8B EC          MOV EBP,ESP
  00677013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00677010
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

## Card B15-1-47

GOG function: CSWVirtualMachineCommands::ExecuteCommandActionDoCommand  gog 0x0077efb0  callconv=(blank)  param_size_bytes=0  gog_size=118  gog_retn=8
Candidate Steam address: 0x006699d0 (size 118, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: StackPopCommand 0x6fdb30)
- best decoy (neighbour/other candidate) scores: 0x00669a50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0077eef0 Global::ExecuteCommandActionBarkString -> 0x00669910  gate_class=AUTO
- GOG-order neighbour after:  0x0077f030 CSWVirtualMachineCommands::ExecuteCommandActionEquipMostDamagingMelee -> 0x00669a50  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [294 ActionDoCommand] to this GOG function (InitCommands slot = nwscript declaration index)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x00640d50 CVirtualMachine::StackPopCommand -> Steam 0x006fdb30  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x006bd530 CSWSObject::AddDoCommandAction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0xfffff82f*} | Steam {0x8*,0x14*,0xfffff82f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x006699d0: 16-byte aligned: YES
  006699d0  55             PUSH EBP
  006699d1  8B EC          MOV EBP,ESP
  006699d3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006699d0
string literals (0):
direct callees (4):
  0x006FDB30  CVirtualMachine::StackPopCommand
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005E1340  CSWSObject::AddDoCommandAction
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006699d0 FUN_006699d0 size=118 ret=8 callers=0 callees=4

undefined4 FUN_006699d0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  int extraout_var;
  undefined4 local_10;
  undefined1 local_c [7];
  char local_5;
  
  local_10 = 0;
  iVar1 = FUN_006fdb30(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    puVar3 = local_c;
    uVar2 = *(undefined4 *)(extraout_var + 8);
    FUN_0051c080(uVar2,puVar3,extraout_var,*(undefined4 *)(DAT_00a1b4a4 + 8));
    local_5 = FUN_0053dfb0(uVar2,puVar3);
    if (local_5 == DAT_0099447f) {
      FUN_005e1340(local_10);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00669a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00669a50: 16-byte aligned: YES
  00669a50  55             PUSH EBP
  00669a51  8B EC          MOV EBP,ESP
  00669a53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00669a50
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00580330  CSWSCreature::EquipMostDamagingMeleeWeapon
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

## Card B15-1-48

GOG function: Global::ExecuteCommandEffectACIncrease  gog 0x007871e0  callconv=__thiscall  param_size_bytes=8  gog_size=534  gog_retn=8
Candidate Steam address: 0x00671c00 (size 534, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006719e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00786fc0 Global::ExecuteCommandEffectACDecrease -> 0x006719e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00787400 Global::ExecuteCommandEffectAreaEffect -> 0x00671e20  gate_class=AUTO
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [115 EffectACIncrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00671c00: 16-byte aligned: YES
  00671c00  55             PUSH EBP
  00671c01  8B EC          MOV EBP,ESP
  00671c03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00671c00
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
// FN 00671c00 FUN_00671c00 size=534 ret=8 callers=0 callees=10

undefined4 __thiscall FUN_00671c00(int param_1,undefined4 param_2,int param_3)

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
        local_3c = (undefined2)DAT_00995fdc;
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

### decoy: 0x006719e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006719e0: 16-byte aligned: YES
  006719e0  55             PUSH EBP
  006719e1  8B EC          MOV EBP,ESP
  006719e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B15-1-49

GOG function: Global::ExecuteCommandVector  gog 0x007aaac0  callconv=(blank)  param_size_bytes=0  gog_size=140  gog_retn=8
Candidate Steam address: 0x00695500 (size 378, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandVectorConversions | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.72 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00695470:STR0/CALLEE1/SIG1/SRV0/VT0
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
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x10,0xfffff82f*,0xfffff830*} | Steam {0x5,0x8*,0xc*,0x48,0x90,0x91,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
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

### decoy: 0x00695470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00695470: 16-byte aligned: YES
  00695470  55             PUSH EBP
  00695471  8B EC          MOV EBP,ESP
  00695473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00695470
string literals (0):
direct callees (2):
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x006FDA40  CVirtualMachine::StackPushVector
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B15-1-50

GOG function: Global::ExecuteCommandEffectDamageDecrease  gog 0x00788c70  callconv=__thiscall  param_size_bytes=8  gog_size=496  gog_retn=8
Candidate Steam address: 0x00673690 (size 496, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00673270:STR0/CALLEE0/SIG0.773/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007889d0 Global::ExecuteCommandEffectDamage -> 0x006733f0  gate_class=AUTO
- GOG-order neighbour after:  0x00788e60 Global::ExecuteCommandEffectDamageImmunityDecrease -> 0x00673880  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [448 EffectDamageDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x8*,0xa*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0xa*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00673690: 16-byte aligned: YES
  00673690  55             PUSH EBP
  00673691  8B EC          MOV EBP,ESP
  00673693  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00673690
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
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00673690 FUN_00673690 size=496 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00673690(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_38;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959fcb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_18);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_24);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      if ((local_18 < 1) || (10 < local_18)) {
        local_18 = 0;
      }
      if ((local_24 < 0) || (DAT_00996144 < local_24)) {
        local_24 = DAT_0099611c;
      }
      iVar2 = FUN_00919723(0x8c,uVar1);
      local_8 = 0;
      if (iVar2 == 0) {
        local_48 = 0;
      }
      else {
        local_48 = FUN_005e4920(1);
      }
      local_8 = 0xffffffff;
      local_1c = local_48;
      local_38 = (undefined2)DAT_00995f54;
      *(undefined2 *)(local_48 + 8) = local_38;
      *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e4e80(0,local_18);
      FUN_005e4e80(1,local_24);
      FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      piVar5 = &local_20;
      uVar3 = *(undefined4 *)(param_1 + 8);
      FUN_0051c080(uVar3,piVar5);
      local_11 = FUN_0053dfb0(uVar3,piVar5);
      if ((local_11 == DAT_0099447f) && (local_20 != 0)) {
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
  }
  ExceptionList = local_10;
  return uVar3;
/* ... 1 more lines truncated by --max-lines */
```

### decoy: 0x00673270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00673270: 16-byte aligned: YES
  00673270  55             PUSH EBP
  00673271  8B EC          MOV EBP,ESP
  00673273  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00673270
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x122 [dec 290]  x1  via EAX
```

## Card B15-1-51

GOG function: Global::ExecuteCommandGetIsDusk  gog 0x007953a0  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067fa20 (size 895, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.22 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067fdc0:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0x8*,0x1dc} | Steam {0x8*,0x8d,0xd8,0xfc,0x2a0,0x2ac,0x304,0x328,0x3f0,0x410,0x1198}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14*,0xfffff830*} | Steam {0x4*,0x6,0x8*,0x14*,0x5d,0x80,0x151,0x2726,0x275c,0xfffff82f,0xfffff830*}

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

## Card B15-1-52

GOG function: Global::ExecuteCommandEffectMissChance  gog 0x0078acf0  callconv=__thiscall  param_size_bytes=8  gog_size=367  gog_retn=8
Candidate Steam address: 0x00675710 (size 367, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00675880:STR0/CALLEE0/SIG0.889/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078ab80 Global::ExecuteCommandEffectLinkEffects -> 0x006755a0  gate_class=AUTO
- GOG-order neighbour after:  0x0078ae60 Global::ExecuteCommandEffectModifyAttacks -> 0x00675880  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [477 EffectMissChance] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 8 common): GOG {0x4*,0x8*,0x2c*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x4*,0x8*,0x2c*,0x64*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00675710: 16-byte aligned: YES
  00675710  55             PUSH EBP
  00675711  8B EC          MOV EBP,ESP
  00675713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00675710
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00675710 FUN_00675710 size=367 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_00675710(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int local_38;
  undefined2 local_30;
  undefined2 local_2c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
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
      local_38 = 0;
    }
    else {
      local_38 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_14 = local_38;
    if ((local_18 < 1) || (100 < local_18)) {
      local_2c = (undefined2)DAT_00995f1c;
      *(undefined2 *)(local_38 + 8) = local_2c;
    }
    else {
      *(ushort *)(local_38 + 0xa [dec 10]) = *(ushort *)(local_38 + 0xa [dec 10]) & 0xffe7 | 8;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 8));
      local_30 = (undefined2)DAT_0099604c;
      *(undefined2 *)(local_14 + 8) = local_30;
      FUN_005e4e80(0,local_18);
      FUN_005e4e80(1,DAT_00996254);
    }
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

### decoy: 0x00675880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00675880: 16-byte aligned: YES
  00675880  55             PUSH EBP
  00675881  8B EC          MOV EBP,ESP
  00675883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00675880
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
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via ECX,EDX
```

## Card B15-1-53

GOG function: Global::ExecuteCommandEffectFactionModifier  gog 0x00789b10  callconv=__thiscall  param_size_bytes=8  gog_size=359  gog_retn=8
Candidate Steam address: 0x00674530 (size 359, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006746a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789930 Global::ExecuteCommandEffectDamageShield -> 0x00674350  gate_class=REVIEW
- GOG-order neighbour after:  0x00789c80 Global::ExecuteCommandEffectForceBody -> 0x006746a0  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [849 EffectFactionModifier] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674530: 16-byte aligned: YES
  00674530  55             PUSH EBP
  00674531  8B EC          MOV EBP,ESP
  00674533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674530
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674530 FUN_00674530 size=359 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00674530(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_40;
  undefined2 local_34;
  int local_20;
  undefined4 local_1c;
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
  iVar2 = FUN_006fd9a0(&local_1c);
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
    local_34 = (undefined2)DAT_009960e4;
    *(undefined2 *)(local_40 + 8) = local_34;
    FUN_005e4e80(0,local_1c);
    piVar5 = &local_20;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,piVar5);
    local_11 = FUN_0053dfb0(uVar3,piVar5);
    if ((local_11 == DAT_0099447f) && (local_20 != 0)) {
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

### decoy: 0x006746a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006746a0: 16-byte aligned: YES
  006746a0  55             PUSH EBP
  006746a1  8B EC          MOV EBP,ESP
  006746a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006746a0
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-54

GOG function: Global::ExecuteCommandEffectMovementSpeedDecrease  gog 0x0078afc0  callconv=__thiscall  param_size_bytes=8  gog_size=395  gog_retn=8
Candidate Steam address: 0x006759e0 (size 395, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00675b70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0078ae60 Global::ExecuteCommandEffectModifyAttacks -> 0x00675880  gate_class=REVIEW
- GOG-order neighbour after:  0x0078b150 Global::ExecuteCommandEffectMovementSpeedIncrease -> 0x00675b70  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [451 EffectMovementSpeedDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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

## Card B15-1-55

GOG function: Global::ExecuteCommandEffectVPRegenModifier  gog 0x0078a0a0  callconv=__thiscall  param_size_bytes=8  gog_size=359  gog_retn=8
Candidate Steam address: 0x00674ac0 (size 359, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00674950:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789f30 Global::ExecuteCommandEffectFPRegenModifier -> 0x00674950  gate_class=REVIEW
- GOG-order neighbour after:  0x0078a210 Global::ExecuteCommandEffectFury -> 0x00674c30  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [780 EffectVPRegenModifier] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00674ac0: 16-byte aligned: YES
  00674ac0  55             PUSH EBP
  00674ac1  8B EC          MOV EBP,ESP
  00674ac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00674ac0
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00674ac0 FUN_00674ac0 size=359 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00674ac0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_40;
  undefined2 local_34;
  int local_20;
  int local_1c;
  undefined4 local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0bb;
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
      local_40 = 0;
    }
    else {
      local_40 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_1c = local_40;
    local_34 = (undefined2)DAT_009960dc;
    *(undefined2 *)(local_40 + 8) = local_34;
    FUN_005e4e80(0,local_18);
    piVar5 = &local_20;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,piVar5);
    local_11 = FUN_0053dfb0(uVar3,piVar5);
    if ((local_11 == DAT_0099447f) && (local_20 != 0)) {
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
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x00674950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674950: 16-byte aligned: YES
  00674950  55             PUSH EBP
  00674951  8B EC          MOV EBP,ESP
  00674953  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674950
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-56

GOG function: Global::ExecuteCommandEffectDamageReduction  gog 0x007893f0  callconv=__thiscall  param_size_bytes=8  gog_size=526  gog_retn=8
Candidate Steam address: 0x00673e10 (size 526, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00674020:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789200 Global::ExecuteCommandEffectDamageIncrease -> 0x00673c20  gate_class=REVIEW
- GOG-order neighbour after:  0x00789600 Global::ExecuteCommandEffectDamageResistance -> 0x00674020  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [119 EffectDamageReduction] to this GOG function (InitCommands slot = nwscript declaration index)

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
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x4*,0x8*,0x3c*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00673e10: 16-byte aligned: YES
  00673e10  55             PUSH EBP
  00673e11  8B EC          MOV EBP,ESP
  00673e13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00673e10
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
  +0x8 [dec 8]  x4  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00673e10 FUN_00673e10 size=526 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_00673e10(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  int *piVar5;
  int local_48;
  undefined2 local_3c;
  int local_28;
  int local_24;
  uint local_20;
  int local_1c;
  uint local_18;
  char local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a05b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006fd9a0(&local_20);
  if (iVar2 == 0) {
    uVar3 = 0xfffff82f;
  }
  else {
    iVar2 = FUN_006fd9a0(&local_24);
    if (iVar2 == 0) {
      uVar3 = 0xfffff82f;
    }
    else {
      iVar2 = FUN_006fd9a0(&local_18);
      if (iVar2 == 0) {
        uVar3 = 0xfffff82f;
      }
      else {
        if ((local_24 < 0) || (DAT_0099616c < local_24)) {
          local_24 = DAT_00996154;
        }
        local_20 = ((int)local_20 < 0) - 1 & local_20;
        local_18 = ((int)local_18 < 0) - 1 & local_18;
        iVar2 = FUN_00919723(0x8c,uVar1);
        local_8 = 0;
        if (iVar2 == 0) {
          local_48 = 0;
        }
        else {
          local_48 = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        local_1c = local_48;
        local_3c = (undefined2)DAT_00995f4c;
        *(undefined2 *)(local_48 + 8) = local_3c;
        *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xffe7 | 8;
        FUN_005e4e80(0,local_20);
        FUN_005e4e80(1,local_24);
        FUN_005e4e80(2,local_18);
        piVar5 = &local_28;
        uVar3 = *(undefined4 *)(param_1 + 8);
        FUN_0051c080(uVar3,piVar5);
        local_11 = FUN_0053dfb0(uVar3,piVar5);
        if ((local_11 == DAT_0099447f) && (local_28 != 0)) {
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
/* ... 7 more lines truncated by --max-lines */
```

### decoy: 0x00674020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674020: 16-byte aligned: YES
  00674020  55             PUSH EBP
  00674021  8B EC          MOV EBP,ESP
  00674023  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674020
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
  +0xA [dec 10]  x2  via EAX,ECX
```

## Card B15-1-57

GOG function: Global::ExecuteCommandGetIsDusk  gog 0x007953a0  callconv=__thiscall  param_size_bytes=8  gog_size=106  gog_retn=8
Candidate Steam address: 0x0067fe30 (size 137, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0067fdc0:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 1 common): GOG {0x8*,0x1dc} | Steam {0x8*,0x1148}
imm sets (* = in both, 2 common): GOG {0x4,0x8*,0x14,0xfffff830*} | Steam {0x8*,0x18,0x7f000000,0xfffff82f,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0067fe30: 16-byte aligned: YES
  0067fe30  55             PUSH EBP
  0067fe31  8B EC          MOV EBP,ESP
  0067fe33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0067fe30
string literals (0):
direct callees (3):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x1148 [dec 4424]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0067fe30 FUN_0067fe30 size=137 ret=8 callers=0 callees=3

undefined4 FUN_0067fe30(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  iVar1 = FUN_006fdaf0(&local_10);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    local_8 = FUN_0051c100(local_10);
    if ((local_8 != 0) && (*(int *)(local_8 + 0x1148) != 0x7f000000)) {
      local_c = 1;
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

## Card B15-1-58

GOG function: Global::ExecuteCommandEffectACDecrease  gog 0x00786fc0  callconv=__thiscall  param_size_bytes=8  gog_size=534  gog_retn=8
Candidate Steam address: 0x006719e0 (size 534, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671c00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00786e00 Global::ExecuteCommandEffectAbilityIncrease -> 0x00671820  gate_class=REVIEW
- GOG-order neighbour after:  0x007871e0 Global::ExecuteCommandEffectACIncrease -> 0x00671c00  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [450 EffectACDecrease] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x122*} | Steam {0x8*,0xa*,0x122*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x40*,0x8c*,0xfffff82f*,0xfffff830*,0xffffffe7*}

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

### decoy: 0x00671c00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00671c00: 16-byte aligned: YES
  00671c00  55             PUSH EBP
  00671c01  8B EC          MOV EBP,ESP
  00671c03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00671c00
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

## Card B15-1-59

GOG function: Global::ExecuteCommandEffectForceBody  gog 0x00789c80  callconv=__thiscall  param_size_bytes=8  gog_size=359  gog_retn=8
Candidate Steam address: 0x006746a0 (size 359, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00674530:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00789b10 Global::ExecuteCommandEffectFactionModifier -> 0x00674530  gate_class=REVIEW
- GOG-order neighbour after:  0x00789df0 Global::ExecuteCommandEffectForceSight -> 0x00674810  gate_class=REVIEW
- GOG-side fact: the GOG command table dispatches nwscript.nss function(s) [770 EffectForceBody] to this GOG function (InitCommands slot = nwscript declaration index)

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
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*} | Steam {0x4*,0x8*,0x34*,0x8c*,0xfffff82f*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006746a0: 16-byte aligned: YES
  006746a0  55             PUSH EBP
  006746a1  8B EC          MOV EBP,ESP
  006746a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006746a0
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006746a0 FUN_006746a0 size=359 ret=8 callers=0 callees=10

undefined4 __fastcall FUN_006746a0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
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
    local_34 = (undefined2)DAT_009960d0;
    *(undefined2 *)(local_40 + 8) = local_34;
    FUN_005e4e80(0,local_20);
    piVar5 = &local_1c;
    uVar3 = *(undefined4 *)(param_1 + 8);
    FUN_0051c080(uVar3,piVar5);
    local_11 = FUN_0053dfb0(uVar3,piVar5);
    if ((local_11 == DAT_0099447f) && (local_1c != 0)) {
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

### decoy: 0x00674530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00674530: 16-byte aligned: YES
  00674530  55             PUSH EBP
  00674531  8B EC          MOV EBP,ESP
  00674533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00674530
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
struct offsets touched (top 1):
  +0x8 [dec 8]  x4  via ECX,EDX
```

## Card B15-1-60

GOG function: Global::ExecuteCommandEffectAbilityIncrease  gog 0x00786e00  callconv=__thiscall  param_size_bytes=8  gog_size=447  gog_retn=8
Candidate Steam address: 0x00671820 (size 447, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00671660:STR0/CALLEE0/SIG1/SRV0/VT0
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
