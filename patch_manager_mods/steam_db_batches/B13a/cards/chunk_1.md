# Review cards B13a chunk 1

Answer each card with one TSV row in `steam_db_batches/B13a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B13a-1-01

GOG function: CSWCCreature::ResetDesiredPositionTimer  gog 0x0041f7b0  callconv=(blank)  param_size_bytes=0  gog_size=34  gog_retn=0
Candidate Steam address: 0x007738e0 (size 34, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00773860:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f730 CSWCCreature::ResetAnimationState -> 0x007737e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f7e0 CSWCCreature::SetSpecialWalkAnimation -> 0x00773910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007738e0: 16-byte aligned: YES
  007738e0  55             PUSH EBP
  007738e1  8B EC          MOV EBP,ESP
  007738e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007738e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007738e0 FUN_007738e0 size=34 ret=0 callers=0 callees=0

void FUN_007738e0(undefined4 param_1,undefined4 param_2,int param_3)

{
  *(undefined4 *)(param_3 + 4) = 0;
  *(undefined4 *)(param_3 + 8) = 0;
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

## Card B13a-1-02

GOG function: CSWCCreature::SetCombatState  gog 0x00414080  callconv=(blank)  param_size_bytes=4  gog_size=397  gog_retn=4
Candidate Steam address: 0x007682b0 (size 397, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRollingFootstepSound | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: SetCombatState 0x57a560); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007680a0:STR0/CALLEE0/SIG0.171/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> 0x007680a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00414210 CSWCCreature::MoveCreatureToPosition -> 0x00768440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: yes
  0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> Steam ?  in candidate callees: n/a
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x0041fa70 Global::GetCreatureRadius -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0047f970 CSWCItem::PowerItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00693090 CSWSCreature::SetCombatState -> Steam 0x0057a560  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x34*,0x94*,0x9c*,0x138*,0x11ac*} | Steam {0x4*,0x34*,0x94*,0x9c*,0x138*,0x11ac*}
imm sets (* = in both, 4 common): GOG {0x4*,0x10*,0x20*,0x7f000000*} | Steam {0x4*,0x10*,0x20*,0x7f000000*}

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

## Card B13a-1-03

GOG function: CSWCCreature::PerformGiveItem  gog 0x00421560  callconv=(blank)  param_size_bytes=12  gog_size=76  gog_retn=12
Candidate Steam address: 0x00775670 (size 76, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007756c0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004214f0 CSWCCreature::PerformUseItem -> 0x00775600  gate_class=REVIEW
- GOG-order neighbour after:  0x004215b0 CSWCCreature::PerformAttack -> 0x007756c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x005067a0 CSWCMessage::SendPlayerToServerInput_GiveItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00775670: 16-byte aligned: YES
  00775670  55             PUSH EBP
  00775671  8B EC          MOV EBP,ESP
  00775673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775670
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087AEC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00775670 FUN_00775670 size=76 ret=12 callers=1 callees=2

void __thiscall FUN_00775670(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  
  iVar1 = FUN_0073f810();
  if (iVar1 != 0) {
    FUN_0087aec0(*(undefined4 *)(param_1 + 4),param_2,param_3,param_4);
  }
  return;
}
```

### decoy: 0x007756c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007756c0: 16-byte aligned: YES
  007756c0  55             PUSH EBP
  007756c1  8B EC          MOV EBP,ESP
  007756c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007756c0
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x008799A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B13a-1-04

GOG function: CSWCCreature::SetSpecialWalkAnimation  gog 0x0041f7e0  callconv=(blank)  param_size_bytes=1  gog_size=38  gog_retn=4
Candidate Steam address: 0x00773910 (size 38, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::ResetAnimationState | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00773940:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f7b0 CSWCCreature::ResetDesiredPositionTimer -> 0x007738e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f810 CSWCCreature::SetRunLooksAwful -> 0x00773940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00472990 CSWCObject::GetAnimationBase -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc8*} | Steam {0xc8*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00773910: 16-byte aligned: YES
  00773910  55             PUSH EBP
  00773911  8B EC          MOV EBP,ESP
  00773913  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00773910
string literals (0):
direct callees (1):
  0x007ED830  CSWCObject::GetAnimationBase
struct offsets touched (top 1):
  +0xC8 [dec 200]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00773910 FUN_00773910 size=38 ret=4 callers=1 callees=1

void __thiscall FUN_00773910(undefined4 param_1,undefined1 param_2)

{
  int iVar1;
  
  iVar1 = FUN_007ed830(param_1);
  *(undefined1 *)(iVar1 + 0xc8 [dec 200]) = param_2;
  return;
}
```

### decoy: 0x00773940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773940: 16-byte aligned: YES
  00773940  55             PUSH EBP
  00773941  8B EC          MOV EBP,ESP
  00773943  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773940
string literals (0):
direct callees (1):
  0x007ED830  CSWCObject::GetAnimationBase
struct offsets touched (top 1):
  +0xCC [dec 204]  x1  via EAX
```

## Card B13a-1-05

GOG function: CSWCCreature::ResetAnimationState  gog 0x0041f730  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=0
Candidate Steam address: 0x007737e0 (size 116, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00773860:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0041f6b0 CSWCCreature::UnloadMonkVisuals -> 0x007737e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f7b0 CSWCCreature::ResetDesiredPositionTimer -> 0x007738e0  gate_class=REVIEW

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

## Card B13a-1-08

GOG function: CSWCCreature::PerformAttack  gog 0x004215b0  callconv=(blank)  param_size_bytes=4  gog_size=57  gog_retn=4
Candidate Steam address: 0x007756c0 (size 57, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: MenuActionBash 0x87bd10, BashPlaceable 0x841d40); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775670:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421560 CSWCCreature::PerformGiveItem -> 0x00775670  gate_class=REVIEW
- GOG-order neighbour after:  0x004215f0 CSWCCreature::PerformCastSpell -> 0x00775700  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x00505280 CSWCMessage::SendPlayerToServerInput_Attack -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005075f0 CSWCDoor::MenuActionBash -> Steam 0x0087bd10  in candidate callers: yes
  0x005355c0 CSWCPlaceable::BashPlaceable -> Steam 0x00841d40  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x007756c0: 16-byte aligned: YES
  007756c0  55             PUSH EBP
  007756c1  8B EC          MOV EBP,ESP
  007756c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007756c0
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x008799A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007756c0 FUN_007756c0 size=57 ret=4 callers=2 callees=2

void __thiscall FUN_007756c0(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  
  iVar1 = FUN_0073f810(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  if (iVar1 != 0) {
    FUN_008799a0(param_2,0);
  }
  return;
}
```

### decoy: 0x00775670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775670: 16-byte aligned: YES
  00775670  55             PUSH EBP
  00775671  8B EC          MOV EBP,ESP
  00775673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775670
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087AEC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B13a-1-09

GOG function: CSWCCreature::UpdateMonkVisuals  gog 0x0041f3d0  callconv=__thiscall  param_size_bytes=0  gog_size=89  gog_retn=0
Candidate Steam address: 0x007737e0 (size 116, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007736d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041e770 CSWCCreature::DoFootstepVisual -> 0x00772900  gate_class=AUTO
- GOG-order neighbour after:  0x0041f430 CSWCCreature::DoFootstepAudio -> 0x00773560  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0081b710 CAurObjectReference::Destructor -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 24): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x10,0x18,0x70,0x130,0x224}
imm sets (* = in both, 0 common): GOG {0x8,0x38} | Steam {0x18}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
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

### decoy: 0x007736d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007736d0: 16-byte aligned: YES
  007736d0  55             PUSH EBP
  007736d1  8B EC          MOV EBP,ESP
  007736d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

## Card B13a-1-13

GOG function: CSWCCreature::GetCurrentWayPoint  gog 0x00415cf0  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00769ed0 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00769780:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004155a0 CSWCCreature::ClientServerSyncHandler_Position -> 0x00769780  gate_class=AUTO
- GOG-order neighbour after:  0x00415d10 CSWCCreature::GetEndWayPoint -> 0x00769ef0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c8*} | Steam {0x1c8*}
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
prologue for 0x00769ed0: 16-byte aligned: YES
  00769ed0  55             PUSH EBP
  00769ed1  8B EC          MOV EBP,ESP
  00769ed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00769ed0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C8 [dec 456]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00769ed0 FUN_00769ed0 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00769ed0(int param_1)

{
  return *(undefined4 *)(param_1 + 0x1c8);
}
```

### decoy: 0x00769780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00769780: 16-byte aligned: YES
  00769780  55             PUSH EBP
  00769781  8B EC          MOV EBP,ESP
  00769783  81 EC B8 00 00 00 SUB ESP,0xb8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00769780
string literals (0):
direct callees (13):
  0x00768440  CSWCCreature::SetCombatState
  0x007EF2C0  CSWCObject::AnimationStationary
  0x007EF3E0  CSWCObject::AnimationWalking
  0x0071B3F0  (unnamed)
  0x007EF470  CSWCObject::AnimationRunning
  0x0085E2B0  Global::GetRunLooksAwfulRate
  0x00773940  CSWCCreature::SetRunLooksAwful
  0x00769420  CSWCCreature::ComputeSpeedFactor
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x007ED910  (unnamed)
  0x0070AE90  (unnamed)
  0x0070B3F0  (unnamed)
  0x00768610  CSWCCreature::RotateCreatureToOrientation
struct offsets touched (top 10):
  +0x68 [dec 104]  x23  via EAX,ECX,EDX
  +0xBA [dec 186]  x6  via EAX,ECX,EDX
  +0x38 [dec 56]  x5  via EDX
  +0x224 [dec 548]  x5  via ECX,EDX
  +0x10 [dec 16]  x4  via ECX
  +0x18 [dec 24]  x4  via EAX
  +0x54 [dec 84]  x4  via EDX
  +0x130 [dec 304]  x4  via EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x3  via EAX,ECX,EDX
```

## Card B13a-1-14

GOG function: CSWCCreature::SetEngagedState  gog 0x00422e80  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00776db0 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775aa0:STR0/CALLEE0/SIG0.022/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00422ea0 Global::SetQuededAnimationCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x13c} | Steam {0x3c0}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00776db0: 16-byte aligned: YES
  00776db0  55             PUSH EBP
  00776db1  8B EC          MOV EBP,ESP
  00776db3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C0 [dec 960]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00776db0 FUN_00776db0 size=84 ret=4 callers=3 callees=0

void __thiscall FUN_00776db0(int param_1,int param_2)

{
  if (param_2 == 0) {
    *(undefined4 *)(param_1 + 0x3c0) = 0;
  }
  else if (param_2 == 1) {
    *(undefined4 *)(param_1 + 0x3c0) = 1;
  }
  else if (param_2 == 2) {
    *(undefined4 *)(param_1 + 0x3c0) = 2;
  }
  return;
}
```

### decoy: 0x00775aa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775aa0: 16-byte aligned: YES
  00775aa0  55             PUSH EBP
  00775aa1  8B EC          MOV EBP,ESP
  00775aa3  81 EC A4 05 00 00 SUB ESP,0x5a4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775aa0
string literals (1):
  0x0099F208  at 007762c1  '.\\\\nwccreature.cpp'
direct callees (39):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x00552E60  CSWSArea::ClosestPathPoint
  0x0051CA40  CServerExoApp::ComputeHeight
  0x005453C0  CSWSObject::GetArea
  0x0054B3B0  CSWSArea::GetRoomIndex
  0x0054B370  CSWSArea::GetRoom_2
  0x00651E30  (unnamed)
  0x006532B0  (unnamed)
  0x005E9E90  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0052BB60  CSWSArea::ClearLineOfSight
  0x0064E080  (unnamed)
  0x00650BE0  (unnamed)
  0x006DAA00  CSWSCreature::IsDebilitated
  0x00776E40  CSWCCreature::InCreatureCombat
  0x0073F430  CClientExoApp::GetModule
  0x004452B0  Vector::operator- (Vector3 subtract)
struct offsets touched (top 10):
  +0x4 [dec 4]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x23  via EAX,ECX,EDX
  +0x3C [dec 60]  x10  via EAX,ECX,EDX
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x260 [dec 608]  x2  via EDX
  +0x380 [dec 896]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x8C [dec 140]  x1  via EDX
  +0x1B0 [dec 432]  x1  via ECX
```

## Card B13a-1-15

GOG function: CSWCCreature::ForceAppearanceUpdate  gog 0x00413e30  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x00768060 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetLoopingAnimation 0x7ed980)
- best decoy (neighbour/other candidate) scores: 0x00767b20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW
- GOG-order neighbour after:  0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> 0x007680a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00472ae0 CSWCObject::GetLoopingAnimation -> Steam 0x007ed980  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x9c*} | Steam {0x9c*}
imm sets (* = in both, 1 common): GOG {0x2711*} | Steam {0x2711*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00768060: 16-byte aligned: YES
  00768060  55             PUSH EBP
  00768061  8B EC          MOV EBP,ESP
  00768063  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00768060
string literals (0):
direct callees (1):
  0x007ED980  CSWCObject::GetLoopingAnimation
struct offsets touched (top 1):
  +0x9C [dec 156]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00768060 FUN_00768060 size=59 ret=0 callers=1 callees=1

undefined4 __fastcall FUN_00768060(int *param_1)

{
  short sVar1;
  
  sVar1 = FUN_007ed980();
  if (sVar1 == 0x2711) {
    (**(code **)(*param_1 + 0x9c))(0x2711,0,0);
  }
  return 1;
}
```

### decoy: 0x00767b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00767b20: 16-byte aligned: YES
  00767b20  55             PUSH EBP
  00767b21  8B EC          MOV EBP,ESP
  00767b23  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00767b20
string literals (0):
direct callees (9):
  0x006C1450  (unnamed)
  0x007107F0  (unnamed)
  0x00710EB0  (unnamed)
  0x00710CA0  (unnamed)
  0x00859CA0  (unnamed)
  0x00859DC0  (unnamed)
  0x00859520  (unnamed)
  0x00859EF0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x308 [dec 776]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x2D8 [dec 728]  x5  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x2DC [dec 732]  x2  via ECX
```

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

## Card B13a-1-17

GOG function: CSWCCreature::HideEquippedHeadItem  gog 0x0041f940  callconv=(blank)  param_size_bytes=0  gog_size=57  gog_retn=0
Candidate Steam address: 0x00773a70 (size 57, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00773ab0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f8e0 CSWCCreature::HideEquippedItems -> 0x00773a10  gate_class=AUTO
- GOG-order neighbour after:  0x0041f980 CSWCCreature::ShowEquippedHeadItem -> 0x00773ab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x68*,0x88*,0x3e8*} | Steam {0x68*,0x88*,0x3e8*}
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
prologue for 0x00773a70: 16-byte aligned: YES
  00773a70  55             PUSH EBP
  00773a71  8B EC          MOV EBP,ESP
  00773a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00773a70
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x88 [dec 136]  x1  via EDX
  +0x3E8 [dec 1000]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00773a70 FUN_00773a70 size=57 ret=0 callers=1 callees=0

void __fastcall FUN_00773a70(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0x88))();
    *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 1;
  }
  return;
}
```

### decoy: 0x00773ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773ab0: 16-byte aligned: YES
  00773ab0  55             PUSH EBP
  00773ab1  8B EC          MOV EBP,ESP
  00773ab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x8C [dec 140]  x1  via EDX
  +0x3E8 [dec 1000]  x1  via ECX
```

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

## Card B13a-1-19

GOG function: CSWCCreature::DeleteSpellLikeAbilityList  gog 0x004216d0  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=0
Candidate Steam address: 0x007757e0 (size 123, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::CreateNewSpellLikeAbilityList | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775760:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421650 CSWCCreature::CreateNewSpellLikeAbilityList -> 0x00775760  gate_class=REVIEW
- GOG-order neighbour after:  0x00421750 CSWCCreature::SetInParty -> 0x00775860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0042c250 unnamed -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3b8*} | Steam {0x3b8*}
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
prologue for 0x007757e0: 16-byte aligned: YES
  007757e0  55             PUSH EBP
  007757e1  8B EC          MOV EBP,ESP
  007757e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007757e0
string literals (0):
direct callees (3):
  0x0064DD50  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x3B8 [dec 952]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007757e0 FUN_007757e0 size=123 ret=0 callers=1 callees=3

void __fastcall FUN_007757e0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x3b8) != 0) {
    FUN_0064dd50(0);
    iVar1 = *(int *)(param_1 + 0x3b8);
    if (iVar1 != 0) {
      Free();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0x3b8) = 0;
  }
  return;
}
```

### decoy: 0x00775760 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775760: 16-byte aligned: YES
  00775760  55             PUSH EBP
  00775761  8B EC          MOV EBP,ESP
  00775763  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00775760
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0064DD10  (unnamed)
struct offsets touched (top 1):
  +0x3B8 [dec 952]  x1  via ECX
```

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

## Card B13a-1-21

GOG function: CSWCCreature::ShowEquippedHeadItem  gog 0x0041f980  callconv=(blank)  param_size_bytes=0  gog_size=57  gog_retn=0
Candidate Steam address: 0x00773ab0 (size 57, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00773a70:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f940 CSWCCreature::HideEquippedHeadItem -> 0x00773a70  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f9c0 CSWCCreature::ShowEquippedItems -> 0x00773af0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x68*,0x8c*,0x3e8*} | Steam {0x68*,0x8c*,0x3e8*}
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
prologue for 0x00773ab0: 16-byte aligned: YES
  00773ab0  55             PUSH EBP
  00773ab1  8B EC          MOV EBP,ESP
  00773ab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00773ab0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x8C [dec 140]  x1  via EDX
  +0x3E8 [dec 1000]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00773ab0 FUN_00773ab0 size=57 ret=0 callers=1 callees=0

void __fastcall FUN_00773ab0(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0x8c))();
    *(undefined4 *)(param_1 + 0x3e8 [dec 1000]) = 0;
  }
  return;
}
```

### decoy: 0x00773a70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773a70: 16-byte aligned: YES
  00773a70  55             PUSH EBP
  00773a71  8B EC          MOV EBP,ESP
  00773a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773a70
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x88 [dec 136]  x1  via EDX
  +0x3E8 [dec 1000]  x1  via ECX
```

## Card B13a-1-22

GOG function: CSWCCreature::PerformUseItem  gog 0x004214f0  callconv=(blank)  param_size_bytes=16  gog_size=99  gog_retn=16
Candidate Steam address: 0x00775600 (size 99, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: MenuActionUseItem 0x842b90, MenuActionSetPlaceableMine 0x842bd0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775670:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421480 CSWCCreature::PerformSkill -> 0x00775590  gate_class=REVIEW
- GOG-order neighbour after:  0x00421560 CSWCCreature::PerformGiveItem -> 0x00775670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x005053d0 CSWCMessage::SendPlayerToServerInput_UseItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00536410 CSWCPlaceable::MenuActionUseItem -> Steam 0x00842b90  in candidate callers: yes
  0x00536450 CSWCPlaceable::MenuActionSetPlaceableMine -> Steam 0x00842bd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x10*,0x14*,0xffff*} | Steam {0x10*,0x14*,0xffff*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00775600: 16-byte aligned: YES
  00775600  55             PUSH EBP
  00775601  8B EC          MOV EBP,ESP
  00775603  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775600
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x00879AF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775600 FUN_00775600 size=99 ret=16 callers=5 callees=2

void __thiscall
FUN_00775600(undefined4 param_1,undefined4 param_2,uint param_3,undefined4 param_4,
            undefined4 param_5)

{
  int iVar1;
  
  iVar1 = FUN_0073f810(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  if (iVar1 != 0) {
    FUN_00879af0(param_2,param_3 & 0xff,(int)param_3 >> 0x10 & 0xff,param_4,param_5);
  }
  return;
}
```

### decoy: 0x00775670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775670: 16-byte aligned: YES
  00775670  55             PUSH EBP
  00775671  8B EC          MOV EBP,ESP
  00775673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775670
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087AEC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B13a-1-23

GOG function: CSWCCreature::SetEquippedItemID  gog 0x0041a710  callconv=(blank)  param_size_bytes=8  gog_size=473  gog_retn=8
Candidate Steam address: 0x0076e8a0 (size 473, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.78) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0076e610:STR0/CALLEE0/SIG0.958/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041a480 CSWCCreature::GetEquippedItemID -> 0x0076e610  gate_class=AUTO
- GOG-order neighbour after:  0x0041a990 CSWCCreature::SetLastInstant -> 0x0076eb20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x228*,0x22c*,0x230*,0x234*,0x238*,0x23c*,0x240*,0x244*,0x248*,0x24c*,0x250*,0x254*,0x258*,0x25c*,...(+3)} | Steam {0x228*,0x22c*,0x230*,0x234*,0x238*,0x23c*,0x240*,0x244*,0x248*,0x24c*,0x250*,0x254*,0x258*,0x25c*,...(+3)}
imm sets (* = in both, 11 common): GOG {0x8*,0x7f*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*,0x40000*,0x80000*} | Steam {0x8*,0x7f*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*,0x40000*,0x80000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0076e8a0: 16-byte aligned: YES
  0076e8a0  55             PUSH EBP
  0076e8a1  8B EC          MOV EBP,ESP
  0076e8a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076e8a0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x228 [dec 552]  x1  via ECX
  +0x22C [dec 556]  x1  via EAX
  +0x230 [dec 560]  x1  via EDX
  +0x234 [dec 564]  x1  via ECX
  +0x238 [dec 568]  x1  via EAX
  +0x23C [dec 572]  x1  via EDX
  +0x240 [dec 576]  x1  via ECX
  +0x244 [dec 580]  x1  via EAX
  +0x248 [dec 584]  x1  via EDX
  +0x24C [dec 588]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0076e8a0 FUN_0076e8a0 size=473 ret=8 callers=4 callees=0

void __thiscall FUN_0076e8a0(int param_1,uint param_2,undefined4 param_3)

{
  if (param_2 < 0x101) {
    if (param_2 == 0x100) {
      *(undefined4 *)(param_1 + 0x248) = param_3;
    }
    else {
      switch(param_2) {
      case 1:
        *(undefined4 *)(param_1 + 0x228) = param_3;
        break;
      case 2:
        *(undefined4 *)(param_1 + 0x22c) = param_3;
        break;
      case 4:
        *(undefined4 *)(param_1 + 0x230) = param_3;
        break;
      case 8:
        *(undefined4 *)(param_1 + 0x234) = param_3;
        break;
      case 0x10:
        *(undefined4 *)(param_1 + 0x238) = param_3;
        break;
      case 0x20:
        *(undefined4 *)(param_1 + 0x23c) = param_3;
        break;
      case 0x40:
        *(undefined4 *)(param_1 + 0x240) = param_3;
        break;
      case 0x80:
        *(undefined4 *)(param_1 + 0x244) = param_3;
      }
    }
  }
  else if (param_2 < 0x10001) {
    if (param_2 == 0x10000) {
      *(undefined4 *)(param_1 + 0x25c) = param_3;
    }
    else if (param_2 < 0x4001) {
      if (param_2 == 0x4000) {
        *(undefined4 *)(param_1 + 0x254) = param_3;
      }
      else if (param_2 == 0x200) {
        *(undefined4 *)(param_1 + 0x24c) = param_3;
      }
      else if (param_2 == 0x400) {
        *(undefined4 *)(param_1 + 0x250) = param_3;
      }
    }
    else if (param_2 == 0x8000) {
      *(undefined4 *)(param_1 + 0x258 [dec 600]) = param_3;
    }
  }
  else if (param_2 == 0x20000) {
    *(undefined4 *)(param_1 + 0x260) = param_3;
  }
  else if (param_2 == 0x40000) {
    *(undefined4 *)(param_1 + 0x264) = param_3;
  }
  else if (param_2 == 0x80000) {
    *(undefined4 *)(param_1 + 0x268) = param_3;
  }
  return;
}
```

### decoy: 0x0076e610 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076e610: 16-byte aligned: YES
  0076e610  55             PUSH EBP
  0076e611  8B EC          MOV EBP,ESP
  0076e613  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B13a-1-24

GOG function: CSWCCreature::UnHilite  gog 0x00420920  callconv=(blank)  param_size_bytes=1  gog_size=42  gog_retn=4
Candidate Steam address: 0x00774a50 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00774a10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004208e0 CSWCCreature::Hilite -> 0x00774a10  gate_class=REVIEW
- GOG-order neighbour after:  0x00420950 CSWCCreature::GetCreatureLight -> 0x00774a80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c9350 CSWCAnimBase::UnHilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
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
prologue for 0x00774a50: 16-byte aligned: YES
  00774a50  55             PUSH EBP
  00774a51  8B EC          MOV EBP,ESP
  00774a53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00774a50
string literals (0):
direct callees (1):
  0x00861480  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00774a50 FUN_00774a50 size=42 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00774a50(int param_1,undefined1 param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x68) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00861480(param_2);
  }
  return uVar1;
}
```

### decoy: 0x00774a10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774a10: 16-byte aligned: YES
  00774a10  55             PUSH EBP
  00774a11  8B EC          MOV EBP,ESP
  00774a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00774a10
string literals (0):
direct callees (1):
  0x00861400  CSWCAnimBase::Hilite
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

## Card B13a-1-25

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077f6c0 (size 819, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::LoadRollingFootstepSound | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
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
disp sets (* = in both, 3 common): GOG {0x448*,0x450*,0x454*} | Steam {0x4,0x10,0x18,0x5c,0x130,0x224,0x448*,0x450*,0x454*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x8,0x12,0x13,0x14,0x24,0x84,0x44c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0077f6c0: 16-byte aligned: YES
  0077f6c0  55             PUSH EBP
  0077f6c1  8B EC          MOV EBP,ESP
  0077f6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077f6c0
string literals (0):
direct callees (13):
  0x00733540  CExoString::Constructor
  0x0071B8B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0071AFA0  C2DA::GetCExoStringEntry
  0x00919723  operator new
  0x00710810  (unnamed)
  0x0070AB90  (unnamed)
  0x0070ACF0  (unnamed)
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0070ACC0  (unnamed)
  0x0070AE30  (unnamed)
  0x0070AE90  (unnamed)
  0x0071B3F0  (unnamed)
struct offsets touched (top 9):
  +0x448 [dec 1096]  x9  via EAX,ECX,EDX
  +0x130 [dec 304]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0x5C [dec 92]  x2  via EAX
  +0x224 [dec 548]  x2  via EAX,EDX
  +0x450 [dec 1104]  x1  via EDX
  +0x454 [dec 1108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077f6c0 FUN_0077f6c0 size=819 ret=0 callers=1 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_0077f6c0(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 local_94;
  undefined1 local_4c [16];
  int local_3c;
  undefined4 local_38;
  uint local_34;
  uint local_30;
  undefined4 *local_2c;
  undefined4 *local_28;
  int local_24;
  undefined4 local_20;
  undefined1 local_1c [11];
  undefined1 local_11;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965443;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (*(int *)(param_1 + 0x448) != 0) {
    local_2c = *(undefined4 **)(param_1 + 0x448);
    local_28 = local_2c;
    if (local_2c != (undefined4 *)0x0) {
      (**(code **)*local_2c)(1);
    }
    *(undefined4 *)(param_1 + 0x448) = 0;
  }
  iVar1 = FUN_0071b8b0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdbc,&local_20);
  if (iVar1 == 0) {
    local_30 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar3 = local_30;
  }
  else {
    iVar1 = FUN_0071afa0(local_20,DAT_00a0fe6c,local_1c);
    if (iVar1 == 0) {
      local_34 = 0;
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar3 = local_34;
    }
    else {
      local_3c = FUN_00919723(8);
      local_8._0_1_ = 1;
      if (local_3c == 0) {
        local_94 = 0;
      }
      else {
        FUN_00710810(local_1c);
        local_94 = FUN_0070ab90(local_4c);
      }
      local_38 = local_94;
      local_8 = (uint)local_8._1_3_ << 8;
      *(undefined4 *)(param_1 + 0x448) = local_94;
      FUN_0070acf0(1);
      iVar1 = *(int *)(param_1 + 4);
      iVar2 = FUN_0073f470();
      if (iVar1 == iVar2) {
        local_11 = 0x13;
      }
      else {
        FUN_0071b8b0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdf4,&local_24);
        if (((local_24 == 1) || (local_24 == 2)) || (local_24 == 3)) {
          local_11 = 0x14;
        }
        else {
          local_11 = 0x12;
/* ... 16 more lines truncated by --max-lines */
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

## Card B13a-1-26

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077fb10 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRollingFootstepSound_2 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077fac0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042ba20 CSWCCreature::PlayRollingFootstepSound -> 0x0077fa00  gate_class=AUTO
- GOG-order neighbour after:  0x0042bb30 CSWCCreature::UpdateRollingFootstepSound_2 -> 0x0077fb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x448*,0x450,0x454} | Steam {0x448*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x83,0x2712}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0077fb10: 16-byte aligned: YES
  0077fb10  55             PUSH EBP
  0077fb11  8B EC          MOV EBP,ESP
  0077fb13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fb10
string literals (0):
direct callees (3):
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x0077FA00  CSWCCreature::PlayRollingFootstepSound
  0x0077FAC0  CSWCCreature::IsInjured
struct offsets touched (top 2):
  +0x448 [dec 1096]  x1  via EAX
  +0x77FB8C [dec 7863180]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fb10 FUN_0077fb10 size=111 ret=0 callers=1 callees=3

void __fastcall FUN_0077fb10(int param_1)

{
  undefined2 uVar1;
  
  if (*(int *)(param_1 + 0x448) != 0) {
    uVar1 = FUN_007ed7d0();
    switch(uVar1) {
    case 0x2712:
    case 0x2713:
    case 0x276d:
    case 0x2795:
      FUN_0077fa00(0);
      break;
    case 0x2714:
    case 0x276e:
      FUN_0077fa00(1);
      break;
    default:
      FUN_0077fac0();
    }
  }
  return;
}
```

### decoy: 0x0077fac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fac0: 16-byte aligned: YES
  0077fac0  55             PUSH EBP
  0077fac1  8B EC          MOV EBP,ESP
  0077fac3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fac0
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 3):
  +0x448 [dec 1096]  x2  via EAX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x1  via ECX
```

## Card B13a-1-27

GOG function: CSWCCreature::GetPortrait_2  gog 0x00410d30  callconv=(blank)  param_size_bytes=5  gog_size=86  gog_retn=8
Candidate Steam address: 0x00765060 (size 86, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00765000:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410cd0 CSWCCreature::GetPortrait -> 0x00765000  gate_class=REVIEW
- GOG-order neighbour after:  0x00410d90 CSWCCreature::GetPortraitId -> 0x007650c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00495600 CSWPortrait::GetPortrait -> Steam ?  in candidate callees: n/a
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x310*} | Steam {0x310*}
imm sets (* = in both, 3 common): GOG {0x8*,0xc*,0x70*} | Steam {0x8*,0xc*,0x70*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00765060: 16-byte aligned: YES
  00765060  55             PUSH EBP
  00765061  8B EC          MOV EBP,ESP
  00765063  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765060
string literals (0):
direct callees (2):
  0x0085E8C0  (unnamed)
  0x00710840  (unnamed)
struct offsets touched (top 1):
  +0x310 [dec 784]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00765060 FUN_00765060 size=86 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_00765060(int param_1,undefined4 param_2,undefined1 param_3)

{
  if (*(int *)(param_1 + 0x310) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    FUN_0085e8c0(param_2,param_3);
  }
  return param_2;
}
```

### decoy: 0x00765000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765000: 16-byte aligned: YES
  00765000  55             PUSH EBP
  00765001  8B EC          MOV EBP,ESP
  00765003  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765000
string literals (0):
direct callees (2):
  0x0084A230  (unnamed)
  0x00710840  (unnamed)
struct offsets touched (top 1):
  +0x310 [dec 784]  x2  via EAX,EDX
```

## Card B13a-1-28

GOG function: CSWCCreature::UpdatePopUpBubble  gog 0x00421410  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x00775520 (size 100, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007754d0:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004213c0 CSWCCreature::SetThrownLightsaber -> 0x007754d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00421480 CSWCCreature::PerformSkill -> 0x00775590  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: yes
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x48*,0x108*,0x3b4*} | Steam {0x4*,0x48*,0x108*,0x3b4*}
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
prologue for 0x00775520: 16-byte aligned: YES
  00775520  55             PUSH EBP
  00775521  8B EC          MOV EBP,ESP
  00775523  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775520
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x48 [dec 72]  x1  via EDX
  +0x108 [dec 264]  x1  via ECX
  +0x3B4 [dec 948]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775520 FUN_00775520 size=100 ret=0 callers=0 callees=2

void FUN_00775520(void)

{
  int iVar1;
  int extraout_var;
  undefined4 uVar2;
  
  uVar2 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_0073f430();
  if ((iVar1 != 0) && (*(int *)(iVar1 + 0x48) != 0)) {
    iVar1 = *(int *)(*(int *)(iVar1 + 0x48) + 0x108);
    if (*(int *)(extraout_var + 0x3b4) == iVar1) {
      FUN_00623320(extraout_var,iVar1,uVar2);
    }
    else {
      FUN_00623320(extraout_var,iVar1,uVar2);
    }
  }
  return;
}
```

### decoy: 0x007754d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007754d0: 16-byte aligned: YES
  007754d0  55             PUSH EBP
  007754d1  8B EC          MOV EBP,ESP
  007754d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007754d0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x3B4 [dec 948]  x2  via EAX,ECX
  +0x108 [dec 264]  x1  via EDX
```

## Card B13a-1-29

GOG function: CSWCCreature::SetAlignmentGoodEvil  gog 0x004291f0  callconv=(blank)  param_size_bytes=2  gog_size=155  gog_retn=4
Candidate Steam address: 0x0077d310 (size 124, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077d270:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004290a0 CSWCCreature::ActionMenuForm -> 0x0077d120  gate_class=AUTO
- GOG-order neighbour after:  0x00429290 CSWCCreature::SetBaseCNPCAlignment -> 0x0077d310  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d4c0 CSWCCreatureAppearance::SetAppearanceAlignment -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x80,0x224,0x310*} | Steam {0x82,0x310*}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x64*} | Steam {0x4*,0x14*,0x64*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0077d310: 16-byte aligned: YES
  0077d310  55             PUSH EBP
  0077d311  8B EC          MOV EBP,ESP
  0077d313  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077d310
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x82 [dec 130]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077d310 FUN_0077d310 size=124 ret=4 callers=1 callees=0

void __thiscall FUN_0077d310(int param_1,short param_2)

{
  short sVar1;
  
  if (*(int *)(param_1 + 0x310) != 0) {
    sVar1 = param_2;
    if (param_2 < -1) {
      sVar1 = -1;
    }
    if (sVar1 < 0x65) {
      if (param_2 < -1) {
        param_2 = -1;
      }
    }
    else {
      param_2 = 100;
    }
    *(short *)(*(int *)(param_1 + 0x310) + 0x82) = param_2;
  }
  return;
}
```

### decoy: 0x0077d270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077d270: 16-byte aligned: YES
  0077d270  55             PUSH EBP
  0077d271  8B EC          MOV EBP,ESP
  0077d273  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077d270
string literals (0):
direct callees (1):
  0x0085D430  (unnamed)
struct offsets touched (top 3):
  +0x224 [dec 548]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x80 [dec 128]  x1  via EDX
```

## Card B13a-1-30

GOG function: CSWCCreature::PerformCastSpell  gog 0x004215f0  callconv=(blank)  param_size_bytes=28  gog_size=82  gog_retn=28
Candidate Steam address: 0x00775700 (size 82, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007756c0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004215b0 CSWCCreature::PerformAttack -> 0x007756c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00421650 CSWCCreature::CreateNewSpellLikeAbilityList -> 0x00775760  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x00505ea0 CSWCMessage::SendPlayerToServerInput_CastSpell -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0xc*,0x1c*} | Steam {0xc*,0x1c*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x00775700: 16-byte aligned: YES
  00775700  55             PUSH EBP
  00775701  8B EC          MOV EBP,ESP
  00775703  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775700
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087A5C0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775700 FUN_00775700 size=82 ret=28 callers=2 callees=2

void __thiscall
FUN_00775700(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined1 param_5,undefined1 param_6,undefined1 param_7,undefined4 param_8)

{
  int iVar1;
  
  iVar1 = FUN_0073f810(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  if (iVar1 != 0) {
    FUN_0087a5c0(param_2,param_5,param_6,param_7,param_8,param_4,param_3);
  }
  return;
}
```

### decoy: 0x007756c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007756c0: 16-byte aligned: YES
  007756c0  55             PUSH EBP
  007756c1  8B EC          MOV EBP,ESP
  007756c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007756c0
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x008799A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B13a-1-31

GOG function: CSWCCreature::GetCameraOffset  gog 0x00421880  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775930 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::DeleteSpellLikeAbilityList | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421860 CSWCCreature::GetHeadHeightOffset -> 0x00775990  gate_class=REVIEW
- GOG-order neighbour after:  0x004218a0 CSWCCreature::GetPauseReadyAnimation -> 0x007759b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d460 CSWCCreatureAppearance::GetCameraOffset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00775930: 16-byte aligned: YES
  00775930  55             PUSH EBP
  00775931  8B EC          MOV EBP,ESP
  00775933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775930
string literals (0):
direct callees (1):
  0x0085D260  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775930 FUN_00775930 size=25 ret=0 callers=0 callees=1
THUNK -> 0x0085D260 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775930(undefined4 param_1)

{
  FUN_0085d260(param_1);
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

## Card B13a-1-32

GOG function: CSWCCreature::UpdateRelativePerception_2  gog 0x00419dc0  callconv=(blank)  param_size_bytes=4  gog_size=470  gog_retn=4
Candidate Steam address: 0x0076df50 (size 470, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0076dde0:STR0/CALLEE0/SIG0.786/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00419c50 CSWCCreature::UpdateRelativePerception -> 0x0076dde0  gate_class=REVIEW
- GOG-order neighbour after:  0x00419fa0 CSWCCreature::EquipToInventorySlot -> 0x0076e130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: yes
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x20*,0xb4*,0xb8*,0x1b8*} | Steam {0x4*,0x20*,0xb4*,0xb8*,0x1b8*}
imm sets (* = in both, 6 common): GOG {0x4*,0x24*,0x44*,0x378*,0x7d1*,0x7f000000*} | Steam {0x4*,0x24*,0x44*,0x378*,0x7d1*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076df50: 16-byte aligned: YES
  0076df50  55             PUSH EBP
  0076df51  8B EC          MOV EBP,ESP
  0076df53  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076df50
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x7  via EAX,EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0076df50 FUN_0076df50 size=470 ret=4 callers=1 callees=2

void __thiscall FUN_0076df50(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  int local_20;
  int *local_1c;
  int local_18;
  int *local_14;
  int *local_10;
  int local_c;
  int local_8;
  
  local_c = 0;
  local_18 = 0;
  local_10 = (int *)0x0;
  local_20 = 0;
  local_1c = (int *)(param_1 + 0x378);
  local_8 = 0;
  do {
    if (*(int *)(param_1 + 0x37c) <= local_8) {
LAB_0076dfcc:
      if ((local_20 == 1) && (local_14 = (int *)FUN_0073f550(param_2), local_14 != (int *)0x0)) {
        if ((((*(byte *)(local_10 + 0x1 [dec 1] {int*: byte +0x4}) & 1) == 0) && ((*(byte *)(local_10 + 0x1 [dec 1] {int*: byte +0x4}) >> 1 & 1) == 1)) ||
           ((*(byte *)(local_10 + 0x1 [dec 1] {int*: byte +0x4}) >> 4 & 1) == 1)) {
          local_c = 1;
        }
        else if (((*(byte *)(local_10 + 0x1 [dec 1] {int*: byte +0x4}) & 1) == 1) && ((*(byte *)(local_10 + 0x1 [dec 1] {int*: byte +0x4}) >> 4 & 1) == 0))
        {
          local_18 = 1;
        }
        iVar1 = FUN_0078eb80(local_14 + 0x9 [dec 9] {int*: byte +0x24});
        if (iVar1 == 1) {
          if ((local_c == 1) && (local_14[0x6e = byte +0x1b8] == 0)) {
            local_2c = 0;
            local_28 = 0;
            local_24 = 0;
            (**(code **)(*local_14 + 0xb4))(0x7d1,0,0x7f000000,0x7f000000,0,0,&local_2c);
            local_14[0x6e = byte +0x1b8] = 1;
          }
          else if ((local_18 == 1) && (local_14[0x6e = byte +0x1b8] == 1)) {
            (**(code **)(*local_14 + 0xb8))(0x7d1);
            local_14[0x6e = byte +0x1b8] = 0;
          }
        }
      }
      return;
    }
    local_10 = *(int **)(*local_1c + local_8 * 4);
    if (*local_10 == param_2) {
      local_20 = 1;
      goto LAB_0076dfcc;
    }
    local_8 = local_8 + 1;
  } while( true );
}
```

### decoy: 0x0076dde0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076dde0: 16-byte aligned: YES
  0076dde0  55             PUSH EBP
  0076dde1  8B EC          MOV EBP,ESP
  0076dde3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076dde0
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0xB4 [dec 180]  x1  via EAX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B13a-1-33

GOG function: CSWCCreature::UpdateRangedAttackData  gog 0x0041ac10  callconv=(blank)  param_size_bytes=4  gog_size=365  gog_retn=4
Candidate Steam address: 0x0076eda0 (size 365, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0076eb20:STR0/CALLEE0/SIG0.727/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041a990 CSWCCreature::SetLastInstant -> 0x0076eb20  gate_class=AUTO
- GOG-order neighbour after:  0x0041ade0 CSWCCreature::GetLastInstant -> 0x0076ef70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x280*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x29c*,0x2a0*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,0x2b4*,...(+1)} | Steam {0x280*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x29c*,0x2a0*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,0x2b4*,...(+1)}
imm sets (* = in both, 11 common): GOG {0x4*,0x8*,0x3f*,0x80*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*} | Steam {0x4*,0x8*,0x3f*,0x80*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076eda0: 16-byte aligned: YES
  0076eda0  55             PUSH EBP
  0076eda1  8B EC          MOV EBP,ESP
  0076eda3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0076eda0 FUN_0076eda0 size=365 ret=4 callers=2 callees=0

undefined4 __thiscall FUN_0076eda0(int param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (param_2 < 0x81) {
    if (param_2 == 0x80) {
      return *(undefined4 *)(param_1 + 0x29c);
    }
    switch(param_2) {
    case 1:
      uVar1 = *(undefined4 *)(param_1 + 0x280);
      break;
    case 2:
      uVar1 = *(undefined4 *)(param_1 + 0x284);
      break;
    default:
      goto switchD_0076ede2_caseD_3;
    case 4:
      uVar1 = *(undefined4 *)(param_1 + 0x288);
      break;
    case 8:
      uVar1 = *(undefined4 *)(param_1 + 0x28c);
      break;
    case 0x10:
      uVar1 = *(undefined4 *)(param_1 + 0x290);
      break;
    case 0x20:
      uVar1 = *(undefined4 *)(param_1 + 0x294);
      break;
    case 0x40:
      uVar1 = *(undefined4 *)(param_1 + 0x298);
    }
  }
  else {
    if (param_2 < 0x4001) {
      if (param_2 == 0x4000) {
        return *(undefined4 *)(param_1 + 0x2ac);
      }
      if (param_2 == 0x100) {
        return *(undefined4 *)(param_1 + 0x2a0);
      }
      if (param_2 == 0x200) {
        return *(undefined4 *)(param_1 + 0x2a4);
      }
      if (param_2 == 0x400) {
        return *(undefined4 *)(param_1 + 0x2a8);
      }
    }
    else {
      if (param_2 == 0x8000) {
        return *(undefined4 *)(param_1 + 0x2b0);
      }
      if (param_2 == 0x10000) {
        return *(undefined4 *)(param_1 + 0x2b4);
      }
      if (param_2 == 0x20000) {
        return *(undefined4 *)(param_1 + 0x2b8);
      }
    }
switchD_0076ede2_caseD_3:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x0076eb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076eb20: 16-byte aligned: YES
  0076eb20  55             PUSH EBP
  0076eb21  8B EC          MOV EBP,ESP
  0076eb23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076eb20
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x280 [dec 640]  x1  via ECX
  +0x284 [dec 644]  x1  via EAX
  +0x288 [dec 648]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x290 [dec 656]  x1  via EAX
  +0x294 [dec 660]  x1  via EDX
  +0x298 [dec 664]  x1  via ECX
  +0x29C [dec 668]  x1  via EAX
  +0x2A0 [dec 672]  x1  via EDX
  +0x2A4 [dec 676]  x1  via ECX
```

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

## Card B13a-1-35

GOG function: CSWCCreature::SetInParty  gog 0x00421750  callconv=(blank)  param_size_bytes=4  gog_size=203  gog_retn=4
Candidate Steam address: 0x00775860 (size 203, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x00421650:0.96>0.57
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 7 (callees: IsPlayerCreature 0x73fe80; callers: FadeObject 0x7f12d0, AddMember 0x5f8d80, CreateParty 0x5f9220); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007756c0:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004216d0 CSWCCreature::DeleteSpellLikeAbilityList -> 0x007757e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00421820 CSWCCreature::GetCameraFollowPart -> 0x007759b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x0040d360 CClientExoApp::IsPlayerCreature -> Steam 0x0073fe80  in candidate callees: yes
  0x00478360 CSWParty::AddCharacter -> Steam ?  in candidate callees: n/a
  0x00478a20 CSWParty::SetLeader -> Steam ?  in candidate callees: n/a
  0x00479870 CSWParty::RemoveCharacter_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00476430 CSWCObject::FadeObject -> Steam 0x007f12d0  in candidate callers: yes
  0x006fe8f0 CSWPartyTable::AddMember -> Steam 0x005f8d80  in candidate callers: yes
  0x006fed90 CSWPartyTable::CreateParty -> Steam 0x005f9220  in candidate callers: yes
  0x00781f70 Global::ExecuteCommandAddPartyMember -> Steam 0x0066c990  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x3bc*} | Steam {0x4*,0x3bc*}
imm sets (* = in both, 3 common): GOG {0x4*,0x1c*,0xfffffffd*} | Steam {0x4*,0x1c*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00775860: 16-byte aligned: YES
  00775860  55             PUSH EBP
  00775861  8B EC          MOV EBP,ESP
  00775863  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775860
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5730  CSWParty::AddCharacter(id)
  0x0073FE80  CClientExoApp::IsPlayerCreature
  0x007E5DF0  CSWParty::SetLeaderBySlot(slot)
  0x007E6C40  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x3BC [dec 956]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00775860 FUN_00775860 size=203 ret=4 callers=8 callees=5

undefined4 __thiscall FUN_00775860(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x3bc) == param_2) {
    uVar1 = 1;
  }
  else {
    *(int *)(param_1 + 0x3bc) = param_2;
    if (*(int *)(param_1 + 0x3bc) == 0) {
      iVar2 = FUN_0073fe80(param_1);
      if (iVar2 != 0) {
        uVar1 = 0xfffffffd;
        FUN_0073fb90(0xfffffffd);
        FUN_007e5df0(uVar1);
      }
      uVar1 = *(undefined4 *)(param_1 + 4);
      FUN_0073fb90(uVar1,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
      uVar1 = FUN_007e6c40(uVar1);
    }
    else {
      uVar1 = *(undefined4 *)(param_1 + 4);
      FUN_0073fb90(uVar1);
      uVar1 = FUN_007e5730(uVar1);
    }
  }
  return uVar1;
}
```

### decoy: 0x007756c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007756c0: 16-byte aligned: YES
  007756c0  55             PUSH EBP
  007756c1  8B EC          MOV EBP,ESP
  007756c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007756c0
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x008799A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B13a-1-36

GOG function: CSWCCreature::Constructor  gog 0x004102b0  callconv=(blank)  param_size_bytes=0  gog_size=2405  gog_retn=0
Candidate Steam address: 0x007645f0 (size 2405, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: CSWGuiInGameCharacter 0x84c3a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00764320:STR0/CALLEE0/SIG0.047/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410290 CSWCCreaturePartyStatus::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: yes
  0x0040ffe0 CSWCCombatAttackData::Constructor -> Steam ?  in candidate callees: n/a
  0x004721b0 CSWCObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00489500 CSWCSpellCastVisual::Constructor -> Steam ?  in candidate callees: n/a
  0x0048a160 Global::CSWCCreatureAppearance -> Steam ?  in candidate callees: n/a
  0x00490650 CSWCLevelUpStats::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00595eb0 CSWGuiInGameCharacter::Constructor -> Steam 0x0084c3a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 115 common): GOG {0x4*,0x8*,0x3c*,0x40*,0x48*,0x58*,0xfc*,0x12c*,0x130*,0x134*,0x138*,0x13c*,0x140*,0x144*,...(+101)} | Steam {0x4*,0x8*,0x3c*,0x40*,0x48*,0x58*,0xfc*,0x12c*,0x130*,0x134*,0x138*,0x13c*,0x140*,0x144*,...(+101)}
imm sets (* = in both, 31 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0xc*,0x11*,0x14*,0x30*,0x48*,0x6c*,0x88*,0x140*,0x1cc*,...(+17)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0xc*,0x11*,0x14*,0x30*,0x48*,0x6c*,0x88*,0x140*,0x1cc*,...(+17)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007645f0: 16-byte aligned: YES
  007645f0  55             PUSH EBP
  007645f1  8B EC          MOV EBP,ESP
  007645f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007645f0 FUN_007645f0 size=2405 ret=0 callers=8 callees=11

undefined4 * __fastcall FUN_007645f0(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009649fd;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007ed050(DAT_00994474);
  local_8 = 0;
  *param_1 = CSWCCreature::vftable;
  param_1[0x73 = byte +0x1cc] = 0;
  param_1[0x74 = byte +0x1d0] = 0;
  param_1[0x75 = byte +0x1d4] = 0;
  param_1[0x76 = byte +0x1d8] = 0;
  param_1[0x77 = byte +0x1dc] = 0;
  param_1[0x78 = byte +0x1e0] = 0;
  param_1[0x79 = byte +0x1e4] = 0;
  param_1[0x7a = byte +0x1e8] = 0;
  param_1[0x7b = byte +0x1ec] = 0;
  param_1[0x82 = byte +0x208] = 0;
  param_1[0x83 = byte +0x20c] = 0;
  param_1[0x84 = byte +0x210] = 0;
  FUN_007107f0(uVar1);
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  param_1[0x116 = byte +0x458] = param_1[0x116 = byte +0x458] & 0xfffffffe;
  param_1[0x116 = byte +0x458] = param_1[0x116 = byte +0x458] | 2;
  param_1[0xef = byte +0x3bc] = 0;
  param_1[0x11b = byte +0x46c] = 0;
  param_1[0xe1 = byte +0x384] = 0;
  *(undefined2 *)(param_1 + 0xc5) = 0xffff;
  iVar2 = FUN_00919723(0x140);
  local_8._0_1_ = 2;
  if (iVar2 == 0) {
    local_88 = 0;
  }
  else {
    local_88 = FUN_00845ac0(param_1);
  }
  local_8._0_1_ = 1;
  param_1[0xc4 = byte +0x310] = local_88;
  param_1[0x71 = byte +0x1c4] = 0;
  param_1[0x72 = byte +0x1c8] = 0;
  *(undefined2 *)(param_1 + 0x7d) = 0;
  *(undefined2 *)(param_1 + 0x12) = 10000;
  param_1[0xb6 = byte +0x2d8] = 0;
  param_1[0x55 = byte +0x154] = 0;
  param_1[0x56 = byte +0x158] = 0;
  param_1[0x70 = byte +0x1c0] = 0;
  param_1[0xfc = byte +0x3f0] = 0;
  param_1[0x9e = byte +0x278] = 0x7f000000;
  param_1[0x9f = byte +0x27c] = 0x7f000000;
  param_1[0x7c = byte +0x1f0] = 0;
  iVar2 = FUN_00919723(0x48);
  local_8._0_1_ = 3;
  if (iVar2 == 0) {
    local_8c = 0;
  }
  else {
    local_8c = FUN_00764320();
  }
/* ... 156 more lines truncated by --max-lines */
```

### decoy: 0x00764320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764320: 16-byte aligned: YES
  00764320  55             PUSH EBP
  00764321  8B EC          MOV EBP,ESP
  00764323  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764320
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x1 [dec 1]  x1  via EDX
  +0xA [dec 10]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1A [dec 26]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
```

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

## Card B13a-1-38

GOG function: CSWCCreature::AnimationUpdate  gog 0x004132e0  callconv=(blank)  param_size_bytes=0  gog_size=592  gog_retn=0
Candidate Steam address: 0x007675b0 (size 592, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.57) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: GetLoopingAnimation 0x7ed980, AnimationStationary 0x7ef2c0, IsInjured 0x51c6f0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007672f0:STR0/CALLEE0/SIG0.142/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413280 Vector::operator+  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00413620 CSWCCreature::StripEquippedItems -> 0x007678f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00413530 unnamed -> Steam ?  in candidate callees: n/a
  0x00413620 CSWCCreature::StripEquippedItems -> Steam ?  in candidate callees: n/a
  0x00414210 CSWCCreature::MoveCreatureToPosition -> Steam ?  in candidate callees: n/a
  0x00414430 CSWCCreature::RotateCreatureToOrientation -> Steam ?  in candidate callees: n/a
  0x00423df0 CSWCCreature::AnimateFireAndForget -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x48*,0x75*,0x9c*,0x138*,0x30c*,0x11b0*} | Steam {0x4*,0x8*,0x48*,0x75*,0x9c*,0x138*,0x30c*,0x11b0*}
imm sets (* = in both, 10 common): GOG {0x3*,0x5*,0x30*,0x1cc*,0x1d8*,0x1e4*,0x2710*,0x2711*,0x276c*,0x27a6*} | Steam {0x3*,0x5*,0x30*,0x1cc*,0x1d8*,0x1e4*,0x2710*,0x2711*,0x276c*,0x27a6*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007675b0: 16-byte aligned: YES
  007675b0  55             PUSH EBP
  007675b1  8B EC          MOV EBP,ESP
  007675b3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007675b0 FUN_007675b0 size=592 ret=0 callers=1 callees=13

void __fastcall FUN_007675b0(int *param_1)

{
  undefined2 uVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  
  iVar2 = FUN_007ef2c0((short)param_1[0x12 = byte +0x48]);
  if (((iVar2 != 0) || (iVar2 = FUN_0077d800(), iVar2 == 0)) ||
     (iVar2 = FUN_0077d800(), *(int *)(iVar2 + 0x11b0) != 0)) {
    if (param_1[0xc3 = byte +0x30c] != DAT_00994484) {
      FUN_007678f0();
    }
    iVar2 = FUN_007ef2c0((short)param_1[0x12 = byte +0x48]);
    if (iVar2 == 0) {
      FUN_00768610(param_1 + 0x73);
    }
    else {
      FUN_00768610(param_1 + 0x76);
    }
    FUN_00768440(param_1 + 0x79);
    iVar2 = FUN_007ef2c0((short)param_1[0x12 = byte +0x48]);
    if (iVar2 == 0) {
      uVar1 = FUN_007ed980();
      (**(code **)(*param_1 + 0x9c))(uVar1,0,0);
    }
    FUN_00777e30();
    if ((((short)param_1[0x12 = byte +0x48] == 0x2711) && (iVar2 = FUN_00767800(0), iVar2 != 1)) &&
       ((iVar2 = FUN_00767800(3), iVar2 != 1 && (param_1[0x4e = byte +0x138] != 1)))) {
      (**(code **)(*param_1 + 0x9c))(10000,0,0);
    }
    iVar2 = FUN_007ef350((short)param_1[0x12 = byte +0x48]);
    if ((iVar2 != 0) &&
       ((iVar2 = FUN_00767800(0), iVar2 == 1 || (iVar2 = FUN_00767800(3), iVar2 == 1)))) {
      (**(code **)(*param_1 + 0x9c))(0x2711,0,0);
    }
    iVar2 = FUN_007ef350((short)param_1[0x12 = byte +0x48]);
    if (((iVar2 != 0) && ((short)param_1[0x12 = byte +0x48] != 0x27a6)) &&
       (iVar2 = FUN_0051c6f0(param_1[1 = byte +0x4]), iVar2 != 0)) {
      iVar2 = FUN_0072fb00();
      if (*(char *)(iVar2 + 0x75) == '\x05') {
        uVar4 = 0;
        FUN_0073fb90(0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
        piVar3 = (int *)FUN_007e5da0(uVar4);
        if (param_1 == piVar3) {
          return;
        }
      }
      (**(code **)(*param_1 + 0x9c))(0x276c,0,0);
    }
  }
  return;
}
```

### decoy: 0x007672f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007672f0: 16-byte aligned: YES
  007672f0  55             PUSH EBP
  007672f1  8B EC          MOV EBP,ESP
  007672f3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007672f0
string literals (0):
direct callees (7):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073F430  CClientExoApp::GetModule
  0x007F7220  (unnamed)
  0x0073FEE0  CClientExoApp::SetInputClass
  0x007E24B0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x40 [dec 64]  x1  via EDX
  +0x75 [dec 117]  x1  via ECX
  +0x80 [dec 128]  x1  via EDX
```

## Card B13a-1-39

GOG function: CSWCCreature::ApplyShadowBlob  gog 0x00420130  callconv=(blank)  param_size_bytes=0  gog_size=458  gog_retn=0
Candidate Steam address: 0x00774260 (size 458, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetHeadTurnVertical | G7 mac: pass(0.92) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'Fx_Shadow_S'@0099f1a4; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00774200:STR0/CALLEE0/SIG0.182/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004200d0 CSWCCreature::GetHeadTurnVertical -> 0x00774260  gate_class=REVIEW
- GOG-order neighbour after:  0x00420310 CSWCCreature::RemoveShadowBlob -> 0x00774440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x0080df50 Global::RotationArc -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (3): 'AFx_Shadow_L'; 'Fx_Shadow_M'; 'Fx_Shadow_S'
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0x10*,0x18*,0x68*,0xa4*,0xb4*,0xd4*,0x108*,0x130*,0x224*} | Steam {0x4*,0x8*,0x10*,0x18*,0x68*,0xa4*,0xb4*,0xd4*,0x108*,0x130*,0x224*}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0xc*,0x1c*,0x48*,0xd8*} | Steam {0x3*,0x4*,0xc*,0x1c*,0x48*,0xd8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00774260: 16-byte aligned: YES
  00774260  55             PUSH EBP
  00774261  8B EC          MOV EBP,ESP
  00774263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00774260 FUN_00774260 size=458 ret=0 callers=0 callees=5

undefined4 __fastcall FUN_00774260(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_40 [16];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974ab8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (param_1[0x1a = byte +0x68] == 0) {
    local_24 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = local_24;
  }
  else {
    iVar1 = FUN_0071b8b0(*(undefined2 *)(param_1[0x89 = byte +0x224] + 0x18),DAT_00a0fdf4,&local_14);
    if (iVar1 == 0) {
      local_14 = 3;
    }
    switch(local_14) {
    case 1:
    case 2:
      FUN_007338d0("Fx_Shadow_S");
      break;
    case 3:
      FUN_007338d0("Fx_Shadow_M");
      break;
    case 4:
    case 5:
      FUN_007338d0("Fx_Shadow_L");
      break;
    default:
      FUN_007338d0("Fx_Shadow_M");
    }
    local_20 = (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xa4))(local_1c);
    local_30 = 0;
    local_2c = 0;
    local_28 = 0x3f800000;
    uVar2 = FUN_005160c0(local_40,0,0,0x3f800000,param_1[0x36 = byte +0xd8],param_1[0x37 = byte +0xdc],param_1[0x38 = byte +0xe0]);
    (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xb4))(uVar2);
    if (param_1[0x35 = byte +0xd4] == 0) {
      (**(code **)(*param_1 + 0x108))();
    }
    uVar2 = local_20;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00774200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774200: 16-byte aligned: YES
  00774200  55             PUSH EBP
  00774201  8B EC          MOV EBP,ESP
  00774203  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B13a-1-40

GOG function: CSWCCreature::GetPersonalActions  gog 0x0042a5d0  callconv=(blank)  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x0077dba0 (size 1005, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetCombatFeatsSkills | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077df90:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042a2f0 CSWCCreature::GetTargetActions -> 0x0077e370  gate_class=AUTO
- GOG-order neighbour after:  0x0042a670 CSWCCreature::GetReticlePoint -> 0x0077e6f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004254d0 CSWCCreature::GetSelfForcePowers -> Steam ?  in candidate callees: n/a
  0x004269a0 CSWCCreature::GetBehaviors -> Steam ?  in candidate callees: n/a
  0x00426e70 CSWCCreature::GetStances -> Steam ?  in candidate callees: n/a
  0x00428490 CSWCCreature::GetUsableItems -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1a,0x34,0x2e4}
imm sets (* = in both, 1 common): GOG {0x2,0x4,0x5,0x8*} | Steam {0x8*,0xb,0x10,0x11,0x12,0x13,0x14,0x1a,0x1c,0x1d,0x1e,0x1f,0x35,0x4d,...(+8)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077dba0: 16-byte aligned: YES
  0077dba0  55             PUSH EBP
  0077dba1  8B EC          MOV EBP,ESP
  0077dba3  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077dba0
string literals (0):
direct callees (7):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005453C0  CSWSObject::GetArea
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x006D5200  (unnamed)
  0x007787E0  CSWCCreature::CheckFeat
  0x00778940  Global::SceneHitCheck
struct offsets touched (top 6):
  +0x4 [dec 4]  x8  via ECX,EDX
  +0xC [dec 12]  x8  via ECX
  +0x8 [dec 8]  x7  via ECX
  +0x34 [dec 52]  x3  via EAX,EDX
  +0x1A [dec 26]  x1  via EAX
  +0x2E4 [dec 740]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077dba0 FUN_0077dba0 size=1005 ret=8 callers=1 callees=7

void __thiscall FUN_0077dba0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  
  iVar2 = FUN_0077d800(param_1);
  if (iVar2 != 0) {
    iVar2 = FUN_005453c0();
    if (*(int *)(iVar2 + 0x2e4) == 0) {
      iVar2 = FUN_0076e610(0x10);
      bVar1 = false;
      if (iVar2 != 0x7f000000) {
        piVar3 = (int *)FUN_0073f4d0(iVar2);
        if (piVar3 != (int *)0x0) {
          iVar2 = (**(code **)(*piVar3 + 0x34))();
          if (iVar2 != 0) {
            iVar2 = (**(code **)(*piVar3 + 0x34))();
            iVar2 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
            if ((iVar2 != 0) && (*(char *)(iVar2 + 0x1a) != '\0')) {
              bVar1 = true;
            }
          }
        }
      }
      if (bVar1) {
        uVar4 = FUN_007787e0(0x52,0x12,0x1d);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x1a,0x5c,0x1e);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x4d,0x14,0x1f);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
      }
      else {
        uVar4 = FUN_007787e0(0x51,0x13,8);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x35,0x5b,0xb);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x53,0x11,0x1c);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0xdc,0xffffffff,0xffffffff);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
      }
    }
  }
  return;
}
```

### decoy: 0x0077df90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077df90: 16-byte aligned: YES
  0077df90  55             PUSH EBP
  0077df91  8B EC          MOV EBP,ESP
  0077df93  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077df90
string literals (0):
direct callees (2):
  0x00740E80  CClientExoApp::GetTargetChanging
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 7):
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x138 [dec 312]  x1  via EDX
  +0x37C [dec 892]  x1  via EDX
```

## Card B13a-1-41

GOG function: CSWCCreature::GetHostileItems_2  gog 0x00428690  callconv=(blank)  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x0077c780 (size 1271, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::PartyMenuUseItemSelect | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
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
  0x00669930 CItemRepository::ItemListGetItem -> Steam 0x00606100  in candidate callees: no
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam 0x0058a2c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x10} | Steam {0x4,0x8,0x98,0xac}
imm sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*,0x24,0x2f,0xe4,0xbfffffff}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077c780: 16-byte aligned: YES
  0077c780  55             PUSH EBP
  0077c781  8B EC          MOV EBP,ESP
  0077c783  81 EC E4 00 00 00 SUB ESP,0xe4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077c780
string literals (0):
direct callees (10):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x006D6E30  CSWItem::GetBaseItem
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005D2F80  CSWSCreature::CanUseItem
  0x0077CC80  CSWCCreature::PartyMenuUseItem
  0x0073F750  ClientApp::GetGuiInGame
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x00775600  CSWCCreature::PerformUseItem
struct offsets touched (top 7):
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x98 [dec 152]  x6  via EAX,ECX,EDX
  +0x15230 [dec 86576]  x3  via EAX,ECX,EDX
  +0xAC [dec 172]  x2  via ECX,EDX
  +0x15234 [dec 86580]  x2  via EAX,ECX
  +0x15238 [dec 86584]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c780 FUN_0077c780 size=1271 ret=8 callers=1 callees=10

void __thiscall FUN_0077c780(int param_1,uint param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  undefined4 uVar5;
  
  if (DAT_00a7ead4 == '\0') {
    iVar2 = FUN_0073f750();
    iVar2 = *(int *)(*(int *)(iVar2 + 0x98) + 0x15234);
    param_2 = param_2 & 0xbfffffff;
    uVar1 = FUN_0051c8b0(param_2);
    piVar4 = (int *)FUN_0073fb90();
    if (*piVar4 != 1) {
      FUN_0051c0e0(uVar1);
      iVar3 = FUN_006d6e30();
      if (*(char *)(iVar3 + 0xac) != '/') {
        iVar3 = FUN_0073f750();
        if (*(int *)(*(int *)(iVar3 + 0x98) + 0x15230) == 0) {
          FUN_0051c0e0(uVar1);
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) == '\x14') {
            FUN_00775600(param_2,0,*(undefined4 *)(param_1 + 4),param_1 + 0x24);
          }
          else {
            iVar2 = FUN_0073f750();
            *(uint *)(*(int *)(iVar2 + 0x98) + 0x15238) = param_2;
            iVar2 = FUN_0073f750();
            *(undefined4 *)(*(int *)(iVar2 + 0x98) + 0x15230) = 1;
          }
        }
        else {
          if (iVar2 == -1) {
            return;
          }
          iVar3 = iVar2;
          FUN_0073fb90(iVar2);
          iVar3 = FUN_007e5da0(iVar3);
          uVar5 = *(undefined4 *)(iVar3 + 4);
          iVar3 = FUN_0073f750();
          *(undefined4 *)(*(int *)(iVar3 + 0x98) + 0x15230) = 0;
          iVar3 = iVar2;
          FUN_0073fb90(iVar2);
          FUN_007e5da0(iVar3);
          FUN_0077d800();
          uVar1 = FUN_0051c0e0(uVar1);
          iVar3 = FUN_005d2f80(uVar1,0,1);
          if (iVar3 != 0) {
            FUN_0073fb90(iVar2);
            iVar2 = FUN_007e5da0(iVar2);
            FUN_00775600(param_2,0,uVar5,iVar2 + 0x24);
          }
        }
        iVar2 = FUN_0073f750();
        *(undefined4 *)(*(int *)(iVar2 + 0x98) + 0x15234) = 0xffffffff;
        return;
      }
    }
    if (iVar2 == -1) {
      uVar5 = 0;
      FUN_0073fb90(0);
      FUN_007e5da0(uVar5);
    }
    else {
      FUN_0073fb90(iVar2);
      FUN_007e5da0(iVar2);
    }
    FUN_0077d800();
    uVar1 = FUN_0051c0e0(uVar1);
    iVar2 = FUN_005d2f80(uVar1,0,1);
    if (iVar2 != 0) {
      FUN_00775600(param_2,0,*(undefined4 *)(param_1 + 4),param_1 + 0x24);
    }
  }
  else {
    uVar1 = FUN_0051c8b0(param_2 & 0xbfffffff);
    iVar2 = FUN_0051c0e0(uVar1);
/* ... 10 more lines truncated by --max-lines */
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

## Card B13a-1-42

GOG function: CSWCCreature::Hilite  gog 0x004208e0  callconv=(blank)  param_size_bytes=5  gog_size=50  gog_retn=16
Candidate Steam address: 0x00774a10 (size 50, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00774a50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004207d0 CSWCCreature::ReAttachCamera -> 0x00774900  gate_class=AUTO
- GOG-order neighbour after:  0x00420920 CSWCCreature::UnHilite -> 0x00774a50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c92d0 CSWCAnimBase::Hilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x00774a10: 16-byte aligned: YES
  00774a10  55             PUSH EBP
  00774a11  8B EC          MOV EBP,ESP
  00774a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00774a10
string literals (0):
direct callees (1):
  0x00861400  CSWCAnimBase::Hilite
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00774a10 FUN_00774a10 size=50 ret=16 callers=0 callees=1

undefined4 __thiscall
FUN_00774a10(int param_1,undefined4 param_2,undefined1 param_3,undefined4 param_4,undefined4 param_5
            )

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x68) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00861400(param_2,param_3,param_5);
  }
  return uVar1;
}
```

### decoy: 0x00774a50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774a50: 16-byte aligned: YES
  00774a50  55             PUSH EBP
  00774a51  8B EC          MOV EBP,ESP
  00774a53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00774a50
string literals (0):
direct callees (1):
  0x00861480  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,EDX
```

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

## Card B13a-1-44

GOG function: CSWCCreature::PerformSkill  gog 0x00421480  callconv=(blank)  param_size_bytes=12  gog_size=98  gog_retn=12
Candidate Steam address: 0x00775590 (size 98, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775670:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421410 CSWCCreature::UpdatePopUpBubble -> 0x00775520  gate_class=REVIEW
- GOG-order neighbour after:  0x004214f0 CSWCCreature::PerformUseItem -> 0x00775600  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccd0 CClientExoApp::GetSWCMessage -> Steam 0x0073f810  in candidate callees: yes
  0x00505300 CSWCMessage::SendPlayerToServerInput_UseSkill -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0xc*,0x18*} | Steam {0xc*,0x18*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00775590: 16-byte aligned: YES
  00775590  55             PUSH EBP
  00775591  8B EC          MOV EBP,ESP
  00775593  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775590
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x00879A20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00775590 FUN_00775590 size=98 ret=12 callers=3 callees=2

void __thiscall FUN_00775590(undefined4 param_1,undefined1 param_2,undefined1 param_3,int param_4)

{
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  if ((param_4 != DAT_00994484) &&
     (local_8 = FUN_0073f810(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4)), local_8 != 0)) {
    local_14 = 0;
    local_10 = 0;
    local_c = 0;
    FUN_00879a20(param_2,param_3,param_4,&local_14);
  }
  return;
}
```

### decoy: 0x00775670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775670: 16-byte aligned: YES
  00775670  55             PUSH EBP
  00775671  8B EC          MOV EBP,ESP
  00775673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775670
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087AEC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B13a-1-45

GOG function: CSWCCreature::SetArea  gog 0x00421370  callconv=(blank)  param_size_bytes=4  gog_size=33  gog_retn=4
Candidate Steam address: 0x007754a0 (size 33, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007756c0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00420a90 CSWCCreature::EvaluateNewWayPoints -> 0x00774bc0  gate_class=AUTO
- GOG-order neighbour after:  0x004213a0 CSWSPlaceable::SetEffectSpellId  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x004213c0 CSWCCreature::SetThrownLightsaber -> Steam ?  in candidate callees: n/a
  0x00476ce0 CSWCObject::SetArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007754a0: 16-byte aligned: YES
  007754a0  55             PUSH EBP
  007754a1  8B EC          MOV EBP,ESP
  007754a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007754a0
string literals (0):
direct callees (2):
  0x007F1B80  CSWCObject::SetArea
  0x007754D0  CSWCCreature::SetThrownLightsaber
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007754a0 FUN_007754a0 size=33 ret=4 callers=0 callees=2

void FUN_007754a0(undefined4 param_1)

{
  undefined4 extraout_var;
  
  FUN_007f1b80(param_1);
  FUN_007754d0(extraout_var);
  return;
}
```

### decoy: 0x007756c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007756c0: 16-byte aligned: YES
  007756c0  55             PUSH EBP
  007756c1  8B EC          MOV EBP,ESP
  007756c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007756c0
string literals (0):
direct callees (2):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x008799A0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B13a-1-46

GOG function: CSWCCreature::BlurWeapon  gog 0x00413850  callconv=(blank)  param_size_bytes=4  gog_size=152  gog_retn=4
Candidate Steam address: 0x00767a80 (size 152, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.60) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007679f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004137c0 CSWCCreature::ClearVisibilityList -> 0x007679f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0xc*,0x34*,0x198*,0x2e4*} | Steam {0x4*,0xc*,0x34*,0x198*,0x2e4*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1c*} | Steam {0x4*,0x10*,0x1c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00767a80: 16-byte aligned: YES
  00767a80  55             PUSH EBP
  00767a81  8B EC          MOV EBP,ESP
  00767a83  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00767a80 FUN_00767a80 size=152 ret=4 callers=4 callees=3

void __thiscall FUN_00767a80(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 0x2e4) != param_2) {
    uVar1 = FUN_0076e610(0x10);
    iVar2 = FUN_0073f530(uVar1);
    if ((iVar2 == 0) || (FUN_006d5200(*(undefined4 *)(iVar2 + 0xc)), *(int *)(iVar2 + 0x198) != 1))
    {
      *(int *)(param_1 + 0x2e4) = param_2;
    }
  }
  return;
}
```

### decoy: 0x007679f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007679f0: 16-byte aligned: YES
  007679f0  55             PUSH EBP
  007679f1  8B EC          MOV EBP,ESP
  007679f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007679f0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x178 [dec 376]  x5  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x17C [dec 380]  x1  via EDX
  +0x180 [dec 384]  x1  via EAX
```

## Card B13a-1-47

GOG function: CSWCCreature::GetPortrait  gog 0x00410cd0  callconv=(blank)  param_size_bytes=8  gog_size=82  gog_retn=8
Candidate Steam address: 0x00765000 (size 82, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00765060:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410cb0 CSWCObject::GetCanSeeHeight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410d30 CSWCCreature::GetPortrait_2 -> 0x00765060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00494dc0 CSWCCreatureStats::GetPortrait -> Steam ?  in candidate callees: n/a
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x310*} | Steam {0x310*}
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
prologue for 0x00765000: 16-byte aligned: YES
  00765000  55             PUSH EBP
  00765001  8B EC          MOV EBP,ESP
  00765003  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765000
string literals (0):
direct callees (2):
  0x0084A230  (unnamed)
  0x00710840  (unnamed)
struct offsets touched (top 1):
  +0x310 [dec 784]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00765000 FUN_00765000 size=82 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_00765000(int param_1,undefined4 param_2,undefined4 param_3)

{
  if (*(int *)(param_1 + 0x310) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    FUN_0084a230(param_2,param_3);
  }
  return param_2;
}
```

### decoy: 0x00765060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765060: 16-byte aligned: YES
  00765060  55             PUSH EBP
  00765061  8B EC          MOV EBP,ESP
  00765063  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765060
string literals (0):
direct callees (2):
  0x0085E8C0  (unnamed)
  0x00710840  (unnamed)
struct offsets touched (top 1):
  +0x310 [dec 784]  x2  via EAX,EDX
```

## Card B13a-1-48

GOG function: CSWCCreature::DeleteLoopingCreatureSound  gog 0x0041cdb0  callconv=(blank)  param_size_bytes=0  gog_size=90  gog_retn=0
Candidate Steam address: 0x00770f40 (size 90, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00770e60:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041ccd0 CSWCCreature::SetNewLoopingCreatureSound -> 0x00770e60  gate_class=AUTO
- GOG-order neighbour after:  0x0041ce10 CSWCCreature::GetArmourClass -> 0x00770fa0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x184*} | Steam {0x184*}
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
prologue for 0x00770f40: 16-byte aligned: YES
  00770f40  55             PUSH EBP
  00770f41  8B EC          MOV EBP,ESP
  00770f43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00770f40
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 1):
  +0x184 [dec 388]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00770f40 FUN_00770f40 size=90 ret=0 callers=1 callees=1

void __fastcall FUN_00770f40(int param_1)

{
  Decref();
  if (*(undefined4 **)(param_1 + 0x184) != (undefined4 *)0x0) {
    (**(code **)**(undefined4 **)(param_1 + 0x184))(1);
  }
  *(undefined4 *)(param_1 + 0x184) = 0;
  return;
}
```

### decoy: 0x00770e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00770e60: 16-byte aligned: YES
  00770e60  55             PUSH EBP
  00770e61  8B EC          MOV EBP,ESP
  00770e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00770e60
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x00710810  (unnamed)
  0x0070AB90  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0070ACC0  (unnamed)
  0x0070AE30  (unnamed)
struct offsets touched (top 2):
  +0x184 [dec 388]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
```

## Card B13a-1-49

GOG function: CSWCCreature::`scalar  gog 0x00410e50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00765110 (size 61, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00765180:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410e20 CSWCCreature::IsHostile -> 0x00765150  gate_class=AUTO
- GOG-order neighbour after:  0x00410e80 CSWCCreature::Destructor -> 0x007651b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00410e80 CSWCCreature::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x310}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00765110: 16-byte aligned: YES
  00765110  55             PUSH EBP
  00765111  8B EC          MOV EBP,ESP
  00765113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765110
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x310 [dec 784]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00765110 FUN_00765110 size=61 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00765110(int param_1,undefined4 param_2)

{
  FUN_007335f0(*(int *)(param_1 + 0x310) + 0x14);
  return param_2;
}
```

### decoy: 0x00765180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765180: 16-byte aligned: YES
  00765180  55             PUSH EBP
  00765181  8B EC          MOV EBP,ESP
  00765183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765180
string literals (0):
direct callees (2):
  0x007651B0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B13a-1-50

GOG function: CSWCCreature::GetArmourClass  gog 0x0041ce10  callconv=(blank)  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x00770fa0 (size 41, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00770f40:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041cdb0 CSWCCreature::DeleteLoopingCreatureSound -> 0x00770f40  gate_class=REVIEW
- GOG-order neighbour after:  0x0041ce40 CSWCCreature::SetPortrait -> 0x00770fd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00410c20 Global::return_zero -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00770fa0: 16-byte aligned: YES
  00770fa0  55             PUSH EBP
  00770fa1  8B EC          MOV EBP,ESP
  00770fa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00770fa0
string literals (0):
direct callees (1):
  0x004558A0  return 0 (proxy vtable slot 0x4)
struct offsets touched (top 1):
  +0x224 [dec 548]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00770fa0 FUN_00770fa0 size=41 ret=0 callers=1 callees=1

undefined4 __fastcall FUN_00770fa0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x224) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_004558a0();
  }
  return uVar1;
}
```

### decoy: 0x00770f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00770f40: 16-byte aligned: YES
  00770f40  55             PUSH EBP
  00770f41  8B EC          MOV EBP,ESP
  00770f43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00770f40
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 1):
  +0x184 [dec 388]  x3  via EAX,ECX
```

## Card B13a-1-51

GOG function: CSWCCreature::UpdateRollingFootstepSound  gog 0x00413e70  callconv=(blank)  param_size_bytes=4  gog_size=516  gog_retn=8
Candidate Steam address: 0x007680a0 (size 516, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.06) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: PlayerFlourishWeapons 0x79f430, ExecuteCommandCreatureFlourishWeapon 0x66b8a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007682b0:STR0/CALLEE0/SIG0.171/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00413e30 CSWCCreature::ForceAppearanceUpdate -> 0x00768060  gate_class=REVIEW
- GOG-order neighbour after:  0x00414080 CSWCCreature::SetCombatState -> 0x007682b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x00472990 CSWCObject::GetAnimationBase -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam 0x0079f430  in candidate callers: yes
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x4c*,0xc5*,0x138*,0x2e8*,0x3b0*} | Steam {0x4*,0x4c*,0xc5*,0x138*,0x2e8*,0x3b0*}
imm sets (* = in both, 12 common): GOG {0x8*,0x10*,0x20*,0x40*,0x272e*,0x2736*,0x2737*,0x2738*,0x2739*,0x273a*,0x27aa*,0x27ab*} | Steam {0x8*,0x10*,0x20*,0x40*,0x272e*,0x2736*,0x2737*,0x2738*,0x2739*,0x273a*,0x27aa*,0x27ab*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x007680a0: 16-byte aligned: YES
  007680a0  55             PUSH EBP
  007680a1  8B EC          MOV EBP,ESP
  007680a3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007680a0 FUN_007680a0 size=516 ret=8 callers=9 callees=3

undefined4 __thiscall FUN_007680a0(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  
  iVar1 = FUN_007ed830();
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (param_2 == 1) {
    piVar3 = (int *)FUN_007ed830();
    iVar1 = (**(code **)(*piVar3 + 0x4c))(0x272e);
    if (iVar1 != 1) {
      piVar3 = (int *)FUN_007ed830();
      iVar1 = (**(code **)(*piVar3 + 0x4c))(0x27ab);
      if (iVar1 != 1) {
        piVar3 = (int *)FUN_007ed830();
        iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2736);
        if (iVar1 != 1) {
          piVar3 = (int *)FUN_007ed830();
          iVar1 = (**(code **)(*piVar3 + 0x4c))(0x27aa);
          if (iVar1 != 1) {
            piVar3 = (int *)FUN_007ed830();
            iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2737);
            if (iVar1 != 1) {
              piVar3 = (int *)FUN_007ed830();
              iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2738);
              if (iVar1 != 1) {
                piVar3 = (int *)FUN_007ed830();
                iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2739);
                if (iVar1 != 1) {
                  piVar3 = (int *)FUN_007ed830();
                  iVar1 = (**(code **)(*piVar3 + 0x4c))(0x273a);
                  if ((iVar1 != 1) && (*(int *)(param_1 + 0x3b0) == 0)) {
                    if (*(short *)(param_1 + 0x2e8) == 0) {
                      return 1;
                    }
                    uVar2 = FUN_0076e610(0x20);
                    FUN_0073f530(uVar2);
                    if (*(int *)(param_1 + 0x138) == 1) {
                      return 1;
                    }
                    FUN_007ed830();
                    uVar2 = FUN_0076e610(0x10);
                    FUN_0073f530(uVar2);
                    return 1;
                  }
                }
              }
            }
          }
        }
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
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

## Card B13a-1-52

GOG function: CSWCCreature::PlaySoundSetSound  gog 0x00411770  callconv=(blank)  param_size_bytes=8  gog_size=836  gog_retn=8
Candidate Steam address: 0x00765aa0 (size 836, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetSoundSetStrres | G7 mac: pass(0.92) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: GetPlayerByGameObjectID 0x73f510; callers: PlayCharacterSelectSound 0x7a12e0, AIActionDisarmMine 0x5b43c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00765df0:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00411610 CSWCCreature::SetSoundSet -> 0x00765940  gate_class=AUTO
- GOG-order neighbour after:  0x00411ac0 CSWCCreature::GetSoundSetStrres -> 0x00765df0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (17), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040c9d0 CClientExoApp::GetPlayerByGameObjectID -> Steam 0x0073f510  in candidate callees: yes
  0x00411ac0 CSWCCreature::GetSoundSetStrres -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
  ... 9 more GOG callees
GOG callers mapped through trusted rows (2 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x0044d8e0 CClientExoAppInternal::PlayCharacterSelectSound -> Steam 0x007a12e0  in candidate callers: yes
  0x006bf180 CSWSCreature::AIActionDisarmMine -> Steam 0x005b43c0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0x10*,0x18*,0x94*,0x9c*,0xcc*,0xd0*,0x130*,0x224*,0x11ac*} | Steam {0x4*,0x10*,0x18*,0x94*,0x9c*,0xcc*,0xd0*,0x130*,0x224*,0x11ac*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x8*,0x9*,0x10*,0x11*,0x24*,0x78*} | Steam {0x2*,0x3*,0x4*,0x8*,0x9*,0x10*,0x11*,0x24*,0x78*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00765aa0: 16-byte aligned: YES
  00765aa0  55             PUSH EBP
  00765aa1  8B EC          MOV EBP,ESP
  00765aa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00765aa0
string literals (0):
direct callees (17):
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x00765DF0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x0073F510  CClientExoApp::GetPlayerByGameObjectID
  0x0071B8B0  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x00919723  operator new
  0x0070AFD0  (unnamed)
  0x0070B380  (unnamed)
  0x0070B350  (unnamed)
  0x0070B3F0  (unnamed)
  0x0070B170  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0xCC [dec 204]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0xD0 [dec 208]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
  +0x224 [dec 548]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00765aa0 FUN_00765aa0 size=836 ret=8 callers=7 callees=17

void __thiscall FUN_00765aa0(int param_1,short param_2,int param_3)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  undefined4 local_84;
  int local_38;
  undefined1 local_34 [8];
  undefined1 local_2c [20];
  uint local_18;
  undefined1 local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00964aab;
  local_10 = ExceptionList;
  local_18 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_18);
  local_8 = 0;
  FUN_007107f0();
  local_8 = 1;
  iVar3 = FUN_00765df0(param_2,local_34);
  if (iVar3 == 0) {
    local_8 = 0xffffffff;
    FUN_00733780();
    goto LAB_00765dcb;
  }
  iVar3 = FUN_0077d800();
  if (iVar3 != 0) {
    piVar4 = (int *)FUN_0077d800();
    iVar3 = (**(code **)(*piVar4 + 0x94))();
    if (iVar3 == 0) {
      piVar4 = (int *)FUN_0077d800();
      if (piVar4[0x46b = byte +0x11ac] == 0) {
        bVar1 = false;
      }
      else {
        sVar2 = (**(code **)(*piVar4 + 0x9c))(0);
        bVar1 = sVar2 < 1;
      }
      if (!bVar1) goto LAB_00765ba5;
    }
    if (param_2 != 0x10) {
      local_8 = 0xffffffff;
      FUN_00733780();
      goto LAB_00765dcb;
    }
  }
LAB_00765ba5:
  iVar3 = FUN_0073f450();
  if (iVar3 == param_1) {
    local_11 = 8;
  }
  else {
    iVar3 = FUN_0073f510(*(undefined4 *)(param_1 + 4));
    if (iVar3 == 0) {
      FUN_0071b8b0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdf4,&local_38);
      if (local_38 < 4) {
        local_11 = 0x11;
      }
      else {
        local_11 = 0x10;
      }
    }
    else {
      local_11 = 9;
    }
  }
  *(undefined4 *)(param_1 + 0xd0) = 0;
  if (*(int *)(param_1 + 0xcc) != 0) {
    iVar3 = *(int *)(param_1 + 0xcc);
    if (iVar3 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar3);
/* ... 36 more lines truncated by --max-lines */
```

### decoy: 0x00765df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765df0: 16-byte aligned: YES
  00765df0  55             PUSH EBP
  00765df1  8B EC          MOV EBP,ESP
  00765df3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765df0
string literals (0):
direct callees (3):
  0x007111E0  CRes::Demand
  0x0085EE80  (unnamed)
  0x007112B0  CRes::Release
struct offsets touched (top 3):
  +0x384 [dec 900]  x4  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX
  +0x314 [dec 788]  x1  via EAX
```

## Card B13a-1-53

GOG function: CSWCCreature::GetCameraOffset  gog 0x00421880  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775970 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetFollowingBody | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421860 CSWCCreature::GetHeadHeightOffset -> 0x00775990  gate_class=REVIEW
- GOG-order neighbour after:  0x004218a0 CSWCCreature::GetPauseReadyAnimation -> 0x007759b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d460 CSWCCreatureAppearance::GetCameraOffset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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

## Card B13a-1-54

GOG function: CSWCCreature::SetEngagedState  gog 0x00422e80  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00778510 (size 706, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::CanSee | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: Update 0x7ac470)
- best decoy (neighbour/other candidate) scores: 0x007783b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00422ea0 Global::SetQuededAnimationCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x13c} | Steam {0x4,0x8,0xc,0x14,0x20,0x38,0x4c,0x5c,0x70,0x98,0x140,0x188,0x31c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x8,0xc,0x24,0x50,0xfe,0xff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00778510: 16-byte aligned: YES
  00778510  55             PUSH EBP
  00778511  8B EC          MOV EBP,ESP
  00778513  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00778510
string literals (0):
direct callees (4):
  0x00465650  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x007783B0  (unnamed)
  0x0087BE80  CSWCDoor::GetServerDoor
struct offsets touched (top 10):
  +0x98 [dec 152]  x6  via EAX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x14 [dec 20]  x3  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x38 [dec 56]  x1  via ECX
  +0x5C [dec 92]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00778510 FUN_00778510 size=706 ret=4 callers=3 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool __thiscall FUN_00778510(int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  int iVar5;
  float fVar6;
  float fVar7;
  undefined4 uVar8;
  int iVar9;
  int *piVar10;
  bool bVar11;
  float10 fVar12;
  int local_1c;
  
  if (param_2 == (int *)0x0) {
    bVar11 = false;
  }
  else {
    iVar1 = param_1[9 = byte +0x24];
    iVar2 = param_1[10 = byte +0x28];
    fVar3 = (float)param_1[0xb = byte +0x2c];
    fVar7 = (float)_DAT_00989f10;
    iVar4 = param_2[9 = byte +0x24];
    iVar5 = param_2[10 = byte +0x28];
    fVar6 = (float)param_2[0xb = byte +0x2c];
    fVar12 = (float10)(**(code **)(*param_2 + 0x140))();
    FUN_00465650();
    DAT_00a2015c = 1;
    DAT_00a2010c = 0;
    uVar8 = (**(code **)(*param_1 + 0x98))(0xff,1);
    FUN_004491c0(uVar8);
    iVar9 = (**(code **)(*param_1 + 0x98))(0xfe,1);
    if (iVar9 != 0) {
      FUN_004491c0(iVar9);
    }
    uVar8 = (**(code **)(*param_2 + 0x98))(0xff,1);
    FUN_004491c0(uVar8);
    iVar9 = (**(code **)(*param_2 + 0x98))(0xfe,1);
    if (iVar9 != 0) {
      FUN_004491c0(iVar9);
    }
    bVar11 = true;
    while (bVar11) {
      bVar11 = false;
      FUN_00465650();
      local_1c = FUN_007783b0(*(undefined4 *)(param_1[8 = byte +0x20] + 0x188),iVar1,iVar2,fVar3 + fVar7,iVar4,
                              iVar5,(float)(fVar12 + (float10)fVar6),0,&DAT_00a20100);
      if (local_1c == 2) {
        piVar10 = (int *)(**(code **)(*DAT_00a20138 + 0x70))();
        if (piVar10 == (int *)0x0) {
          local_1c = 1;
        }
        else {
          iVar9 = (**(code **)(*piVar10 + 0x14))();
          if (iVar9 == 0) {
            iVar9 = (**(code **)(*piVar10 + 0x4c))();
            if (iVar9 != 0) {
              piVar10 = (int *)(**(code **)(*piVar10 + 0x4c))();
              uVar8 = (**(code **)(*piVar10 + 0x98))(0xff,1);
              FUN_004491c0(uVar8);
              bVar11 = true;
            }
          }
          else {
            (**(code **)(*piVar10 + 0x14))();
            iVar9 = FUN_0087be80();
            if ((iVar9 == 0) || (*(char *)(iVar9 + 0x31c) != '\0')) {
              piVar10 = (int *)(**(code **)(*piVar10 + 0x14))();
              uVar8 = (**(code **)(*piVar10 + 0x98))(0xff,1);
              FUN_004491c0(uVar8);
              bVar11 = true;
            }
            else {
              local_1c = 1;
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x007783b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007783b0: 16-byte aligned: YES
  007783b0  55             PUSH EBP
  007783b1  8B EC          MOV EBP,ESP
  007783b3  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007783b0
string literals (0):
direct callees (6):
  0x0064E080  (unnamed)
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x0054B370  CSWSArea::GetRoom_2
  0x0064E0C0  (unnamed)
  0x005FF6A0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x38 [dec 56]  x1  via EDX
```

## Card B13a-1-55

GOG function: CSWCCreature::GetHeadTurnVertical  gog 0x004200d0  callconv=(blank)  param_size_bytes=0  gog_size=93  gog_retn=0
Candidate Steam address: 0x00774260 (size 458, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.42) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00774200:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00420070 CSWCCreature::GetHeadTurnHorizontal -> 0x00774200  gate_class=REVIEW
- GOG-order neighbour after:  0x00420130 CSWCCreature::ApplyShadowBlob -> 0x00774260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x10*,0x18*,0x130*,0x224*} | Steam {0x4,0x8,0x10*,0x18*,0x68,0xa4,0xb4,0xd4,0x108,0x130*,0x224*}
imm sets (* = in both, 0 common): GOG {0x14} | Steam {0x3,0x4,0xc,0x1c,0x48,0xd8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00774260: 16-byte aligned: YES
  00774260  55             PUSH EBP
  00774261  8B EC          MOV EBP,ESP
  00774263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00774260 FUN_00774260 size=458 ret=0 callers=0 callees=5

undefined4 __fastcall FUN_00774260(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_40 [16];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974ab8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (param_1[0x1a = byte +0x68] == 0) {
    local_24 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = local_24;
  }
  else {
    iVar1 = FUN_0071b8b0(*(undefined2 *)(param_1[0x89 = byte +0x224] + 0x18),DAT_00a0fdf4,&local_14);
    if (iVar1 == 0) {
      local_14 = 3;
    }
    switch(local_14) {
    case 1:
    case 2:
      FUN_007338d0("Fx_Shadow_S");
      break;
    case 3:
      FUN_007338d0("Fx_Shadow_M");
      break;
    case 4:
    case 5:
      FUN_007338d0("Fx_Shadow_L");
      break;
    default:
      FUN_007338d0("Fx_Shadow_M");
    }
    local_20 = (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xa4))(local_1c);
    local_30 = 0;
    local_2c = 0;
    local_28 = 0x3f800000;
    uVar2 = FUN_005160c0(local_40,0,0,0x3f800000,param_1[0x36 = byte +0xd8],param_1[0x37 = byte +0xdc],param_1[0x38 = byte +0xe0]);
    (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xb4))(uVar2);
    if (param_1[0x35 = byte +0xd4] == 0) {
      (**(code **)(*param_1 + 0x108))();
    }
    uVar2 = local_20;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00774200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774200: 16-byte aligned: YES
  00774200  55             PUSH EBP
  00774201  8B EC          MOV EBP,ESP
  00774203  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B13a-1-56

GOG function: CSWCCreature::SetThrownLightsaber  gog 0x004213c0  callconv=(blank)  param_size_bytes=0  gog_size=66  gog_retn=0
Candidate Steam address: 0x007754d0 (size 66, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: LoadArea 0x7a2fb0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775520:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004213a0 CSWSPlaceable::SetEffectSpellId  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00421410 CSWCCreature::UpdatePopUpBubble -> 0x00775520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x20*,0x108*,0x3b4*} | Steam {0x20*,0x108*,0x3b4*}
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
prologue for 0x007754d0: 16-byte aligned: YES
  007754d0  55             PUSH EBP
  007754d1  8B EC          MOV EBP,ESP
  007754d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007754d0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x3B4 [dec 948]  x2  via EAX,ECX
  +0x108 [dec 264]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007754d0 FUN_007754d0 size=66 ret=0 callers=2 callees=0

void __fastcall FUN_007754d0(int param_1)

{
  if (*(int *)(param_1 + 0x20) == 0) {
    *(undefined4 *)(param_1 + 0x3b4) = DAT_00994484;
  }
  else {
    *(undefined4 *)(param_1 + 0x3b4) = *(undefined4 *)(*(int *)(param_1 + 0x20) + 0x108);
  }
  return;
}
```

### decoy: 0x00775520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775520: 16-byte aligned: YES
  00775520  55             PUSH EBP
  00775521  8B EC          MOV EBP,ESP
  00775523  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775520
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x48 [dec 72]  x1  via EDX
  +0x108 [dec 264]  x1  via ECX
  +0x3B4 [dec 948]  x1  via EAX
```

## Card B13a-1-57

GOG function: CSWCCreature::GetStartWayPoint  gog 0x00415d50  callconv=(blank)  param_size_bytes=0  gog_size=38  gog_retn=0
Candidate Steam address: 0x00769f30 (size 38, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00769ef0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00415d10 CSWCCreature::GetEndWayPoint -> 0x00769ef0  gate_class=AUTO
- GOG-order neighbour after:  0x00415d80 CSWCCreature::LoadModel -> 0x00769f60  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c4*,0x1f4*} | Steam {0x1c4*,0x1f4*}
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
prologue for 0x00769f30: 16-byte aligned: YES
  00769f30  55             PUSH EBP
  00769f31  8B EC          MOV EBP,ESP
  00769f33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00769f30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1C4 [dec 452]  x1  via EDX
  +0x1F4 [dec 500]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00769f30 FUN_00769f30 size=38 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00769f30(int param_1)

{
  undefined4 uVar1;
  
  if (*(short *)(param_1 + 0x1f4 [dec 500]) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(param_1 + 0x1c4);
  }
  return uVar1;
}
```

### decoy: 0x00769ef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00769ef0: 16-byte aligned: YES
  00769ef0  55             PUSH EBP
  00769ef1  8B EC          MOV EBP,ESP
  00769ef3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00769ef0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1F4 [dec 500]  x2  via EAX,EDX
  +0x1C4 [dec 452]  x1  via ECX
```

## Card B13a-1-58

GOG function: CSWCCreature::HandleConjuringCreature  gog 0x004138f0  callconv=(blank)  param_size_bytes=0  gog_size=1337  gog_retn=0
Candidate Steam address: 0x00767b20 (size 1337, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.55) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00767a80:STR0/CALLEE0/SIG0.069/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413850 CSWCCreature::BlurWeapon -> 0x00767a80  gate_class=REVIEW
- GOG-order neighbour after:  0x00413e30 CSWCCreature::ForceAppearanceUpdate -> 0x00768060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x004895b0 CSWCSpellCastVisual::LoadConjureVisual -> Steam ?  in candidate callees: n/a
  0x00489d30 CSWCSpellCastVisual::UnloadConjureVisual -> Steam ?  in candidate callees: n/a
  0x00489e50 CSWCSpellCastVisual::UnloadCastVisual -> Steam ?  in candidate callees: n/a
  0x00489f80 CSWCSpellCastVisual::UpdateTimer -> Steam ?  in candidate callees: n/a
  0x004ed450 CSWSpellArray::GetSpell -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x18*,0x1b*,0x1c*,0x20*,0x24*,0x44*,0x48*,0x104*,0x128*,0x224*,0x2d8*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x18*,0x1b*,0x1c*,0x20*,0x24*,0x44*,0x48*,0x104*,0x128*,0x224*,0x2d8*,...(+3)}
imm sets (* = in both, 24 common): GOG {0x10*,0x34*,0x52*,0x62*,0x72*,0x82*,0x92*,0x9c*,0xa2*,0x1388*,0x271f*,0x2720*,0x2721*,0x2722*,...(+10)} | Steam {0x10*,0x34*,0x52*,0x62*,0x72*,0x82*,0x92*,0x9c*,0xa2*,0x1388*,0x271f*,0x2720*,0x2721*,0x2722*,...(+10)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00767b20: 16-byte aligned: YES
  00767b20  55             PUSH EBP
  00767b21  8B EC          MOV EBP,ESP
  00767b23  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00767b20
string literals (0):
direct callees (9):
  0x006C1450  (unnamed)
  0x007107F0  (unnamed)
  0x00710EB0  (unnamed)
  0x00710CA0  (unnamed)
  0x00859CA0  (unnamed)
  0x00859DC0  (unnamed)
  0x00859520  (unnamed)
  0x00859EF0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x308 [dec 776]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x2D8 [dec 728]  x5  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x2DC [dec 732]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00767b20 FUN_00767b20 size=1337 ret=0 callers=1 callees=9

void __fastcall FUN_00767b20(int param_1)

{
  int iVar1;
  undefined1 local_74 [16];
  undefined1 local_64 [16];
  undefined1 local_54 [16];
  undefined1 local_44 [16];
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  uint local_14;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  local_c = 0;
  if (((((*(short *)(param_1 + 0x48) == 0x271f) || (*(short *)(param_1 + 0x48) == 0x2720)) ||
       (*(short *)(param_1 + 0x48) == 11000)) ||
      (((*(short *)(param_1 + 0x48) == 0x2721 || (*(short *)(param_1 + 0x48) == 0x2722)) ||
       ((*(short *)(param_1 + 0x48) == 0x2723 ||
        ((*(short *)(param_1 + 0x48) == 0x2724 || (*(short *)(param_1 + 0x48) == 0x274d)))))))) ||
     ((*(short *)(param_1 + 0x48) == 0x280a ||
      ((((*(short *)(param_1 + 0x48) == 0x280b || (*(short *)(param_1 + 0x48) == 0x280c)) ||
        (*(short *)(param_1 + 0x48) == 0x280d)) ||
       ((*(short *)(param_1 + 0x48) == 0x2815 || (*(short *)(param_1 + 0x48) == 0x28a1)))))))) {
    local_c = 1;
    local_8 = *(undefined4 *)(param_1 + 0x2dc);
    local_10 = FUN_006c1450(local_8);
  }
  if ((((*(short *)(param_1 + 0x48) == 0x2721) || (*(short *)(param_1 + 0x48) == 0x2722)) ||
      ((((*(short *)(param_1 + 0x48) == 0x2723 ||
         (((*(short *)(param_1 + 0x48) == 0x2724 || (*(short *)(param_1 + 0x48) == 0x274d)) ||
          (*(short *)(param_1 + 0x48) == 0x280a)))) ||
        (((*(short *)(param_1 + 0x48) == 0x280b || (*(short *)(param_1 + 0x48) == 0x280c)) ||
         (*(short *)(param_1 + 0x48) == 0x280d)))) ||
       ((*(short *)(param_1 + 0x48) == 0x2815 || (*(short *)(param_1 + 0x48) == 0x28a1)))))) &&
     (*(int *)(param_1 + 0x2d8) == 0)) {
    iVar1 = *(int *)(param_1 + 0x308);
    *(undefined4 *)(iVar1 + 0x18) = 5000;
    *(undefined4 *)(iVar1 + 0x1c) = 5000;
    *(undefined4 *)(*(int *)(param_1 + 0x308) + 0x20) = 0;
    *(undefined4 *)(param_1 + 0x2d8) = 1;
  }
  if (((local_c == 1) && (*(int *)(param_1 + 0x2d8) == 0)) && (local_10 != 0)) {
    FUN_007107f0();
    FUN_00710eb0(local_44,&DAT_00986925);
    if (*(int *)((uint)*(ushort *)(*(int *)(param_1 + 0x310) + 0x24) * 0x34 +
                 *(int *)(DAT_00a1b4d0 + 0x128) + 0x20) != 0) {
      if (*(char *)(*(int *)(param_1 + 0x224) + 0x1b) == '\x01') {
        FUN_00710ca0(local_54,local_10 + 0x92);
      }
      else {
        FUN_00710ca0(local_64,local_10 + 0x82);
      }
    }
    FUN_007107f0();
    FUN_00710ca0(local_74,local_10 + 0xa2);
    FUN_00859ca0();
    FUN_00859dc0();
    FUN_00859520(*(undefined4 *)(local_10 + 0x52),*(undefined4 *)(local_10 + 0x56),
                 *(undefined4 *)(local_10 + 0x5a),*(undefined4 *)(local_10 + 0x5e),
                 *(undefined4 *)(local_10 + 0x62),*(undefined4 *)(local_10 + 0x66),
                 *(undefined4 *)(local_10 + 0x6a),*(undefined4 *)(local_10 + 0x6e),
                 *(undefined4 *)(local_10 + 0x72),*(undefined4 *)(local_10 + 0x76),
                 *(undefined4 *)(local_10 + 0x7a),*(undefined4 *)(local_10 + 0x7e),local_24,local_20
                 ,local_1c,local_18,local_34,local_30,local_2c,local_28,param_1);
    iVar1 = *(int *)(param_1 + 0x308);
    *(undefined4 *)(iVar1 + 0x18) = 5000;
    *(undefined4 *)(iVar1 + 0x1c) = 5000;
    *(undefined4 *)(*(int *)(param_1 + 0x308) + 0x20) = 0;
  }
  if ((local_c == 0) && (*(int *)(param_1 + 0x2d8) == 1)) {
/* ... 11 more lines truncated by --max-lines */
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

## Card B13a-1-59

GOG function: CSWCCreature::UpdateRelativePerception  gog 0x00419c50  callconv=(blank)  param_size_bytes=4  gog_size=363  gog_retn=4
Candidate Steam address: 0x0076dde0 (size 363, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0076df50:STR0/CALLEE0/SIG0.786/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00419850 CSWCCreature::SetStealthState -> 0x0076d9e0  gate_class=AUTO
- GOG-order neighbour after:  0x00419dc0 CSWCCreature::UpdateRelativePerception_2 -> 0x0076df50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: yes
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x20*,0xb4*,0xb8*,0x1b8*} | Steam {0x4*,0x20*,0xb4*,0xb8*,0x1b8*}
imm sets (* = in both, 5 common): GOG {0x4*,0x24*,0x30*,0x7d1*,0x7f000000*} | Steam {0x4*,0x24*,0x30*,0x7d1*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076dde0: 16-byte aligned: YES
  0076dde0  55             PUSH EBP
  0076dde1  8B EC          MOV EBP,ESP
  0076dde3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076dde0
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0xB4 [dec 180]  x1  via EAX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0076dde0 FUN_0076dde0 size=363 ret=4 callers=2 callees=2

void FUN_0076dde0(undefined4 *param_1)

{
  int iVar1;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  int *local_c;
  int local_8;
  
  local_8 = 0;
  local_10 = 0;
  local_c = (int *)FUN_0073f550(*param_1);
  if (local_c != (int *)0x0) {
    if ((((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 1) == 0) && ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 1 & 1) == 1)) ||
       ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 4 & 1) == 1)) {
      local_8 = 1;
    }
    else if (((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 1) == 1) && ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 4 & 1) == 0)) {
      local_10 = 1;
    }
    iVar1 = FUN_0078eb80(local_c + 0x9 [dec 9] {int*: byte +0x24});
    if (iVar1 == 1) {
      if ((local_8 == 1) && (local_c[0x6e = byte +0x1b8] == 0)) {
        local_1c = 0;
        local_18 = 0;
        local_14 = 0;
        (**(code **)(*local_c + 0xb4))(0x7d1,0,0x7f000000,0x7f000000,0,0,&local_1c);
        local_c[0x6e = byte +0x1b8] = 1;
      }
      else if ((local_10 == 1) && (local_c[0x6e = byte +0x1b8] == 1)) {
        (**(code **)(*local_c + 0xb8))(0x7d1);
        local_c[0x6e = byte +0x1b8] = 0;
      }
    }
  }
  return;
}
```

### decoy: 0x0076df50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076df50: 16-byte aligned: YES
  0076df50  55             PUSH EBP
  0076df51  8B EC          MOV EBP,ESP
  0076df53  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076df50
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x7  via EAX,EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xB8 [dec 184]  x1  via EDX
```

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
