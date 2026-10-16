# Review cards B13a R1 pilot part 1

Follow steam_db_batches/REVIEWER_BRIEF.md. One TSV row per card, card order, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | label_suspect | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence.

## Card B13a-1-20

GOG function: CSWCCreature::UnloadMonkVisuals  gog 0x0041f6b0  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=0
Candidate Steam address: 0x007737e0 (size 116, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00773860:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0041f5a0 CSWCCreature::SetDesiredOrientationForLock -> 0x007736d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f730 CSWCCreature::ResetAnimationState -> 0x007737e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x10*,0x18*,0x70*,0x130*,0x224*} | Steam {0x10*,0x18*,0x70*,0x130*,0x224*}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x007737e0: 16-byte aligned: YES
  007737e0  55             PUSH EBP
  007737e1  8B EC          MOV EBP,ESP
  007737e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007737e0
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007737e0 FUN_007737e0 size=116 ret=0 callers=0 callees=1

void FUN_007737e0(int *param_1,undefined4 param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = (**(code **)(*param_1 + 0x70))();
  if (iVar1 != 0) {
    FUN_0071b3f0(*(undefined2 *)(*(int *)(iVar1 + 0x224) + 0x18),DAT_00a0fde8,&local_8);
    *param_3 = local_8;
  }
  return;
}
```

### decoy: 0x00773860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773860: 16-byte aligned: YES
  00773860  55             PUSH EBP
  00773861  8B EC          MOV EBP,ESP
  00773863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773860
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

### Static facts (script output, report-only)
- size 116 B, RET 0
- callees: 0x0071b3f0 (CALL)
- indirect: 0x007737f6 slot 0x70
- globals: 0x00a1b4d0 READ
- twins: 0x00773860 (distinguishing: 0x00773834: MOV ECX,dword ptr [0x00a0fde8])

### Live evidence (REA, report-only)
- function | float out-value: arg object vtbl+0x70 gives sub-object +0x224, word +0x18 and global 0x00A1B4D0 chain into 0x71B3F0; writes float to [EBP+0x10] only when vtbl+0x70 result non-null | confirmed | procedure_assembly 0x7737f6: CALL EAX; 0x773843: CALL 0x0071b3f0; 0x77384e: FSTP float ptr [EAX]
- xref | callers 0: none; callees: 0x71b3f0; size 116 B RET none | confirmed | procedure_callers->[]; procedure_callees->[0x71b3f0]; 0x773853: RET

## Card B13a-1-43

GOG function: CSWCCreature::CreateNewSpellLikeAbilityList  gog 0x00421650  callconv=(blank)  param_size_bytes=0  gog_size=128  gog_retn=0
Candidate Steam address: 0x00775760 (size 128, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007757e0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004215f0 CSWCCreature::PerformCastSpell -> 0x00775700  gate_class=REVIEW
- GOG-order neighbour after:  0x004216d0 CSWCCreature::DeleteSpellLikeAbilityList -> 0x007757e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0042c210 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3b8*} | Steam {0x3b8*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x10*} | Steam {0x4*,0xc*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00775760: 16-byte aligned: YES
  00775760  55             PUSH EBP
  00775761  8B EC          MOV EBP,ESP
  00775763  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00775760
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0064DD10  (unnamed)
struct offsets touched (top 1):
  +0x3B8 [dec 952]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00775760 FUN_00775760 size=128 ret=0 callers=1 callees=2

void __fastcall FUN_00775760(int param_1)

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
  iVar1 = FUN_00919723(0xc,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_0064dd10(0);
  }
  *(undefined4 *)(param_1 + 0x3b8) = local_20;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007757e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007757e0: 16-byte aligned: YES
  007757e0  55             PUSH EBP
  007757e1  8B EC          MOV EBP,ESP
  007757e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007757e0
string literals (0):
direct callees (3):
  0x0064DD50  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x3B8 [dec 952]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 128 B, RET 0
- callers 1: 0x00813200
- callees: 0x00919723 operator new (CALL), 0x0064dd10 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | SEH-framed thiscall: calls 0x919723(0xc), calls 0x64dd10 on result if nonnull, stores result in this+0x3b8; no string refs | confirmed | procedure_assembly 0x775788: PUSH 0xc; 0x7757cb: MOV dword ptr [ECX + 0x3b8], EDX
- xref | callers 1: 0x813200; callees: 0x919723,0x64dd10 | confirmed | procedure_callers/procedure_callees 0x00775760; 128 B plain RET

## Card B13a-2-09

GOG function: CSWCCreature::SetCombatState  gog 0x00414080  callconv=(blank)  param_size_bytes=4  gog_size=397  gog_retn=4
Candidate Steam address: 0x00768440 (size 429, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007682b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> 0x007680a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00414210 CSWCCreature::MoveCreatureToPosition -> 0x00768440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: no
  0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> Steam ?  in candidate callees: n/a
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x0041fa70 Global::GetCreatureRadius -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0047f970 CSWCItem::PowerItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00693090 CSWSCreature::SetCombatState -> Steam 0x0057a560  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x34,0x94,0x9c,0x138,0x11ac} | Steam {0x44,0x8c,0x1f0}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x20,0x7f000000} | Steam {0x4*,0x5,0x8,0xc,0x24,0x8c,0x1e4}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00768440: 16-byte aligned: YES
  00768440  55             PUSH EBP
  00768441  8B EC          MOV EBP,ESP
  00768443  81 EC 8C 00 00 00 SUB ESP,0x8c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00768440
string literals (0):
direct callees (4):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 3):
  +0x1F0 [dec 496]  x8  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX
  +0x44 [dec 68]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00768440 FUN_00768440 size=429 ret=4 callers=3 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00768440(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 local_4c [12];
  undefined1 local_40 [12];
  undefined1 local_34 [12];
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined8 local_c;
  
  local_c = _DAT_00996378;
  local_18 = 0;
  local_14 = 0;
  local_10 = 0;
  if ((uint)param_1[0x11 = byte +0x44] < (uint)param_1[0x7c = byte +0x1f0]) {
    param_1[0x7c = byte +0x1f0] = param_1[0x7c = byte +0x1f0] - param_1[0x11 = byte +0x44];
    FUN_004452b0(&local_24,param_1 + 0x79,param_1 + 0x9 [dec 9] {int*: byte +0x24});
    fVar2 = (float10)FUN_0044b8a0(&local_24,&local_24);
    if ((float10)_DAT_00993768 <= fVar2) {
      if (local_20 * local_20 + local_24 * local_24 < (float)_DAT_0099efa0) {
        local_28 = (float)((float10)(uint)param_1[0x7c = byte +0x1f0] /
                          (float10)(uint)(param_1[0x7c = byte +0x1f0] + param_1[0x11 = byte +0x44]));
        uVar1 = FUN_004452b0(local_34,param_1 + 0x9 [dec 9] {int*: byte +0x24},param_2);
        uVar1 = FUN_0045cfe0(local_40,local_28,uVar1);
        uVar1 = FUN_0044c8a0(local_4c,param_2,uVar1);
        (**(code **)(*param_1 + 0x8c))(uVar1);
      }
      else {
        param_1[0x7c = byte +0x1f0] = 0;
        (**(code **)(*param_1 + 0x8c))(param_2);
      }
    }
    else {
      param_1[0x7c = byte +0x1f0] = 0;
      (**(code **)(*param_1 + 0x8c))(param_2);
    }
  }
  else {
    param_1[0x7c = byte +0x1f0] = 0;
    (**(code **)(*param_1 + 0x8c))(param_2);
  }
  return;
}
```

### decoy: 0x007682b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007682b0: 16-byte aligned: YES
  007682b0  55             PUSH EBP
  007682b1  8B EC          MOV EBP,ESP
  007682b3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007682b0
string literals (0):
direct callees (6):
  0x007680A0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x00773BA0  (unnamed)
struct offsets touched (top 6):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x138 [dec 312]  x2  via EAX,EDX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

### Static facts (script output, report-only)
- size 429 B, RET 4
- callers 3: 0x007664b0, 0x007675b0, 0x00769780
- callees: 0x004452b0 (CALL), 0x0044b8a0 (CALL), 0x0045cfe0 (CALL), 0x0044c8a0 (CALL)
- indirect: 0x00768494 slot 0x8c, 0x0076850c slot 0x8c, 0x0076854d slot 0x8c, 0x007685e5 slot 0x8c
- twins: none

### Live evidence (REA, report-only)
- function | countdown at this+0x1F0 (step this+0x44): expiry or squared length >= const 0x0099EFA0 resets and calls vtbl+0x8C(arg); else float-scaled vtbl+0x8C(arg) | confirmed | procedure_assembly 0x76846d: CMP EDX, dword ptr [ECX + 0x1f0]; 0x768494: CALL EDX
- xref | callers 3: 0x7664b0, 0x7675b0, 0x769780; callees: 0x4452b0, 0x44b8a0, 0x44c8a0, 0x45cfe0; size 429 B RET 4 | confirmed | procedure_callers->[0x7664b0,0x7675b0,0x769780]; procedure_callees->[0x4452b0,0x44b8a0,0x44c8a0,0x45cfe0]; 0x7685ea: RET 0x4

## Card B13a-1-11

GOG function: CSWCCreature::BarkBubble  gog 0x0041ae50  callconv=(blank)  param_size_bytes=4  gog_size=120  gog_retn=4
Candidate Steam address: 0x0076efe0 (size 120, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0076eda0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041ade0 CSWCCreature::GetLastInstant -> 0x0076ef70  gate_class=AUTO
- GOG-order neighbour after:  0x0041aee0 CSWCCreature::UpdateAttackData -> 0x0076f070  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 9 common): GOG {0x4*,0x5*,0x8*,0x10*,0x20*,0x4000*,0x8000*,0x10000*,0x7f000000*} | Steam {0x4*,0x5*,0x8*,0x10*,0x20*,0x4000*,0x8000*,0x10000*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076efe0: 16-byte aligned: YES
  0076efe0  55             PUSH EBP
  0076efe1  8B EC          MOV EBP,ESP
  0076efe3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076efe0
string literals (0):
direct callees (1):
  0x0076E610  CSWCCreature::GetEquippedItemID
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0076efe0 FUN_0076efe0 size=120 ret=4 callers=1 callees=1

undefined4 FUN_0076efe0(undefined4 param_1)

{
  undefined4 uVar1;
  
  switch(param_1) {
  case 1:
  case 6:
    uVar1 = FUN_0076e610(0x10);
    break;
  case 2:
    uVar1 = FUN_0076e610(0x20);
    break;
  case 3:
    uVar1 = FUN_0076e610(0x4000);
    break;
  case 4:
    uVar1 = FUN_0076e610(0x8000);
    break;
  case 5:
    uVar1 = FUN_0076e610(0x10000);
    break;
  default:
    uVar1 = 0x7f000000;
  }
  return uVar1;
}
```

### decoy: 0x0076eda0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076eda0: 16-byte aligned: YES
  0076eda0  55             PUSH EBP
  0076eda1  8B EC          MOV EBP,ESP
  0076eda3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076eda0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x280 [dec 640]  x1  via ECX
  +0x284 [dec 644]  x1  via EDX
  +0x288 [dec 648]  x1  via EAX
  +0x28C [dec 652]  x1  via ECX
  +0x290 [dec 656]  x1  via EDX
  +0x294 [dec 660]  x1  via EAX
  +0x298 [dec 664]  x1  via ECX
  +0x29C [dec 668]  x1  via EDX
  +0x2A0 [dec 672]  x1  via EAX
  +0x2A4 [dec 676]  x1  via ECX
```

### Static facts (script output, report-only)
- size 120 B, RET 4
- callers 1: 0x007711f0
- callees: 0x0076e610 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | switch on arg1 (1..6, jump table at 0x76f058) calls 0x76e610 with pushed 0x10/0x20/0x4000/0x8000/0x10000; default returns 0x7f000000 | confirmed | procedure_assembly 0x76f001: JMP dword ptr [EDX*0x4 + 0x76f058]; 0x76f04d: MOV EAX, 0x7f000000
- xref | callers 1: 0x7711f0; callees: 0x76e610 | confirmed | procedure_callers/procedure_callees 0x0076efe0; 120 B RET 0x4

## Card B13a-2-13

GOG function: CSWCCreature::GetEquipmentForcePowers  gog 0x004248c0  callconv=(blank)  param_size_bytes=16  gog_size=55  gog_retn=16
Candidate Steam address: 0x00778900 (size 55, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00778ba0:STR0/CALLEE0/SIG0.028/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004247a0 CSWCCreature::CheckFeat -> 0x007787e0  gate_class=AUTO
- GOG-order neighbour after:  0x00424900 CSWCCreature::EnableFeatForMenu -> 0x00778940  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x10*} | Steam {0x8*,0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00778900: 16-byte aligned: YES
  00778900  55             PUSH EBP
  00778901  8B EC          MOV EBP,ESP
  00778903  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00778900
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00778900 FUN_00778900 size=55 ret=16 callers=2 callees=0

int FUN_00778900(int param_1,int param_2,int param_3,int param_4)

{
  undefined4 local_8;
  
  local_8 = 0;
  if (((param_1 == param_2) || (param_1 == param_3)) || (param_1 == param_4)) {
    local_8 = param_4;
  }
  return local_8;
}
```

### decoy: 0x00778ba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00778ba0: 16-byte aligned: YES
  00778ba0  55             PUSH EBP
  00778ba1  8B EC          MOV EBP,ESP
  00778ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00778ba0
string literals (0):
direct callees (20):
  0x0084A640  (unnamed)
  0x005933B0  (unnamed)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x007FDB50  CSWCItem::GetActivePropertySpellID
  0x006C1450  (unnamed)
  0x006C11C0  (unnamed)
  0x005934C0  (unnamed)
  0x00414280  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00593450  CExoArrayList<uint16>::Add
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x006C12D0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710C20  (unnamed)
  0x00710CA0  (unnamed)
  0x007FEE70  CSWCItem::GetServerItem
  0x00604260  CSWSItem::GetActiveProperty
  0x00603B50  CSWSItem::GetUsedActivePropertyUsesLeft
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x6 [dec 6]  x2  via EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0x19C [dec 412]  x2  via EAX,ECX
  +0x14 [dec 20]  x1  via EAX
  +0x13C [dec 316]  x1  via EAX
  +0x194 [dec 404]  x1  via EAX
  +0x198 [dec 408]  x1  via EAX
```

### Static facts (script output, report-only)
- size 55 B, RET 16
- callers 2: 0x0076f300, 0x0076f940
- twins: none

### Live evidence (REA, report-only)
- function | leaf: returns arg4 if arg1 equals arg2, arg3 or arg4, else 0; no calls; no strings | confirmed | procedure_info entry 0x778900 (start yes); procedure_assembly 0x778913: CMP EAX, dword ptr [EBP + 0xc]; 0x778934: RET 0x10
- xref | callers 2: 0x0076f300, 0x0076f940; callees: none; size 55 B RET 16 | confirmed | procedure_info body 0x778900-0x778936; procedure_callees []; 0x778934: RET 0x10

## Card B13a-1-60

GOG function: CSWCCreature::GetHostileItems_2  gog 0x00428690  callconv=(blank)  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x0077c6a0 (size 107, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetHostileItems | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077c710:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428620 CSWCCreature::GetHostileItems -> 0x0077c6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00428700 CSWCCreature::PartyMenuUseItemSelect -> 0x0077c780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004282c0 CSWCCreature::CreateHostileItemEntry_2 -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00669930 CItemRepository::ItemListGetItem -> Steam 0x00606100  in candidate callees: yes
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam 0x0058a2c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
imm sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077c6a0: 16-byte aligned: YES
  0077c6a0  55             PUSH EBP
  0077c6a1  8B EC          MOV EBP,ESP
  0077c6a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077c6a0
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C1B0  CSWCCreature::CreateHostileItemEntry
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c6a0 FUN_0077c6a0 size=107 ret=8 callers=1 callees=4

void __thiscall FUN_0077c6a0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  uVar2 = 1;
  FUN_0077d800(1,param_1);
  iVar1 = FUN_0058a2c0(uVar2);
  for (local_8 = 0; local_8 < *(uint *)(iVar1 + 0x10); local_8 = local_8 + 1) {
    uVar2 = FUN_00606100(local_8);
    FUN_0077c1b0(param_2,uVar2,param_3);
  }
  return;
}
```

### decoy: 0x0077c710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077c710: 16-byte aligned: YES
  0077c710  55             PUSH EBP
  0077c711  8B EC          MOV EBP,ESP
  0077c713  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077c710
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C340  CSWCCreature::CreateHostileItemEntry_2
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### Static facts (script output, report-only)
- size 107 B, RET 8
- callers 1: 0x0077e370
- callees: 0x0077d800 (CALL), 0x0058a2c0 CSWSCreature::GetItemRepository (CALL), 0x00606100 CItemRepository::ItemListGetItem (CALL), 0x0077c1b0 (CALL)
- twins: 0x0077c710 (distinguishing: 0x0077c6fe: CALL 0x0077c1b0)

### Live evidence (REA, report-only)
- function | calls 0x0077d800 and 0x0058a2c0; loops over count at +0x10 calling 0x00606100 and 0x0077c1b0 per item; returns with RET 0x8 | confirmed | procedure_assembly 0x77c6b5: CALL 0x0058a2c0; 0x77c708: RET 0x8 (size 107; body 0x77c6a0-0x77c70a)
- xref | callers 1: 0x77e370; callees: 0x58a2c0, 0x606100, 0x77c1b0, 0x77d800; strings/data: none | confirmed | procedure_callers/procedure_callees 0x0077c6a0; size 107 B RET 0x8

## Card B13a-1-06

GOG function: CSWCCreature::StripEquippedItems  gog 0x00413620  callconv=(blank)  param_size_bytes=0  gog_size=248  gog_retn=0
Candidate Steam address: 0x007678f0 (size 248, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::ClearVisibilityList | G7 mac: pass(0.43) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007675b0:STR0/CALLEE0/SIG0.233/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004132e0 CSWCCreature::AnimationUpdate -> 0x007675b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00413720 Vector::operator-  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040c970 CClientExoApp::GetGameObjectArray -> Steam 0x0073f4b0  in candidate callees: yes
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x88*,0x30c*} | Steam {0x4*,0x8*,0x88*,0x30c*}
imm sets (* = in both, 7 common): GOG {0x8*,0xc*,0x24*,0x30*,0x41*,0x4c*,0x1d8*} | Steam {0x8*,0xc*,0x24*,0x30*,0x41*,0x4c*,0x1d8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007678f0: 16-byte aligned: YES
  007678f0  55             PUSH EBP
  007678f1  8B EC          MOV EBP,ESP
  007678f3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007678f0
string literals (0):
direct callees (5):
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x88 [dec 136]  x1  via EAX
  +0x30C [dec 780]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007678f0 FUN_007678f0 size=248 ret=0 callers=1 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_007678f0(int *param_1)

{
  char cVar1;
  undefined4 uVar2;
  int *piVar3;
  float10 fVar4;
  undefined1 local_48 [24];
  int local_30;
  undefined4 local_2c;
  undefined1 local_28 [12];
  undefined1 local_1c [8];
  undefined4 local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_2c = *(undefined4 *)(DAT_00a1b4a4 + 4);
  local_c = FUN_0073f4b0();
  if (local_c != 0) {
    local_30 = param_1[0xc3 = byte +0x30c];
    cVar1 = FUN_0053dfb0(local_30,&local_8);
    if (cVar1 == DAT_0099447f) {
      local_10 = local_8;
      if (local_8 != 0) {
        FUN_004452b0(local_1c,local_8 + 0x24,param_1 + 0x9 [dec 9] {int*: byte +0x24});
        fVar4 = (float10)FUN_0044b8a0(local_1c,local_1c);
        if ((float10)_DAT_00993768 < fVar4) {
          local_14 = 0;
          uVar2 = FUN_005143c0(local_28,local_1c);
          piVar3 = (int *)FUN_005143c0(local_48,uVar2);
          param_1[0x76 = byte +0x1d8] = *piVar3;
          param_1[0x77 = byte +0x1dc] = piVar3[1 = byte +0x4];
          param_1[0x78 = byte +0x1e0] = piVar3[2 = byte +0x8];
        }
      }
    }
    else {
      (**(code **)(*param_1 + 0x88))(param_1 + 0xc);
    }
  }
  return;
}
```

### decoy: 0x007675b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007675b0: 16-byte aligned: YES
  007675b0  55             PUSH EBP
  007675b1  8B EC          MOV EBP,ESP
  007675b3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007675b0
string literals (0):
direct callees (13):
  0x007EF2C0  CSWCObject::AnimationStationary
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x007678F0  CSWCCreature::ClearVisibilityList
  0x00768610  CSWCCreature::RotateCreatureToOrientation
  0x00768440  CSWCCreature::SetCombatState
  0x007ED980  CSWCObject::GetLoopingAnimation
  0x00777E30  CSWCCreature::AnimateFireAndForget
  0x00767800  (unnamed)
  0x007EF350  CSWCObject::AnimationPause
  0x0051C6F0  CServerExoApp::IsInjured
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
struct offsets touched (top 8):
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x9C [dec 156]  x4  via EAX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x75 [dec 117]  x1  via EDX
  +0x138 [dec 312]  x1  via EAX
  +0x30C [dec 780]  x1  via ECX
  +0x11B0 [dec 4528]  x1  via EDX
```

### Static facts (script output, report-only)
- size 248 B, RET 0
- callers 1: 0x007675b0
- callees: 0x0073f4b0 CClientExoApp::GetGameObjectArray (CALL), 0x0053dfb0 CGameObjectArray::GetGameObject (CALL), 0x004452b0 (CALL), 0x0044b8a0 (CALL), 0x005143c0 (CALL)
- indirect: 0x007679e2 slot 0x88
- twins: none

### Live evidence (REA, report-only)
- function | calls 0x0073f4b0 then 0x0053dfb0; on match calls 0x004452b0, 0x0044b8a0, 0x005143c0 (twice) and copies 12 bytes to this+0x1d8; on mismatch indirect vtable call slot +0x88 | confirmed | procedure_assembly 0x767907: CALL 0x0073f4b0; 0x7679e7: RET (size 248, no operand; body 0x7678f0-0x7679e7)
- xref | callers 1: 0x7675b0; callees: 0x4452b0, 0x44b8a0, 0x5143c0, 0x53dfb0, 0x73f4b0; strings/data: none | confirmed | procedure_callers/procedure_callees 0x007678f0; size 248 B RET none

## Card B13a-1-10

GOG function: CSWCCreature::SetDesiredOrientationForLock  gog 0x0041f5a0  callconv=(blank)  param_size_bytes=0  gog_size=265  gog_retn=0
Candidate Steam address: 0x007736d0 (size 265, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007737e0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f430 CSWCCreature::DoFootstepAudio -> 0x00773560  gate_class=AUTO
- GOG-order neighbour after:  0x0041f6b0 CSWCCreature::UnloadMonkVisuals -> 0x007737e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x0041ade0 CSWCCreature::GetLastInstant -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0x7c*} | Steam {0x68*,0x7c*}
imm sets (* = in both, 2 common): GOG {0x14*,0x18*} | Steam {0x14*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007736d0: 16-byte aligned: YES
  007736d0  55             PUSH EBP
  007736d1  8B EC          MOV EBP,ESP
  007736d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007736d0
string literals (0):
direct callees (4):
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0076EF70  CSWCCreature::GetLastInstant
  0x00733570  CExoString::CExoString_char
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x68 [dec 104]  x5  via EAX,EDX
  +0x7C [dec 124]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007736d0 FUN_007736d0 size=265 ret=0 callers=1 callees=4

void __fastcall FUN_007736d0(int param_1)

{
  undefined4 uVar1;
  undefined1 local_24 [8];
  undefined1 local_1c [11];
  byte local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958e00;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  for (local_11 = 0; local_11 < 0x14; local_11 = local_11 + 1) {
    uVar1 = FUN_0076e610(local_11);
    FUN_0076ef70(uVar1);
  }
  if (*(int *)(param_1 + 0x68) != 0) {
    FUN_00733570(&DAT_00986925);
    local_8 = 0;
    (**(code **)(**(int **)(param_1 + 0x68) + 0x7c))(DAT_00994484,local_1c,0,0x3f800000);
    local_8 = 0xffffffff;
    FUN_00733780();
    FUN_00733570(&DAT_00986925);
    local_8 = 1;
    (**(code **)(**(int **)(param_1 + 0x68) + 0x7c))(DAT_00994484,local_24,1,0x3f800000);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007737e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007737e0: 16-byte aligned: YES
  007737e0  55             PUSH EBP
  007737e1  8B EC          MOV EBP,ESP
  007737e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007737e0
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

### Static facts (script output, report-only)
- size 265 B, RET 0
- callers 1: 0x0082b0c0
- callees: 0x0076e610 (CALL), 0x0076ef70 (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00733780 CExoString::Destructor (CALL)
- indirect: 0x0077376e slot 0x7c, 0x007737b9 slot 0x7c
- twins: none

### Live evidence (REA, report-only)
- function | loops 0..19 calling 0x0076e610 and passing result to 0x0076ef70; if this+0x68 nonzero: calls 0x00733570 with empty string 0x986925 and vtable slot +0x7c twice; 0x00733780 cleanup | confirmed | procedure_assembly 0x773717: CALL 0x0076e610; 0x7737d8: RET (size 265, no operand; body 0x7736d0-0x7737d8)
- xref | callers 1: 0x82b0c0; callees: 0x733570, 0x733780, 0x76e610, 0x76ef70; strings/data: string empty 0x986925 | confirmed | procedure_callers/procedure_callees 0x007736d0; size 265 B RET none

## Card B13a-1-37

GOG function: CSWCCreature::RemoveShadowBlob  gog 0x00420310  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00774440 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00774260:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00420130 CSWCCreature::ApplyShadowBlob -> 0x00774260  gate_class=REVIEW
- GOG-order neighbour after:  0x00420330 CSWCCreature::TurnOnShadows -> 0x00774460  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00477020 CSWCObject::RemoveShadowBlob -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00774440: 16-byte aligned: YES
  00774440  55             PUSH EBP
  00774441  8B EC          MOV EBP,ESP
  00774443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00774440
string literals (0):
direct callees (1):
  0x007F1EC0  CSWCObject::RemoveShadowBlob
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00774440 FUN_00774440 size=19 ret=0 callers=0 callees=1
THUNK -> 0x007F1EC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00774440(undefined4 param_1)

{
  FUN_007f1ec0(param_1);
  return;
}
```

### decoy: 0x00774260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774260: 16-byte aligned: YES
  00774260  55             PUSH EBP
  00774261  8B EC          MOV EBP,ESP
  00774263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00774260
string literals (4):
  0x0099F1A4  at 00774328  'Fx_Shadow_S'
  0x0099F198  at 00774337  'Fx_Shadow_M'
  0x0099F18C  at 00774346  'Fx_Shadow_L'
  0x0099F198  at 00774355  'Fx_Shadow_M'
direct callees (5):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x0071B8B0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x005160C0  (unnamed)
struct offsets touched (top 10):
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x10 [dec 16]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
  +0xA4 [dec 164]  x1  via EDX
  +0xB4 [dec 180]  x1  via EDX
  +0xD4 [dec 212]  x1  via ECX
  +0x108 [dec 264]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
```

### Static facts (script output, report-only)
- size 19 B, RET 0
- callees: 0x007f1ec0 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | no-stack-arg forwarder: sets ECX=this, calls 0x7f1ec0; DATA xref from 0x99ef14 (content unread) | confirmed | procedure_assembly 0x77444a: CALL 0x007f1ec0; 0x774452: RET
- xref | callers 0: none; callees: 0x7f1ec0 | confirmed | procedure_callers/procedure_callees 0x00774440; 19 B plain RET

## Card B13a-2-28

GOG function: CSWCCreature::Constructor  gog 0x004102b0  callconv=(blank)  param_size_bytes=0  gog_size=2405  gog_retn=0
Candidate Steam address: 0x007645d0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007645f0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410290 CSWCCreaturePartyStatus::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040ffe0 CSWCCombatAttackData::Constructor -> Steam ?  in candidate callees: n/a
  0x004721b0 CSWCObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00489500 CSWCSpellCastVisual::Constructor -> Steam ?  in candidate callees: n/a
  0x0048a160 Global::CSWCCreatureAppearance -> Steam ?  in candidate callees: n/a
  0x00490650 CSWCLevelUpStats::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00595eb0 CSWGuiInGameCharacter::Constructor -> Steam 0x0084c3a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x3c,0x40,0x48,0x58,0xfc,0x12c,0x130,0x134,0x138,0x13c,0x140,0x144,...(+101)} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x5,0x6,0xc,0x11,0x14,0x30,0x48,0x6c,0x88,0x140,0x1cc,...(+17)} | Steam {0x18}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007645d0: 16-byte aligned: YES
  007645d0  55             PUSH EBP
  007645d1  8B EC          MOV EBP,ESP
  007645d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007645d0
string literals (0):
direct callees (1):
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007645d0 FUN_007645d0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00733780 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007645d0(undefined4 param_1)

{
  FUN_00733780(param_1);
  return;
}
```

### decoy: 0x007645f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007645f0: 16-byte aligned: YES
  007645f0  55             PUSH EBP
  007645f1  8B EC          MOV EBP,ESP
  007645f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007645f0
string literals (0):
direct callees (11):
  0x007ED050  (unnamed)
  0x007107F0  (unnamed)
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x00845AC0  (unnamed)
  0x00764320  (unnamed)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0085A0D0  Global::CSWCCreatureAppearance
  0x00859470  (unnamed)
  0x0091EB8B  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x458 [dec 1112]  x4  via EAX,EDX
  +0x224 [dec 548]  x2  via EAX,EDX
  +0x3C [dec 60]  x1  via EAX
  +0x40 [dec 64]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x58 [dec 88]  x1  via EAX
  +0x12C [dec 300]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

### Static facts (script output, report-only)
- size 22 B, RET 0
- callers 1: 0x007651b0
- callees: 0x00733780 CExoString::Destructor (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | calls 0x00733780 with ECX = this+0x18, then returns | confirmed | asm: 0x7645da: ADD ECX, 0x18 | 0x7645dd: CALL 0x00733780 | 0x7645e5: RET
- xref | callers 1: 0x007651b0; callees: 0x00733780; size 22 B RET 0 | confirmed | asm: 0x7645e5: RET | 0x7645dd: CALL 0x00733780 | procedure_info length 22, body 0x7645d0-0x7645e5

## Card B13a-2-05

GOG function: CSWCCreature::ActionMenuSwapImplantRegen  gog 0x00426430  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a490 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a470:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426240 CSWCCreature::ActionSwapImplant -> 0x0077a280  gate_class=AUTO
- GOG-order neighbour after:  0x00426450 CSWCCreature::ActionMenuSwapImplantStr -> 0x0077a4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x408} | Steam {0x8*,0x409}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a490: 16-byte aligned: YES
  0077a490  55             PUSH EBP
  0077a491  8B EC          MOV EBP,ESP
  0077a493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a490
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a490 FUN_0077a490 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a490(void)

{
  FUN_0077a280(0x409);
  return;
}
```

### decoy: 0x0077a470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a470: 16-byte aligned: YES
  0077a470  55             PUSH EBP
  0077a471  8B EC          MOV EBP,ESP
  0077a473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a470
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 26 B, RET 8
- callees: 0x0077a280 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | calls 0x0077a280 with pushed immediate 0x409 and ECX = this; RET 8 | confirmed | asm: 0x77a497: PUSH 0x409 | 0x77a49c: MOV ECX, dword ptr [EBP + -0x4] | 0x77a49f: CALL 0x0077a280 | 0x77a4a7: RET 0x8
- xref | callers 0: none; callees: 0x0077a280; size 26 B RET 8 | confirmed | asm: 0x77a4a7: RET 0x8 | 0x77a49f: CALL 0x0077a280 | procedure_info length 26, body 0x77a490-0x77a4a9

## Card B13a-2-26

GOG function: CSWCCreature::PerformUseHostileItem  gog 0x00428d50  callconv=(blank)  param_size_bytes=8  gog_size=45  gog_retn=8
Candidate Steam address: 0x0077cdd0 (size 45, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077cd00:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428c80 CSWCCreature::PartyMenuUseHostileItem -> 0x0077cd00  gate_class=AUTO
- GOG-order neighbour after:  0x00428d80 CSWCCreature::ActionMenuAttack -> 0x0077ce00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00428c00 CSWCCreature::PartyMenuUseItem -> Steam ?  in candidate callees: n/a
  0x0042bd40 CSWCCreature::SetCombatMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0077cdd0: 16-byte aligned: YES
  0077cdd0  55             PUSH EBP
  0077cdd1  8B EC          MOV EBP,ESP
  0077cdd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077cdd0
string literals (0):
direct callees (2):
  0x0077FD20  CSWCCreature::SetCombatMode
  0x0077CC80  CSWCCreature::PartyMenuUseItem
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077cdd0 FUN_0077cdd0 size=45 ret=8 callers=2 callees=2

void FUN_0077cdd0(undefined4 param_1,int param_2)

{
  if (param_2 != 0) {
    FUN_0077fd20(1);
  }
  FUN_0077cc80(param_1,param_2);
  return;
}
```

### decoy: 0x0077cd00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077cd00: 16-byte aligned: YES
  0077cd00  55             PUSH EBP
  0077cd01  8B EC          MOV EBP,ESP
  0077cd03  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077cd00
string literals (0):
direct callees (4):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x006D6E30  CSWItem::GetBaseItem
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x0077CDD0  CSWCCreature::PerformUseHostileItem
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

### Static facts (script output, report-only)
- size 45 B, RET 8
- callers 2: 0x0077cd00, 0x00899300
- callees: 0x0077fd20 (CALL), 0x0077cc80 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | wrapper: if arg2 nonzero calls 0x77fd20(arg2, 1) first; then calls 0x77cc80(this, arg1, arg2); no strings; RET 8 | confirmed | procedure_info entry 0x77cdd0 (start yes); procedure_assembly 0x77cde2: CALL 0x0077fd20; 0x77cdf2: CALL 0x0077cc80; 0x77cdfa: RET 0x8
- xref | callers 2: 0x0077cd00, 0x00899300; callees: 0x0077cc80, 0x0077fd20; size 45 B RET 8 | confirmed | procedure_info body 0x77cdd0-0x77cdfc; procedure_callers 2 entries; 0x77cdfa: RET 0x8

## Card B13a-2-31

GOG function: CSWCCreature::UpdateRelativePerception_2  gog 0x00419dc0  callconv=(blank)  param_size_bytes=4  gog_size=470  gog_retn=4
Candidate Steam address: 0x0076e610 (size 429, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetEquippedItemID | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0076e130:STR0/CALLEE0/SIG0.119/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00419c50 CSWCCreature::UpdateRelativePerception -> 0x0076dde0  gate_class=REVIEW
- GOG-order neighbour after:  0x00419fa0 CSWCCreature::EquipToInventorySlot -> 0x0076e130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: no
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x20,0xb4,0xb8,0x1b8} | Steam {0x228,0x22c,0x230,0x234,0x238,0x23c,0x240,0x244,0x248,0x24c,0x250,0x254,0x258,0x25c,...(+3)}
imm sets (* = in both, 1 common): GOG {0x4*,0x24,0x44,0x378,0x7d1,0x7f000000} | Steam {0x4*,0x8,0x7f,0x100,0x200,0x400,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076e610: 16-byte aligned: YES
  0076e610  55             PUSH EBP
  0076e611  8B EC          MOV EBP,ESP
  0076e613  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076e610
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x228 [dec 552]  x1  via ECX
  +0x22C [dec 556]  x1  via EDX
  +0x230 [dec 560]  x1  via EAX
  +0x234 [dec 564]  x1  via ECX
  +0x238 [dec 568]  x1  via EDX
  +0x23C [dec 572]  x1  via EAX
  +0x240 [dec 576]  x1  via ECX
  +0x244 [dec 580]  x1  via EDX
  +0x248 [dec 584]  x1  via EAX
  +0x24C [dec 588]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0076e610 FUN_0076e610 size=429 ret=4 callers=41 callees=0

undefined4 __thiscall FUN_0076e610(int param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (param_2 < 0x101) {
    if (param_2 == 0x100) {
      return *(undefined4 *)(param_1 + 0x248);
    }
    switch(param_2) {
    case 1:
      uVar1 = *(undefined4 *)(param_1 + 0x228);
      break;
    case 2:
      uVar1 = *(undefined4 *)(param_1 + 0x22c);
      break;
    default:
      goto switchD_0076e652_caseD_3;
    case 4:
      uVar1 = *(undefined4 *)(param_1 + 0x230);
      break;
    case 8:
      uVar1 = *(undefined4 *)(param_1 + 0x234);
      break;
    case 0x10:
      uVar1 = *(undefined4 *)(param_1 + 0x238);
      break;
    case 0x20:
      uVar1 = *(undefined4 *)(param_1 + 0x23c);
      break;
    case 0x40:
      uVar1 = *(undefined4 *)(param_1 + 0x240);
      break;
    case 0x80:
      uVar1 = *(undefined4 *)(param_1 + 0x244);
    }
  }
  else {
    if (param_2 < 0x10001) {
      if (param_2 == 0x10000) {
        return *(undefined4 *)(param_1 + 0x25c);
      }
      if (param_2 < 0x4001) {
        if (param_2 == 0x4000) {
          return *(undefined4 *)(param_1 + 0x254);
        }
        if (param_2 == 0x200) {
          return *(undefined4 *)(param_1 + 0x24c);
        }
        if (param_2 == 0x400) {
          return *(undefined4 *)(param_1 + 0x250);
        }
      }
      else if (param_2 == 0x8000) {
        return *(undefined4 *)(param_1 + 0x258 [dec 600]);
      }
    }
    else {
      if (param_2 == 0x20000) {
        return *(undefined4 *)(param_1 + 0x260);
      }
      if (param_2 == 0x40000) {
        return *(undefined4 *)(param_1 + 0x264);
      }
      if (param_2 == 0x80000) {
        return *(undefined4 *)(param_1 + 0x268);
      }
    }
switchD_0076e652_caseD_3:
    uVar1 = DAT_00994484;
  }
  return uVar1;
}
```

### decoy: 0x0076e130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076e130: 16-byte aligned: YES
  0076e130  55             PUSH EBP
  0076e131  8B EC          MOV EBP,ESP
  0076e133  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076e130
string literals (0):
direct callees (9):
  0x007FD700  CSWCItem::Destructor
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0076EF70  CSWCCreature::GetLastInstant
  0x0076E8A0  CSWCCreature::SetEquippedItemID
  0x0076D590  CSWCCreature::UpdateWeaponClass
  0x0076EDA0  CSWCCreature::UpdateRangedAttackData
  0x0076D890  CSWCCreature::UpdateVisibleWeapons
  0x00768060  CSWCCreature::ForceAppearanceUpdate
struct offsets touched (top 8):
  +0x10 [dec 16]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
  +0x80 [dec 128]  x1  via EAX
  +0x84 [dec 132]  x1  via EDX
  +0x164 [dec 356]  x1  via EDX
```

### Static facts (script output, report-only)
- size 429 B, RET 4
- callers 41: 0x0066b8a0 Global::ExecuteCommandCreatureFlourishWeapon, 0x007637e0, 0x00767a80, 0x007680a0, 0x007682b0, ...
- twins: none

### Live evidence (REA, report-only)
- function | selector accessor: 0x100/0x200/0x400/0x4000/0x8000/0x10000/0x20000/0x40000/0x80000 return this+0x248/0x24c/0x250/0x254/0x258/0x25c/0x260/0x264/0x268; selectors 1..0x80 via jump table (targets unresolved); other values return global 0x00994484; body non-contiguous (18 ranges, span 484 B); no strings | confirmed | procedure_info entry 0x76e610 (start yes); procedure_assembly 0x76e62f: JZ 0x0076e774; 0x76e777: MOV EAX, dword ptr [EAX + 0x248]; 0x76e7f1: RET 0x4
- xref | callers 41: 0x0066b8a0, 0x007637e0, 0x00767a80, 0x007680a0, 0x007682b0; callees: none; size 429 B RET 4 | confirmed | procedure_info 18 body ranges 0x76e610-0x76e7f3; procedure_callers 41 entries; 0x76e7f1: RET 0x4

## Card B13a-2-24

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077fac0 (size 77, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0077fa00:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042ba20 CSWCCreature::PlayRollingFootstepSound -> 0x0077fa00  gate_class=AUTO
- GOG-order neighbour after:  0x0042bb30 CSWCCreature::UpdateRollingFootstepSound_2 -> 0x0077fb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x448*,0x450*,0x454*} | Steam {0x448*,0x450*,0x454*}
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
prologue for 0x0077fac0: 16-byte aligned: YES
  0077fac0  55             PUSH EBP
  0077fac1  8B EC          MOV EBP,ESP
  0077fac3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fac0
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 3):
  +0x448 [dec 1096]  x2  via EAX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fac0 FUN_0077fac0 size=77 ret=0 callers=1 callees=1

void __fastcall FUN_0077fac0(int param_1)

{
  if ((*(int *)(param_1 + 0x448) != 0) && (*(int *)(param_1 + 0x450) != 0)) {
    Decref();
    *(undefined4 *)(param_1 + 0x450) = 0;
    *(undefined4 *)(param_1 + 0x454) = 0;
  }
  return;
}
```

### decoy: 0x0077fa00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fa00: 16-byte aligned: YES
  0077fa00  55             PUSH EBP
  0077fa01  8B EC          MOV EBP,ESP
  0077fa03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fa00
string literals (0):
direct callees (3):
  0x0070AE90  (unnamed)
  0x0070AFA0  (unnamed)
  0x0070AD50  (unnamed)
struct offsets touched (top 4):
  +0x448 [dec 1096]  x5  via EAX,ECX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x2  via ECX,EDX
  +0x44C [dec 1100]  x1  via EAX
```

### Static facts (script output, report-only)
- size 77 B, RET 0
- callers 1: 0x0077fb10
- callees: 0x0070af80 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | guarded call 0x0070af80 on this+0x448, then clears this+0x450 and this+0x454 | confirmed | asm: 0x77faca: CMP dword ptr [EAX + 0x448], 0x0 | 0x77faea: CALL 0x0070af80 | 0x77faf2: MOV dword ptr [EAX + 0x450], 0x0
- xref | callers 1: 0x0077fb10; callees: 0x0070af80; size 77 B RET 0 | confirmed | asm: 0x77fb0c: RET | 0x77faea: CALL 0x0070af80 | procedure_info length 77, body 0x77fac0-0x77fb0c

## Card B13a-2-36

GOG function: CSWCCreature::ActionMenuSwapImplantEnd  gog 0x00426490  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a4b0 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a4d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426470 CSWCCreature::ActionMenuSwapImplantAgi -> 0x0077a4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x004264b0 CSWCCreature::GetHostileForcePowers -> 0x0077a4f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x40a} | Steam {0x8*,0x40b}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a4b0: 16-byte aligned: YES
  0077a4b0  55             PUSH EBP
  0077a4b1  8B EC          MOV EBP,ESP
  0077a4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a4b0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a4b0 FUN_0077a4b0 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a4b0(void)

{
  FUN_0077a280(0x40b);
  return;
}
```

### decoy: 0x0077a4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a4d0: 16-byte aligned: YES
  0077a4d0  55             PUSH EBP
  0077a4d1  8B EC          MOV EBP,ESP
  0077a4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a4d0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 26 B, RET 8
- callees: 0x0077a280 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | thin wrapper: pushes 0x40B and calls 0x77A280 with this; RET 8 | confirmed | procedure_assembly 0x77a4b7: PUSH 0x40b; 0x77a4bf: CALL 0x0077a280
- xref | callers 0: none; callees: 0x77a280; size 26 B RET 8 | confirmed | procedure_callers->[]; procedure_callees->[0x77a280]; 0x77a4c7: RET 0x8

## Card B13a-2-14

GOG function: CSWCCreature::SetSWAnimationSpeed  gog 0x0042bd00  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x0077fce0 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077fc10:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042bc30 CSWCCreature::SetLockOrientationToObject -> 0x0077fc10  gate_class=AUTO
- GOG-order neighbour after:  0x0042bd40 CSWCCreature::SetCombatMode -> 0x0077fd20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11dc*} | Steam {0x11dc*}
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
prologue for 0x0077fce0: 16-byte aligned: YES
  0077fce0  55             PUSH EBP
  0077fce1  8B EC          MOV EBP,ESP
  0077fce3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fce0
string literals (0):
direct callees (1):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
struct offsets touched (top 1):
  +0x11DC [dec 4572]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fce0 FUN_0077fce0 size=59 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_0077fce0(undefined4 param_1)

{
  float fVar1;
  int iVar2;
  
  iVar2 = FUN_0077d800(param_1);
  fVar1 = _DAT_00994f50;
  if (iVar2 != 0) {
    iVar2 = FUN_0077d800(param_1);
    fVar1 = *(float *)(iVar2 + 0x11dc);
  }
  return (float10)fVar1;
}
```

### decoy: 0x0077fc10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fc10: 16-byte aligned: YES
  0077fc10  55             PUSH EBP
  0077fc11  8B EC          MOV EBP,ESP
  0077fc13  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fc10
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x30C [dec 780]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

### Static facts (script output, report-only)
- size 59 B, RET 0
- callers 1: 0x00873360
- callees: 0x0077d800 (CALL)
- twins: none

### Live evidence (REA, report-only)
- function | calls 0x77d800; returns float at this+0x11dc if result nonzero, else float const 0x994f50; RET no operand | confirmed | procedure_info length=59; procedure_assembly 0x77fd03: FLD float ptr [EAX + 0x11dc]; procedure_callers=[0x873360]
- xref | callers 1: 0x873360; callees: 0x77d800; data users: none | confirmed | procedure_callers/procedure_callees 0x0077fce0; size 59 B RET no operand

## Card B13a-1-18

GOG function: CSWCCreature::MoveCreatureToPosition  gog 0x00414210  callconv=(blank)  param_size_bytes=4  gog_size=429  gog_retn=4
Candidate Steam address: 0x00768440 (size 429, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::SetCombatState | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007685f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00414080 CSWCCreature::SetCombatState -> 0x007682b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00414410 Global::Unblur  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x004143c0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x44*,0x8c*,0x1f0*} | Steam {0x44*,0x8c*,0x1f0*}
imm sets (* = in both, 7 common): GOG {0x4*,0x5*,0x8*,0xc*,0x24*,0x8c*,0x1e4*} | Steam {0x4*,0x5*,0x8*,0xc*,0x24*,0x8c*,0x1e4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00768440: 16-byte aligned: YES
  00768440  55             PUSH EBP
  00768441  8B EC          MOV EBP,ESP
  00768443  81 EC 8C 00 00 00 SUB ESP,0x8c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00768440
string literals (0):
direct callees (4):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 3):
  +0x1F0 [dec 496]  x8  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX
  +0x44 [dec 68]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00768440 FUN_00768440 size=429 ret=4 callers=3 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00768440(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 local_4c [12];
  undefined1 local_40 [12];
  undefined1 local_34 [12];
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined8 local_c;
  
  local_c = _DAT_00996378;
  local_18 = 0;
  local_14 = 0;
  local_10 = 0;
  if ((uint)param_1[0x11 = byte +0x44] < (uint)param_1[0x7c = byte +0x1f0]) {
    param_1[0x7c = byte +0x1f0] = param_1[0x7c = byte +0x1f0] - param_1[0x11 = byte +0x44];
    FUN_004452b0(&local_24,param_1 + 0x79,param_1 + 0x9 [dec 9] {int*: byte +0x24});
    fVar2 = (float10)FUN_0044b8a0(&local_24,&local_24);
    if ((float10)_DAT_00993768 <= fVar2) {
      if (local_20 * local_20 + local_24 * local_24 < (float)_DAT_0099efa0) {
        local_28 = (float)((float10)(uint)param_1[0x7c = byte +0x1f0] /
                          (float10)(uint)(param_1[0x7c = byte +0x1f0] + param_1[0x11 = byte +0x44]));
        uVar1 = FUN_004452b0(local_34,param_1 + 0x9 [dec 9] {int*: byte +0x24},param_2);
        uVar1 = FUN_0045cfe0(local_40,local_28,uVar1);
        uVar1 = FUN_0044c8a0(local_4c,param_2,uVar1);
        (**(code **)(*param_1 + 0x8c))(uVar1);
      }
      else {
        param_1[0x7c = byte +0x1f0] = 0;
        (**(code **)(*param_1 + 0x8c))(param_2);
      }
    }
    else {
      param_1[0x7c = byte +0x1f0] = 0;
      (**(code **)(*param_1 + 0x8c))(param_2);
    }
  }
  else {
    param_1[0x7c = byte +0x1f0] = 0;
    (**(code **)(*param_1 + 0x8c))(param_2);
  }
  return;
}
```

### decoy: 0x007685f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007685f0: 16-byte aligned: YES
  007685f0  55             PUSH EBP
  007685f1  8B EC          MOV EBP,ESP
  007685f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007685f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1F0 [dec 496]  x1  via EAX
```

### Static facts (script output, report-only)
- size 429 B, RET 4
- callers 3: 0x007664b0, 0x007675b0, 0x00769780
- callees: 0x004452b0 (CALL), 0x0044b8a0 (CALL), 0x0045cfe0 (CALL), 0x0044c8a0 (CALL)
- indirect: 0x00768494 slot 0x8c, 0x0076850c slot 0x8c, 0x0076854d slot 0x8c, 0x007685e5 slot 0x8c
- twins: none

### Live evidence (REA, report-only)
- function | countdown at this+0x1F0 (step this+0x44): expiry or squared length >= const 0x0099EFA0 resets and calls vtbl+0x8C(arg); else float-scaled vtbl+0x8C(arg) | confirmed | procedure_assembly 0x76846d: CMP EDX, dword ptr [ECX + 0x1f0]; 0x768494: CALL EDX
- xref | callers 3: 0x7664b0, 0x7675b0, 0x769780; callees: 0x4452b0, 0x44b8a0, 0x44c8a0, 0x45cfe0; size 429 B RET 4 | confirmed | procedure_callers->[0x7664b0,0x7675b0,0x769780]; procedure_callees->[0x4452b0,0x44b8a0,0x44c8a0,0x45cfe0]; 0x7685ea: RET 0x4

## Card B13a-1-16

GOG function: CSWCCreature::GetFollowingBody  gog 0x00421840  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775970 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421820 CSWCCreature::GetCameraFollowPart -> 0x007759b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00421860 CSWCCreature::GetHeadHeightOffset -> 0x00775990  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d360 CSWCCreatureAppearance::GetFollowingBody -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
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
prologue for 0x00775970: 16-byte aligned: YES
  00775970  55             PUSH EBP
  00775971  8B EC          MOV EBP,ESP
  00775973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775970
string literals (0):
direct callees (1):
  0x0085D320  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775970 FUN_00775970 size=25 ret=0 callers=0 callees=1
THUNK -> 0x0085D320 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775970(undefined4 param_1)

{
  FUN_0085d320(param_1);
  return;
}
```

### decoy: 0x00775950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775950: 16-byte aligned: YES
  00775950  55             PUSH EBP
  00775951  8B EC          MOV EBP,ESP
  00775953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775950
string literals (0):
direct callees (1):
  0x0085D2D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### Static facts (script output, report-only)
- size 25 B, RET 0
- callees: 0x0085d320 (CALL)
- twins: 0x00775930, 0x00775950, 0x00775990 (distinguishing: 0x00775980: CALL 0x0085d320)

### Live evidence (REA, report-only)
- function | loads this+0x224 into ECX and calls 0x85d320; RET no operand | confirmed | procedure_info length=25; procedure_assembly 0x77597a: MOV ECX, dword ptr [EAX + 0x224]; procedure_callers=none; xrefs DATA 0x99eeec
- xref | callers 0: none; callees: 0x85d320; data users: 0x99eeec | confirmed | procedure_callers/procedure_callees 0x00775970; size 25 B RET no operand

## Card B13a-2-02

GOG function: CSWCCreature::UpdateRollingFootstepSound  gog 0x00413e70  callconv=(blank)  param_size_bytes=4  gog_size=516  gog_retn=8
Candidate Steam address: 0x00769620 (size 179, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::ClientServerSyncHandler_Animation | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007696e0:STR0/CALLEE0/SIG0.103/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00413e30 CSWCCreature::ForceAppearanceUpdate -> 0x00768060  gate_class=REVIEW
- GOG-order neighbour after:  0x00414080 CSWCCreature::SetCombatState -> 0x007682b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x00472990 CSWCObject::GetAnimationBase -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam 0x0079f430  in candidate callers: no
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x4c*,0xc5,0x138,0x2e8,0x3b0} | Steam {0x4c*,0x9c,0xa0}
imm sets (* = in both, 1 common): GOG {0x8*,0x10,0x20,0x40,0x272e,0x2736,0x2737,0x2738,0x2739,0x273a,0x27aa,0x27ab} | Steam {0x8*,0x2716,0x2718,0x2719,0x271b,0x271c,0x271e,0x277d,0x27ac}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x00769620: 16-byte aligned: YES
  00769620  55             PUSH EBP
  00769621  8B EC          MOV EBP,ESP
  00769623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00769620
string literals (0):
direct callees (2):
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x007EF5D0  CSWCObject::AnimationFireAndForget
struct offsets touched (top 3):
  +0x4C [dec 76]  x1  via EAX
  +0x9C [dec 156]  x1  via EAX
  +0xA0 [dec 160]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00769620 FUN_00769620 size=179 ret=8 callers=0 callees=2

void __thiscall FUN_00769620(int *param_1,uint param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  
  param_1[0x13 = byte +0x4c] = param_3;
  if ((((((param_2 != 0x2719) && (param_2 != 0x277d)) && (param_2 != 0x271c)) &&
       ((param_2 != 0x271b && (param_2 != 0x271e)))) &&
      ((param_2 != 0x2716 && ((param_2 != 0x2718 && (param_2 != 0x27ac)))))) &&
     (uVar1 = FUN_007ed7d0(), param_2 != (uVar1 & 0xffff))) {
    iVar2 = FUN_007ef5d0(param_2 & 0xffff);
    if (iVar2 == 0) {
      (**(code **)(*param_1 + 0x9c))(param_2 & 0xffff,0,0);
    }
    else {
      (**(code **)(*param_1 + 0xa0))(param_2 & 0xffff,0);
    }
  }
  return;
}
```

### decoy: 0x007696e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007696e0: 16-byte aligned: YES
  007696e0  55             PUSH EBP
  007696e1  8B EC          MOV EBP,ESP
  007696e3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007696e0
string literals (0):
direct callees (4):
  0x005143C0  (unnamed)
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x007EF2C0  CSWCObject::AnimationStationary
  0x00768610  CSWCCreature::RotateCreatureToOrientation
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 179 B, RET 8
- callees: 0x007ed7d0 CSWCObject::GetCurrentAnimation (CALL), 0x007ef5d0 CSWCObject::AnimationFireAndForget (CALL)
- indirect: 0x007696b0 slot 0xa0, 0x007696cb slot 0x9c
- twins: none

### Live evidence (REA, report-only)
- function | stores float at this+0x4c; returns early for 8 listed values; else calls 0x7ed7d0, 0x7ef5d0 and vtbl+0xa0 or vtbl+0x9c; RET 0x8 | confirmed | procedure_info length=179; procedure_assembly 0x76962d: FSTP float ptr [EAX + 0x4c]; procedure_callers=none; xrefs DATA 0x99ef48
- xref | callers 0: none; callees: 0x7ed7d0, 0x7ef5d0; data users: 0x99ef48 | confirmed | procedure_callers/procedure_callees 0x00769620; size 179 B RET 0x8

## Card B13a-1-12

GOG function: CSWCCreature::GetCameraFollowPart  gog 0x00421820  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x007759b0 (size 188, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775990:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421750 CSWCCreature::SetInParty -> 0x00775860  gate_class=REVIEW
- GOG-order neighbour after:  0x00421840 CSWCCreature::GetFollowingBody -> 0x00775970  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d2f0 CSWCCreatureAppearance::GetCameraFollowPart -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x224} | Steam {0x4,0x8,0x75,0x138}
imm sets (* = in both, 0 common): GOG {} | Steam {0x5,0x24,0x2710,0x2711,0x276c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007759b0: 16-byte aligned: YES
  007759b0  55             PUSH EBP
  007759b1  8B EC          MOV EBP,ESP
  007759b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007759b0
string literals (0):
direct callees (4):
  0x0051C6F0  CServerExoApp::IsInjured
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x75 [dec 117]  x1  via EDX
  +0x138 [dec 312]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007759b0 FUN_007759b0 size=188 ret=0 callers=4 callees=4

undefined4 __fastcall FUN_007759b0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x138) != 0) {
    return 0x2711;
  }
  if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
    return 10000;
  }
  iVar1 = FUN_0051c6f0(*(undefined4 *)(param_1 + 4));
  if (iVar1 == 0) {
LAB_00775a63:
    uVar2 = 10000;
  }
  else {
    iVar1 = FUN_0072fb00();
    if (*(char *)(iVar1 + 0x75) == '\x05') {
      uVar2 = 0;
      FUN_0073fb90(0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
      iVar1 = FUN_007e5da0(uVar2);
      if (param_1 == iVar1) goto LAB_00775a63;
    }
    uVar2 = 0x276c;
  }
  return uVar2;
}
```

### decoy: 0x00775990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775990: 16-byte aligned: YES
  00775990  55             PUSH EBP
  00775991  8B EC          MOV EBP,ESP
  00775993  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775990
string literals (0):
direct callees (1):
  0x0085D3D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### Static facts (script output, report-only)
- size 188 B, RET 0
- callers 4: 0x0056d6a0 CSWSCreature::GetPauseReadyAnimation, 0x00776fe0, 0x007779b0, 0x007ed9a0
- callees: 0x0051c6f0 CServerExoApp::IsInjured (CALL), 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5da0 Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0) (CALL)
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

### Live evidence (REA, report-only)
- function | returns 0x2711 if this+0x138 nonzero; else calls 0x0051c6f0, 0x0072fb00, 0x0073fb90, 0x007e5da0 and returns 0x2710 or 0x276c | confirmed | procedure_assembly 0x7759cb: MOV EAX, 0x2711; 0x775a72: RET (body 0x7759b0-0x775a72 in 3 ranges, 188 B, span 195, no operand)
- xref | callers 4: 0x56d6a0, 0x776fe0, 0x7779b0, 0x7ed9a0; callees: 0x51c6f0, 0x72fb00, 0x73fb90, 0x7e5da0; strings/data: none | confirmed | procedure_callers/procedure_callees 0x007759b0; size 188 B RET none

## Card B13a-1-34

GOG function: CSWCCreature::ClearVisibilityList  gog 0x004137c0  callconv=(blank)  param_size_bytes=0  gog_size=141  gog_retn=0
Candidate Steam address: 0x007679f0 (size 141, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00767a80:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413780 Vector::Dot  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00413850 CSWCCreature::BlurWeapon -> 0x00767a80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4c*,0x178*,0x17c*,0x180*} | Steam {0x4c*,0x178*,0x17c*,0x180*}
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
prologue for 0x007679f0: 16-byte aligned: YES
  007679f0  55             PUSH EBP
  007679f1  8B EC          MOV EBP,ESP
  007679f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007679f0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x178 [dec 376]  x5  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x17C [dec 380]  x1  via EDX
  +0x180 [dec 384]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007679f0 FUN_007679f0 size=141 ret=0 callers=2 callees=0

void __fastcall FUN_007679f0(int param_1)

{
  if (*(int *)(param_1 + 0x178) != 0) {
    (**(code **)(**(int **)(param_1 + 0x178) + 0x4c))(0);
    if (*(undefined4 **)(param_1 + 0x178) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x178))(1);
    }
    *(undefined4 *)(param_1 + 0x178) = 0;
    *(undefined4 *)(param_1 + 0x17c) = 0;
    *(undefined4 *)(param_1 + 0x180) = 0;
  }
  return;
}
```

### decoy: 0x00767a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00767a80: 16-byte aligned: YES
  00767a80  55             PUSH EBP
  00767a81  8B EC          MOV EBP,ESP
  00767a83  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00767a80
string literals (0):
direct callees (3):
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x006D5200  (unnamed)
struct offsets touched (top 5):
  +0x2E4 [dec 740]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x34 [dec 52]  x1  via EDX
  +0x198 [dec 408]  x1  via EAX
```

### Static facts (script output, report-only)
- size 141 B, RET 0
- callers 2: 0x0085a9e0, 0x0085b940
- indirect: 0x00767a1e slot 0x4c, 0x00767a44 slot 0x0
- twins: none

### Live evidence (REA, report-only)
- function | if this+0x178 nonzero: indirect vtable calls (slot +0x4c, then slot +0x0); then zeroes this+0x178, +0x17c, +0x180; no resolved direct callees | confirmed | procedure_assembly 0x767a1e: CALL EAX; 0x767a7c: RET (size 141, no operand; body 0x7679f0-0x767a7c)
- xref | callers 2: 0x85a9e0, 0x85b940; callees: none (indirect only); strings/data: none | confirmed | procedure_callers/procedure_callees 0x007679f0; size 141 B RET none

## Card B13a-2-20

GOG function: CSWCCreature::Constructor  gog 0x004102b0  callconv=(blank)  param_size_bytes=0  gog_size=2405  gog_retn=0
Candidate Steam address: 0x00764f80 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::ShowShadowBlob | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764f60:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410290 CSWCCreaturePartyStatus::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040ffe0 CSWCCombatAttackData::Constructor -> Steam ?  in candidate callees: n/a
  0x004721b0 CSWCObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00489500 CSWCSpellCastVisual::Constructor -> Steam ?  in candidate callees: n/a
  0x0048a160 Global::CSWCCreatureAppearance -> Steam ?  in candidate callees: n/a
  0x00490650 CSWCLevelUpStats::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00595eb0 CSWGuiInGameCharacter::Constructor -> Steam 0x0084c3a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x3c,0x40,0x48,0x58,0xfc,0x12c,0x130,0x134,0x138,0x13c,0x140,0x144,...(+101)} | Steam {0x68,0xac}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x5,0x6,0xc,0x11,0x14,0x30,0x48,0x6c,0x88,0x140,0x1cc,...(+17)} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00764f80: 16-byte aligned: YES
  00764f80  55             PUSH EBP
  00764f81  8B EC          MOV EBP,ESP
  00764f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xAC [dec 172]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00764f80 FUN_00764f80 size=44 ret=0 callers=0 callees=0

void __fastcall FUN_00764f80(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xac))(0);
  }
  return;
}
```

### decoy: 0x00764f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764f60: 16-byte aligned: YES
  00764f60  55             PUSH EBP
  00764f61  8B EC          MOV EBP,ESP
  00764f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764f60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 44 B, RET 0
- indirect: 0x00764fa6 slot 0xac
- twins: none

### Live evidence (REA, report-only)
- function | if this+0x68 nonzero: indirect vtable call (slot +0xac) on the this+0x68 object; no resolved direct callees; no resolved callers | confirmed | procedure_assembly 0x764fa6: CALL EAX; 0x764fab: RET (size 44, no operand; body 0x764f80-0x764fab)
- xref | callers 0: none; callees: none (indirect only); strings/data: data 0x99ef18, 0x9a2330, 0x9a2700, 0x9a28e8, 0x9a38b8, 0x9a4df0, 0x9a5600, 0x9a5760, 0x9a6768, 0x9a9180, 0x9a9320 | confirmed | procedure_callers/procedure_callees 0x00764f80; size 44 B RET none

## Card B13a-2-35

GOG function: CSWCCreature::BlurWeapon  gog 0x00413850  callconv=(blank)  param_size_bytes=4  gog_size=152  gog_retn=4
Candidate Steam address: 0x007682b0 (size 397, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRollingFootstepSound | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007680a0:STR0/CALLEE0/SIG0.086/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004137c0 CSWCCreature::ClearVisibilityList -> 0x007679f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xc,0x34*,0x198,0x2e4} | Steam {0x4*,0x34*,0x94,0x9c,0x138,0x11ac}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*,0x1c} | Steam {0x4*,0x10*,0x20,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007682b0: 16-byte aligned: YES
  007682b0  55             PUSH EBP
  007682b1  8B EC          MOV EBP,ESP
  007682b3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007682b0
string literals (0):
direct callees (6):
  0x007680A0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x00773BA0  (unnamed)
struct offsets touched (top 6):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x138 [dec 312]  x2  via EAX,EDX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007682b0 FUN_007682b0 size=397 ret=4 callers=3 callees=6

void __thiscall FUN_007682b0(int param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  iVar4 = *(int *)(param_1 + 0x138);
  *(int *)(param_1 + 0x138) = param_2;
  if (param_2 != 0) {
    FUN_007680a0(0,1);
  }
  if (iVar4 != param_2) {
    piVar3 = (int *)FUN_0077d800();
    if (piVar3 != (int *)0x0) {
      iVar4 = (**(code **)(*piVar3 + 0x94))();
      if (iVar4 == 0) {
        if (piVar3[0x46b = byte +0x11ac] == 0) {
          bVar1 = false;
        }
        else {
          sVar2 = (**(code **)(*piVar3 + 0x9c))(0);
          bVar1 = sVar2 < 1;
        }
        if (!bVar1) {
          iVar4 = FUN_0076e610(0x10);
          if (iVar4 != 0x7f000000) {
            piVar3 = (int *)FUN_0073f4d0(iVar4);
            if (piVar3 != (int *)0x0) {
              iVar4 = (**(code **)(*piVar3 + 0x34))();
              if (iVar4 != 0) {
                uVar7 = 0;
                uVar6 = 1;
                uVar5 = 0;
                iVar4 = param_2;
                (**(code **)(*piVar3 + 0x34))(param_2,0,1,0);
                FUN_007fe6c0(iVar4,uVar5,uVar6,uVar7);
              }
            }
          }
          iVar4 = FUN_0076e610(0x20);
          if (iVar4 != 0x7f000000) {
            uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 4);
            piVar3 = (int *)FUN_0073f4d0(iVar4);
            if (piVar3 != (int *)0x0) {
              iVar4 = (**(code **)(*piVar3 + 0x34))(param_1,uVar5);
              if (iVar4 != 0) {
                uVar7 = 0;
                uVar6 = 1;
                uVar5 = 0;
                iVar4 = param_2;
                (**(code **)(*piVar3 + 0x34))(param_2,0,1,0);
                FUN_007fe6c0(iVar4,uVar5,uVar6,uVar7);
              }
            }
          }
        }
      }
    }
  }
  FUN_00773ba0(param_2);
  return;
}
```

### decoy: 0x007680a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007680a0: 16-byte aligned: YES
  007680a0  55             PUSH EBP
  007680a1  8B EC          MOV EBP,ESP
  007680a3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007680a0
string literals (0):
direct callees (3):
  0x007ED830  CSWCObject::GetAnimationBase
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 6):
  +0x4C [dec 76]  x8  via EDX
  +0x4 [dec 4]  x2  via EDX
  +0xC5 [dec 197]  x1  via EAX
  +0x138 [dec 312]  x1  via ECX
  +0x2E8 [dec 744]  x1  via EDX
  +0x3B0 [dec 944]  x1  via ECX
```

### Static facts (script output, report-only)
- size 397 B, RET 4
- callers 3: 0x0057a560 CSWSCreature::SetCombatState, 0x00807730, 0x008079b0
- callees: 0x007680a0 (CALL), 0x0077d800 (CALL), 0x0076e610 (CALL), 0x0073f4d0 CClientExoApp::GetGameObject (CALL), 0x007fe6c0 (CALL), 0x00773ba0 (CALL)
- indirect: 0x00768312 slot 0x94, 0x00768338 slot 0x9c, 0x007683a4 slot 0x34, 0x007683bf slot 0x34, 0x00768409 slot 0x34, ...
- globals: 0x00a1b4a4 READ
- twins: none

### Live evidence (REA, report-only)
- function | 1-arg setter: stores arg in this+0x138 (old value kept); if arg nonzero calls 0x7680a0(1,0); if changed probes vtbl+0x94/+0x9c/+0x34 objects; calls 0x773ba0(arg) at end | confirmed | procedure_assembly 0x7682cb: MOV dword ptr [EDX + 0x138], EAX; 0x76842d: MOV EAX, dword ptr [EBP + 0x8]
- xref | callers 3: 0x57a560,0x807730,0x8079b0; callees: 0x7680a0,0x76e610,0x773ba0,0x77d800,0x73f4d0,0x7fe6c0 | confirmed | procedure_callers/procedure_callees 0x007682b0; 397 B RET 0x4

## Card B13a-1-07

GOG function: CSWCCreature::GetHeadTurnHorizontal  gog 0x00420070  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x00774200 (size 93, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007741a0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041fac0 CSWCCreature::LookAt -> 0x00773bf0  gate_class=AUTO
- GOG-order neighbour after:  0x004200d0 CSWCCreature::GetHeadTurnVertical -> 0x00774260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x006d1100 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader -> Steam 0x005c6340  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x10*,0x18*,0x130*,0x224*} | Steam {0x10*,0x18*,0x130*,0x224*}
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
prologue for 0x00774200: 16-byte aligned: YES
  00774200  55             PUSH EBP
  00774201  8B EC          MOV EBP,ESP
  00774203  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00774200
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 4):
  +0x10 [dec 16]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00774200 FUN_00774200 size=93 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_00774200(int param_1)

{
  int iVar1;
  float local_8;
  
  iVar1 = FUN_0071b3f0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdc8,&local_8);
  if (iVar1 == 0) {
    local_8 = _DAT_009919f0;
  }
  return (float10)local_8;
}
```

### decoy: 0x007741a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007741a0: 16-byte aligned: YES
  007741a0  55             PUSH EBP
  007741a1  8B EC          MOV EBP,ESP
  007741a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007741a0
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 4):
  +0x10 [dec 16]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
  +0x224 [dec 548]  x1  via EAX
```

### Static facts (script output, report-only)
- size 93 B, RET 0
- callers 1: 0x00773bf0
- callees: 0x0071b3f0 (CALL)
- globals: 0x00a1b4d0 READ
- twins: none

### Live evidence (REA, report-only)
- function | reads this+0x224 word; calls 0x0071b3f0 with out-ptr; returns out-param float, or const 0x009919f0 if callee returns 0 | confirmed | procedure_assembly 0x774244: CALL 0x0071b3f0; 0x77425c: RET (size 93, no operand; body 0x774200-0x77425c)
- xref | callers 1: 0x773bf0; callees: 0x71b3f0; strings/data: none | confirmed | procedure_callers/procedure_callees 0x00774200; size 93 B RET none

## Card B13a-2-25

GOG function: CSWCCreature::IsDriveAnimationBlocked  gog 0x00422dd0  callconv=(blank)  param_size_bytes=8  gog_size=37  gog_retn=8
Candidate Steam address: 0x00776e10 (size 37, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00422e00:0.83>0.09
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00776db0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422ca0 CWalkHitInfo::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x3c8*,0x3cc*} | Steam {0x3c8*,0x3cc*}
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
prologue for 0x00776e10: 16-byte aligned: YES
  00776e10  55             PUSH EBP
  00776e11  8B EC          MOV EBP,ESP
  00776e13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776e10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C8 [dec 968]  x1  via ECX
  +0x3CC [dec 972]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00776e10 FUN_00776e10 size=37 ret=8 callers=1 callees=0

void __thiscall FUN_00776e10(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x3cc) = param_3;
  *(undefined4 *)(param_1 + 0x3c8) = param_2;
  return;
}
```

### decoy: 0x00776db0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00776db0: 16-byte aligned: YES
  00776db0  55             PUSH EBP
  00776db1  8B EC          MOV EBP,ESP
  00776db3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00776db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C0 [dec 960]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 37 B, RET 8
- callers 1: 0x00865830
- twins: none

### Live evidence (REA, report-only)
- function | stores two stack floats to this+0x3c8 and this+0x3cc; RET 0x8; no callees | confirmed | procedure_info length=37; procedure_assembly 0x776e29: FSTP float ptr [ECX + 0x3c8]; procedure_callers=[0x865830]; xrefs CALL 0x867a33 only
- xref | callers 1: 0x865830; callees: none; data users: none | confirmed | procedure_callers/procedure_callees 0x00776e10; size 37 B RET 0x8
