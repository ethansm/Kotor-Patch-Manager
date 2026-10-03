# Review cards B12a chunk 1

Answer each card with one TSV row in `steam_db_batches/B12a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B12a-1-01

GOG function: CSWSCreature::ResolveMeleeSpecialAttack  gog 0x0065b5a0  callconv=(blank)  param_size_bytes=4  gog_size=4075  gog_retn=20
Candidate Steam address: 0x00542e00 (size 849, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::DoDamageImmunity | G7 mac: pass(0.70) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00541fd0:STR0/CALLEE0/SIG0.236/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0065b530 CSWSCreature::ResolveRangedHit -> 0x006df230  gate_class=AUTO
- GOG-order neighbour after:  0x0065c680 CSWSCreature::ResolveRangedMiss -> 0x00543ae0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x0050e640 CSWRules::RollDice -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam 0x0051c280  in candidate callees: no
  0x0065a560 unnamed -> Steam ?  in candidate callees: n/a
  0x0065aca0 Global::rand_wincompatible -> Steam ?  in candidate callees: n/a
  0x00660930 CSWSCreature::SavingThrowRoll -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam 0x006a4e00  in candidate callees: no
  ... 12 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8,0xa,0xc,0x30*,0x3a,0x5c,0x64,0xee,0x122,0x145,0x154,0xabc*,0x10dc*,...(+1)} | Steam {0x4*,0x30*,0xb4,0xabc*,0x10dc*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3,0x4*,0x5,0x6,0x7,0x8,0xa,0xb,0xe,0xf,0x10,0x11,0x13,...(+17)} | Steam {0x2*,0x4*,0x14*,0x34,0x3e,0x64*,0x74,0xa0}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x00542e00: 16-byte aligned: YES
  00542e00  55             PUSH EBP
  00542e01  8B EC          MOV EBP,ESP
  00542e03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00542e00
string literals (0):
direct callees (10):
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x005E1C70  (unnamed)
  0x005908C0  CSWSCombatRound::GetAttack
  0x0083EA60  CExoArrayList<uint32>::Add
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX
  +0x30 [dec 48]  x2  via EAX,EDX
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0xB4 [dec 180]  x1  via EAX
  +0xABC [dec 2748]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00542e00 FUN_00542e00 size=849 ret=20 callers=0 callees=10

int __thiscall
FUN_00542e00(int *param_1,int param_2,int param_3,undefined2 param_4,int param_5,int param_6)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  int local_7c;
  int local_78;
  undefined4 local_74;
  int local_70;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f8f1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  cVar1 = (**(code **)(*param_1 + 0xb4))(param_4,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  iVar2 = (int)cVar1;
  if (iVar2 < 1) {
    local_70 = (iVar2 * param_3) / 100;
  }
  else if ((iVar2 * param_3) / 100 < 1) {
    local_70 = 1;
  }
  else {
    local_70 = (iVar2 * param_3) / 100;
  }
  local_18 = param_3 - local_70;
  if ((param_5 == 0) && (0 < iVar2)) {
    if ((param_6 == 1) && (param_2 != 0)) {
      iVar2 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar2 == 0) {
        local_74 = 0;
      }
      else {
        local_74 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1bd0(0,0x3e);
      FUN_005e1bd0(1,param_3 - local_18);
      FUN_005e1bd0(2,param_4);
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      FUN_005908c0(*(undefined1 *)(*(int *)(param_2 + 0x10dc) + 0xabc));
      FUN_0083ea60(local_74);
    }
    else {
      iVar2 = FUN_00919723(0x34);
      local_8 = 1;
      if (iVar2 == 0) {
        local_78 = 0;
      }
      else {
        local_78 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      iVar2 = FUN_00919723(0x34);
      local_8 = 2;
      if (iVar2 == 0) {
        local_7c = 0;
      }
      else {
        local_7c = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1bd0(0,0x3e);
      FUN_005e1bd0(1,param_3 - local_18);
      FUN_005e1bd0(2,param_4);
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      FUN_005e2be0(local_7c);
      iVar2 = (**(code **)(*param_1 + 0x30))();
      if (iVar2 == 0) {
        if (local_78 != 0) {
          FUN_005e25d0();
          FUN_009196fd(local_78);
/* ... 21 more lines truncated by --max-lines */
```

### decoy: 0x00541fd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00541fd0: 16-byte aligned: YES
  00541fd0  55             PUSH EBP
  00541fd1  8B EC          MOV EBP,ESP
  00541fd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00541fd0
string literals (0):
direct callees (15):
  0x005E4E60  CGameEffect::GetInteger
  0x005908C0  CSWSCombatRound::GetAttack
  0x005E4E80  CGameEffect::SetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E1BD0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x10DC [dec 4316]  x13  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x11  via EAX,ECX,EDX
  +0x30 [dec 48]  x8  via EAX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0xABC [dec 2748]  x5  via EAX,ECX,EDX
  +0x64 [dec 100]  x3  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x118 [dec 280]  x2  via EAX,EDX
  +0x116 [dec 278]  x1  via ECX
  +0x117 [dec 279]  x1  via EAX
```

## Card B12a-1-02

GOG function: CSWSCreature::ResolveRangedMissToParry  gog 0x0065ef50  callconv=(blank)  param_size_bytes=0  gog_size=123  gog_retn=0
Candidate Steam address: 0x006e2c50 (size 123, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x0065e710:0.79>0.23
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: IsLightsaber 0x615c80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005467b0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065ecb0 CSWSCreature::ResolveMoveToForceJump -> 0x006e29b0  gate_class=AUTO
- GOG-order neighbour after:  0x0065efd0 CSWSCreature::ResolveOnHitVisuals -> 0x006e2cd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006794b0 CSWSItem::IsLightsaber -> Steam 0x00615c80  in candidate callees: yes
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam 0x006d0620  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1150*} | Steam {0x1150*}
imm sets (* = in both, 3 common): GOG {0x10*,0x18*,0x20*} | Steam {0x10*,0x18*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006e2c50: 16-byte aligned: YES
  006e2c50  55             PUSH EBP
  006e2c51  8B EC          MOV EBP,ESP
  006e2c53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006e2c50
string literals (0):
direct callees (2):
  0x006D0620  CSWInventory::GetItemInSlot
  0x00615C80  CSWSItem::IsLightsaber
struct offsets touched (top 1):
  +0x1150 [dec 4432]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006e2c50 FUN_006e2c50 size=123 ret=0 callers=3 callees=2

undefined4 __fastcall FUN_006e2c50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 local_8;
  
  local_8 = 0;
  uVar3 = *(undefined4 *)(param_1 + 0x1150);
  iVar1 = FUN_006d0620(0x10);
  if (iVar1 != 0) {
    iVar1 = FUN_00615c80();
    if (iVar1 != 0) {
      local_8 = 1;
    }
  }
  uVar2 = *(undefined4 *)(param_1 + 0x1150);
  iVar1 = FUN_006d0620(0x20);
  if (iVar1 != 0) {
    iVar1 = FUN_00615c80(param_1,uVar2,uVar3);
    if (iVar1 != 0) {
      local_8 = 1;
    }
  }
  return local_8;
}
```

### decoy: 0x005467b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005467b0: 16-byte aligned: YES
  005467b0  55             PUSH EBP
  005467b1  8B EC          MOV EBP,ESP
  005467b3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005467b0
string literals (0):
direct callees (8):
  0x0073F750  ClientApp::GetGuiInGame
  0x007D0C70  CGuiInGame::UpdateDialogLipSync
  0x006CF5D0  DialogStartPacifyHostiles
  0x00545520  CSWSObject::IsDialogDelay
  0x00546A50  (unnamed)
  0x00546540  CSWSObject::SendDialogEntry
  0x00546650  CSWSObject::SendDialogReplies
  0x00546900  CSWSObject::StopDialog
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x48 [dec 72]  x2  via ECX
  +0x4C [dec 76]  x2  via EAX,ECX
  +0x54 [dec 84]  x2  via EAX
  +0x25C [dec 604]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EDX
  +0x60 [dec 96]  x1  via ECX
  +0xD8 [dec 216]  x1  via ECX
```

## Card B12a-1-03

GOG function: CSWSCreature::BroadcastDeathDataToParty  gog 0x00691f60  callconv=(blank)  param_size_bytes=4  gog_size=338  gog_retn=4
Candidate Steam address: 0x00579430 (size 338, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00579590:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00691df0 CSWSCreature::BroadcastDamageDataToParty -> 0x005792c0  gate_class=AUTO
- GOG-order neighbour after:  0x006920c0 CSWSCreature::BroadcastFloatyData -> 0x00579590  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam 0x0051c2e0  in candidate callees: yes
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: yes
  0x006db8d0 CSWSCreature::GetFaction -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007df3e0 CSWSMessage::SendServerToPlayerCCMessage -> Steam ?  in candidate callees: n/a
  0x007e4990 CSWSFaction::GetMemberList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x90*} | Steam {0x4*,0x8*,0x90*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x50*,0x94*} | Steam {0x4*,0x8*,0xc*,0x50*,0x94*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00579430: 16-byte aligned: YES
  00579430  55             PUSH EBP
  00579431  8B EC          MOV EBP,ESP
  00579433  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00579430
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00579430 FUN_00579430 size=338 ret=4 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00579430(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  float10 fVar3;
  int *piVar4;
  int extraout_var;
  undefined1 local_20 [12];
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  iVar1 = FUN_005d0cb0();
  if (iVar1 != 0) {
    piVar4 = &local_14;
    iVar1 = extraout_var;
    FUN_005d0cb0(piVar4);
    FUN_006d3470(piVar4);
    local_8 = 0;
    while( true ) {
      iVar2 = FUN_005d0cb0();
      if (*(int *)(iVar2 + 4) <= local_8) break;
      local_c = FUN_0051c600(*(undefined4 *)(local_14 + local_8 * 4));
      if (local_c != 0) {
        local_10 = FUN_00632a20();
        if (local_10 != 0) {
          if (*(int *)(iVar1 + 0x90) == *(int *)(local_10 + 0x90)) {
            iVar2 = FUN_00632a20();
            FUN_004452b0(local_20,iVar2 + 0x94,iVar1 + 0x94);
            fVar3 = (float10)FUN_0044b8a0(local_20,local_20);
            if (fVar3 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
              FUN_0051c2e0(*(undefined4 *)(local_c + 4),4,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                           iVar1,*(undefined4 *)(DAT_00a1b4a4 + 8));
              FUN_00648cc0();
            }
          }
        }
      }
      local_8 = local_8 + 1;
    }
  }
  return;
}
```

### decoy: 0x00579590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00579590: 16-byte aligned: YES
  00579590  55             PUSH EBP
  00579591  8B EC          MOV EBP,ESP
  00579593  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00579590
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,ECX
```

## Card B12a-1-04

GOG function: CSWSCreature::AddPickUpItemActions  gog 0x00693ac0  callconv=(blank)  param_size_bytes=0  gog_size=116  gog_retn=12
Candidate Steam address: 0x0057af90 (size 116, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0057b010:STR0/CALLEE0/SIG0.133/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00693760 CSWSCreature::BroadcastCombatStateToParty -> 0x0057ac30  gate_class=AUTO
- GOG-order neighbour after:  0x00693b40 CSWSCreature::UseFeat -> 0x0057b010  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x004ecc00 CSWItem::GetBaseItem -> Steam 0x006d6e30  in candidate callees: yes
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam 0x006d0620  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8a*,0xfa*,0x1150*,0x1198*} | Steam {0x8a*,0xfa*,0x1150*,0x1198*}
imm sets (* = in both, 3 common): GOG {0x2*,0xc*,0x1c*} | Steam {0x2*,0xc*,0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x0057af90: 16-byte aligned: YES
  0057af90  55             PUSH EBP
  0057af91  8B EC          MOV EBP,ESP
  0057af93  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057af90
string literals (0):
direct callees (2):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D6E30  CSWItem::GetBaseItem
struct offsets touched (top 4):
  +0x8A [dec 138]  x1  via EDX
  +0xFA [dec 250]  x1  via EDX
  +0x1150 [dec 4432]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057af90 FUN_0057af90 size=116 ret=12 callers=2 callees=2

void __fastcall FUN_0057af90(int param_1)

{
  int iVar1;
  undefined1 local_5;
  
  iVar1 = FUN_006d0620(2);
  local_5 = 0;
  if (iVar1 != 0) {
    iVar1 = FUN_006d6e30();
    if (iVar1 != 0) {
      local_5 = *(undefined1 *)(iVar1 + 0x8a);
    }
  }
  *(undefined1 *)(*(int *)(param_1 + 0x1198) + 0xfa) = local_5;
  return;
}
```

### decoy: 0x0057b010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057b010: 16-byte aligned: YES
  0057b010  55             PUSH EBP
  0057b011  8B EC          MOV EBP,ESP
  0057b013  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057b010
string literals (0):
direct callees (3):
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x006A20F0  (unnamed)
  0x00590A50  CSWSCombatRound::AddAttackAction
struct offsets touched (top 4):
  +0xFE8 [dec 4072]  x3  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x10DC [dec 4316]  x1  via EAX
  +0x57B19C [dec 5747100]  x1  via ECX
```

## Card B12a-1-05

GOG function: CSWSCreature::SetSilentMove  gog 0x0067efe0  callconv=(blank)  param_size_bytes=4  gog_size=87  gog_retn=4
Candidate Steam address: 0x005663b0 (size 87, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: ApplyDampenSound 0x5b2a70, RemoveDampenSound 0x5b2a90)
- best decoy (neighbour/other candidate) scores: 0x00566410:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067ed60 CSWSCreature::AddEquipItemActions -> 0x00566130  gate_class=AUTO
- GOG-order neighbour after:  0x0067f040 CSWSCreature::AddStanceActions -> 0x00566410  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007c9e80 CSWSItemPropertyHandler::ApplyDampenSound -> Steam 0x005b2a70  in candidate callers: yes
  0x007c9ea0 CSWSItemPropertyHandler::RemoveDampenSound -> Steam 0x005b2a90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x518*} | Steam {0x518*}
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
prologue for 0x005663b0: 16-byte aligned: YES
  005663b0  55             PUSH EBP
  005663b1  8B EC          MOV EBP,ESP
  005663b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005663b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x518 [dec 1304]  x6  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005663b0 FUN_005663b0 size=87 ret=4 callers=2 callees=0

/* WARNING: Removing unreachable block (ram,0x005663f7) */

void __thiscall FUN_005663b0(int param_1,int param_2)

{
  if (param_2 == 0) {
    *(char *)(param_1 + 0x518) = *(char *)(param_1 + 0x518) + -1;
  }
  else {
    *(char *)(param_1 + 0x518) = *(char *)(param_1 + 0x518) + '\x01';
  }
  return;
}
```

### decoy: 0x00566410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00566410: 16-byte aligned: YES
  00566410  55             PUSH EBP
  00566411  8B EC          MOV EBP,ESP
  00566413  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00566410
string literals (0):
direct callees (9):
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00590D10  CSWSCombatRound::AddStanceAction
  0x00563C90  CSWSCreature::ActionManager
  0x0053F560  (unnamed)
  0x00540CA0  CSWSObject::AddActionToFront
  0x005718A0  CSWSCreature::ComputeAIState
  0x00541080  CSWSObject::ClearAllActions
  0x0053F7F0  CSWSObject::AddAction
  0x0057AC30  CSWSCreature::BroadcastCombatStateToParty
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0xEC [dec 236]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x10DC [dec 4316]  x1  via EDX
```

## Card B12a-1-06

GOG function: CSWSCreature::BroadcastFloatyData  gog 0x006920c0  callconv=(blank)  param_size_bytes=4  gog_size=322  gog_retn=4
Candidate Steam address: 0x00579590 (size 322, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00579430:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00691f60 CSWSCreature::BroadcastDeathDataToParty -> 0x00579430  gate_class=REVIEW
- GOG-order neighbour after:  0x00692210 CSWSCreature::BroadcastSkillData -> 0x005796e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam 0x0051c2e0  in candidate callees: yes
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: yes
  0x006db8d0 CSWSCreature::GetFaction -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007df3e0 CSWSMessage::SendServerToPlayerCCMessage -> Steam ?  in candidate callees: n/a
  0x007e4990 CSWSFaction::GetMemberList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x90*} | Steam {0x4*,0x8*,0x90*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xb*,0xc*,0x50*,0x94*} | Steam {0x4*,0x8*,0xb*,0xc*,0x50*,0x94*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00579590: 16-byte aligned: YES
  00579590  55             PUSH EBP
  00579591  8B EC          MOV EBP,ESP
  00579593  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00579590
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00579590 FUN_00579590 size=322 ret=4 callers=0 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00579590(undefined4 param_1)

{
  int iVar1;
  float10 fVar2;
  int *piVar3;
  int extraout_var;
  int iVar4;
  undefined1 local_20 [12];
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  piVar3 = &local_14;
  FUN_005d0cb0(piVar3);
  iVar4 = extraout_var;
  FUN_006d3470(piVar3);
  local_8 = 0;
  while( true ) {
    iVar1 = FUN_005d0cb0();
    if (*(int *)(iVar1 + 4) <= local_8) break;
    local_c = FUN_0051c600(*(undefined4 *)(local_14 + local_8 * 4));
    if (local_c != 0) {
      local_10 = FUN_00632a20();
      if (local_10 != 0) {
        if (*(int *)(iVar4 + 0x90) == *(int *)(local_10 + 0x90)) {
          iVar1 = FUN_00632a20();
          FUN_004452b0(local_20,iVar1 + 0x94,iVar4 + 0x94);
          fVar2 = (float10)FUN_0044b8a0(local_20,local_20);
          if (fVar2 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
            FUN_0051c2e0(*(undefined4 *)(local_c + 4),0xb,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                         iVar4,*(undefined4 *)(DAT_00a1b4a4 + 8));
            FUN_00648cc0();
          }
        }
      }
    }
    local_8 = local_8 + 1;
  }
  return;
}
```

### decoy: 0x00579430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00579430: 16-byte aligned: YES
  00579430  55             PUSH EBP
  00579431  8B EC          MOV EBP,ESP
  00579433  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00579430
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x90 [dec 144]  x2  via EAX,ECX
```

## Card B12a-1-07

GOG function: CSWSCreature::ResolveRangedMiss  gog 0x0065c680  callconv=(blank)  param_size_bytes=2  gog_size=135  gog_retn=4
Candidate Steam address: 0x00543ae0 (size 761, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::DoSpellImmunity | G7 mac: pass(0.12) | G8 mac_rotation: rotation:0x0065c900:0.55>0.25
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00543f10:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065b5a0 CSWSCreature::ResolveMeleeSpecialAttack -> 0x00542e00  gate_class=REVIEW
- GOG-order neighbour after:  0x0065c710 CSWSCreature::ResolveMeleeAnimations -> 0x00543ae0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xd8} | Steam {0x4,0x8,0x30,0x160,0x1e4,0x1198}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x271f,0x2724,0x274d,0x2756,0x2757,0x28a1,0x2af8} | Steam {0x2,0x4*,0x34,0x44,0x70,0x148}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00543ae0: 16-byte aligned: YES
  00543ae0  55             PUSH EBP
  00543ae1  8B EC          MOV EBP,ESP
  00543ae3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00543ae0
string literals (0):
direct callees (9):
  0x005E4E60  CGameEffect::GetInteger
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 6):
  +0x30 [dec 48]  x6  via EAX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x160 [dec 352]  x1  via EAX
  +0x1E4 [dec 484]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00543ae0 FUN_00543ae0 size=761 ret=4 callers=1 callees=9

undefined4 __thiscall FUN_00543ae0(int *param_1,int *param_2)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_78;
  int local_74;
  uint local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f976;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  bVar1 = false;
  iVar3 = param_2[0x79 = byte +0x1e4];
  iVar2 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if ((iVar2 != 0) && (0 < param_1[0x53 = byte +0x14c])) {
    iVar2 = (**(code **)(*param_1 + 0x30))();
    local_24 = (uint)*(ushort *)(*(int *)(iVar2 + 0x1198) + 0x160);
    while( true ) {
      if ((param_1[0x53 = byte +0x14c] <= (int)local_24) ||
         ((int)DAT_00995fe4 < (int)(uint)*(ushort *)(*(int *)(param_1[0x52 = byte +0x148] + local_24 * 4) + 8)))
      goto LAB_00543c15;
      if ((*(ushort *)(*(int *)(param_1[0x52 = byte +0x148] + local_24 * 4) + 8) == DAT_00995fe4) &&
         ((iVar2 = FUN_005e4e60(0), iVar2 == iVar3 || (iVar2 = FUN_005e4e60(0), iVar2 == -1))))
      break;
      local_24 = local_24 + 1;
    }
    bVar1 = true;
  }
LAB_00543c15:
  if (bVar1) {
    iVar2 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar2 == 0) {
      local_74 = 0;
    }
    else {
      local_74 = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    iVar2 = FUN_00919723(0x34);
    local_8 = 1;
    if (iVar2 == 0) {
      local_78 = 0;
    }
    else {
      local_78 = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(1,iVar3);
    FUN_005e1c70(0,param_1[1 = byte +0x4]);
    FUN_005e2be0(local_78);
    iVar3 = (**(code **)(*param_1 + 0x30))();
    if (iVar3 == 0) {
      if (local_74 != 0) {
        FUN_005e25d0();
        FUN_009196fd(local_74);
      }
    }
    else {
      uVar4 = 0x44;
      (**(code **)(*param_1 + 0x30))(0x44,local_74);
      FUN_00584da0(uVar4,local_74);
    }
    iVar3 = (**(code **)(*param_2 + 0x30))();
    if (iVar3 == 0) {
      if (local_78 != 0) {
        FUN_005e25d0();
        FUN_009196fd(local_78);
      }
    }
    else {
      uVar4 = 0x44;
/* ... 13 more lines truncated by --max-lines */
```

### decoy: 0x00543f10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00543f10: 16-byte aligned: YES
  00543f10  55             PUSH EBP
  00543f11  8B EC          MOV EBP,ESP
  00543f13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00543f10
string literals (0):
direct callees (1):
  0x0054A430  CSWSObject::SetDirtyFlag
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B12a-1-08

GOG function: CSWSCreature::GetLockOrientationToObject  gog 0x0067be30  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00563170 (size 141, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563200:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067ab60 CSWSCreature::Constructor -> 0x00561f30  gate_class=AUTO
- GOG-order neighbour after:  0x0067be50 CSWSCreature::GetDialogResref -> 0x00563220  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x104c} | Steam {0x2,0x4,0x8,0x18}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x14,0xffff,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00563170: 16-byte aligned: YES
  00563170  55             PUSH EBP
  00563171  8B EC          MOV EBP,ESP
  00563173  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563170
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563170 FUN_00563170 size=141 ret=0 callers=0 callees=0

undefined2 * __fastcall FUN_00563170(undefined2 *param_1)

{
  *(undefined4 *)(param_1 + 0x6 [dec 6] {undefined2*: byte +0xc}) = 0;
  *(undefined4 *)(param_1 + 0x8 [dec 8] {undefined2*: byte +0x10}) = 0;
  *(undefined4 *)(param_1 + 0xa [dec 10] {undefined2*: byte +0x14}) = 0;
  *param_1 = 0xffff;
  param_1[1] = 0xffff;
  param_1[2] = 0xffff;
  *(undefined4 *)(param_1 + 0x6 [dec 6] {undefined2*: byte +0xc}) = 0;
  *(undefined4 *)(param_1 + 0x8 [dec 8] {undefined2*: byte +0x10}) = 0;
  *(undefined4 *)(param_1 + 0xa [dec 10] {undefined2*: byte +0x14}) = 0;
  *(undefined4 *)(param_1 + 0xc) = 0x7f000000;
  *(undefined4 *)(param_1 + 0x4 [dec 4] {undefined2*: byte +0x8}) = 0xffffffff;
  return param_1;
}
```

### decoy: 0x00563200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563200: 16-byte aligned: YES
  00563200  55             PUSH EBP
  00563201  8B EC          MOV EBP,ESP
  00563203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563200
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x104C [dec 4172]  x1  via EAX
```

## Card B12a-1-09

GOG function: CSWSCreature::AddHealActions  gog 0x00680b30  callconv=(blank)  param_size_bytes=9  gog_size=282  gog_retn=12
Candidate Steam address: 0x00567f00 (size 282, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: rotation:0x00680c50:0.62>0.35
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ExecuteCommandPutDownItem 0x68f560); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00567e40:STR0/CALLEE0/SIG0.336/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00680a70 CSWSCreature::AddDropItemActions -> 0x00567e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00680c50 CSWSCreature::Rest -> 0x00568020  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam 0x0051c0e0  in candidate callees: yes
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam 0x006d0620  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006a64d0 CSWSObject::AddAction -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x007a4bb0 Global::ExecuteCommandPutDownItem -> Steam 0x0068f560  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x14*,0xec*,0x520*,0x1150*} | Steam {0x8*,0x14*,0xec*,0x520*,0x1150*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0x4*,0x8*,0x9*,0xc*,0x24*,0xc2*,0xffff*} | Steam {0x2*,0x3*,0x4*,0x8*,0x9*,0xc*,0x24*,0xc2*,0xffff*}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 9 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00567f00: 16-byte aligned: YES
  00567f00  55             PUSH EBP
  00567f01  8B EC          MOV EBP,ESP
  00567f03  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00567f00
string literals (0):
direct callees (5):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x005453C0  CSWSObject::GetArea
  0x006D0620  CSWInventory::GetItemInSlot
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0xEC [dec 236]  x1  via ECX
  +0x520 [dec 1312]  x1  via EAX
  +0x1150 [dec 4432]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00567f00 FUN_00567f00 size=282 ret=12 callers=3 callees=5

undefined4 __thiscall FUN_00567f00(int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0051c0e0(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar1 = FUN_005453c0();
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else {
      if (((*(int *)(param_1 + 0x520) == 1) && (iVar1 = FUN_006d0620(2), iVar1 != 0)) &&
         (*(int *)(iVar1 + 0x14) == param_2)) {
        FUN_00584da0(0xc2,0);
        return 0;
      }
      if (*(int *)(param_1 + 0xec) == 0) {
        uVar2 = 0;
      }
      else {
        FUN_0053f7f0(9,0xffff,3,&param_2,2,param_3,2,param_3 + 4,2,param_3 + 8,1,&stack0x0000000c,0,
                     0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
        uVar2 = 1;
      }
    }
  }
  return uVar2;
}
```

### decoy: 0x00567e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00567e40: 16-byte aligned: YES
  00567e40  55             PUSH EBP
  00567e41  8B EC          MOV EBP,ESP
  00567e43  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00567e40
string literals (0):
direct callees (3):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x005453C0  CSWSObject::GetArea
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0xEC [dec 236]  x1  via EAX
```

## Card B12a-1-10

GOG function: CSWSCreature::SetEffectSpellId  gog 0x0067bfa0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00563370 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563350:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067bf80 CSWSCreature::GetEffectSpellId -> 0x00563350  gate_class=REVIEW
- GOG-order neighbour after:  0x0067bfc0 CSWSCreature::GetPortrait -> 0x00563390  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xff8*} | Steam {0xff8*}
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
prologue for 0x00563370: 16-byte aligned: YES
  00563370  55             PUSH EBP
  00563371  8B EC          MOV EBP,ESP
  00563373  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563370
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563370 FUN_00563370 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_00563370(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xff8) = param_2;
  return;
}
```

### decoy: 0x00563350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563350: 16-byte aligned: YES
  00563350  55             PUSH EBP
  00563351  8B EC          MOV EBP,ESP
  00563353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563350
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```

## Card B12a-1-11

GOG function: CSWSCreature::BroadcastBreakdownMessage  gog 0x00692c30  callconv=(blank)  param_size_bytes=8  gog_size=314  gog_retn=8
Candidate Steam address: 0x0057a100 (size 314, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00579fc0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00692af0 CSWSCreature::BroadcastAttackSummaryToParty -> 0x00579fc0  gate_class=REVIEW
- GOG-order neighbour after:  0x00692d70 CSWSCreature::BroadcastAttackBreakdown -> 0x0057a240  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam 0x0051c2e0  in candidate callees: yes
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: yes
  0x006db8d0 CSWSCreature::GetFaction -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007df3e0 CSWSMessage::SendServerToPlayerCCMessage -> Steam ?  in candidate callees: n/a
  0x007e4990 CSWSFaction::GetMemberList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x90*} | Steam {0x4*,0x8*,0x90*}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x4c*,0x94*} | Steam {0x8*,0xc*,0x4c*,0x94*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0057a100: 16-byte aligned: YES
  0057a100  55             PUSH EBP
  0057a101  8B EC          MOV EBP,ESP
  0057a103  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057a100
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057a100 FUN_0057a100 size=314 ret=8 callers=6 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057a100(undefined1 param_1,undefined4 param_2)

{
  float10 fVar1;
  int extraout_var;
  int iVar2;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = FUN_005d0cb0();
  if (local_8 != 0) {
    local_10 = 0;
    iVar2 = extraout_var;
    local_c = FUN_006d3470(&local_10);
    for (local_14 = 0; local_14 < local_c; local_14 = local_14 + 1) {
      local_18 = FUN_0051c600(*(undefined4 *)(local_10 + local_14 * 4));
      if (local_18 != 0) {
        local_1c = FUN_00632a20();
        if (local_1c != 0) {
          if (*(int *)(iVar2 + 0x90) == *(int *)(local_1c + 0x90)) {
            FUN_004452b0(local_28,local_1c + 0x94,iVar2 + 0x94);
            fVar1 = (float10)FUN_0044b8a0(local_28,local_28);
            if (fVar1 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
              FUN_0051c2e0(*(undefined4 *)(local_18 + 4),param_1,param_2,0,
                           _DAT_009919f0 * _DAT_009919f0,iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
              FUN_00648cc0();
            }
          }
        }
      }
    }
  }
  return;
}
```

### decoy: 0x00579fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00579fc0: 16-byte aligned: YES
  00579fc0  55             PUSH EBP
  00579fc1  8B EC          MOV EBP,ESP
  00579fc3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00579fc0
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B12a-1-12

GOG function: CSWSCreature::SetEffectSpellId  gog 0x0067bfa0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00563220 (size 91, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetDialogResref | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563200:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067bf80 CSWSCreature::GetEffectSpellId -> 0x00563350  gate_class=REVIEW
- GOG-order neighbour after:  0x0067bfc0 CSWSCreature::GetPortrait -> 0x00563390  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xff8} | Steam {0x4,0x8,0xc,0x1198}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x44}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00563220: 16-byte aligned: YES
  00563220  55             PUSH EBP
  00563221  8B EC          MOV EBP,ESP
  00563223  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563220
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x1198 [dec 4504]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00563220 FUN_00563220 size=91 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_00563220(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x1198) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    iVar1 = *(int *)(param_1 + 0x1198);
    *param_2 = *(undefined4 *)(iVar1 + 0x44);
    param_2[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0x48);
    param_2[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x4c);
    param_2[3 = byte +0xc] = *(undefined4 *)(iVar1 + 0x50);
  }
  return param_2;
}
```

### decoy: 0x00563200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563200: 16-byte aligned: YES
  00563200  55             PUSH EBP
  00563201  8B EC          MOV EBP,ESP
  00563203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563200
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x104C [dec 4172]  x1  via EAX
```

## Card B12a-1-13

GOG function: CSWSCreature::GetWeaponHitShots  gog 0x0065aa20  callconv=(blank)  param_size_bytes=4  gog_size=187  gog_retn=4
Candidate Steam address: 0x006de720 (size 187, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006de660:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065a960 CSWSCreature::GetWeaponDischargeShots -> 0x006de660  gate_class=REVIEW
- GOG-order neighbour after:  0x0065ab90 CSWSCreature::GetWeaponDischargeHand -> 0x006de890  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x0065a650 CSWSCreature::GetWeaponDischargeRow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xe0*,0x130*} | Steam {0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x24*} | Steam {0x4*,0x24*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006de720: 16-byte aligned: YES
  006de720  55             PUSH EBP
  006de721  8B EC          MOV EBP,ESP
  006de723  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006de720
string literals (0):
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006de720 FUN_006de720 size=187 ret=4 callers=1 callees=4

undefined4 FUN_006de720(undefined4 param_1)

{
  uint uVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095d960;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_006de350(local_1c,param_1);
  local_8 = 0;
  local_14 = 0;
  uVar2 = extraout_var;
  FUN_00733570(&DAT_0099a94c);
  local_8._0_1_ = 1;
  uVar3 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xe0);
  FUN_0071bad0(local_1c,local_24,&local_14);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780(uVar1,uVar2,uVar3);
  uVar3 = local_14;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x006de660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006de660: 16-byte aligned: YES
  006de660  55             PUSH EBP
  006de661  8B EC          MOV EBP,ESP
  006de663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006de660
string literals (1):
  0x0099A968  at 006de6a6  'Shots'
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B12a-1-14

GOG function: CSWSCreature::ResolveInitiative  gog 0x0068a4b0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00571980 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005718a0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0068a3d0 CSWSCreature::ComputeAIState -> 0x005718a0  gate_class=AUTO
- GOG-order neighbour after:  0x0068a4d0 CSWSCreature::ComputeAIStateOnAction -> 0x005719a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0xa*} | Steam {0x4*,0xa*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00571980: 16-byte aligned: YES
  00571980  55             PUSH EBP
  00571981  8B EC          MOV EBP,ESP
  00571983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00571980
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00571980 FUN_00571980 size=28 ret=4 callers=2 callees=0

bool FUN_00571980(int param_1)

{
  return param_1 < 10;
}
```

### decoy: 0x005718a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005718a0: 16-byte aligned: YES
  005718a0  55             PUSH EBP
  005718a1  8B EC          MOV EBP,ESP
  005718a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005718a0
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0091DBA0  _memcpy
  0x005719A0  CSWSCreature::ComputeAIStateOnAction
  0x0058C370  (unnamed)
struct offsets touched (top 4):
  +0x84 [dec 132]  x2  via EAX,ECX
  +0x1116 [dec 4374]  x2  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
  +0x1118 [dec 4376]  x1  via EAX
```

## Card B12a-1-15

GOG function: CSWSCreature::BroadcastAttackSummaryToParty  gog 0x00692af0  callconv=(blank)  param_size_bytes=4  gog_size=312  gog_retn=4
Candidate Steam address: 0x00579fc0 (size 312, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0057a100:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006929a0 CSWSCreature::BroadcastVoiceChat -> 0x00579e70  gate_class=AUTO
- GOG-order neighbour after:  0x00692c30 CSWSCreature::BroadcastBreakdownMessage -> 0x0057a100  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam 0x0051c2e0  in candidate callees: yes
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: yes
  0x006db8d0 CSWSCreature::GetFaction -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007df3e0 CSWSMessage::SendServerToPlayerCCMessage -> Steam ?  in candidate callees: n/a
  0x007e4990 CSWSFaction::GetMemberList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x90*} | Steam {0x4*,0x8*,0x90*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x12*,0x4c*,0x94*} | Steam {0x4*,0x8*,0xc*,0x12*,0x4c*,0x94*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00579fc0: 16-byte aligned: YES
  00579fc0  55             PUSH EBP
  00579fc1  8B EC          MOV EBP,ESP
  00579fc3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00579fc0
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00579fc0 FUN_00579fc0 size=312 ret=4 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00579fc0(undefined4 param_1)

{
  float10 fVar1;
  int extraout_var;
  int iVar2;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = FUN_005d0cb0();
  if (local_10 != 0) {
    local_c = 0;
    iVar2 = extraout_var;
    local_8 = FUN_006d3470(&local_c);
    for (local_14 = 0; local_14 < local_8; local_14 = local_14 + 1) {
      local_18 = FUN_0051c600(*(undefined4 *)(local_c + local_14 * 4));
      if (((local_18 != 0) && (local_1c = FUN_00632a20(), local_1c != 0)) &&
         (*(int *)(iVar2 + 0x90) == *(int *)(local_1c + 0x90))) {
        FUN_004452b0(local_28,local_1c + 0x94,iVar2 + 0x94);
        fVar1 = (float10)FUN_0044b8a0(local_28,local_28);
        if (fVar1 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
          FUN_0051c2e0(*(undefined4 *)(local_18 + 4),0x12,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                       iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
          FUN_00648cc0();
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0057a100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057a100: 16-byte aligned: YES
  0057a100  55             PUSH EBP
  0057a101  8B EC          MOV EBP,ESP
  0057a103  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057a100
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B12a-1-16

GOG function: CSWSCreature::GetActivity  gog 0x0067c9a0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x00563d70 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExecuteCommandIsStealthed 0x68bcf0)
- best decoy (neighbour/other candidate) scores: 0x00563da0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067c950 CSWSCreature::ActivityManager -> 0x00563d20  gate_class=REVIEW
- GOG-order neighbour after:  0x0067c9d0 CSWSCreature::SetActivity -> 0x00563da0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x007a1340 Global::ExecuteCommandIsStealthed -> Steam 0x0068bcf0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1120*} | Steam {0x1120*}
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
prologue for 0x00563d70: 16-byte aligned: YES
  00563d70  55             PUSH EBP
  00563d71  8B EC          MOV EBP,ESP
  00563d73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563d70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1120 [dec 4384]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563d70 FUN_00563d70 size=36 ret=4 callers=7 callees=0

bool __thiscall FUN_00563d70(int param_1,uint param_2)

{
  return (*(uint *)(param_1 + 0x1120) & param_2) != 0;
}
```

### decoy: 0x00563da0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563da0: 16-byte aligned: YES
  00563da0  55             PUSH EBP
  00563da1  8B EC          MOV EBP,ESP
  00563da3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563da0
string literals (0):
direct callees (4):
  0x00563F40  CSWSCreature::ClearActivities
  0x005B7AA0  CSWSCreature::SetStealthMode
  0x005B7A60  (unnamed)
  0x00546900  CSWSObject::StopDialog
struct offsets touched (top 2):
  +0x1120 [dec 4384]  x10  via EAX,ECX,EDX
  +0x1124 [dec 4388]  x1  via EAX
```

## Card B12a-1-17

GOG function: CSWSCreature::ApplyOnHitHold  gog 0x006614d0  callconv=(blank)  param_size_bytes=12  gog_size=305  gog_retn=12
Candidate Steam address: 0x006e51d0 (size 305, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x00661950:0.81>0.47
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006e5030:STR0/CALLEE1/SIG0.643/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00661330 CSWSCreature::CreateLinkedEffect -> 0x006e5030  gate_class=AUTO
- GOG-order neighbour after:  0x00661610 CSWSCreature::ApplyOnHitConfusion -> 0x00548b10  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734f20 CGameEffect::SetLinked -> Steam 0x005e5260  in candidate callees: yes
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x2c*,0x8c*} | Steam {0x4*,0xc*,0x2c*,0x8c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x006e51d0: 16-byte aligned: YES
  006e51d0  55             PUSH EBP
  006e51d1  8B EC          MOV EBP,ESP
  006e51d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006e51d0
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E5260  CGameEffect::SetLinked
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006e51d0 FUN_006e51d0 size=305 ret=12 callers=7 callees=5

int __thiscall FUN_006e51d0(int param_1,undefined4 param_2,undefined2 param_3,undefined4 param_4)

{
  uint uVar1;
  int iVar2;
  int local_3c;
  int local_38;
  undefined2 local_2c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095dd06;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x8c,uVar1);
  local_8 = 0;
  if (iVar2 == 0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if (local_38 == 0) {
    local_3c = 0;
  }
  else {
    iVar2 = FUN_00919723(0x8c,uVar1);
    local_8 = 1;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    if (local_3c == 0) {
      local_3c = 0;
    }
    else {
      *(undefined2 *)(local_38 + 8) = param_3;
      FUN_005e4e80(0,param_4);
      local_2c = (undefined2)DAT_00995fbc;
      *(undefined2 *)(local_3c + 8) = local_2c;
      FUN_005e5ea0(*(undefined4 *)(param_1 + 4));
      FUN_005e5260(param_2,local_38);
    }
  }
  ExceptionList = local_10;
  return local_3c;
}
```

### decoy: 0x006e5030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e5030: 16-byte aligned: YES
  006e5030  55             PUSH EBP
  006e5031  8B EC          MOV EBP,ESP
  006e5033  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006e5030
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x005E5260  CGameEffect::SetLinked
struct offsets touched (top 2):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
```

## Card B12a-1-18

GOG function: CSWSCreature::GetLockOrientationToObject  gog 0x0067be30  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00563280 (size 47, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetDialogInterruptable | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563220:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067ab60 CSWSCreature::Constructor -> 0x00561f30  gate_class=AUTO
- GOG-order neighbour after:  0x0067be50 CSWSCreature::GetDialogResref -> 0x00563220  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x104c} | Steam {0x54,0x1198}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00563280: 16-byte aligned: YES
  00563280  55             PUSH EBP
  00563281  8B EC          MOV EBP,ESP
  00563283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1198 [dec 4504]  x2  via EAX,ECX
  +0x54 [dec 84]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00563280 FUN_00563280 size=47 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00563280(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x1198) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 0x1198) + 0x54);
  }
  return uVar1;
}
```

### decoy: 0x00563220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563220: 16-byte aligned: YES
  00563220  55             PUSH EBP
  00563221  8B EC          MOV EBP,ESP
  00563223  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563220
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x1198 [dec 4504]  x2  via EAX,ECX
```

## Card B12a-1-19

GOG function: CSWSCreature::SaveFollowInfo  gog 0x0068dce0  callconv=(blank)  param_size_bytes=8  gog_size=72  gog_retn=8
Candidate Steam address: 0x005751b0 (size 72, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00575200:STR1/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0068d030 CSWSCreature::SaveCreature -> 0x00574500  gate_class=AUTO
- GOG-order neighbour after:  0x0068dd30 CSWSCreature::LoadFollowInfo -> 0x00575200  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00625aa0 CResGFF::AddStructToStruct -> Steam 0x00719940  in candidate callees: yes
  0x0067a8c0 CSWSCreaturePartyFollowInfo::Save -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'FollowInfo'
disp sets (* = in both, 1 common): GOG {0x500*} | Steam {0x500*}
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
prologue for 0x005751b0: 16-byte aligned: YES
  005751b0  55             PUSH EBP
  005751b1  8B EC          MOV EBP,ESP
  005751b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005751b0
string literals (1):
  0x0099547C  at 005751c7  'FollowInfo'
direct callees (2):
  0x00719940  CResGFF::AddStructToStruct
  0x00561C90  (unnamed)
struct offsets touched (top 1):
  +0x500 [dec 1280]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005751b0 FUN_005751b0 size=72 ret=8 callers=1 callees=2

void __thiscall FUN_005751b0(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined1 local_8 [4];
  
  if (*(int *)(param_1 + 0x500) != 0) {
    FUN_00719940(local_8,param_3,"FollowInfo",0);
    FUN_00561c90(param_2,local_8);
  }
  return;
}
```

### decoy: 0x00575200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00575200: 16-byte aligned: YES
  00575200  55             PUSH EBP
  00575201  8B EC          MOV EBP,ESP
  00575203  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00575200
string literals (1):
  0x0099547C  at 00575228  'FollowInfo'
direct callees (4):
  0x00718960  CResGFF::GetStructFromStruct
  0x00919723  operator new
  0x005752C0  CFollowInfo::CFollowInfo
  0x00561D80  (unnamed)
struct offsets touched (top 1):
  +0x500 [dec 1280]  x3  via ECX,EDX
```

## Card B12a-1-20

GOG function: CSWSCreature::MaxCleaveRange  gog 0x0068a070  callconv=(blank)  param_size_bytes=12  gog_size=47  gog_retn=12
Candidate Steam address: 0x00571540 (size 47, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: MaxAttackRange 0x571570)
- best decoy (neighbour/other candidate) scores: 0x00571570:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00689f20 CSWSCreature::ResetCombatAnimations -> 0x005713f0  gate_class=AUTO
- GOG-order neighbour after:  0x0068a0a0 CSWSCreature::MaxAttackRange -> 0x00571570  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068a0a0 CSWSCreature::MaxAttackRange -> Steam 0x00571570  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00571540: 16-byte aligned: YES
  00571540  55             PUSH EBP
  00571541  8B EC          MOV EBP,ESP
  00571543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00571540
string literals (0):
direct callees (1):
  0x00571570  CSWSCreature::MaxAttackRange
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00571540 FUN_00571540 size=47 ret=12 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00571540(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  float10 fVar1;
  
  fVar1 = (float10)FUN_00571570(param_1,param_2,param_3);
  return (float10)(float)(fVar1 + (float10)_DAT_00987490);
}
```

### decoy: 0x00571570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00571570: 16-byte aligned: YES
  00571570  55             PUSH EBP
  00571571  8B EC          MOV EBP,ESP
  00571573  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00571570
string literals (0):
direct callees (5):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D6E30  CSWItem::GetBaseItem
  0x006B7BB0  CSWSCreatureStats::GetSkillRank
  0x00585F10  CSWSCreature::GetUseRange
  0x005716A0  CSWSCreature::DesiredAttackRange
struct offsets touched (top 4):
  +0x1A [dec 26]  x1  via EDX
  +0x34 [dec 52]  x1  via EDX
  +0x1150 [dec 4432]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B12a-1-21

GOG function: CSWSCreature::IsInTotalDefense_2  gog 0x00657710  callconv=(blank)  param_size_bytes=12  gog_size=60  gog_retn=12
Candidate Steam address: 0x006db460 (size 60, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006db410:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006571b0 CSWSCreature::ResolveRangedAttack -> 0x0053e450  gate_class=REJECT
- GOG-order neighbour after:  0x00657750 CSWSCreature::ResolveRangedSpecialAttack -> 0x006db4a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040d990 CClientExoApp::GetClientRangedAnimation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x006db460: 16-byte aligned: YES
  006db460  55             PUSH EBP
  006db461  8B EC          MOV EBP,ESP
  006db463  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006db460
string literals (0):
direct callees (1):
  0x007404D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006db460 FUN_006db460 size=60 ret=12 callers=1 callees=1
THUNK -> 0x007404D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_006db460(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_007404d0(*(undefined4 *)(param_1 + 4),param_2,param_3,param_4);
  return;
}
```

### decoy: 0x006db410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006db410: 16-byte aligned: YES
  006db410  55             PUSH EBP
  006db411  8B EC          MOV EBP,ESP
  006db413  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006db410
string literals (0):
direct callees (1):
  0x00740500  CClientExoApp::GetClientMeleeAnimation
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B12a-1-22

GOG function: CSWSCreature::AddUseTalentOnObjectActions  gog 0x006963e0  callconv=(blank)  param_size_bytes=29  gog_size=476  gog_retn=32
Candidate Steam address: 0x0057d8b0 (size 476, retn 32)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::UseTalentOnObject | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: AddAttackActions 0x568540, GetItemDetailsForPower 0x6c06b0; callers: ExecuteCommandActionUseTalent 0x66bdc0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0057da90:STR0/CALLEE2/SIG0.682/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006962c0 CSWSCreature::AddDisappearActions -> 0x0057d790  gate_class=AUTO
- GOG-order neighbour after:  0x006965c0 CSWSCreature::AddUseTalentAtLocationActions -> 0x0057da90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0067df40 CSWSCreature::AddCastSpellActions -> Steam ?  in candidate callees: n/a
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callees: yes
  0x00693db0 CSWSCreature::UseSkill -> Steam ?  in candidate callees: n/a
  0x00693ff0 CSWSCreature::UseItem -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006fcb30 CSWSCreatureStats::GetItemDetailsForPower -> Steam 0x006c06b0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007813a0 Global::ExecuteCommandActionUseTalent -> Steam 0x0066bdc0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xec*,0x1198*} | Steam {0x4*,0x8*,0xec*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x4*,0xc*,0x20*,0x34*,0x94*,0x120*,0x5dc*,0x2719*,0x7f000000*} | Steam {0x2*,0x4*,0xc*,0x20*,0x34*,0x94*,0x120*,0x5dc*,0x2719*,0x7f000000*}

### abi
```
GOG param bytes: 29; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(29/4)*4 = 32 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: TRUE (GOG 29 bytes -> x86 slots 32; ASM RET 32)
```

### prologue
```
prologue for 0x0057d8b0: 16-byte aligned: YES
  0057d8b0  55             PUSH EBP
  0057d8b1  8B EC          MOV EBP,ESP
  0057d8b3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057d8b0
string literals (0):
direct callees (7):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005453C0  CSWSObject::GetArea
  0x0057B4C0  CSWSCreature::UseItem
  0x006C06B0  CSWSCreatureStats::GetItemDetailsForPower
  0x00565310  CSWSCreature::AddCastSpellActions
  0x00568540  CSWSCreature::AddAttackActions
  0x0057B280  CSWSCreature::UseSkill
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xEC [dec 236]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0057d8b0 FUN_0057d8b0 size=476 ret=32 callers=2 callees=7

undefined4 __thiscall
FUN_0057d8b0(int param_1,int param_2,uint param_3,undefined4 param_4,undefined1 param_5,int param_6,
            uint param_7,undefined1 param_8,undefined1 param_9)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined1 local_8 [4];
  
  iVar1 = FUN_0051c100(param_4);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (*(int *)(param_1 + 0xec) == 0) {
    uVar2 = 0;
  }
  else {
    iVar3 = FUN_005453c0();
    if (iVar3 == 0) {
      uVar2 = 0;
    }
    else {
      local_1c = *(undefined4 *)(iVar1 + 0x94);
      local_18 = *(undefined4 *)(iVar1 + 0x98);
      local_14 = *(undefined4 *)(iVar1 + 0x9c);
      iVar1 = FUN_005453c0();
      local_10 = *(undefined4 *)(iVar1 + 0x124);
      if (param_2 == 0) {
        if ((param_6 == 0x7f000000) || (param_7 == 0xffffffff)) {
          iVar1 = FUN_006c06b0(param_3,&local_c,local_8);
          if (iVar1 == 1) {
            FUN_0057b4c0(local_c,local_8[0],0,param_4,&local_1c,local_10,0);
          }
          else {
            FUN_00565310(param_3,param_5,0,param_9,0,local_1c,local_18,local_14,param_4,0,0,0,0,0,0,
                         0xffffffff,param_8,0);
          }
        }
        else {
          FUN_0057b4c0(param_6,param_7 & 0xff,0,param_4,&local_1c,local_10,0);
        }
      }
      else if (param_2 == 1) {
        FUN_00568540(param_4,param_3 & 0xffff,0,0,1,0,0,1,0x2719,0x5dc,1,0,0,4,0);
      }
      else if (param_2 == 2) {
        FUN_0057b280(param_3 & 0xff,0,param_4,&local_1c,local_10,0x7f000000,0);
      }
      uVar2 = 1;
    }
  }
  return uVar2;
}
```

### decoy: 0x0057da90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057da90: 16-byte aligned: YES
  0057da90  55             PUSH EBP
  0057da91  8B EC          MOV EBP,ESP
  0057da93  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057da90
string literals (0):
direct callees (5):
  0x005453C0  CSWSObject::GetArea
  0x00541080  CSWSObject::ClearAllActions
  0x0057B4C0  CSWSCreature::UseItem
  0x006C06B0  CSWSCreatureStats::GetItemDetailsForPower
  0x00565310  CSWSCreature::AddCastSpellActions
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xEC [dec 236]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B12a-1-23

GOG function: CSWSCreature::SetEffectSpellId  gog 0x0067bfa0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00563390 (size 100, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563370:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067bf80 CSWSCreature::GetEffectSpellId -> 0x00563350  gate_class=REVIEW
- GOG-order neighbour after:  0x0067bfc0 CSWSCreature::GetPortrait -> 0x00563390  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xff8} | Steam {0x4,0x8,0xc,0x1198}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x17a}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00563390: 16-byte aligned: YES
  00563390  55             PUSH EBP
  00563391  8B EC          MOV EBP,ESP
  00563393  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563390
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00563390 FUN_00563390 size=100 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_00563390(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x1198) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    iVar1 = *(int *)(param_1 + 0x1198);
    *param_2 = *(undefined4 *)(iVar1 + 0x17a);
    param_2[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0x17e);
    param_2[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x182);
    param_2[3 = byte +0xc] = *(undefined4 *)(iVar1 + 0x186);
  }
  return param_2;
}
```

### decoy: 0x00563370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563370: 16-byte aligned: YES
  00563370  55             PUSH EBP
  00563371  8B EC          MOV EBP,ESP
  00563373  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563370
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```

## Card B12a-1-24

GOG function: CSWSCreature::AddDropItemActions  gog 0x00680a70  callconv=(blank)  param_size_bytes=9  gog_size=181  gog_retn=12
Candidate Steam address: 0x00567e40 (size 181, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x00680b30:0.76>0.49
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ExecuteCommandPickUpItem 0x68da90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00568020:STR0/CALLEE0/SIG0.452/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00680300 CSWSCreature::RunUnequip -> 0x005676d0  gate_class=AUTO
- GOG-order neighbour after:  0x00680b30 CSWSCreature::AddHealActions -> 0x00567f00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam 0x0051c0e0  in candidate callees: yes
  0x006a64d0 CSWSObject::AddAction -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007a30e0 Global::ExecuteCommandPickUpItem -> Steam 0x0068da90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0xec*} | Steam {0x8*,0xec*}
imm sets (* = in both, 5 common): GOG {0x3*,0x7*,0xc*,0x14*,0xffff*} | Steam {0x3*,0x7*,0xc*,0x14*,0xffff*}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 9 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00567e40: 16-byte aligned: YES
  00567e40  55             PUSH EBP
  00567e41  8B EC          MOV EBP,ESP
  00567e43  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00567e40
string literals (0):
direct callees (3):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x005453C0  CSWSObject::GetArea
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00567e40 FUN_00567e40 size=181 ret=12 callers=2 callees=3

undefined4 __thiscall FUN_00567e40(int param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0051c0e0(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    iVar1 = FUN_005453c0();
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else if (*(int *)(param_1 + 0xec) == 0) {
      uVar2 = 0;
    }
    else {
      FUN_0053f7f0(7,0xffff,3,&param_2,3,&stack0x00000008,1,&stack0x0000000c,0,0,0,0,0,0,0,0,0,0,0,0
                   ,0,0,0,0,0,0,0,0);
      uVar2 = 1;
    }
  }
  return uVar2;
}
```

### decoy: 0x00568020 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00568020: 16-byte aligned: YES
  00568020  55             PUSH EBP
  00568021  8B EC          MOV EBP,ESP
  00568023  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00568020
string literals (0):
direct callees (3):
  0x006B7A00  CSWSCreatureStats::GetCanUseSkill
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0053F7F0  CSWSObject::AddAction
struct offsets touched (top 2):
  +0xEC [dec 236]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B12a-1-25

GOG function: CSWSCreature::GetEffectSpellId  gog 0x0067bf80  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00563460 (size 35, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetDead | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563400:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067bf50 CSWSCreature::GetLastName -> 0x00563320  gate_class=AUTO
- GOG-order neighbour after:  0x0067bfa0 CSWSCreature::SetEffectSpellId -> 0x00563370  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xff8} | Steam {0x11ac}
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
prologue for 0x00563460: 16-byte aligned: YES
  00563460  55             PUSH EBP
  00563461  8B EC          MOV EBP,ESP
  00563463  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563460
string literals (0):
direct callees (1):
  0x005477F0  CSWSObject::GetDead
struct offsets touched (top 1):
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563460 FUN_00563460 size=35 ret=0 callers=0 callees=1

undefined4 __fastcall FUN_00563460(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x11ac) == 0) {
    uVar1 = FUN_005477f0();
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00563400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563400: 16-byte aligned: YES
  00563400  55             PUSH EBP
  00563401  8B EC          MOV EBP,ESP
  00563403  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563400
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 4):
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
```

## Card B12a-1-26

GOG function: CSWSCreature::DoStealthDetection  gog 0x00690a00  callconv=(blank)  param_size_bytes=20  gog_size=197  gog_retn=20
Candidate Steam address: 0x00577ed0 (size 197, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00576dc0:STR0/CALLEE0/SIG0.023/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0068f8f0 CSWSCreature::DoPerceptionUpdateOnCreature -> 0x00576dc0  gate_class=AUTO
- GOG-order neighbour after:  0x00690ad0 CSWSCreature::DoListenDetection -> 0x00577fa0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00690ad0 CSWSCreature::DoListenDetection -> Steam ?  in candidate callees: n/a
  0x00691380 CSWSCreature::DoSpotDetection -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x10*,0x14*} | Steam {0x10*,0x14*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00577ed0: 16-byte aligned: YES
  00577ed0  55             PUSH EBP
  00577ed1  8B EC          MOV EBP,ESP
  00577ed3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00577ed0
string literals (0):
direct callees (3):
  0x005453C0  CSWSObject::GetArea
  0x00577FA0  CSWSCreature::DoListenDetection
  0x00578850  CSWSCreature::DoSpotDetection
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00577ed0 FUN_00577ed0 size=197 ret=20 callers=1 callees=3

undefined4 FUN_00577ed0(int param_1,int param_2,int *param_3,int *param_4,undefined4 param_5)

{
  int iVar1;
  int iVar2;
  undefined4 local_14;
  undefined4 local_c;
  
  local_c = 0;
  if (param_1 == 0) {
    *param_3 = 0;
    *param_4 = 0;
    local_14 = 0;
  }
  else {
    iVar1 = FUN_005453c0();
    iVar2 = FUN_005453c0();
    if (iVar1 == iVar2) {
      iVar1 = FUN_00577fa0(param_1,param_5);
      if (param_2 != 0) {
        local_c = FUN_00578850(param_1,param_5);
      }
      *param_4 = iVar1;
      *param_3 = local_c;
      if ((local_c == 0) && (iVar1 == 0)) {
        local_14 = 0;
      }
      else {
        local_14 = 1;
      }
    }
    else {
      *param_3 = 0;
      *param_4 = 0;
      local_14 = 0;
    }
  }
  return local_14;
}
```

### decoy: 0x00576dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00576dc0: 16-byte aligned: YES
  00576dc0  55             PUSH EBP
  00576dc1  8B EC          MOV EBP,ESP
  00576dc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00576dc0
string literals (0):
direct callees (34):
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051C350  CServerExoApp::GetActiveTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AF60  CWorldTimer::SubtractWorldTimes
  0x0091EB8B  (unnamed)
  0x005802C0  CSWSCreature::GetDetectModeState
  0x005453C0  CSWSObject::GetArea
  0x0058B140  CSWSCreature::GetSpotCheckDistance
  0x0058B210  CSWSCreature::GetListenCheckDistance
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0052BB60  CSWSArea::ClearLineOfSight
  0x0051BD90  CServerExoApp::GetModule
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00585420  CSWSCreature::GetIsBlindOrInDarkness
  0x00585480  CSWSCreature::GetInvisible
  0x005D1750  CSWSCreature::GetVisibleListElement
  0x005867E0  CSWSCreature::ClearHostileActionsVersus
  0x00577ED0  CSWSCreature::DoStealthDetection
struct offsets touched (top 10):
  +0x4 [dec 4]  x47  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x11C8 [dec 4552]  x6  via EAX,ECX,EDX
  +0x124 [dec 292]  x2  via EAX,EDX
  +0x3BC [dec 956]  x2  via EAX,ECX
  +0x3C0 [dec 960]  x2  via EAX,ECX
  +0x511 [dec 1297]  x2  via ECX
  +0xFF5 [dec 4085]  x2  via ECX
  +0x1026 [dec 4134]  x2  via EAX
  +0x1027 [dec 4135]  x2  via EAX
```

## Card B12a-1-27

GOG function: CSWSCreature::GetDetectMode  gog 0x00693490  callconv=(blank)  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x0057a960 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0057ac30:STR0/CALLEE1/SIG0.042/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00693090 CSWSCreature::SetCombatState -> 0x0057a560  gate_class=AUTO
- GOG-order neighbour after:  0x006934c0 CSWSCreature::RecomputeAmbientAnimationState -> 0x0057a990  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x51c*,0x11ac*} | Steam {0x51c*,0x11ac*}
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
prologue for 0x0057a960: 16-byte aligned: YES
  0057a960  55             PUSH EBP
  0057a961  8B EC          MOV EBP,ESP
  0057a963  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057a960
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x51C [dec 1308]  x1  via EAX
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057a960 FUN_0057a960 size=43 ret=0 callers=3 callees=0

void __fastcall FUN_0057a960(int param_1)

{
  *(uint *)(param_1 + 0x51c) = (uint)(*(int *)(param_1 + 0x11ac) == 0);
  return;
}
```

### decoy: 0x0057ac30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057ac30: 16-byte aligned: YES
  0057ac30  55             PUSH EBP
  0057ac31  8B EC          MOV EBP,ESP
  0057ac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0057ac30
string literals (1):
  0x00995574  at 0057ae40  'PrimaryRange'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005E13C0  CSWSObject::GetReputation
  0x0057A560  CSWSCreature::SetCombatState
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0058B140  CSWSCreature::GetSpotCheckDistance
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0057DC20  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x6  via EDX
  +0x8 [dec 8]  x2  via EAX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0xBC [dec 188]  x1  via EAX
  +0xFC [dec 252]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
  +0x260 [dec 608]  x1  via ECX
  +0x550 [dec 1360]  x1  via EAX
  +0x568 [dec 1384]  x1  via EDX
```

## Card B12a-1-28

GOG function: CSWSCreature::ResolveRangedAnimations  gog 0x0065c900  callconv=(blank)  param_size_bytes=10  gog_size=79  gog_retn=12
Candidate Steam address: 0x00544130 (size 80, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation:0x0065c710:0.69>0.52
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00544180:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065c710 CSWSCreature::ResolveMeleeAnimations -> 0x00543ae0  gate_class=REJECT
- GOG-order neighbour after:  0x0065c950 CSWSCreature::SignalMeleeDamage -> 0x006e0650  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00774f70 CSWSCombatRound::GetAttack -> Steam 0x005908c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x10,0x14,0x10dc} | Steam {0x4,0x8}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0x4,0xc*,0x154}

### abi
```
GOG param bytes: 10; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(10/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 10 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00544130: 16-byte aligned: YES
  00544130  55             PUSH EBP
  00544131  8B EC          MOV EBP,ESP
  00544133  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00544130
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00544130 FUN_00544130 size=80 ret=12 callers=1 callees=2

void __thiscall
FUN_00544130(undefined4 param_1,undefined2 param_2,undefined4 param_3,undefined1 param_4)

{
  undefined2 *puVar1;
  
  puVar1 = (undefined2 *)FUN_00919723(0xc,param_1);
  *puVar1 = param_2;
  *(undefined4 *)(puVar1 + 0x2 [dec 2] {undefined2*: byte +0x4}) = param_3;
  *(undefined1 *)(puVar1 + 0x4 [dec 4] {undefined2*: byte +0x8}) = param_4;
  FUN_0083ea60(puVar1);
  return;
}
```

### decoy: 0x00544180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00544180: 16-byte aligned: YES
  00544180  55             PUSH EBP
  00544181  8B EC          MOV EBP,ESP
  00544183  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00544180
string literals (0):
direct callees (2):
  0x009196FD  operator delete
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x154 [dec 340]  x3  via EAX,EDX
  +0x158 [dec 344]  x1  via EAX
```

## Card B12a-1-29

GOG function: CSWSCreature::SetAmbientAnimationState  gog 0x00692510  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x005799e0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005798f0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00692420 CSWSCreature::BroadcastMineDetectionData -> 0x005798f0  gate_class=AUTO
- GOG-order neighbour after:  0x00692540 CSWSCreature::BroadcastSpellCast -> 0x00579a10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x578*,0x57c*} | Steam {0x578*,0x57c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x5dc*} | Steam {0x4*,0x5dc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005799e0: 16-byte aligned: YES
  005799e0  55             PUSH EBP
  005799e1  8B EC          MOV EBP,ESP
  005799e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005799e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x578 [dec 1400]  x1  via EAX
  +0x57C [dec 1404]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005799e0 FUN_005799e0 size=44 ret=4 callers=1 callees=0

void __thiscall FUN_005799e0(int param_1,int param_2)

{
  *(int *)(param_1 + 0x578) = param_2;
  if (param_2 == 1) {
    *(undefined4 *)(param_1 + 0x57c) = 0x5dc;
  }
  return;
}
```

### decoy: 0x005798f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005798f0: 16-byte aligned: YES
  005798f0  55             PUSH EBP
  005798f1  8B EC          MOV EBP,ESP
  005798f3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005798f0
string literals (0):
direct callees (6):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x90 [dec 144]  x2  via EAX,ECX
```

## Card B12a-1-30

GOG function: CSWSCreature::BroadcastStealthDetectionData  gog 0x00692f53  callconv=(blank)  param_size_bytes=0  gog_size=309  gog_retn=4
Candidate Steam address: 0x0057a420 (size 312, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::BroadcastPoisoned | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0057a100:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00692f50 CSWSCreature::BroadcastPoisoned -> 0x0057a420  gate_class=REVIEW
- GOG-order neighbour after:  0x00693090 CSWSCreature::SetCombatState -> 0x0057a560  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam 0x0051c2e0  in candidate callees: yes
  0x0064c020 CServerExoApp::GetClientObjectByObjectId -> Steam 0x0051c600  in candidate callees: yes
  0x006db8d0 CSWSCreature::GetFaction -> Steam ?  in candidate callees: n/a
  0x0075ca90 CSWSPlayer::GetGameObject -> Steam ?  in candidate callees: n/a
  0x007df3e0 CSWSMessage::SendServerToPlayerCCMessage -> Steam ?  in candidate callees: n/a
  0x007e4990 CSWSFaction::GetMemberList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x90*} | Steam {0x4*,0x8*,0x90*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x1b*,0x4c*,0x94*} | Steam {0x4*,0x8*,0xc*,0x1b*,0x4c*,0x94*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x0057a420: 16-byte aligned: YES
  0057a420  55             PUSH EBP
  0057a421  8B EC          MOV EBP,ESP
  0057a423  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057a420
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057a420 FUN_0057a420 size=312 ret=4 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057a420(undefined4 param_1)

{
  float10 fVar1;
  int extraout_var;
  int iVar2;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = FUN_005d0cb0();
  if (local_10 != 0) {
    local_c = 0;
    iVar2 = extraout_var;
    local_8 = FUN_006d3470(&local_c);
    for (local_14 = 0; local_14 < local_8; local_14 = local_14 + 1) {
      local_18 = FUN_0051c600(*(undefined4 *)(local_c + local_14 * 4));
      if (((local_18 != 0) && (local_1c = FUN_00632a20(), local_1c != 0)) &&
         (*(int *)(iVar2 + 0x90) == *(int *)(local_1c + 0x90))) {
        FUN_004452b0(local_28,local_1c + 0x94,iVar2 + 0x94);
        fVar1 = (float10)FUN_0044b8a0(local_28,local_28);
        if (fVar1 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
          FUN_0051c2e0(*(undefined4 *)(local_18 + 4),0x1b,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                       iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
          FUN_00648cc0();
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0057a100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057a100: 16-byte aligned: YES
  0057a100  55             PUSH EBP
  0057a101  8B EC          MOV EBP,ESP
  0057a103  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057a100
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B12a-1-31

GOG function: CSWSCreature::DoCombatStep  gog 0x0065fd40  callconv=(blank)  param_size_bytes=1  gog_size=3044  gog_retn=12
Candidate Steam address: 0x00547100 (size 1274, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::BroadcastDialog | G7 mac: pass(0.43) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.22 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00547600:STR0/CALLEE0/SIG0.275/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0065f950 CSWSCreature::ResolveDamageShields -> 0x006e3650  gate_class=AUTO
- GOG-order neighbour after:  0x00660930 CSWSCreature::SavingThrowRoll -> 0x00547d60  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x004143c0 unnamed -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: no
  0x00686e10 CSWSCreature::RangeWeaponEquipped -> Steam 0x0056e1e0  in candidate callees: no
  0x0068a1d0 CSWSCreature::DesiredAttackRange -> Steam 0x005716a0  in candidate callees: no
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  ... 12 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc,0x10,0x14,0x30*,0x7c,0x1b0,0x380,0x10dc,0x1114} | Steam {0x4*,0x8*,0x30*,0x94,0x1c0}
imm sets (* = in both, 5 common): GOG {0x2,0x3,0x4*,0x5*,0x8,0xc*,0x10,0x41*,0x94*,0x230,0x2711,0x2728,0x2729,0x272a,...(+5)} | Steam {0x4*,0x5*,0x7,0xc*,0x34,0x41*,0x94*,0xcc,0x194}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x00547100: 16-byte aligned: YES
  00547100  55             PUSH EBP
  00547101  8B EC          MOV EBP,ESP
  00547103  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00547100
string literals (0):
direct callees (15):
  0x005453C0  CSWSObject::GetArea
  0x0051C080  CServerExoApp::GetObjectArray
  0x00522AF0  CSWSArea::GetFirstObjectIndiceByX
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005D1750  CSWSCreature::GetVisibleListElement
  0x0058B210  CSWSCreature::GetListenCheckDistance
  0x00546F10  CSWSObject::TestListenExpression
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x00546DD0  (unnamed)
  0x005E1BD0  (unnamed)
  0x005E1CB0  CSWCMessagePayload::SetString
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 5):
  +0x4 [dec 4]  x8  via ECX,EDX
  +0x30 [dec 48]  x6  via EAX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x94 [dec 148]  x2  via EAX,EDX
  +0x1C0 [dec 448]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00547100 FUN_00547100 size=1274 ret=12 callers=1 callees=15

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00547100(int *param_1,undefined4 param_2,float param_3,int param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  char cVar4;
  char cVar5;
  int iVar6;
  int iVar7;
  float10 fVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined2 *local_dc;
  int local_60;
  float local_54;
  float local_50;
  float local_4c;
  int local_48;
  int local_44;
  float local_40;
  float local_3c;
  float local_38;
  int local_34;
  float local_30;
  int local_2c;
  int local_28;
  int *local_24;
  int local_20;
  undefined4 local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094faab;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = -1;
  local_24 = (int *)0x0;
  local_34 = FUN_005453c0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_1c = FUN_0051c080();
  local_18 = (int *)(local_34 + 0x194);
  local_30 = 0.0;
  local_2c = 0;
  local_28 = 0;
  if (local_34 != 0) {
    FUN_00522af0(&local_20,(float)param_1[0x25 = byte +0x94] - param_3);
    while ((cVar4 = DAT_0099447f, -1 < local_20 && (local_20 < local_18[1 = byte +0x4]))) {
      local_14 = *(int *)(*local_18 + local_20 * 4);
      local_20 = local_20 + 1;
      cVar5 = FUN_0053dfb0(local_14,&local_24);
      if (cVar5 != cVar4) {
        ExceptionList = local_10;
        return;
      }
      if ((local_24 != (int *)0x0) && (local_14 != param_1[1 = byte +0x4])) {
        local_30 = (float)local_24[0x25 = byte +0x94];
        local_2c = local_24[0x26 = byte +0x98];
        local_28 = local_24[0x27 = byte +0x9c];
        if ((float)param_1[0x25 = byte +0x94] + param_3 < local_30) {
          ExceptionList = local_10;
          return;
        }
        FUN_004452b0(&local_40,param_1 + 0x25,&local_30);
        if ((local_38 * local_38 + local_3c * local_3c + local_40 * local_40 <= param_3 * param_3)
           && (local_24[0x70 = byte +0x1c0] != 0)) {
          local_44 = 1;
          iVar6 = (**(code **)(*local_24 + 0x30))();
          if ((iVar6 == 0) || (iVar6 = (**(code **)(*param_1 + 0x30))(), iVar6 == 0)) {
            iVar6 = (**(code **)(*local_24 + 0x30))();
            if ((iVar6 != 0) && (iVar6 = (**(code **)(*param_1 + 0x30))(), iVar6 == 0)) {
              FUN_004452b0(&local_54,param_1 + 0x25,&local_30);
              fVar1 = local_54 * local_54;
/* ... 56 more lines truncated by --max-lines */
```

### decoy: 0x00547600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00547600: 16-byte aligned: YES
  00547600  55             PUSH EBP
  00547601  8B EC          MOV EBP,ESP
  00547603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00547600
string literals (0):
direct callees (2):
  0x0053E060  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
```

## Card B12a-1-32

GOG function: CSWSCreature::BroadcastPoisoned  gog 0x00692f50  callconv=(blank)  param_size_bytes=4  gog_size=3  gog_retn=None
Candidate Steam address: 0x0057ac30 (size 863, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::BroadcastCombatStateToParty | G7 mac: pass(0.09) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0057a990:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00692f00 CSWSCreature::BroadcastDeflectBreakdown -> 0x0057a3d0  gate_class=AUTO
- GOG-order neighbour after:  0x00692f53 CSWSCreature::BroadcastStealthDetectionData -> 0x0057a420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007425f0 CSWSEffectListHandler::OnApplyPoison -> Steam 0x0059fd20  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10,0x30,0xbc,0xfc,0x130,0x260,0x550,0x568,0x11ac}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x4,0xa,0xb,0xc,0x84,0x94,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0057ac30: 16-byte aligned: YES
  0057ac30  55             PUSH EBP
  0057ac31  8B EC          MOV EBP,ESP
  0057ac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0057ac30
string literals (1):
  0x00995574  at 0057ae40  'PrimaryRange'
direct callees (12):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005E13C0  CSWSObject::GetReputation
  0x0057A560  CSWSCreature::SetCombatState
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0058B140  CSWSCreature::GetSpotCheckDistance
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0057DC20  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x6  via EDX
  +0x8 [dec 8]  x2  via EAX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0xBC [dec 188]  x1  via EAX
  +0xFC [dec 252]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
  +0x260 [dec 608]  x1  via ECX
  +0x550 [dec 1360]  x1  via EAX
  +0x568 [dec 1384]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0057ac30 FUN_0057ac30 size=863 ret=4 callers=7 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0057ac30(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  float10 fVar3;
  undefined4 uVar4;
  int *piVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_44 [8];
  float local_3c;
  float local_38;
  float local_34;
  float local_30;
  int *local_2c;
  int local_28;
  int *local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009510f8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = FUN_0051c100(param_2);
  local_20 = 0x7f000000;
  local_18 = 0x7f000000;
  if (local_1c != 0) {
    local_20 = *(int *)(local_1c + 0x550);
    local_18 = *(int *)(local_1c + 0x568);
  }
  iVar2 = FUN_005e13c0(param_2,0);
  if ((iVar2 < 0xb) || (param_2 == *(int *)(param_1 + 4))) {
    if ((local_20 == *(int *)(param_1 + 4)) || (local_18 == *(int *)(param_1 + 4))) {
      FUN_0057a560(1,1,0);
    }
    else {
      FUN_0057a560(1,2,0);
    }
  }
  if (*(int *)(param_1 + 0xfc) == 0) {
    piVar5 = &local_28;
    FUN_005d0cb0(piVar5,uVar1);
    FUN_006d3470(piVar5);
    local_14 = 0;
    while (iVar2 = FUN_005d0cb0(), local_14 < *(int *)(iVar2 + 4)) {
      local_2c = (int *)FUN_0051c0a0(*(undefined4 *)(local_28 + local_14 * 4));
      if ((local_2c != (int *)0x0) && (iVar2 = (**(code **)(*local_2c + 0x10))(), iVar2 != 0)) {
        local_24 = (int *)(**(code **)(*local_2c + 0x10))();
        FUN_004452b0(&local_38,param_1 + 0x94,local_24 + 0x25);
        local_3c = 0.0;
        if (*(int *)(param_1 + 0x11ac) == 0) {
          iVar2 = (**(code **)(*local_24 + 0x30))();
          if (iVar2 == 0) {
            FUN_00733570("PrimaryRange");
            local_8 = 0;
            FUN_0071b670(0xb,local_44,&local_3c);
            local_8 = 0xffffffff;
            FUN_00733780();
          }
          else {
            (**(code **)(*local_24 + 0x30))();
            fVar3 = (float10)FUN_0058b140();
            local_3c = (float)fVar3;
          }
        }
        else {
          local_3c = _DAT_009919f0;
        }
        if ((local_30 * local_30 + local_34 * local_34 + local_38 * local_38 <= local_3c * local_3c)
/* ... 32 more lines truncated by --max-lines */
```

### decoy: 0x0057a990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057a990: 16-byte aligned: YES
  0057a990  55             PUSH EBP
  0057a991  8B EC          MOV EBP,ESP
  0057a993  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057a990
string literals (0):
direct callees (1):
  0x0058B780  (unnamed)
struct offsets touched (top 6):
  +0xFF5 [dec 4085]  x14  via EAX
  +0x504 [dec 1284]  x3  via EAX
  +0x7C [dec 124]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12a-1-33

GOG function: CSWSCreature::GetLockOrientationToObject  gog 0x0067be30  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00563200 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563170:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067ab60 CSWSCreature::Constructor -> 0x00561f30  gate_class=AUTO
- GOG-order neighbour after:  0x0067be50 CSWSCreature::GetDialogResref -> 0x00563220  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x104c*} | Steam {0x104c*}
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
prologue for 0x00563200: 16-byte aligned: YES
  00563200  55             PUSH EBP
  00563201  8B EC          MOV EBP,ESP
  00563203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563200
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x104C [dec 4172]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563200 FUN_00563200 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00563200(int param_1)

{
  return *(undefined4 *)(param_1 + 0x104c);
}
```

### decoy: 0x00563170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563170: 16-byte aligned: YES
  00563170  55             PUSH EBP
  00563171  8B EC          MOV EBP,ESP
  00563173  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563170
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
```

## Card B12a-1-34

GOG function: CSWSCreature::GetWeaponDischargeTime  gog 0x00658710  callconv=(blank)  param_size_bytes=4  gog_size=194  gog_retn=4
Candidate Steam address: 0x006dc460 (size 194, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006dc530:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00658540 CSWSCreature::ResolvePostRangedDamage -> 0x006dc290  gate_class=AUTO
- GOG-order neighbour after:  0x006587e0 CSWSCreature::IsLightsaberEquipped -> 0x006dc530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0065aca0 Global::rand_wincompatible -> Steam ?  in candidate callees: n/a
  0x0065ad40 unnamed -> Steam ?  in candidate callees: n/a
  0x006612b0 unnamed -> Steam ?  in candidate callees: n/a
  0x00774f70 CSWSCombatRound::GetAttack -> Steam 0x005908c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x10*,0x12*,0x5c*,0xabc*,0x10dc*} | Steam {0x4*,0x8*,0x10*,0x12*,0x5c*,0xabc*,0x10dc*}
imm sets (* = in both, 6 common): GOG {0x4*,0x14*,0x68*,0x94*,0x271c*,0x271e*} | Steam {0x4*,0x14*,0x68*,0x94*,0x271c*,0x271e*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006dc460: 16-byte aligned: YES
  006dc460  55             PUSH EBP
  006dc461  8B EC          MOV EBP,ESP
  006dc463  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006dc460
string literals (0):
direct callees (4):
  0x005908C0  CSWSCombatRound::GetAttack
  0x006DE9A0  (unnamed)
  0x006DEA40  (unnamed)
  0x006E4FB0  (unnamed)
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x12 [dec 18]  x2  via EAX,ECX
  +0x5C [dec 92]  x2  via EAX,ECX
  +0x10 [dec 16]  x1  via ECX
  +0xABC [dec 2748]  x1  via EDX
  +0x10DC [dec 4316]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006dc460 FUN_006dc460 size=194 ret=4 callers=1 callees=4

void __thiscall FUN_006dc460(int param_1,int param_2)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  
  if (param_2 != 0) {
    iVar2 = FUN_005908c0(*(undefined1 *)(*(int *)(param_1 + 0x10dc) + 0xabc));
    iVar3 = FUN_006de9a0(*(undefined1 *)(iVar2 + 0x5c));
    if (iVar3 == 0) {
      iVar3 = FUN_006dea40(*(undefined1 *)(iVar2 + 0x5c));
      if (iVar3 != 0) {
        *(undefined2 *)(iVar2 + 0x12) = 0x271c;
      }
    }
    else {
      *(undefined2 *)(iVar2 + 0x12) = 0x271e;
    }
    *(undefined4 *)(iVar2 + 0x68) = *(undefined4 *)(param_2 + 0x94);
    *(undefined4 *)(iVar2 + 0x6c) = *(undefined4 *)(param_2 + 0x98);
    *(undefined4 *)(iVar2 + 0x70) = *(undefined4 *)(param_2 + 0x9c);
    uVar1 = FUN_006e4fb0(param_2 + 0x94,0);
    *(undefined2 *)(iVar2 + 0x10) = uVar1;
  }
  return;
}
```

### decoy: 0x006dc530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006dc530: 16-byte aligned: YES
  006dc530  55             PUSH EBP
  006dc531  8B EC          MOV EBP,ESP
  006dc533  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006dc530
string literals (0):
direct callees (3):
  0x005908C0  CSWSCombatRound::GetAttack
  0x006DC5C0  CSWSCreature::ResolveProjectileMissLocation
  0x006E4FB0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x10 [dec 16]  x1  via EDX
  +0xABC [dec 2748]  x1  via EDX
```

## Card B12a-1-35

GOG function: CSWSCreature::GetWeaponDischargeShots  gog 0x0065a960  callconv=(blank)  param_size_bytes=4  gog_size=187  gog_retn=4
Candidate Steam address: 0x006de660 (size 187, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR 'Shots'@0099a968
- best decoy (neighbour/other candidate) scores: 0x006de720:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065a650 CSWSCreature::GetWeaponDischargeRow -> 0x006de350  gate_class=AUTO
- GOG-order neighbour after:  0x0065aa20 CSWSCreature::GetWeaponHitShots -> 0x006de720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x0065a650 CSWSCreature::GetWeaponDischargeRow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'Shots'
disp sets (* = in both, 2 common): GOG {0xe0*,0x130*} | Steam {0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x24*} | Steam {0x4*,0x24*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006de660: 16-byte aligned: YES
  006de660  55             PUSH EBP
  006de661  8B EC          MOV EBP,ESP
  006de663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006de660
string literals (1):
  0x0099A968  at 006de6a6  'Shots'
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006de660 FUN_006de660 size=187 ret=4 callers=1 callees=4

undefined4 FUN_006de660(undefined4 param_1)

{
  uint uVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095d960;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_006de350(local_1c,param_1);
  local_8 = 0;
  local_14 = 0;
  uVar2 = extraout_var;
  FUN_00733570("Shots");
  local_8._0_1_ = 1;
  uVar3 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xe0);
  FUN_0071bad0(local_1c,local_24,&local_14);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780(uVar1,uVar2,uVar3);
  uVar3 = local_14;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x006de720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006de720: 16-byte aligned: YES
  006de720  55             PUSH EBP
  006de721  8B EC          MOV EBP,ESP
  006de723  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006de720
string literals (0):
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B12a-1-36

GOG function: CSWSCreature::ActivityManager  gog 0x0067c950  callconv=(blank)  param_size_bytes=4  gog_size=71  gog_retn=4
Candidate Steam address: 0x00563d20 (size 71, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00563d70:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067c8c0 CSWSCreature::ActionManager -> 0x00563c90  gate_class=AUTO
- GOG-order neighbour after:  0x0067c9a0 CSWSCreature::GetActivity -> 0x00563d70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0067c9d0 CSWSCreature::SetActivity -> Steam ?  in candidate callees: n/a
  0x0067cb70 CSWSCreature::ClearActivities -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 27): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0xf*,0xff*,0x3f00*} | Steam {0x4*,0xf*,0xff*,0x3f00*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00563d20: 16-byte aligned: YES
  00563d20  55             PUSH EBP
  00563d21  8B EC          MOV EBP,ESP
  00563d23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563d20
string literals (0):
direct callees (2):
  0x00563DA0  CSWSCreature::SetActivity
  0x00563F40  CSWSCreature::ClearActivities
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00563d20 FUN_00563d20 size=71 ret=4 callers=27 callees=2

void FUN_00563d20(int param_1)

{
  if (param_1 == 4) {
    FUN_00563f40(0xf);
  }
  else {
    FUN_00563da0(0x3f00,0,0);
  }
  if (param_1 == 1) {
    FUN_00563f40(0xff);
  }
  return;
}
```

### decoy: 0x00563d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563d70: 16-byte aligned: YES
  00563d70  55             PUSH EBP
  00563d71  8B EC          MOV EBP,ESP
  00563d73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563d70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1120 [dec 4384]  x1  via EAX
```

## Card B12a-1-37

GOG function: CSWSCreature::SetLastInstantEquip  gog 0x00696db0  callconv=(blank)  param_size_bytes=0  gog_size=202  gog_retn=0
Candidate Steam address: 0x0057e530 (size 901, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetArmorClass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0057e280:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00696c70 CSWSCreature::CancelRest -> 0x0057e140  gate_class=AUTO
- GOG-order neighbour after:  0x00696e80 CSWSCreature::Rest_2 -> 0x0057e350  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x007382d0 CSWSEffectListHandler::OnApplyDamage -> Steam 0x005959e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {0x8d,0xab,0xfa,0xfb,0xfc,0xfd,0xfe,0xff,0x100,0x101,0x102,0x103,0x104,0x105,...(+7)}
imm sets (* = in both, 0 common): GOG {0x12,0x13,0x24,0x148} | Steam {0x4,0xa,0x28,0x74,0x238}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0057e530: 16-byte aligned: YES
  0057e530  55             PUSH EBP
  0057e531  8B EC          MOV EBP,ESP
  0057e533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0057e530
string literals (1):
  0x009955A8  at 0057e675  'ArmorMult'
direct callees (10):
  0x006AD2F0  CSWSCreatureStats::GetClassLevel
  0x006F2ED0  Global::GetArmorBonus
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091C860  _ftol2_sse
  0x006BAD60  (unnamed)
  0x006B5DB0  CSWSCreatureStats::GetDEXMod
  0x006B6600  CSWSCreatureStats::GetTotalACFeatMod
  0x006DAA00  CSWSCreature::IsDebilitated
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x10  via EAX,ECX,EDX
  +0x8D [dec 141]  x2  via EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0x105 [dec 261]  x2  via ECX,EDX
  +0x251 [dec 593]  x2  via EAX
  +0xFA [dec 250]  x1  via EDX
  +0xFB [dec 251]  x1  via EAX
  +0xFC [dec 252]  x1  via EAX
  +0xFD [dec 253]  x1  via ECX
  +0xFE [dec 254]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0057e530 FUN_0057e530 size=901 ret=0 callers=9 callees=10

short __fastcall FUN_0057e530(int param_1)

{
  int iVar1;
  int iVar2;
  undefined1 uVar3;
  char cVar4;
  short sVar5;
  uint uVar6;
  int iVar7;
  char local_84;
  byte local_46;
  undefined1 local_38 [8];
  undefined4 local_30;
  undefined1 local_29;
  undefined1 local_28 [4];
  uint local_24;
  short local_20;
  int local_1c;
  short local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009511f8;
  local_10 = ExceptionList;
  uVar6 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = 10;
  local_20 = 0;
  local_14 = 0;
  while( true ) {
    if ((int)(uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d) <= (int)local_14) break;
    if ((local_14 & 0xff) < (uint)*(byte *)(*(int *)(param_1 + 0x1198) + 0x8d)) {
      local_46 = *(byte *)(*(int *)(param_1 + 0x1198) + 0xab + (local_14 & 0xff) * 0x28);
    }
    else {
      local_46 = 0xff;
    }
    local_1c = (uint)local_46 * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124);
    uVar3 = FUN_006ad2f0(local_14 & 0xff,0);
    cVar4 = FUN_006f2ed0(uVar3);
    local_20 = local_20 + cVar4;
    local_14 = local_14 + 1;
  }
  if ((*(int *)(param_1 + 0x11ac) == 0) &&
     (local_24 = (uint)*(byte *)(param_1 + 0x250), local_24 != 0)) {
    local_30 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x154);
    FUN_00733570("ArmorMult");
    local_8 = 0;
    FUN_0071b670(local_24,local_38,local_28);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (*(char *)(param_1 + 0x251) == '\0') {
      local_29 = 1;
    }
    else {
      local_29 = *(undefined1 *)(param_1 + 0x251);
    }
    sVar5 = FUN_0091c860();
    local_20 = local_20 + sVar5;
  }
  iVar7 = *(int *)(param_1 + 0x1198);
  iVar1 = *(int *)(param_1 + 0x1198);
  iVar2 = *(int *)(param_1 + 0x1198);
  if ((int)*(char *)(iVar2 + 0x104) - (int)*(char *)(iVar2 + 0x105) < 0xb) {
    local_84 = *(char *)(iVar2 + 0x104) - *(char *)(iVar2 + 0x105);
  }
  else {
    local_84 = '\n';
  }
  local_18 = local_18 + local_20 +
             (short)(char)(*(char *)(iVar7 + 0xfa) +
                          (*(char *)(iVar7 + 0xfc) - *(char *)(iVar7 + 0xfd))) +
             (short)(char)(*(char *)(iVar1 + 0xfb) +
                          (*(char *)(iVar1 + 0x100) - *(char *)(iVar1 + 0x101))) +
             (short)(char)(*(char *)(*(int *)(param_1 + 0x1198) + 0xfe) -
/* ... 15 more lines truncated by --max-lines */
```

### decoy: 0x0057e280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057e280: 16-byte aligned: YES
  0057e280  55             PUSH EBP
  0057e281  8B EC          MOV EBP,ESP
  0057e283  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057e280
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card B12a-1-38

GOG function: CSWSCreature::Destructor  gog 0x0067c0c0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00563390 (size 100, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563370:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067c090 CSWSCreature::GetDead -> 0x00563460  gate_class=REVIEW
- GOG-order neighbour after:  0x0067c0f0 CSWSCreature::Destructor_2 -> 0x005634c0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0067c0f0 CSWSCreature::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1198}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x17a}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00563390: 16-byte aligned: YES
  00563390  55             PUSH EBP
  00563391  8B EC          MOV EBP,ESP
  00563393  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563390
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00563390 FUN_00563390 size=100 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_00563390(int param_1,undefined4 *param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x1198) == 0) {
    FUN_00710840(&DAT_00986925);
  }
  else {
    iVar1 = *(int *)(param_1 + 0x1198);
    *param_2 = *(undefined4 *)(iVar1 + 0x17a);
    param_2[1 = byte +0x4] = *(undefined4 *)(iVar1 + 0x17e);
    param_2[2 = byte +0x8] = *(undefined4 *)(iVar1 + 0x182);
    param_2[3 = byte +0xc] = *(undefined4 *)(iVar1 + 0x186);
  }
  return param_2;
}
```

### decoy: 0x00563370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563370: 16-byte aligned: YES
  00563370  55             PUSH EBP
  00563371  8B EC          MOV EBP,ESP
  00563373  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563370
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```

## Card B12a-1-39

GOG function: CSWSCreature::GetBlind  gog 0x0068f7d0  callconv=(blank)  param_size_bytes=0  gog_size=277  gog_retn=8
Candidate Steam address: 0x00576ca0 (size 277, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00690a00:0.61>0.04
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00576750:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0068f280 CSWSCreature::PostProcess -> 0x00576750  gate_class=AUTO
- GOG-order neighbour after:  0x0068f8f0 CSWSCreature::DoPerceptionUpdateOnCreature -> 0x00576dc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x006207c0 CExoLinkedListInternal::Remove_2 -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x006a54f0 CSWSObjectActionNode::Destructor -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x6c*,0x74*} | Steam {0x6c*,0x74*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x28*,0x100*} | Steam {0x4*,0x8*,0x28*,0x100*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x00576ca0: 16-byte aligned: YES
  00576ca0  55             PUSH EBP
  00576ca1  8B EC          MOV EBP,ESP
  00576ca3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00576ca0
string literals (0):
direct callees (7):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x0058C370  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0053E740  Action-list free params
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x6C [dec 108]  x1  via ECX
  +0x74 [dec 116]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00576ca0 FUN_00576ca0 size=277 ret=8 callers=3 callees=7

undefined4 __thiscall FUN_00576ca0(int *param_1,undefined4 param_2,short param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_1c;
  undefined4 local_c;
  int local_8;
  
  if (param_1 != (int *)0xffffff00) {
    local_8 = FUN_00521060();
    local_c = FUN_007a1720();
  }
  local_1c = 0;
  while (uVar1 = local_c, local_8 != 0) {
    if (*(short *)(local_8 + 0x6c) == param_3) {
      local_8 = FUN_0058c370(&local_c);
      iVar2 = FUN_00561430(uVar1);
      iVar3 = FUN_00521060();
      iVar2 = (**(code **)(*param_1 + 0x74))(iVar2,iVar2 == iVar3);
      if (iVar2 == 1) {
        iVar2 = FUN_0064dde0(uVar1);
        if ((iVar2 != 0) && (iVar2 != 0)) {
          FUN_0053e740();
          FUN_009196fd(iVar2);
        }
        local_1c = 1;
      }
    }
    else {
      local_8 = FUN_0058c370(&local_c);
    }
  }
  return local_1c;
}
```

### decoy: 0x00576750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00576750: 16-byte aligned: YES
  00576750  55             PUSH EBP
  00576751  8B EC          MOV EBP,ESP
  00576753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00576750
string literals (0):
direct callees (21):
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051E120  CServerAIMaster::SetAILevel
  0x006D0620  CSWInventory::GetItemInSlot
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0051BD90  CServerExoApp::GetModule
  0x0055E970  CSWSModule::AddObjectToLookupTable
  0x0051CC80  CServerExoApp::GetFactionManager
  0x006D3330  CSWSFaction::AddMember(objId, bSuppress)
  0x00664490  (unnamed)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x0057FCE0  CSWSCreature::UpdatePersonalSpace
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x7C [dec 124]  x4  via EAX,ECX,EDX
  +0x2C8 [dec 712]  x4  via EAX,ECX,EDX
  +0x10E8 [dec 4328]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x6C [dec 108]  x2  via EAX,ECX
  +0x78 [dec 120]  x2  via EAX,EDX
```

## Card B12a-1-40

GOG function: CSWSCreature::GetDead  gog 0x0067c090  callconv=(blank)  param_size_bytes=0  gog_size=35  gog_retn=0
Candidate Steam address: 0x00563460 (size 35, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetDead 0x5477f0)
- best decoy (neighbour/other candidate) scores: 0x00563400:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067c030 CSWSCreature::SetPortrait -> 0x00563400  gate_class=AUTO
- GOG-order neighbour after:  0x0067c0c0 CSWSCreature::Destructor -> 0x00563390  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006ae4d0 CSWSObject::GetDead -> Steam 0x005477f0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11ac*} | Steam {0x11ac*}
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
prologue for 0x00563460: 16-byte aligned: YES
  00563460  55             PUSH EBP
  00563461  8B EC          MOV EBP,ESP
  00563463  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563460
string literals (0):
direct callees (1):
  0x005477F0  CSWSObject::GetDead
struct offsets touched (top 1):
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563460 FUN_00563460 size=35 ret=0 callers=0 callees=1

undefined4 __fastcall FUN_00563460(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x11ac) == 0) {
    uVar1 = FUN_005477f0();
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x00563400 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563400: 16-byte aligned: YES
  00563400  55             PUSH EBP
  00563401  8B EC          MOV EBP,ESP
  00563403  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563400
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 4):
  +0x1198 [dec 4504]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
```

## Card B12a-1-41

GOG function: CSWSCreature::BroadcastPoisoned  gog 0x00692f50  callconv=(blank)  param_size_bytes=4  gog_size=3  gog_retn=None
Candidate Steam address: 0x0057a420 (size 312, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: OnApplyPoison 0x59fd20)
- best decoy (neighbour/other candidate) scores: 0x0057a3d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00692f00 CSWSCreature::BroadcastDeflectBreakdown -> 0x0057a3d0  gate_class=AUTO
- GOG-order neighbour after:  0x00692f53 CSWSCreature::BroadcastStealthDetectionData -> 0x0057a420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007425f0 CSWSEffectListHandler::OnApplyPoison -> Steam 0x0059fd20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x90}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1b,0x4c,0x94}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0057a420: 16-byte aligned: YES
  0057a420  55             PUSH EBP
  0057a421  8B EC          MOV EBP,ESP
  0057a423  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057a420
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057a420 FUN_0057a420 size=312 ret=4 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057a420(undefined4 param_1)

{
  float10 fVar1;
  int extraout_var;
  int iVar2;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = FUN_005d0cb0();
  if (local_10 != 0) {
    local_c = 0;
    iVar2 = extraout_var;
    local_8 = FUN_006d3470(&local_c);
    for (local_14 = 0; local_14 < local_8; local_14 = local_14 + 1) {
      local_18 = FUN_0051c600(*(undefined4 *)(local_c + local_14 * 4));
      if (((local_18 != 0) && (local_1c = FUN_00632a20(), local_1c != 0)) &&
         (*(int *)(iVar2 + 0x90) == *(int *)(local_1c + 0x90))) {
        FUN_004452b0(local_28,local_1c + 0x94,iVar2 + 0x94);
        fVar1 = (float10)FUN_0044b8a0(local_28,local_28);
        if (fVar1 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
          FUN_0051c2e0(*(undefined4 *)(local_18 + 4),0x1b,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                       iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
          FUN_00648cc0();
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0057a3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057a3d0: 16-byte aligned: YES
  0057a3d0  55             PUSH EBP
  0057a3d1  8B EC          MOV EBP,ESP
  0057a3d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057a3d0
string literals (0):
direct callees (2):
  0x0057A100  CSWSCreature::BroadcastBreakdownMessage
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B12a-1-42

GOG function: CSWSCreature::ApplyOnHitSleep  gog 0x0065dc30  callconv=(blank)  param_size_bytes=4  gog_size=808  gog_retn=4
Candidate Steam address: 0x006e1930 (size 808, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.27) | G8 mac_rotation: rotation:0x0065e710:0.65>0.42
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006e1780:STR0/CALLEE0/SIG0.609/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065da80 CSWSCreature::ResolvePostMeleeDamage -> 0x006e1780  gate_class=AUTO
- GOG-order neighbour after:  0x0065df60 CSWSCreature::ResolveDefensiveEffects -> 0x006e1c60  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00699780 CSWSCreature::GetIsWeaponEffective -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam 0x005e4920  in candidate callees: yes
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam 0x005e5ea0  in candidate callees: yes
  0x00771940 CSWSCombatAttackData::GetTotalDamage -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0xa*,0x30*,0x3a*,0x48*,0x4c*,0x9c*,0xabc*,0xacc*,0x10dc*} | Steam {0x4*,0x8*,0xa*,0x30*,0x3a*,0x48*,0x4c*,0x9c*,0xabc*,0xacc*,0x10dc*}
imm sets (* = in both, 12 common): GOG {0x2*,0x3*,0x4*,0xc*,0x34*,0x75*,0x80*,0x88*,0x8c*,0x94*,0xa0*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0xc*,0x34*,0x75*,0x80*,0x88*,0x8c*,0x94*,0xa0*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006e1930: 16-byte aligned: YES
  006e1930  55             PUSH EBP
  006e1931  8B EC          MOV EBP,ESP
  006e1933  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006e1930
string literals (0):
direct callees (11):
  0x005908C0  CSWSCombatRound::GetAttack
  0x0058D290  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00580C50  CSWSCreature::GetIsWeaponEffective
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x3  via EDX
  +0x10DC [dec 4316]  x3  via EAX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EDX
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x4C [dec 76]  x2  via ECX,EDX
  +0x9C [dec 156]  x2  via EDX
  +0xACC [dec 2764]  x2  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0x3A [dec 58]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006e1930 FUN_006e1930 size=808 ret=4 callers=1 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_006e1930(int param_1,int *param_2)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  undefined1 *local_90;
  int local_8c;
  undefined2 local_64;
  float local_38;
  float local_34;
  float local_30;
  uint local_2c;
  uint local_28;
  int local_24;
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095db26;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_2 != (int *)0x0) {
    local_24 = *(int *)(param_1 + 0x10dc);
    local_28 = (uint)*(byte *)(local_24 + 0xabc);
    local_14 = FUN_005908c0(local_28);
    local_18 = FUN_0058d290(1);
    local_2c = (uint)(*(char *)(local_14 + 0x3a) == '\x02');
    iVar3 = (**(code **)(*param_2 + 0x30))(uVar2);
    if (iVar3 == 0) {
      if ((param_2 != (int *)0x0) && (sVar1 = (**(code **)(*param_2 + 0x9c))(0), sVar1 <= local_18))
      {
        *(undefined4 *)(local_14 + 0x48) = 1;
      }
    }
    else {
      local_20 = (**(code **)(*param_2 + 0x30))();
      piVar4 = (int *)(**(code **)(*param_2 + 0x30))();
      sVar1 = (**(code **)(*piVar4 + 0x9c))(0);
      if ((sVar1 <= local_18) || (*(int *)(local_14 + 0x4c) == 1)) {
        *(undefined4 *)(local_14 + 0x48) = 1;
      }
      FUN_004452b0(&local_38,param_1 + 0x94,param_2 + 0x25);
      if ((*(int *)(local_14 + 0x4c) == 1) &&
         (local_30 * local_30 + local_34 * local_34 + local_38 * local_38 <=
          _DAT_00987704 * _DAT_00987704)) {
        iVar3 = FUN_00919723(0x8c);
        local_8 = 0;
        if (iVar3 == 0) {
          local_8c = 0;
        }
        else {
          local_8c = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        local_1c = local_8c;
        local_64 = (undefined2)DAT_00995f68;
        *(undefined2 *)(local_8c + 8) = local_64;
        *(ushort *)(local_8c + 0xa [dec 10]) = *(ushort *)(local_8c + 0xa [dec 10]) & 0xfff8 | (ushort)PTR_009967ac;
        FUN_005e5ea0(*(undefined4 *)(param_1 + 4));
        FUN_005e4e80(0,0);
        FUN_005e4e80(1,1);
        FUN_0083ea60(local_1c);
      }
    }
    if (((local_18 < 1) && (*(int *)(*(int *)(param_1 + 0x10dc) + 0xacc) == 0)) &&
       (iVar3 = FUN_00580c50(param_2[1 = byte +0x4],local_2c), iVar3 == 0)) {
      iVar3 = FUN_00919723(0x34);
      local_8 = 1;
      if (iVar3 == 0) {
/* ... 15 more lines truncated by --max-lines */
```

### decoy: 0x006e1780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e1780: 16-byte aligned: YES
  006e1780  55             PUSH EBP
  006e1781  8B EC          MOV EBP,ESP
  006e1783  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006e1780
string literals (0):
direct callees (7):
  0x005908C0  CSWSCombatRound::GetAttack
  0x0058D290  (unnamed)
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 10):
  +0xA [dec 10]  x2  via EDX
  +0x30 [dec 48]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x3A [dec 58]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x4C [dec 76]  x1  via ECX
  +0x9C [dec 156]  x1  via EAX
  +0xABC [dec 2748]  x1  via EDX
```

## Card B12a-1-43

GOG function: CSWSCreature::RemoveMindTrickEffects  gog 0x0068ddf0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x00575340 (size 161, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.38) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005752c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0068dd30 CSWSCreature::LoadFollowInfo -> 0x00575200  gate_class=AUTO
- GOG-order neighbour after:  0x0068df20 CSWSCreature::LoadJournal -> 0x005753f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068de70 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x28,0x2c,0x30,0x34,0x38}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1c*,0x20} | Steam {0x4*,0x10*,0x14,0x1c*,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00575340: 16-byte aligned: YES
  00575340  55             PUSH EBP
  00575341  8B EC          MOV EBP,ESP
  00575343  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00575340
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00575340 FUN_00575340 size=161 ret=0 callers=1 callees=0

void __fastcall FUN_00575340(undefined4 *param_1)

{
  *param_1 = 0x7f000000;
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[10 = byte +0x28] = 0;
  param_1[0xb = byte +0x2c] = 0;
  param_1[0xc = byte +0x30] = DAT_009944b8;
  param_1[0xd = byte +0x34] = 0;
  param_1[0xe = byte +0x38] = 1;
  return;
}
```

### decoy: 0x005752c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005752c0: 16-byte aligned: YES
  005752c0  55             PUSH EBP
  005752c1  8B EC          MOV EBP,ESP
  005752c3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005752c0
string literals (0):
direct callees (1):
  0x00575340  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

## Card B12a-1-44

GOG function: CSWSCreature::BroadcastPoisoned  gog 0x00692f50  callconv=(blank)  param_size_bytes=4  gog_size=3  gog_retn=None
Candidate Steam address: 0x00579fc0 (size 312, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::BroadcastAttackSummaryToParty | G7 mac: pass(0.50) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00579e70:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00692f00 CSWSCreature::BroadcastDeflectBreakdown -> 0x0057a3d0  gate_class=AUTO
- GOG-order neighbour after:  0x00692f53 CSWSCreature::BroadcastStealthDetectionData -> 0x0057a420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007425f0 CSWSEffectListHandler::OnApplyPoison -> Steam 0x0059fd20  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x90}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x12,0x4c,0x94}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00579fc0: 16-byte aligned: YES
  00579fc0  55             PUSH EBP
  00579fc1  8B EC          MOV EBP,ESP
  00579fc3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00579fc0
string literals (0):
direct callees (8):
  0x005D0CB0  CSWSCreature::GetFaction
  0x006D3470  (unnamed)
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x00648CC0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x90 [dec 144]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00579fc0 FUN_00579fc0 size=312 ret=4 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00579fc0(undefined4 param_1)

{
  float10 fVar1;
  int extraout_var;
  int iVar2;
  undefined1 local_28 [12];
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = FUN_005d0cb0();
  if (local_10 != 0) {
    local_c = 0;
    iVar2 = extraout_var;
    local_8 = FUN_006d3470(&local_c);
    for (local_14 = 0; local_14 < local_8; local_14 = local_14 + 1) {
      local_18 = FUN_0051c600(*(undefined4 *)(local_c + local_14 * 4));
      if (((local_18 != 0) && (local_1c = FUN_00632a20(), local_1c != 0)) &&
         (*(int *)(iVar2 + 0x90) == *(int *)(local_1c + 0x90))) {
        FUN_004452b0(local_28,local_1c + 0x94,iVar2 + 0x94);
        fVar1 = (float10)FUN_0044b8a0(local_28,local_28);
        if (fVar1 <= (float10)(_DAT_009919f0 * _DAT_009919f0)) {
          FUN_0051c2e0(*(undefined4 *)(local_18 + 4),0x12,param_1,0,_DAT_009919f0 * _DAT_009919f0,
                       iVar2,*(undefined4 *)(DAT_00a1b4a4 + 8));
          FUN_00648cc0();
        }
      }
    }
  }
  return;
}
```

### decoy: 0x00579e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00579e70: 16-byte aligned: YES
  00579e70  55             PUSH EBP
  00579e71  8B EC          MOV EBP,ESP
  00579e73  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00579e70
string literals (0):
direct callees (7):
  0x005453C0  CSWSObject::GetArea
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0051C2E0  CServerExoApp::GetSWSMessage
  0x0064B5A0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x90 [dec 144]  x2  via EAX,ECX
```

## Card B12a-1-45

GOG function: CSWSCreature::GetWeaponDischargeShots  gog 0x0065a960  callconv=(blank)  param_size_bytes=4  gog_size=187  gog_retn=4
Candidate Steam address: 0x006ddb00 (size 1956, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::SignalRangedDamage | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006de2b0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065a650 CSWSCreature::GetWeaponDischargeRow -> 0x006de350  gate_class=AUTO
- GOG-order neighbour after:  0x0065aa20 CSWSCreature::GetWeaponHitShots -> 0x006de720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x0065a650 CSWSCreature::GetWeaponDischargeRow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'Shots'
disp sets (* = in both, 0 common): GOG {0xe0,0x130} | Steam {0x4,0x8,0xa,0x10,0x18,0x1c,0x38,0x5c,0xa8,0xac,0xabc,0x10dc}
imm sets (* = in both, 1 common): GOG {0x4*,0x24} | Steam {0x4*,0x8,0xa,0xe,0xf,0x10,0x13,0x14,0x15,0x16,0x68,0x88,0x8c,0x94,...(+3)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006ddb00: 16-byte aligned: YES
  006ddb00  55             PUSH EBP
  006ddb01  8B EC          MOV EBP,ESP
  006ddb03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006ddb00
string literals (0):
direct callees (21):
  0x005908C0  CSWSCombatRound::GetAttack
  0x0058DE40  (unnamed)
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006DEA40  (unnamed)
  0x006DC5C0  CSWSCreature::ResolveProjectileMissLocation
  0x006E4FB0  (unnamed)
  0x006DCA70  CSWSCreature::ResolveProjectileMissImpact
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4DD0  CGameEffect::SetNumIntegers
  0x005E4E80  CGameEffect::SetInteger
  0x0057F870  CSWSCreature::GetDamageFlags
  0x00581FC0  (unnamed)
  0x005E4E60  CGameEffect::GetInteger
  0x006E6E70  (unnamed)
  0x006DC140  CSWSCreature::ResolveSafeProjectile
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 10):
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,EDX
  +0x5C [dec 92]  x3  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x38 [dec 56]  x1  via ECX
  +0xA8 [dec 168]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0xABC [dec 2748]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006ddb00 FUN_006ddb00 size=1956 ret=4 callers=1 callees=21

void __thiscall FUN_006ddb00(int *param_1,undefined4 param_2)

{
  short sVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  int local_1c4;
  undefined2 local_180;
  undefined4 local_144;
  undefined4 local_140;
  undefined4 local_13c;
  int local_138;
  int *local_134;
  int local_130;
  int *local_12c;
  int local_128;
  undefined4 local_124;
  int local_120;
  uint local_11c;
  uint local_118;
  int local_114;
  undefined1 local_110 [16];
  int local_100;
  int local_fc;
  undefined4 local_e4;
  uint local_28;
  undefined4 local_24;
  int local_20;
  int *local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095d879;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_120 = param_1[0x437 = byte +0x10dc];
  local_11c = (uint)*(byte *)(local_120 + 0xabc);
  local_28 = uVar2;
  local_14 = FUN_005908c0(local_11c);
  local_118 = (uint)*(ushort *)(local_14 + 0x10);
  FUN_0058de40(local_110);
  local_8 = 0;
  local_20 = local_fc;
  local_124 = local_e4;
  local_114 = local_100;
  local_24 = FUN_0051c280(uVar2);
  if (local_114 != 0) {
    FUN_0051e170(0,local_11c,param_1[1 = byte +0x4],param_2,DAT_00992430,local_14);
  }
  local_1c = (int *)(local_14 + 0xa0);
  for (local_18 = 0; local_18 < local_1c[1 = byte +0x4]; local_18 = local_18 + 1) {
    FUN_0051e170(0,local_118,param_1[1 = byte +0x4],param_2,DAT_0099244c,
                 *(undefined4 *)(*local_1c + local_18 * 4));
  }
  local_1c[1 = byte +0x4] = 0;
  if (local_20 == 0) {
    FUN_006dca70(local_14 + 0x68,local_118);
  }
  else {
    local_128 = 1;
    local_138 = 0;
    local_130 = FUN_0051c100(param_2);
    iVar3 = FUN_006dea40(*(undefined1 *)(local_14 + 0x5c));
    if ((iVar3 != 0) && (local_130 != 0)) {
      if ((*(char *)(local_14 + 0x5c) == '\b') || (*(char *)(local_14 + 0x5c) == '\n')) {
        local_128 = 0;
        FUN_006dc5c0(&local_144,param_1);
        uVar4 = FUN_006e4fb0(&local_144,0);
        FUN_006dca70(&local_144,uVar4);
        *(undefined4 *)(local_14 + 0x68) = local_144;
        *(undefined4 *)(local_14 + 0x6c) = local_140;
        *(undefined4 *)(local_14 + 0x70) = local_13c;
/* ... 70 more lines truncated by --max-lines */
```

### decoy: 0x006de2b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006de2b0: 16-byte aligned: YES
  006de2b0  55             PUSH EBP
  006de2b1  8B EC          MOV EBP,ESP
  006de2b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006de2b0
string literals (0):
direct callees (3):
  0x00733700  CExoString::Constructor_5
  0x0071BF10  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xEC [dec 236]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
```

## Card B12a-1-46

GOG function: CSWSCreature::CancelAllCombatModes  gog 0x00656dd0  callconv=(blank)  param_size_bytes=4  gog_size=93  gog_retn=4
Candidate Steam address: 0x006dab20 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::EndAttackAction | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: ClearAllSpecialAttacks 0x5915f0; callers: AIActionPhysicalAttacks 0x6d7390); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006dab80:STR0/CALLEE1/SIG0.143/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00656d70 CSWSCreature::IsInTotalDefense -> 0x006daac0  gate_class=AUTO
- GOG-order neighbour after:  0x00656e30 CSWSCreature::HasTargetBeenDetected -> 0x006dab80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00695ea0 CSWSCreature::SetLockOrientationToObject -> Steam ?  in candidate callees: n/a
  0x006c28a0 CSWSCreature::SetCombatMode -> Steam ?  in candidate callees: n/a
  0x00775ca0 CSWSCombatRound::ClearAllSpecialAttacks -> Steam 0x005915f0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x7c*,0x10dc*} | Steam {0x7c*,0x10dc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x2710*,0x7f000000*} | Steam {0x4*,0x8*,0x2710*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006dab20: 16-byte aligned: YES
  006dab20  55             PUSH EBP
  006dab21  8B EC          MOV EBP,ESP
  006dab23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006dab20
string literals (0):
direct callees (3):
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x005B7AE0  CSWSCreature::SetCombatMode
  0x005915F0  CSWSCombatRound::ClearAllSpecialAttacks
struct offsets touched (top 2):
  +0x7C [dec 124]  x1  via EDX
  +0x10DC [dec 4316]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006dab20 FUN_006dab20 size=93 ret=4 callers=3 callees=3

void FUN_006dab20(void)

{
  undefined4 uStack00000004;
  undefined4 uVar1;
  int *extraout_var;
  int *piVar2;
  
  uStack00000004 = 1;
  FUN_0057d370(0x7f000000,0);
  uVar1 = 10000;
  piVar2 = extraout_var;
  (**(code **)(*extraout_var + 0x7c))(10000);
  FUN_005b7ae0(0,1);
  FUN_005915f0(uVar1,piVar2,piVar2[0x437 = byte +0x10dc]);
  return;
}
```

### decoy: 0x006dab80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006dab80: 16-byte aligned: YES
  006dab80  55             PUSH EBP
  006dab81  8B EC          MOV EBP,ESP
  006dab83  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006dab80
string literals (0):
direct callees (2):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005D1750  CSWSCreature::GetVisibleListElement
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B12a-1-47

GOG function: CSWSCreature::GetWeaponDischargeShots  gog 0x0065a960  callconv=(blank)  param_size_bytes=4  gog_size=187  gog_retn=4
Candidate Steam address: 0x006de720 (size 187, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetWeaponHitShots | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006de660:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0065a650 CSWSCreature::GetWeaponDischargeRow -> 0x006de350  gate_class=AUTO
- GOG-order neighbour after:  0x0065aa20 CSWSCreature::GetWeaponHitShots -> 0x006de720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x0065a650 CSWSCreature::GetWeaponDischargeRow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'Shots'
disp sets (* = in both, 2 common): GOG {0xe0*,0x130*} | Steam {0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x24*} | Steam {0x4*,0x24*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006de720: 16-byte aligned: YES
  006de720  55             PUSH EBP
  006de721  8B EC          MOV EBP,ESP
  006de723  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006de720
string literals (0):
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006de720 FUN_006de720 size=187 ret=4 callers=1 callees=4

undefined4 FUN_006de720(undefined4 param_1)

{
  uint uVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095d960;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_006de350(local_1c,param_1);
  local_8 = 0;
  local_14 = 0;
  uVar2 = extraout_var;
  FUN_00733570(&DAT_0099a94c);
  local_8._0_1_ = 1;
  uVar3 = *(undefined4 *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0xe0);
  FUN_0071bad0(local_1c,local_24,&local_14);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780(uVar1,uVar2,uVar3);
  uVar3 = local_14;
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x006de660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006de660: 16-byte aligned: YES
  006de660  55             PUSH EBP
  006de661  8B EC          MOV EBP,ESP
  006de663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006de660
string literals (1):
  0x0099A968  at 006de6a6  'Shots'
direct callees (4):
  0x006DE350  CSWSCreature::GetWeaponDischargeRow
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0xE0 [dec 224]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B12a-1-48

GOG function: CSWSCreature::DoCombatStep  gog 0x0065fd40  callconv=(blank)  param_size_bytes=1  gog_size=3044  gog_retn=12
Candidate Steam address: 0x00547730 (size 192, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::RemoveEffectByType | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005476c0:STR0/CALLEE0/SIG0.053/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0065f950 CSWSCreature::ResolveDamageShields -> 0x006e3650  gate_class=AUTO
- GOG-order neighbour after:  0x00660930 CSWSCreature::SavingThrowRoll -> 0x00547d60  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x004143c0 unnamed -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: no
  0x00686e10 CSWSCreature::RangeWeaponEquipped -> Steam 0x0056e1e0  in candidate callees: no
  0x0068a1d0 CSWSCreature::DesiredAttackRange -> Steam 0x005716a0  in candidate callees: no
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  ... 12 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8*,0xc,0x10,0x14,0x30,0x7c,0x1b0,0x380,0x10dc,0x1114} | Steam {0x8*,0x1c,0x148,0x14c}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x8,0xc*,0x10,0x41,0x94,0x230,0x2711,0x2728,0x2729,0x272a,...(+5)} | Steam {0xc*,0x1c}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x00547730: 16-byte aligned: YES
  00547730  55             PUSH EBP
  00547731  8B EC          MOV EBP,ESP
  00547733  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00547730
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544640  CSWSObject::RemoveEffect
struct offsets touched (top 4):
  +0x148 [dec 328]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x14C [dec 332]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00547730 FUN_00547730 size=192 ret=12 callers=1 callees=2

void __thiscall FUN_00547730(int param_1,int param_2,uint param_3,int param_4)

{
  int iVar1;
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x14c); local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 0x1c) == param_2) {
      if (*(ushort *)(*(int *)(*(int *)(param_1 + 0x148) + local_8 * 4) + 8) == param_3) {
        iVar1 = FUN_005e4e60(0);
        if (iVar1 == param_4) {
          FUN_00544640(*(undefined4 *)(*(int *)(param_1 + 0x148) + local_8 * 4));
        }
      }
    }
  }
  return;
}
```

### decoy: 0x005476c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005476c0: 16-byte aligned: YES
  005476c0  55             PUSH EBP
  005476c1  8B EC          MOV EBP,ESP
  005476c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005476c0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x1C [dec 28]  x1  via EDX
  +0x148 [dec 328]  x1  via EAX
  +0x14C [dec 332]  x1  via ECX
```

## Card B12a-1-49

GOG function: CSWSCreature::GetEffectSpellId  gog 0x0067bf80  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x00563350 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00563370:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0067bf50 CSWSCreature::GetLastName -> 0x00563320  gate_class=AUTO
- GOG-order neighbour after:  0x0067bfa0 CSWSCreature::SetEffectSpellId -> 0x00563370  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xff8*} | Steam {0xff8*}
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
prologue for 0x00563350: 16-byte aligned: YES
  00563350  55             PUSH EBP
  00563351  8B EC          MOV EBP,ESP
  00563353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00563350
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00563350 FUN_00563350 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00563350(int param_1)

{
  return *(undefined4 *)(param_1 + 0xff8);
}
```

### decoy: 0x00563370 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00563370: 16-byte aligned: YES
  00563370  55             PUSH EBP
  00563371  8B EC          MOV EBP,ESP
  00563373  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00563370
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF8 [dec 4088]  x1  via EAX
```
