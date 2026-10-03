# Review cards B0 chunk 9

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-9-01

GOG function: CSWSEffectListHandler::OnApplyArcaneSpellFailure  gog 0x0073f2a0  callconv=(blank)  param_size_bytes=8  gog_size=153  gog_retn=12
Candidate Steam address: 0x0059c9b0 (size 153, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ca50:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073f180 CSWSEffectListHandler::OnRemoveEnemyAttackBonus -> 0x0059c890  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f340 CSWSEffectListHandler::OnRemoveArcaneSpellFailure -> 0x0059ca50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x0084bb80 Global::return_void_0x4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x30*,0x128*,0x384*,0x1198*} | Steam {0x30*,0x128*,0x384*,0x1198*}
imm sets (* = in both, 2 common): GOG {0xc*,0x14*} | Steam {0xc*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059c9b0: 16-byte aligned: YES
  0059c9b0  55             PUSH EBP
  0059c9b1  8B EC          MOV EBP,ESP
  0059c9b3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c9b0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x0040E770  (unnamed)
struct offsets touched (top 4):
  +0x30 [dec 48]  x4  via EDX
  +0x128 [dec 296]  x1  via EDX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c9b0 FUN_0059c9b0 size=153 ret=12 callers=0 callees=2

undefined4 FUN_0059c9b0(int *param_1,int param_2)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_1 + 0x30))();
    iVar2 = *(int *)(iVar2 + 0x1198);
    if (param_2 != 0) {
      iVar3 = (**(code **)(*param_1 + 0x30))();
      if (iVar3 != 0) {
        iVar3 = (**(code **)(*param_1 + 0x30))();
        *(undefined4 *)(iVar3 + 0x384 [dec 900]) = 1;
      }
      bVar1 = *(byte *)(iVar2 + 0x128);
      iVar2 = FUN_005e4e60(0);
      FUN_0040e770((uint)bVar1 + iVar2);
    }
  }
  return 0;
}
```

### decoy: 0x0059ca50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ca50: 16-byte aligned: YES
  0059ca50  55             PUSH EBP
  0059ca51  8B EC          MOV EBP,ESP
  0059ca53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ca50
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x0040E770  (unnamed)
struct offsets touched (top 5):
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-9-02

GOG function: CSWSCreatureStats::GetWeaponSpecialization  gog 0x006e5390  callconv=(blank)  param_size_bytes=0  gog_size=202  gog_retn=0
Candidate Steam address: 0x006a8c20 (size 625, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreatureStats::GetRangedDamageBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006a8ea0:STR0/CALLEE0/SIG0.268/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e5320 CSWSCreatureStats::GetWeaponFinesse -> 0x006a8ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x006e5460 CSWSCreatureStats::GetUseMonkAttackTables -> 0x006a8fe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068f220 CSWSCreature::GetUseMonkAbilities -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x24*,0x8d,0x53c} | Steam {0x6,0x8,0xc,0x1a,0x24*,0x34,0xee,0x1150}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x8*,0xc,0x10*,0x20} | Steam {0x2*,0x4*,0x6,0x7,0x8*,0x10*,0x1e,0x48,0x99,0x9a,0x9b,0xf0,0xf1,0xf2,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006a8c20: 16-byte aligned: YES
  006a8c20  55             PUSH EBP
  006a8c21  8B EC          MOV EBP,ESP
  006a8c23  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006a8c20
string literals (0):
direct callees (5):
  0x006D0620  (unnamed)
  0x006D5200  (unnamed)
  0x006042E0  CSWSItem::GetPropertyByType
  0x006B8EE0  CSWSCreatureStats::UnReadySpell
  0x006B83F0  CSWSCreatureStats::HasFeat
struct offsets touched (top 8):
  +0xEE [dec 238]  x5  via EAX,EDX
  +0x6 [dec 6]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x1A [dec 26]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
  +0x1150 [dec 4432]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006a8c20 FUN_006a8c20 size=625 ret=0 callers=1 callees=5

int __fastcall FUN_006a8c20(int param_1)

{
  int iVar1;
  uint local_4c;
  int local_18;
  uint local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  local_10 = FUN_006d0620(0x10);
  if ((local_10 == 0) ||
     (iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc)), *(char *)(iVar1 + 0x1a) != '\0')) {
    if (local_10 != 0) {
      local_14 = 0;
      iVar1 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc));
      if (*(char *)(iVar1 + 8) == '\a') {
        local_14 = (uint)*(char *)(param_1 + 0xee);
      }
      else {
        if (*(char *)(param_1 + 0xee) < '\0') {
          local_14 = (uint)*(char *)(param_1 + 0xee);
        }
        iVar1 = FUN_006042e0(&local_18,0x1e,0);
        if (iVar1 != 0) {
          if ((int)*(char *)(param_1 + 0xee) < (int)(uint)*(ushort *)(local_18 + 6)) {
            local_4c = (uint)*(char *)(param_1 + 0xee);
          }
          else {
            local_4c = (uint)*(ushort *)(local_18 + 6);
          }
          local_14 = local_4c;
        }
      }
      local_8 = local_8 + local_14;
    }
    local_c = 0;
    iVar1 = FUN_006b8ee0(local_10);
    if (iVar1 != 0) {
      local_c = 2;
    }
    local_8 = local_8 + local_c;
    iVar1 = FUN_006b83f0(0x9b);
    if (iVar1 == 0) {
      iVar1 = FUN_006b83f0(0x9a);
      if (iVar1 == 0) {
        iVar1 = FUN_006b83f0(0x99);
        if (iVar1 != 0) {
          local_8 = local_8 + 2;
        }
      }
      else {
        local_8 = local_8 + 4;
      }
    }
    else {
      local_8 = local_8 + 6;
    }
    iVar1 = FUN_006b83f0(0xf4);
    if (iVar1 == 0) {
      iVar1 = FUN_006b83f0(0xf3);
      if (iVar1 == 0) {
        iVar1 = FUN_006b83f0(0xf2);
        if (iVar1 == 0) {
          iVar1 = FUN_006b83f0(0xf1);
          if (iVar1 == 0) {
            iVar1 = FUN_006b83f0(0xf0);
            if (iVar1 != 0) {
              local_8 = local_8 + 1;
            }
          }
          else {
            local_8 = local_8 + 2;
          }
        }
        else {
          local_8 = local_8 + 4;
/* ... 15 more lines truncated by --max-lines */
```

### decoy: 0x006a8ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006a8ea0: 16-byte aligned: YES
  006a8ea0  55             PUSH EBP
  006a8ea1  8B EC          MOV EBP,ESP
  006a8ea3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006a8ea0
string literals (0):
direct callees (2):
  0x005766F0  CSWSCreature::GetUseMonkAbilities
  0x006D0620  (unnamed)
struct offsets touched (top 2):
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x1150 [dec 4432]  x1  via ECX
```

## Card B0-9-03

GOG function: CSWSEffectListHandler::OnApplyMovementSpeedDecrease  gog 0x00741470  callconv=(blank)  param_size_bytes=8  gog_size=226  gog_retn=12
Candidate Steam address: 0x0059eb80 (size 226, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ea10:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741300 CSWSEffectListHandler::OnRemoveMovementSpeedIncrease -> 0x0059ea10  gate_class=REVIEW
- GOG-order neighbour after:  0x00741560 CSWSEffectListHandler::OnApplyVisualEffect -> 0x0059ec70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0068f140 CSWSCreature::GetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x0068f1d0 CSWSCreature::SetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x18*,0x30*,0xfc*,0x1198*} | Steam {0x8*,0x18*,0x30*,0xfc*,0x1198*}
imm sets (* = in both, 4 common): GOG {0xc*,0x18*,0x28*,0x64*} | Steam {0xc*,0x18*,0x28*,0x64*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059eb80: 16-byte aligned: YES
  0059eb80  55             PUSH EBP
  0059eb81  8B EC          MOV EBP,ESP
  0059eb83  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059eb80
string literals (0):
direct callees (5):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005E4E60  CGameEffect::GetInteger
  0x00576610  CSWSCreature::GetMovementRateFactor
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 5):
  +0x30 [dec 48]  x2  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059eb80 FUN_0059eb80 size=226 ret=12 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059eb80(int *param_1,int param_2)

{
  float fVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  float10 fVar5;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_1 + 0x30))();
    uVar3 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar4 = FUN_006bc570(0x18,uVar3);
    if (iVar4 != 0) {
      return 1;
    }
    iVar4 = FUN_005e4e60(0);
    if ((99 < iVar4) || (*(int *)(iVar2 + 0xfc) != 0)) {
      return 1;
    }
    iVar2 = FUN_005e4e60(0);
    fVar1 = (float)_DAT_0098ba38;
    fVar5 = (float10)FUN_00576610(0);
    FUN_005766a0((float)(fVar5 * (float10)(1.0 - (float)iVar2 / fVar1)));
  }
  return 0;
}
```

### decoy: 0x0059ea10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

## Card B0-9-04

GOG function: CGuiInGame::GetCanHeadFollowInDialog  gog 0x004dfff0  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf710 (size 112, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CGuiInGame::GetCanOrientInDialog
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.30 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf780:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0060 CGuiInGame::TutorialReasonWillShowWindow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0xf28,0x7f000000*} | Steam {0x4*,0xc*,0xf24,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf710: 16-byte aligned: YES
  007cf710  55             PUSH EBP
  007cf711  8B EC          MOV EBP,ESP
  007cf713  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf710
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007cf710 FUN_007cf710 size=112 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_007cf710(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  int local_c;
  int *local_8;
  
  if (param_2 == 0x7f000000) {
    uVar1 = 0;
  }
  else {
    local_c = FUN_007a1720(param_1);
    local_8 = (int *)FUN_00521060();
    while (local_c != 0) {
      if (*local_8 == param_2) {
        return 0;
      }
      local_8 = (int *)FUN_0058c370(&local_c);
    }
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007cf780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf780: 16-byte aligned: YES
  007cf780  55             PUSH EBP
  007cf781  8B EC          MOV EBP,ESP
  007cf783  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf780
string literals (0):
direct callees (8):
  0x0073F4D0  ClientApp::GetClientObject
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x0054AA10  CExoLinkedList::AddHead
  0x00773BF0  CSWCCreature::LookAt
  0x0064DDE0  (unnamed)
struct offsets touched (top 3):
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

## Card B0-9-05

GOG function: CSWSEffectListHandler::OnRemoveForceResistanceDecrease  gog 0x007423b0  callconv=(blank)  param_size_bytes=8  gog_size=449  gog_retn=8
Candidate Steam address: 0x0059fae0 (size 449, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059f7d0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007422a0 CSWSEffectListHandler::OnApplyForceResistanceDecrease -> 0x0059f9d0  gate_class=AUTO
- GOG-order neighbour after:  0x00742580 CSWSEffectListHandler::OnApplyRegenerate -> 0x0059fcb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006f8880 CSWSCreatureStats::SetForceResistancePenalty -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x384*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x384*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x21*,0x22*,0x4c*,0x148*} | Steam {0x8*,0x21*,0x22*,0x4c*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059fae0: 16-byte aligned: YES
  0059fae0  55             PUSH EBP
  0059fae1  8B EC          MOV EBP,ESP
  0059fae3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059fae0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC400  CSWSCreatureStats::SetForceResistancePenalty
struct offsets touched (top 5):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059fae0 FUN_0059fae0 size=449 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059fae0(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  int *piVar2;
  int local_18;
  uint local_8;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    (**(code **)(*param_2 + 0x30))();
    piVar2 = param_2 + 0x52;
    local_8 = 0;
    for (local_18 = 0; local_18 < param_2[0x53 = byte +0x14c]; local_18 = local_18 + 1) {
      if ((*(short *)(*(int *)(*piVar2 + local_18 * 4) + 8) == 0x21) ||
         (*(short *)(*(int *)(*piVar2 + local_18 * 4) + 8) == 0x22)) {
        if (*(int *)(*piVar2 + local_18 * 4) != param_3) {
          if (*(short *)(*(int *)(*piVar2 + local_18 * 4) + 8) == 0x22) {
            FUN_005e4e60(0);
          }
          else {
            iVar1 = FUN_005e4e60(0);
            if ((int)local_8 < iVar1) {
              local_8 = FUN_005e4e60(0);
            }
          }
        }
      }
      else if (0x22 < *(ushort *)(*(int *)(*piVar2 + local_18 * 4) + 8)) break;
    }
    iVar1 = (**(code **)(*param_2 + 0x30))();
    if (iVar1 != 0) {
      iVar1 = (**(code **)(*param_2 + 0x30))();
      *(undefined4 *)(iVar1 + 0x384 [dec 900]) = 1;
    }
    FUN_006bc400(local_8 & 0xff);
  }
  return 1;
}
```

### decoy: 0x0059f7d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f7d0: 16-byte aligned: YES
  0059f7d0  55             PUSH EBP
  0059f7d1  8B EC          MOV EBP,ESP
  0059f7d3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059f7d0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC3D0  CSWSCreatureStats::SetForceResistance
struct offsets touched (top 5):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-06

GOG function: CSWSEffectListHandler::OnRemoveSlowInternal  gog 0x007444e0  callconv=(blank)  param_size_bytes=4  gog_size=55  gog_retn=8
Candidate Steam address: 0x005a1c10 (size 55, retn 8)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyInvisibility
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a18d0:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007441a0 CSWSEffectListHandler::OnApplySlowInternal -> 0x005a18d0  gate_class=AUTO
- GOG-order neighbour after:  0x00744520 CSWSEffectListHandler::OnApplyInvisibility -> 0x005a1c50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0xfd8*} | Steam {0x30*,0xfd8*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a1c10: 16-byte aligned: YES
  005a1c10  55             PUSH EBP
  005a1c11  8B EC          MOV EBP,ESP
  005a1c13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a1c10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0xFD8 [dec 4056]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a1c10 FUN_005a1c10 size=55 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005a1c10(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0xfd8) = 0;
  }
  return 1;
}
```

### decoy: 0x005a18d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a18d0: 16-byte aligned: YES
  005a18d0  55             PUSH EBP
  005a18d1  8B EC          MOV EBP,ESP
  005a18d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a18d0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x122 [dec 290]  x3  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0xFD4 [dec 4052]  x1  via EAX
  +0xFD8 [dec 4056]  x1  via ECX
```

## Card B0-9-07

GOG function: CSWSCreatureStats::GetWeaponSpecialization  gog 0x006e5390  callconv=(blank)  param_size_bytes=0  gog_size=202  gog_retn=0
Candidate Steam address: 0x006a8fe0 (size 277, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreatureStats::GetUseMonkAttackTables
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006a8f10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006e5320 CSWSCreatureStats::GetWeaponFinesse -> 0x006a8ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x006e5460 CSWSCreatureStats::GetUseMonkAttackTables -> 0x006a8fe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0068f220 CSWSCreature::GetUseMonkAbilities -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x24*,0x8d*,0x53c*} | Steam {0x24*,0x8d*,0x53c*}
imm sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xc*,0x10*,0x20*} | Steam {0x2*,0x4*,0x6,0x8*,0xa,0xc*,0x10*,0x14,0x20*}

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

## Card B0-9-08

GOG function: CSWSEffectListHandler::OnApplyDisappear  gog 0x00748db0  callconv=(blank)  param_size_bytes=8  gog_size=163  gog_retn=12
Candidate Steam address: 0x005a64c0 (size 163, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a6570:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748b70 CSWSEffectListHandler::OnRemoveDisappearAppear -> 0x005a6280  gate_class=REVIEW
- GOG-order neighbour after:  0x00748e60 CSWSEffectListHandler::OnApplyAppear -> 0x005a6570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x006962c0 CSWSCreature::AddDisappearActions -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x18*,0x30*} | Steam {0x4*,0x8*,0x18*,0x30*}
imm sets (* = in both, 3 common): GOG {0xc*,0x14*,0x7d0*} | Steam {0xc*,0x14*,0x7d0*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a64c0: 16-byte aligned: YES
  005a64c0  55             PUSH EBP
  005a64c1  8B EC          MOV EBP,ESP
  005a64c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a64c0
string literals (0):
direct callees (4):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a64c0 FUN_005a64c0 size=163 ret=12 callers=0 callees=4

undefined4 __thiscall FUN_005a64c0(undefined4 param_1,int *param_2,int param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  if ((char)param_2[2 = byte +0x8] == DAT_00994474) {
    (**(code **)(*param_2 + 0x30))();
    FUN_00541080(1);
    FUN_0057d790(0);
  }
  else if ((char)param_2[2 = byte +0x8] == DAT_00994478) {
    iVar4 = param_2[1 = byte +0x4];
    uVar3 = *(undefined4 *)(param_3 + 0x18);
    uVar6 = 0;
    uVar2 = 2000;
    uVar1 = 0;
    uVar5 = DAT_00992420;
    FUN_0051c280(0,2000,uVar3,iVar4,DAT_00992420,0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
    FUN_0051e170(uVar1,uVar2,uVar3,iVar4,uVar5,uVar6);
  }
  return 1;
}
```

### decoy: 0x005a6570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a6570: 16-byte aligned: YES
  005a6570  55             PUSH EBP
  005a6571  8B EC          MOV EBP,ESP
  005a6573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a6570
string literals (0):
direct callees (2):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D700  CSWSCreature::AddAppearActions
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-09

GOG function: CSWSEffectListHandler::OnRemoveDisappearAppear  gog 0x00748b70  callconv=(blank)  param_size_bytes=8  gog_size=573  gog_retn=8
Candidate Steam address: 0x005a6280 (size 573, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a60b0:STR0/CALLEE0/SIG0.261/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007489a0 CSWSEffectListHandler::OnApplyDisappearAppear -> 0x005a60b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00748db0 CSWSEffectListHandler::OnApplyDisappear -> 0x005a64c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0064bb40 CServerExoApp::GetAreaByGameObjectID -> Steam ?  in candidate callees: n/a
  0x00681800 CSWSCreature::AddToArea -> Steam ?  in candidate callees: n/a
  0x00696230 CSWSCreature::AddAppearActions -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
  0x007054d0 CSWSArea::ComputeHeight -> Steam ?  in candidate callees: n/a
  0x00706160 CSWSArea::ComputeSafeLocation -> Steam ?  in candidate callees: n/a
  0x00734b60 CGameEffect::GetFloat -> Steam ?  in candidate callees: n/a
  0x00734b80 CGameEffect::SetFloat -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x30*,0x48*,0x380*,0x10dc*} | Steam {0x8*,0x30*,0x48*,0x380*,0x10dc*}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x60*,0x7d0*,0x7f000000*} | Steam {0x2*,0x8*,0x60*,0x7d0*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005a6280: 16-byte aligned: YES
  005a6280  55             PUSH EBP
  005a6281  8B EC          MOV EBP,ESP
  005a6283  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6280
string literals (0):
direct callees (12):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0057D700  CSWSCreature::AddAppearActions
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x0054B130  CSWSArea::ComputeHeight
  0x00568BD0  CSWSCreature::AddToArea
  0x005E4EC0  CGameEffect::SetFloat
  0x005F2A80  CSWSPlaceable::AddToArea
struct offsets touched (top 5):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x380 [dec 896]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6280 FUN_005a6280 size=573 ret=8 callers=0 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005a6280(int *param_1)

{
  int iVar1;
  float10 fVar2;
  float10 fVar3;
  float10 fVar4;
  float fVar5;
  float fVar6;
  undefined4 uVar7;
  float *pfVar8;
  undefined4 uVar9;
  float local_34;
  float local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  float local_14;
  float local_10;
  float local_c;
  float local_8;
  
  fVar2 = (float10)FUN_005e4ea0(0);
  fVar3 = (float10)FUN_005e4ea0(1);
  fVar4 = (float10)FUN_005e4ea0(2);
  local_8 = (float)fVar4;
  local_1c = 0;
  local_18 = 0;
  local_14 = 0.0;
  local_10 = (float)fVar2;
  local_c = (float)fVar3;
  local_28 = FUN_005e4ee0(0);
  local_24 = FUN_0051c120(local_28);
  if ((char)param_1[2 = byte +0x8] == DAT_00994474) {
    local_20 = (**(code **)(*param_1 + 0x30))();
    FUN_00541080(1);
    FUN_0058fc60(0,0x7f000000);
    FUN_0058fcd0(2000,0);
    FUN_0058fc60(1,0x7f000000);
    FUN_0057d700();
    iVar1 = FUN_0054bdc0(&local_10,_DAT_0098bd6c,*(undefined4 *)(local_20 + 0x380),0,&local_1c);
    if (iVar1 == 0) {
      fVar2 = (float10)FUN_0054b130(&local_10,0,0,0);
      FUN_005e4ec0(2,(float)fVar2);
      uVar9 = 0;
      uVar7 = 0;
      fVar2 = (float10)FUN_005e4ea0(2);
      fVar6 = (float)fVar2;
      fVar2 = (float10)FUN_005e4ea0(1);
      fVar5 = (float)fVar2;
      fVar2 = (float10)FUN_005e4ea0(0);
      FUN_00568bd0(local_24,(float)fVar2,fVar5,fVar6,uVar7,uVar9);
    }
    else {
      fVar2 = (float10)FUN_0054b130(&local_1c,0,0,0);
      local_14 = (float)fVar2;
      FUN_00568bd0(local_24,local_1c,local_18,local_14,0,0);
    }
  }
  else if ((char)param_1[2 = byte +0x8] == DAT_00994478) {
    fVar2 = (float10)FUN_005e4ea0(0);
    fVar3 = (float10)FUN_005e4ea0(1);
    fVar4 = (float10)FUN_005e4ea0(2);
    local_2c = (float)fVar4;
    uVar9 = 0;
    pfVar8 = &local_34;
    uVar7 = local_24;
    local_34 = (float)fVar2;
    local_30 = (float)fVar3;
    (**(code **)(*param_1 + 0x48))(local_24,pfVar8,0);
    FUN_005f2a80(uVar7,pfVar8,uVar9);
  }
  return 1;
}
```

### decoy: 0x005a60b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a60b0: 16-byte aligned: YES
  005a60b0  55             PUSH EBP
  005a60b1  8B EC          MOV EBP,ESP
  005a60b3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a60b0
string literals (0):
direct callees (10):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x005864D0  CSWSCreature::PacifyCreature
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 8):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
  +0x90 [dec 144]  x1  via EAX
  +0x350 [dec 848]  x1  via ECX
  +0x360 [dec 864]  x1  via EDX
```

## Card B0-9-10

GOG function: CSWSEffectListHandler::OnApplySilence  gog 0x00745540  callconv=(blank)  param_size_bytes=12  gog_size=536  gog_retn=12
Candidate Steam address: 0x005a2c70 (size 536, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2710:STR0/CALLEE0/SIG0.567/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744fe0 CSWSEffectListHandler::OnApplyCurse -> 0x005a2710  gate_class=REVIEW
- GOG-order neighbour after:  0x00745760 CSWSEffectListHandler::OnRemoveSilence -> 0x005a2e90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x18*,0x30*,0xfc*,0xfd0*,0x1198*} | Steam {0x4*,0x8*,0x18*,0x30*,0xfc*,0xfd0*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x4*,0xb*,0xc*,0x17*,0x34*,0x50*,0x8c*,0x91*,0xfffffff7*} | Steam {0x2*,0x4*,0xb*,0xc*,0x17*,0x34*,0x50*,0x8c*,0x91*,0xfffffff7*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a2c70: 16-byte aligned: YES
  005a2c70  55             PUSH EBP
  005a2c71  8B EC          MOV EBP,ESP
  005a2c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2c70
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0xFD0 [dec 4048]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2c70 FUN_005a2c70 size=536 ret=12 callers=0 callees=10

undefined4 FUN_005a2c70(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009526a1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(0xb,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_58 = 0;
      }
      else {
        local_58 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_5c = 0;
        }
        else {
          local_5c = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_5c);
        FUN_00584da0(0x91,local_5c);
      }
      uVar4 = 0x91;
      (**(code **)(*param_1 + 0x30))(0x91,local_58);
      FUN_00584da0(uVar4,local_58);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    iVar2 = FUN_00919723(0x8c);
    local_8 = 2;
    if (iVar2 == 0) {
      local_60 = 0;
    }
    else {
      local_60 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    *(undefined2 *)(local_60 + 8) = 0x17;
    FUN_005e4e80(0,0xfffffff7);
    FUN_00544210(local_60,param_3,0);
    *(undefined4 *)(iVar1 + 0xfd0) = 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005a2710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2710: 16-byte aligned: YES
  005a2710  55             PUSH EBP
  005a2711  8B EC          MOV EBP,ESP
  005a2713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2710
string literals (0):
direct callees (13):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4E60  CGameEffect::GetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 6):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-9-11

GOG function: CSWSEffectListHandler::OnRemoveSetStateInternal  gog 0x00741150  callconv=(blank)  param_size_bytes=8  gog_size=234  gog_retn=8
Candidate Steam address: 0x0059e860 (size 234, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ea10:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00740720 CSWSEffectListHandler::OnApplySetStateInternal -> 0x0059de30  gate_class=REVIEW
- GOG-order neighbour after:  0x00741260 CSWSEffectListHandler::OnApplyMovementSpeedIncrease -> 0x0059e970  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x006934c0 CSWSCreature::RecomputeAmbientAnimationState -> Steam ?  in candidate callees: n/a
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x30*,0x270*,0x10e8*} | Steam {0x4*,0x30*,0x270*,0x10e8*}
imm sets (* = in both, 6 common): GOG {0x3*,0x6*,0x8*,0xe*,0x12*,0x18*} | Steam {0x3*,0x6*,0x8*,0xe*,0x12*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059e860: 16-byte aligned: YES
  0059e860  55             PUSH EBP
  0059e861  8B EC          MOV EBP,ESP
  0059e863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059e860
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x006FD8D0  CVirtualMachine::RunScript
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x005D1510  CSWSCreature::ClearVisibleList
  0x005D0F30  CSWSObject::UpdatePerception
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x10E8 [dec 4328]  x1  via ECX
  +0x59E958 [dec 5892440]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059e860 FUN_0059e860 size=234 ret=8 callers=0 callees=5

/* WARNING: Removing unreachable block (ram,0x0059e8ed) */

undefined4 FUN_0059e860(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if ((param_2 != 0) && (iVar1 != 0)) {
    uVar4 = extraout_var;
    iVar2 = FUN_005e4e60(0);
    iVar3 = iVar2 + -1;
    switch(iVar2) {
    case 1:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 0xe:
    case 0xf:
    case 0x10:
    case 0x11:
    case 0x12:
    case 0x13:
      FUN_006fd8d0(iVar1 + 0x2a0,*(undefined4 *)(iVar1 + 4),1);
      FUN_0057a990(iVar3,uVar4);
      break;
    case 0xc:
    case 0xd:
      FUN_0057a990(iVar3,uVar4);
    }
    if (*(int *)(iVar1 + 0x10e8) == 0) {
      FUN_005d1510();
      FUN_005d0f30(0);
    }
  }
  return 1;
}
```

### decoy: 0x0059ea10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

## Card B0-9-12

GOG function: CSWSItem::AddUpgrade  gog 0x00679590  callconv=(blank)  param_size_bytes=6  gog_size=296  gog_retn=8
Candidate Steam address: 0x006140c0 (size 3404, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::GetDamageTypeString
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00610fc0:STR0/CALLEE0/SIG0.062/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x12,0x28c,0x294} | Steam {0x4}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x28*} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc*,0xd,0xe,0xf,...(+42)}

### abi
```
GOG param bytes: 6; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(6/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 6 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006140c0: 16-byte aligned: YES
  006140c0  55             PUSH EBP
  006140c1  8B EC          MOV EBP,ESP
  006140c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006140c0
string literals (0):
direct callees (5):
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
struct offsets touched (top 1):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006140c0 FUN_006140c0 size=3404 ret=8 callers=1 callees=5

void FUN_006140c0(uint param_1)

{
  undefined4 uVar1;
  undefined1 local_17c [8];
  undefined1 local_174 [8];
  undefined1 local_16c [8];
  undefined1 local_164 [8];
  undefined1 local_15c [8];
  undefined1 local_154 [8];
  undefined1 local_14c [8];
  undefined1 local_144 [8];
  undefined1 local_13c [8];
  undefined1 local_134 [8];
  undefined1 local_12c [8];
  undefined1 local_124 [8];
  undefined1 local_11c [8];
  undefined1 local_114 [8];
  undefined1 local_10c [8];
  undefined1 local_104 [8];
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  undefined1 local_ec [8];
  undefined1 local_e4 [8];
  undefined1 local_dc [8];
  undefined1 local_d4 [8];
  undefined1 local_cc [8];
  undefined1 local_c4 [8];
  undefined1 local_bc [8];
  undefined1 local_b4 [8];
  undefined1 local_ac [8];
  undefined1 local_a4 [8];
  undefined1 local_9c [8];
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
  undefined1 local_3c [8];
  undefined1 local_34 [8];
  undefined1 local_2c [8];
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00956c85;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = 0;
  if ((((param_1 & 1) != 0) || ((param_1 >> 1 & 1) != 0)) || ((param_1 >> 2 & 1) != 0)) {
    uVar1 = FUN_007408f0(local_1c,0x9698);
    local_8 = 0;
    FUN_007337c0(uVar1);
    local_8 = 0xffffffff;
    FUN_00733780();
    local_14 = local_14 + 1;
  }
  if ((param_1 >> 3 & 1) != 0) {
    if (0 < local_14) {
      FUN_00733570(&DAT_009975c8);
      local_8 = 1;
      uVar1 = FUN_00733df0(local_2c,local_24);
      local_8._0_1_ = 2;
      FUN_007337c0(uVar1);
      local_8 = CONCAT31(local_8._1_3_,1);
      FUN_00733780();
      local_8 = 0xffffffff;
      FUN_00733780();
    }
/* ... 243 more lines truncated by --max-lines */
```

### decoy: 0x00610fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00610fc0: 16-byte aligned: YES
  00610fc0  55             PUSH EBP
  00610fc1  8B EC          MOV EBP,ESP
  00610fc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00610fc0
string literals (4):
  0x009976E0  at 0061156e  '%d/%d\\n'
  0x009976D8  at 00611e4f  '%s -%d'
  0x009884D0  at 0061345d  '%s %d'
  0x009976C4  at 00613ac2  '%d%% %s, %d%s '
direct callees (21):
  0x00733540  CExoString::CExoString(empty)
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x00604260  CSWSItem::GetActiveProperty
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00603B50  CSWSItem::GetUsedActivePropertyUsesLeft
  0x00734270  CExoString::Format
  0x006042A0  CSWSItem::GetPassiveProperty
  0x00614E10  CSWSItem::GetPropertyStrings
  0x00615B90  CSWSItem::GetExistsHigherValuedProperty
  0x00734010  CExoString::CStr
  0x0091C7AD  (unnamed)
  0x00734190  CExoString::Find_2
  0x00734500  CExoString::Left
  0x007335F0  CExoString::CExoString_copy
  0x00733700  CExoString::Constructor_5
  0x00615A00  CSWSItem::GetPropertyChanceDurationDC
struct offsets touched (top 10):
  +0x2 [dec 2]  x22  via EAX,ECX,EDX
  +0x6 [dec 6]  x22  via EAX,ECX,EDX
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x11 [dec 17]  x6  via EAX,EDX
  +0x2D4 [dec 724]  x6  via EAX,ECX
  +0x28C [dec 652]  x5  via ECX,EDX
  +0x29C [dec 668]  x5  via EAX,ECX,EDX
  +0x9 [dec 9]  x1  via EAX
  +0x288 [dec 648]  x1  via ECX
  +0x614058 [dec 6373464]  x1  via ECX
```

## Card B0-9-13

GOG function: CSWSEffectListHandler::OnRemoveAreaOfEffect  gog 0x00741de0  callconv=(blank)  param_size_bytes=8  gog_size=176  gog_retn=8
Candidate Steam address: 0x0059f4f0 (size 176, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059f7d0:STR0/CALLEE0/SIG0.283/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741a40 CSWSEffectListHandler::OnApplyAreaOfEffect -> 0x0059f150  gate_class=REVIEW
- GOG-order neighbour after:  0x00741e90 CSWSEffectListHandler::OnApplyBeam -> 0x0059f5a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x30*} | Steam {0x8*,0x30*}
imm sets (* = in both, 2 common): GOG {0x8*,0x1c*} | Steam {0x8*,0x1c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059f4f0: 16-byte aligned: YES
  0059f4f0  55             PUSH EBP
  0059f4f1  8B EC          MOV EBP,ESP
  0059f4f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059f4f0
string literals (0):
direct callees (5):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059f4f0 FUN_0059f4f0 size=176 ret=8 callers=0 callees=5

undefined4 __thiscall FUN_0059f4f0(undefined4 param_1,int *param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int *piVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  int local_c;
  char local_5;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    uVar2 = (**(code **)(*param_2 + 0x30))();
    uVar3 = FUN_005e4ee0(0);
    uVar10 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    piVar8 = &local_c;
    uVar6 = uVar3;
    FUN_0051c080(uVar3,piVar8);
    local_5 = FUN_0053dfb0(uVar6,piVar8);
    if ((local_5 == DAT_0099447f) && (local_c != 0)) {
      uVar9 = 0;
      uVar5 = 0;
      uVar4 = 0;
      uVar6 = uVar3;
      uVar7 = DAT_00992420;
      FUN_0051c280(0,0,uVar3,uVar3,DAT_00992420,0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 8),uVar10,
                   uVar2);
      FUN_0051e170(uVar4,uVar5,uVar3,uVar6,uVar7,uVar9);
    }
  }
  return 1;
}
```

### decoy: 0x0059f7d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f7d0: 16-byte aligned: YES
  0059f7d0  55             PUSH EBP
  0059f7d1  8B EC          MOV EBP,ESP
  0059f7d3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059f7d0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC3D0  CSWSCreatureStats::SetForceResistance
struct offsets touched (top 5):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-14

GOG function: CSWSEffectListHandler::OnApplySetAIState  gog 0x0073eca0  callconv=(blank)  param_size_bytes=8  gog_size=104  gog_retn=12
Candidate Steam address: 0x0059c3b0 (size 104, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c420:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073e870 CSWSEffectListHandler::OnApplyDisarm -> 0x0059bf80  gate_class=REVIEW
- GOG-order neighbour after:  0x0073ed10 CSWSEffectListHandler::OnRemoveSetAIState -> 0x0059c420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x1114*} | Steam {0x30*,0x1114*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059c3b0: 16-byte aligned: YES
  0059c3b0  55             PUSH EBP
  0059c3b1  8B EC          MOV EBP,ESP
  0059c3b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c3b0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1114 [dec 4372]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c3b0 FUN_0059c3b0 size=104 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_0059c3b0(undefined4 param_1,int *param_2,int param_3)

{
  ushort uVar1;
  ushort uVar2;
  int iVar3;
  
  iVar3 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar3 != 0) {
    iVar3 = (**(code **)(*param_2 + 0x30))();
    if (param_3 != 0) {
      uVar1 = *(ushort *)(iVar3 + 0x1114);
      uVar2 = FUN_005e4e60(0);
      *(ushort *)(iVar3 + 0x1114) = uVar1 & uVar2;
    }
  }
  return 0;
}
```

### decoy: 0x0059c420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c420: 16-byte aligned: YES
  0059c420  55             PUSH EBP
  0059c421  8B EC          MOV EBP,ESP
  0059c423  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c420
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x1114 [dec 4372]  x2  via ECX,EDX
  +0x144 [dec 324]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-9-15

GOG function: CSWSItem::AddUpgrade  gog 0x00679590  callconv=(blank)  param_size_bytes=6  gog_size=296  gog_retn=8
Candidate Steam address: 0x00615e90 (size 349, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItem::RemoveUpgrade
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00615d60:STR0/CALLEE0/SIG1/SRV0/VT0
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
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x28} | Steam {0x4*,0x8*,0xc*,0x1c*,0x3c}

### abi
```
GOG param bytes: 6; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(6/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 6 bytes -> x86 slots 8; ASM RET 8)
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

## Card B0-9-16

GOG function: CSWSEffectListHandler::OnApplyDeaf  gog 0x0073ee40  callconv=(blank)  param_size_bytes=12  gog_size=658  gog_retn=12
Candidate Steam address: 0x0059c550 (size 658, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c420:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073ed10 CSWSEffectListHandler::OnRemoveSetAIState -> 0x0059c420  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f0e0 CSWSEffectListHandler::OnApplyEnemyAttackBonus -> 0x0059c7f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x18*,0x30*,0xfc*,0x1198*} | Steam {0x4*,0x8*,0x18*,0x30*,0xfc*,0x1198*}
imm sets (* = in both, 12 common): GOG {0x2*,0x3*,0x4*,0x8*,0xc*,0x14*,0x17*,0x19*,0x34*,0x60*,0x8c*,0xffffffef*} | Steam {0x2*,0x3*,0x4*,0x8*,0xc*,0x14*,0x17*,0x19*,0x34*,0x60*,0x8c*,0xffffffef*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059c550: 16-byte aligned: YES
  0059c550  55             PUSH EBP
  0059c551  8B EC          MOV EBP,ESP
  0059c553  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059c550
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c550 FUN_0059c550 size=658 ret=12 callers=0 callees=10

undefined4 FUN_0059c550(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_70;
  int local_6c;
  undefined4 local_68;
  undefined4 local_64;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095210c;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_1 + 0x30))();
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(8,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_64 = 0;
      }
      else {
        local_64 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_68 = 0;
        }
        else {
          local_68 = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_68);
        FUN_00584da0(0x8c,local_68);
      }
      uVar4 = 0x8c;
      (**(code **)(*param_1 + 0x30))(0x8c,local_64);
      FUN_00584da0(uVar4,local_64);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    if (param_2 != 0) {
      iVar1 = FUN_00919723(0x8c);
      local_8 = 2;
      if (iVar1 == 0) {
        local_6c = 0;
      }
      else {
        local_6c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      iVar1 = FUN_00919723(0x8c);
      local_8 = 3;
      if (iVar1 == 0) {
        local_70 = 0;
      }
      else {
        local_70 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_6c + 8) = 0x17;
      FUN_005e4e80(0,0xffffffef);
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x0059c420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c420: 16-byte aligned: YES
  0059c420  55             PUSH EBP
  0059c421  8B EC          MOV EBP,ESP
  0059c423  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c420
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x1114 [dec 4372]  x2  via ECX,EDX
  +0x144 [dec 324]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-9-17

GOG function: CSWSEffectListHandler::OnRemoveKnockdown  gog 0x0073e770  callconv=(blank)  param_size_bytes=8  gog_size=250  gog_retn=8
Candidate Steam address: 0x0059be80 (size 250, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059b8d0:STR0/CALLEE0/SIG0.269/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073e1c0 CSWSEffectListHandler::OnApplyKnockdown -> 0x0059b8d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e870 CSWSEffectListHandler::OnApplyDisarm -> 0x0059bf80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00774310 CSWSCombatRound::SetRoundPaused -> Steam ?  in candidate callees: n/a
  0x00774380 CSWSCombatRound::SetPauseTimer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x30*,0x7c*,0x94*,0x9c*,0xec*,0xaa8*,0x10dc*,0x11ac*} | Steam {0x30*,0x7c*,0x94*,0x9c*,0xec*,0xaa8*,0x10dc*,0x11ac*}
imm sets (* = in both, 4 common): GOG {0x8*,0x1c*,0x2711*,0x7f000000*} | Steam {0x8*,0x1c*,0x2711*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059be80: 16-byte aligned: YES
  0059be80  55             PUSH EBP
  0059be81  8B EC          MOV EBP,ESP
  0059be83  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059be80
string literals (0):
direct callees (2):
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
struct offsets touched (top 8):
  +0x10DC [dec 4316]  x3  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0x7C [dec 124]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0xEC [dec 236]  x1  via ECX
  +0xAA8 [dec 2728]  x1  via EAX
  +0x11AC [dec 4524]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059be80 FUN_0059be80 size=250 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059be80(undefined4 param_1,int *param_2,int param_3)

{
  bool bVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  
  piVar3 = (int *)(**(code **)(*param_2 + 0x30))(param_1);
  if ((piVar3 != (int *)0x0) && (param_3 != 0)) {
    if (*(int *)(piVar3[0x437 = byte +0x10dc] + 0xaa8) != 0) {
      FUN_0058fc60(0,0x7f000000);
      FUN_0058fcd0(0,0);
    }
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
        (**(code **)(*piVar3 + 0x7c))(0x2711);
        param_2[0x3b = byte +0xec] = 1;
      }
    }
  }
  return 1;
}
```

### decoy: 0x0059b8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b8d0: 16-byte aligned: YES
  0059b8d0  55             PUSH EBP
  0059b8d1  8B EC          MOV EBP,ESP
  0059b8d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059b8d0
string literals (0):
direct callees (15):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005143C0  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x7C [dec 124]  x3  via EDX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10DC [dec 4316]  x2  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EAX
```

## Card B0-9-18

GOG function: CSWSEffectListHandler::OnApplyBeam  gog 0x00741e90  callconv=(blank)  param_size_bytes=12  gog_size=321  gog_retn=12
Candidate Steam address: 0x0059f5a0 (size 321, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059f4f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741de0 CSWSEffectListHandler::OnRemoveAreaOfEffect -> 0x0059f4f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00741fe0 CSWSEffectListHandler::OnApplyForceResistanceIncrease -> 0x0059f710  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
  0x00734bc0 CGameEffect::SetObjectID -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 6 common): GOG {0x2*,0x4*,0xc*,0x14*,0x1e*,0x8c*} | Steam {0x2*,0x4*,0xc*,0x14*,0x1e*,0x8c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059f5a0: 16-byte aligned: YES
  0059f5a0  55             PUSH EBP
  0059f5a1  8B EC          MOV EBP,ESP
  0059f5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059f5a0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4F00  CGameEffect::SetObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 1):
  +0x8 [dec 8]  x5  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059f5a0 FUN_0059f5a0 size=321 ret=12 callers=0 callees=7

undefined4 FUN_0059f5a0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949bbb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((((*(char *)(param_1 + 8) == DAT_00994474) || (*(char *)(param_1 + 8) == DAT_00994478)) ||
      (*(char *)(param_1 + 8) == DAT_00994479)) || (*(char *)(param_1 + 8) == DAT_00994476)) {
    iVar1 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    *(undefined2 *)(local_24 + 8) = 0x1e;
    uVar2 = FUN_005e4ee0(0);
    FUN_005e4f00(0,uVar2);
    uVar2 = FUN_005e4e60(0);
    FUN_005e4e80(0,uVar2);
    uVar2 = FUN_005e4e60(1);
    FUN_005e4e80(1,uVar2);
    uVar2 = FUN_005e4e60(2);
    FUN_005e4e80(2,uVar2);
    FUN_00544210(local_24,param_3,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x0059f4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f4f0: 16-byte aligned: YES
  0059f4f0  55             PUSH EBP
  0059f4f1  8B EC          MOV EBP,ESP
  0059f4f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059f4f0
string literals (0):
direct callees (5):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
```

## Card B0-9-19

GOG function: CSWSEffectListHandler::OnRemoveVisualEffect  gog 0x00741a10  callconv=(blank)  param_size_bytes=8  gog_size=37  gog_retn=8
Candidate Steam address: 0x0059f120 (size 37, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059f4f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741560 CSWSEffectListHandler::OnApplyVisualEffect -> 0x0059ec70  gate_class=REVIEW
- GOG-order neighbour after:  0x00741a40 CSWSEffectListHandler::OnApplyAreaOfEffect -> 0x0059f150  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006aae60 CSWSObject::RemoveLoopingVisualEffect -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
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
prologue for 0x0059f120: 16-byte aligned: YES
  0059f120  55             PUSH EBP
  0059f121  8B EC          MOV EBP,ESP
  0059f123  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059f120
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544180  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0059f120 FUN_0059f120 size=37 ret=8 callers=0 callees=2

undefined4 FUN_0059f120(void)

{
  undefined4 uVar1;
  
  uVar1 = FUN_005e4e60(0);
  FUN_00544180(uVar1);
  return 1;
}
```

### decoy: 0x0059f4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f4f0: 16-byte aligned: YES
  0059f4f0  55             PUSH EBP
  0059f4f1  8B EC          MOV EBP,ESP
  0059f4f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059f4f0
string literals (0):
direct callees (5):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
```

## Card B0-9-20

GOG function: CSWSEffectListHandler::OnApplyDispelBestMagic  gog 0x00745c80  callconv=(blank)  param_size_bytes=8  gog_size=1132  gog_retn=12
Candidate Steam address: 0x005a3390 (size 1132, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2ee0:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007457d0 CSWSEffectListHandler::OnApplyDispelAllMagic -> 0x005a2ee0  gate_class=REVIEW
- GOG-order neighbour after:  0x007460f0 CSWSEffectListHandler::OnApplyTaunt -> 0x005a3800  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x0050e640 CSWRules::RollDice -> Steam ?  in candidate callees: n/a
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549cb0 CSWCCMessageData::Destructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  ... 12 more GOG callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xa*,0x18*,0x1c*,0x30*,0x1198*} | Steam {0x4*,0x8*,0xa*,0x18*,0x1c*,0x30*,0x1198*}
imm sets (* = in both, 11 common): GOG {0x4*,0x7*,0x8*,0xb*,0xc*,0xf*,0x14*,0x18*,0x34*,0xb8*,0x148*} | Steam {0x4*,0x7*,0x8*,0xb*,0xc*,0xf*,0x14*,0x18*,0x34*,0xb8*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a3390: 16-byte aligned: YES
  005a3390  55             PUSH EBP
  005a3391  8B EC          MOV EBP,ESP
  005a3393  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a3390
string literals (0):
direct callees (20):
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005AA160  (unnamed)
  0x005AA250  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
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
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005AA1A0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 7):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a3390 FUN_005a3390 size=1132 ret=12 callers=0 callees=20

undefined4 FUN_005a3390(int param_1,int param_2)

{
  char cVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 *puVar9;
  int local_c4;
  int *local_54[4 = byte +0x10];
  int local_44;
  int local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_2c;
  undefined4 *local_28;
  uint local_24;
  int local_20;
  int local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952713;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_2c = FUN_0051c080(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_18 = (int *)(param_1 + 0x148);
  cVar1 = FUN_0053dfb0(*(undefined4 *)(param_2 + 0x18),local_54);
  if (((cVar1 == DAT_0099447f) && (local_54[0] != (int *)0x0)) &&
     (iVar2 = (**(code **)(*local_54[0] + 0x30))(), iVar2 != 0)) {
    (**(code **)(*local_54[0] + 0x30))();
    local_24 = FUN_006a4e00(0);
    local_24 = local_24 & 0xff;
    FUN_005aa160(0);
    local_8 = 0;
    local_44 = 0;
    do {
      local_40 = 0;
      local_3c = 0;
      local_20 = 0;
      while ((local_20 < local_18[1 = byte +0x4] && (local_40 == 0))) {
        local_28 = *(undefined4 **)(*local_18 + local_20 * 4);
        if (((*(ushort *)((int)local_28 + 0xa [dec 10]) & 0x18) == 8) &&
           (((*(ushort *)((int)local_28 + 0xa [dec 10]) & 7) == DAT_009967b4 ||
            ((*(ushort *)((int)local_28 + 0xa [dec 10]) & 7) == DAT_009967b8)))) {
          local_38 = *local_28;
          local_34 = local_28[1 = byte +0x4];
          iVar2 = FUN_005aa250(local_38,local_34);
          if (iVar2 == 0) {
            local_14 = FUN_0051c100(local_28[6 = byte +0x18]);
            if (local_14 == 0) {
              if (local_3c == 0) {
                local_1c = local_20;
                local_40 = 1;
              }
            }
            else {
              uVar3 = FUN_006a4e00(0);
              if (local_3c < (int)(uVar3 & 0xff)) {
                local_1c = local_20;
                local_40 = 1;
              }
            }
          }
        }
        local_20 = local_20 + 1;
      }
      if (local_40 != 0) {
        local_28 = *(undefined4 **)(*local_18 + local_1c * 4);
        uVar3 = FUN_006a2000(1,0x14);
/* ... 45 more lines truncated by --max-lines */
```

### decoy: 0x005a2ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2ee0: 16-byte aligned: YES
  005a2ee0  55             PUSH EBP
  005a2ee1  8B EC          MOV EBP,ESP
  005a2ee3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2ee0
string literals (0):
direct callees (19):
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005AA160  (unnamed)
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005AA250  (unnamed)
  0x006A2000  CSWRules::RollDice(n,sides)
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E1BD0  (unnamed)
  0x005AA210  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0051C2E0  (unnamed)
  0x00648CC0  (unnamed)
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 7):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EDX
  +0xA [dec 10]  x3  via ECX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x1198 [dec 4504]  x2  via ECX
```

## Card B0-9-21

GOG function: CSWSEffectListHandler::OnApplyDisappearAppear  gog 0x007489a0  callconv=(blank)  param_size_bytes=8  gog_size=463  gog_retn=12
Candidate Steam address: 0x005a60b0 (size 463, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a6280:STR0/CALLEE0/SIG0.261/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748940 CSWSEffectListHandler::OnApplyTurnResistance -> 0x005a6050  gate_class=REVIEW
- GOG-order neighbour after:  0x00748b70 CSWSEffectListHandler::OnRemoveDisappearAppear -> 0x005a6280  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0064bb40 CServerExoApp::GetAreaByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x006962c0 CSWSCreature::AddDisappearActions -> Steam ?  in candidate callees: n/a
  0x0069efb0 CSWSCreature::PacifyCreature -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
  0x00734b60 CGameEffect::GetFloat -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x18*,0x30*,0x90*,0x350*,0x360*,0x10dc*} | Steam {0x4*,0x8*,0x18*,0x30*,0x90*,0x350*,0x360*,0x10dc*}
imm sets (* = in both, 6 common): GOG {0x2*,0xc*,0x50*,0x94*,0x354*,0x7d0*} | Steam {0x2*,0xc*,0x50*,0x94*,0x354*,0x7d0*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a60b0: 16-byte aligned: YES
  005a60b0  55             PUSH EBP
  005a60b1  8B EC          MOV EBP,ESP
  005a60b3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a60b0
string literals (0):
direct callees (10):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x005864D0  CSWSCreature::PacifyCreature
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 8):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
  +0x90 [dec 144]  x1  via EAX
  +0x350 [dec 848]  x1  via ECX
  +0x360 [dec 864]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a60b0 FUN_005a60b0 size=463 ret=12 callers=0 callees=10

undefined4 FUN_005a60b0(int *param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 extraout_var;
  
  FUN_005e4ea0(0);
  uVar3 = extraout_var;
  FUN_005e4ea0(1);
  FUN_005e4ea0(2);
  uVar1 = FUN_005e4ee0(0);
  iVar2 = FUN_0051c120(uVar1);
  if (iVar2 == 0) {
    uVar3 = 1;
  }
  else {
    if ((char)param_1[2 = byte +0x8] == DAT_00994474) {
      iVar2 = (**(code **)(*param_1 + 0x30))();
      FUN_00541080(1);
      FUN_005864d0();
      *(undefined4 *)(iVar2 + 0x350) = uVar1;
      iVar4 = (**(code **)(*param_1 + 0x30))();
      *(undefined4 *)(iVar2 + 0x354) = *(undefined4 *)(iVar4 + 0x94);
      *(undefined4 *)(iVar2 + 0x358) = *(undefined4 *)(iVar4 + 0x98);
      *(undefined4 *)(iVar2 + 0x35c) = *(undefined4 *)(iVar4 + 0x9c);
      *(undefined4 *)(iVar2 + 0x360) = 0;
      FUN_0058fcd0(0,1);
      FUN_0058fc60(1,param_1[1 = byte +0x4]);
      FUN_0057d790(1);
    }
    else {
      if ((char)param_1[2 = byte +0x8] != DAT_00994478) {
        return 1;
      }
      iVar2 = param_1[1 = byte +0x4];
      uVar1 = *(undefined4 *)(param_2 + 0x18);
      uVar8 = 0;
      uVar6 = 2000;
      uVar5 = 0;
      uVar7 = DAT_00992404;
      FUN_0051c280(0,2000,uVar1,iVar2,DAT_00992404,0,uVar3,*(undefined4 *)(DAT_00a1b4a4 + 8));
      FUN_0051e170(uVar5,uVar6,uVar1,iVar2,uVar7,uVar8);
    }
    uVar3 = 0;
  }
  return uVar3;
}
```

### decoy: 0x005a6280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a6280: 16-byte aligned: YES
  005a6280  55             PUSH EBP
  005a6281  8B EC          MOV EBP,ESP
  005a6283  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a6280
string literals (0):
direct callees (12):
  0x005E4EA0  CGameEffect::GetFloat
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C120  (unnamed)
  0x00541080  CSWSObject::ClearAllActions
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x0057D700  CSWSCreature::AddAppearActions
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x0054B130  CSWSArea::ComputeHeight
  0x00568BD0  CSWSCreature::AddToArea
  0x005E4EC0  CGameEffect::SetFloat
  0x005F2A80  CSWSPlaceable::AddToArea
struct offsets touched (top 5):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x380 [dec 896]  x1  via ECX
```

## Card B0-9-22

GOG function: CSWSEffectListHandler::OnApplyMovementSpeedIncrease  gog 0x00741260  callconv=(blank)  param_size_bytes=8  gog_size=145  gog_retn=12
Candidate Steam address: 0x0059e970 (size 145, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ea10:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741150 CSWSEffectListHandler::OnRemoveSetStateInternal -> 0x0059e860  gate_class=REVIEW
- GOG-order neighbour after:  0x00741300 CSWSEffectListHandler::OnRemoveMovementSpeedIncrease -> 0x0059ea10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0068f140 CSWSCreature::GetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x0068f1d0 CSWSCreature::SetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
imm sets (* = in both, 3 common): GOG {0xc*,0x14*,0x64*} | Steam {0xc*,0x14*,0x64*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059e970: 16-byte aligned: YES
  0059e970  55             PUSH EBP
  0059e971  8B EC          MOV EBP,ESP
  0059e973  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059e970
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00576610  CSWSCreature::GetMovementRateFactor
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 1):
  +0x30 [dec 48]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059e970 FUN_0059e970 size=145 ret=12 callers=0 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059e970(int *param_1)

{
  float fVar1;
  int iVar2;
  float10 fVar3;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    (**(code **)(*param_1 + 0x30))();
    iVar2 = FUN_005e4e60(0);
    if (iVar2 < 100) {
      iVar2 = FUN_005e4e60(0);
      FUN_005e4e80(0,iVar2 + 0x64 [dec 100]);
    }
    iVar2 = FUN_005e4e60(0);
    fVar1 = (float)_DAT_0098ba38;
    fVar3 = (float10)FUN_00576610(0);
    FUN_005766a0((float)(fVar3 * (float10)((float)iVar2 / fVar1)));
  }
  return 0;
}
```

### decoy: 0x0059ea10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

## Card B0-9-23

GOG function: CSWSEffectListHandler::OnRemoveSetAIState  gog 0x0073ed10  callconv=(blank)  param_size_bytes=8  gog_size=296  gog_retn=8
Candidate Steam address: 0x0059c420 (size 296, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c550:STR0/CALLEE0/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073eca0 CSWSEffectListHandler::OnApplySetAIState -> 0x0059c3b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073ee40 CSWSEffectListHandler::OnApplyDeaf -> 0x0059c550  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x30*,0x144*,0x1114*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x144*,0x1114*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x17*,0x34*,0x148*,0xffff*} | Steam {0x8*,0x17*,0x34*,0x148*,0xffff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059c420: 16-byte aligned: YES
  0059c420  55             PUSH EBP
  0059c421  8B EC          MOV EBP,ESP
  0059c423  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c420
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x1114 [dec 4372]  x2  via ECX,EDX
  +0x144 [dec 324]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c420 FUN_0059c420 size=296 ret=8 callers=0 callees=1

undefined4 __thiscall FUN_0059c420(undefined4 param_1,int *param_2,int param_3)

{
  int iVar1;
  ushort uVar2;
  int iVar3;
  undefined4 local_10;
  undefined2 local_c;
  
  iVar3 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar3 != 0) {
    iVar3 = (**(code **)(*param_2 + 0x30))();
    if (*(short *)(iVar3 + 0x1114) != 0) {
      local_c = 0xffff;
      if (0 < *(int *)(iVar3 + 0x14c)) {
        for (local_10 = (uint)*(ushort *)(*(int *)(iVar3 + 0x1198) + 0x144);
            (int)local_10 < *(int *)(iVar3 + 0x14c); local_10 = local_10 + 1) {
          if (*(short *)(*(int *)(*(int *)(iVar3 + 0x148) + local_10 * 4) + 8) != 0x17) break;
          iVar1 = *(int *)(*(int *)(iVar3 + 0x148) + local_10 * 4);
          if ((*(short *)(iVar1 + 8) == 0x17) && (iVar1 != param_3)) {
            uVar2 = FUN_005e4e60(0);
            local_c = local_c & uVar2;
          }
        }
      }
      *(ushort *)(iVar3 + 0x1114) = local_c;
    }
  }
  return 1;
}
```

### decoy: 0x0059c550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c550: 16-byte aligned: YES
  0059c550  55             PUSH EBP
  0059c551  8B EC          MOV EBP,ESP
  0059c553  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059c550
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-9-24

GOG function: CSWSEffectListHandler::OnRemoveTimestop  gog 0x00747810  callconv=(blank)  param_size_bytes=4  gog_size=76  gog_retn=8
Candidate Steam address: 0x005a4f20 (size 76, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyTimestop
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a4d70:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00747660 CSWSEffectListHandler::OnApplyTimestop -> 0x005a4d70  gate_class=AUTO
- GOG-order neighbour after:  0x00747860 CSWSEffectListHandler::OnApplyRacialType -> 0x005a4f70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c0d0 CServerExoApp::TogglePauseState -> Steam ?  in candidate callees: n/a
  0x0064c1e0 CServerExoApp::RemoveFromExclusionList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0x8*,0x10*} | Steam {0x8*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a4f20: 16-byte aligned: YES
  005a4f20  55             PUSH EBP
  005a4f21  8B EC          MOV EBP,ESP
  005a4f23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a4f20
string literals (0):
direct callees (2):
  0x0051C6B0  (unnamed)
  0x0051C7C0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a4f20 FUN_005a4f20 size=76 ret=8 callers=0 callees=2

undefined4 FUN_005a4f20(int param_1)

{
  FUN_0051c6b0(1);
  FUN_0051c7c0(*(undefined4 *)(param_1 + 4),1);
  return 1;
}
```

### decoy: 0x005a4d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a4d70: 16-byte aligned: YES
  005a4d70  55             PUSH EBP
  005a4d71  8B EC          MOV EBP,ESP
  005a4d73  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a4d70
string literals (0):
direct callees (10):
  0x0051C740  (unnamed)
  0x0051C790  (unnamed)
  0x0051C6B0  (unnamed)
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051B270  Global::GetCalendarDayFromSeconds
  0x0051B300  Global::GetTimeOfDayFromSeconds
  0x0051C350  CServerExoApp::GetActiveTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AEF0  CWorldTimer::AddWorldTimes
  0x005E4F80  CGameEffect::SetExpiryTime
struct offsets touched (top 3):
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via ECX
```

## Card B0-9-25

GOG function: CSWSEffectListHandler::OnRemoveSetState  gog 0x00740440  callconv=(blank)  param_size_bytes=8  gog_size=733  gog_retn=8
Candidate Steam address: 0x0059db50 (size 733, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059cba0:STR0/CALLEE0/SIG0.321/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073f490 CSWSEffectListHandler::OnApplySetState -> 0x0059cba0  gate_class=REVIEW
- GOG-order neighbour after:  0x00740720 CSWSEffectListHandler::OnApplySetStateInternal -> 0x0059de30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xa*,0x18*,0x20*,0x30*,0xec*,0xff5*} | Steam {0x4*,0x8*,0xa*,0x18*,0x20*,0x30*,0xec*,0xff5*}
imm sets (* = in both, 8 common): GOG {0x4*,0x8*,0x9*,0x6c*,0x8c*,0x148*,0x7f000000*,0xfffffff8*} | Steam {0x4*,0x8*,0x9*,0x6c*,0x8c*,0x148*,0x7f000000*,0xfffffff8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059db50: 16-byte aligned: YES
  0059db50  55             PUSH EBP
  0059db51  8B EC          MOV EBP,ESP
  0059db53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059db50
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 8):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EDX
  +0xEC [dec 236]  x2  via EAX,ECX
  +0xFF5 [dec 4085]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x20 [dec 32]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059db50 FUN_0059db50 size=733 ret=8 callers=0 callees=7

undefined4 FUN_0059db50(int *param_1,int param_2)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int iVar4;
  int local_7c;
  undefined4 local_28;
  int local_24;
  int local_1c;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095225b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar2 != 0) {
    param_1[0x3b = byte +0xec] = 1;
    *(undefined1 *)(iVar2 + 0xff5) = 0;
    piVar3 = param_1 + 0x52;
    if (0 < param_1[0x53 = byte +0x14c]) {
      local_1c = 0;
      local_24 = 0;
      local_28 = 0x7f000000;
      while ((local_1c < param_1[0x53 = byte +0x14c] && (*(ushort *)(*(int *)(*piVar3 + local_1c * 4) + 8) < 9)))
      {
        if ((*(short *)(*(int *)(*piVar3 + local_1c * 4) + 8) == 8) &&
           ((*(int *)(*piVar3 + local_1c * 4) != param_2 &&
            (iVar4 = FUN_005e4e60(0), local_24 < iVar4)))) {
          local_24 = FUN_005e4e60(0);
          local_28 = *(undefined4 *)(*(int *)(*piVar3 + local_1c * 4) + 0x18);
        }
        local_1c = local_1c + 1;
      }
      while ((local_1c < param_1[0x53 = byte +0x14c] && (*(ushort *)(*(int *)(*piVar3 + local_1c * 4) + 8) < 10)))
      {
        if (*(short *)(*(int *)(*piVar3 + local_1c * 4) + 8) == 9) {
          puVar1 = *(undefined4 **)(*piVar3 + local_1c * 4);
          FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
        }
        else {
          local_1c = local_1c + 1;
        }
      }
      if (local_24 != 0) {
        *(undefined1 *)(iVar2 + 0xff5) = (undefined1)local_24;
        param_1[0x3b = byte +0xec] = 0;
        iVar2 = FUN_00919723(0x8c);
        local_8 = 0;
        if (iVar2 == 0) {
          local_7c = 0;
        }
        else {
          local_7c = FUN_005e4920(1);
        }
        local_8 = 0xffffffff;
        FUN_005e5ea0(local_28);
        *(ushort *)(local_7c + 0xa [dec 10]) = *(ushort *)(local_7c + 0xa [dec 10]) & 0xfff8 | DAT_009967b8;
        *(undefined4 *)(local_7c + 0x20) = 0;
        *(undefined2 *)(local_7c + 8) = 9;
        FUN_005e4e80(0,local_24);
        FUN_00544210(local_7c,0,0);
      }
    }
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x0059cba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059cba0: 16-byte aligned: YES
  0059cba0  55             PUSH EBP
  0059cba1  8B EC          MOV EBP,ESP
  0059cba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059cba0
string literals (0):
direct callees (17):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC9A0  CSWSCreatureStats::GetEffectLinkImmunity
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x00544790  CSWSObject::RemoveEffectById
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
struct offsets touched (top 10):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x30 [dec 48]  x10  via EDX
  +0x150 [dec 336]  x9  via ECX,EDX
  +0x8 [dec 8]  x5  via EAX,EDX
  +0xA [dec 10]  x2  via EDX
  +0x18 [dec 24]  x2  via ECX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0xFF5 [dec 4085]  x2  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x145 [dec 325]  x1  via EDX
```

## Card B0-9-26

GOG function: CSWSEffectListHandler::OnApplyVision  gog 0x00744e00  callconv=(blank)  param_size_bytes=8  gog_size=262  gog_retn=12
Candidate Steam address: 0x005a2530 (size 262, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2450:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00744d20 CSWSEffectListHandler::OnApplyLight -> 0x005a2450  gate_class=REVIEW
- GOG-order neighbour after:  0x00744f10 CSWSEffectListHandler::OnApplyModifyNumAttacks -> 0x005a2640  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x30*} | Steam {0x8*,0x30*}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0x1e*,0x24*,0x8c*} | Steam {0x4*,0xc*,0x1e*,0x24*,0x8c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a2530: 16-byte aligned: YES
  005a2530  55             PUSH EBP
  005a2531  8B EC          MOV EBP,ESP
  005a2533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2530
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2530 FUN_005a2530 size=262 ret=12 callers=0 callees=5

undefined4 FUN_005a2530(int *param_1,undefined4 param_2)

{
  int iVar1;
  int local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969c6b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    FUN_005e4e60(0);
    iVar1 = FUN_00919723(0x8c);
    local_8 = 0;
    if (iVar1 == 0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    if ((local_30 != 0) && (*(undefined2 *)(local_30 + 8) = 0x1e, local_30 != 0)) {
      FUN_005e4ca0();
      FUN_009196fd(local_30);
    }
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005a2450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2450: 16-byte aligned: YES
  005a2450  55             PUSH EBP
  005a2451  8B EC          MOV EBP,ESP
  005a2453  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2450
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-9-27

GOG function: CSWSEffectListHandler::OnApplyCurse  gog 0x00744fe0  callconv=(blank)  param_size_bytes=12  gog_size=1376  gog_retn=12
Candidate Steam address: 0x005a2710 (size 1376, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2c70:STR0/CALLEE0/SIG0.567/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744f70 CSWSEffectListHandler::OnRemoveModifyNumAttacks -> 0x005a26a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00745540 CSWSEffectListHandler::OnApplySilence -> 0x005a2c70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x18*,0x30*,0xfc*,0x1198*} | Steam {0x4*,0x8*,0x18*,0x30*,0xfc*,0x1198*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0xc*,0x11*,0x25*,0x34*,0x8c*,0x8d*,0xac*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0xc*,0x11*,0x25*,0x34*,0x8c*,0x8d*,0xac*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a2710: 16-byte aligned: YES
  005a2710  55             PUSH EBP
  005a2711  8B EC          MOV EBP,ESP
  005a2713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2710
string literals (0):
direct callees (13):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4E60  CGameEffect::GetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 6):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2710 FUN_005a2710 size=1376 ret=12 callers=0 callees=13

undefined4 FUN_005a2710(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952658;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    iVar1 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar2 = FUN_006bc570(0x11,iVar1);
    if (iVar2 != 0) {
      iVar2 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar2 == 0) {
        local_a0 = 0;
      }
      else {
        local_a0 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar1 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_a4 = 0;
        }
        else {
          local_a4 = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_a4);
        FUN_00584da0(0x8d,local_a4);
      }
      uVar5 = 0x8d;
      (**(code **)(*param_1 + 0x30))(0x8d,local_a0);
      FUN_00584da0(uVar5,local_a0);
      ExceptionList = local_10;
      return 1;
    }
    iVar2 = FUN_00919723(0x8c);
    local_8 = 2;
    if (iVar2 == 0) {
      local_a8 = 0;
    }
    else {
      local_a8 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    *(undefined2 *)(local_a8 + 8) = 0x25;
    FUN_005e4e80(0,0);
    uVar5 = FUN_005e4e60(0);
    FUN_005e4e80(1,uVar5);
    FUN_00544210(local_a8,param_3,0);
    iVar2 = FUN_00919723(0x8c);
    local_8 = 3;
/* ... 78 more lines truncated by --max-lines */
```

### decoy: 0x005a2c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2c70: 16-byte aligned: YES
  005a2c70  55             PUSH EBP
  005a2c71  8B EC          MOV EBP,ESP
  005a2c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2c70
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0xFD0 [dec 4048]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-9-28

GOG function: CSWSEffectListHandler::OnApplyForceJumpDelayed  gog 0x0074a0d0  callconv=(blank)  param_size_bytes=8  gog_size=1603  gog_retn=12
Candidate Steam address: 0x005a7820 (size 1603, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (AddMoveToPointActionToFront 0x5644b0,DesiredAttackRange 0x5716a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a7e70:STR0/CALLEE0/SIG0.537/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00749ec0 CSWSEffectListHandler::OnApplyForceJump -> 0x005a7610  gate_class=REVIEW
- GOG-order neighbour after:  0x0074a720 CSWSEffectListHandler::OnApplyForcePush -> 0x005a7e70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c7b0 CServerExoApp::TestSafeLocation -> Steam ?  in candidate callees: n/a
  0x0067d0e0 CSWSCreature::AddMoveToPointActionToFront -> Steam 0x005644b0  in candidate callees: yes
  0x0068a1d0 CSWSCreature::DesiredAttackRange -> Steam 0x005716a0  in candidate callees: yes
  0x00695ea0 CSWSCreature::SetLockOrientationToObject -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 7 more GOG callees
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xa*,0xc*,0x30*,0x8c*,0x90*,0x94*,0x98*,0x9c*,0x380*,0xb08*,0x10dc*,0x11b0*} | Steam {0x4*,0x8*,0xa*,0xc*,0x30*,0x8c*,0x90*,0x94*,0x98*,0x9c*,0x380*,0xb08*,0x10dc*,0x11b0*}
imm sets (* = in both, 13 common): GOG {0x4*,0x5*,0x6*,0x8*,0xc*,0xd*,0x10*,0x8c*,0x94*,0x13c*,0xffff*,0x7f000000*,0xfffffff8*} | Steam {0x4*,0x5*,0x6*,0x8*,0xc*,0xd*,0x10*,0x8c*,0x94*,0x13c*,0xffff*,0x7f000000*,0xfffffff8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a7820: 16-byte aligned: YES
  005a7820  55             PUSH EBP
  005a7821  8B EC          MOV EBP,ESP
  005a7823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a7820
string literals (0):
direct callees (15):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x005143C0  (unnamed)
  0x005716A0  CSWSCreature::DesiredAttackRange
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0051CD90  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x009196FD  operator delete
  0x00919723  operator new
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x90 [dec 144]  x7  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x94 [dec 148]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a7820 FUN_005a7820 size=1603 ret=12 callers=0 callees=15

undefined4 FUN_005a7820(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  int local_14c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined4 *local_7c;
  undefined4 local_78;
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  float local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_3c;
  undefined4 *local_38;
  float local_34;
  float local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 *local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952a3e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (local_14 != 0) {
    uVar1 = FUN_005e4ee0(0);
    local_18 = FUN_0051c100(uVar1);
    if (local_18 != 0) {
      FUN_0057d370(*(undefined4 *)(local_18 + 4),0);
      local_5c = *(float *)(local_18 + 0x94) - *(float *)(local_14 + 0x94);
      local_58 = *(float *)(local_18 + 0x98) - *(float *)(local_14 + 0x98);
      local_54 = *(float *)(local_18 + 0x9c) - *(float *)(local_14 + 0x9c);
      FUN_005143c0(&local_34,&local_5c);
      fVar3 = (float10)FUN_005716a0(*(undefined4 *)(local_18 + 4),1);
      local_60 = (float)fVar3;
      local_68 = local_60 * local_34;
      local_64 = local_60 * local_30;
      local_60 = local_60 * local_2c;
      FUN_004452b0(&local_28,local_18 + 0x94,&local_68);
      iVar2 = FUN_0051cd90(*(undefined4 *)(local_18 + 4),&local_28);
      if (iVar2 == 0) {
        fVar3 = (float10)FUN_005716a0(*(undefined4 *)(local_18 + 4),
                                      *(undefined4 *)(*(int *)(local_14 + 0x10dc) + 0xb08));
        local_6c = (float)fVar3;
        local_74 = local_6c * local_34;
        local_70 = local_6c * local_30;
        local_6c = local_6c * local_2c;
        FUN_0044c8a0(&local_48,local_18 + 0x94,&local_74);
        local_28 = *(undefined4 *)(local_18 + 0x94);
        local_24 = *(undefined4 *)(local_18 + 0x98);
        local_20 = *(undefined4 *)(local_18 + 0x9c);
        if (*(int *)(*(int *)(local_18 + 0x380) + 0x8c) != 0) {
          local_78 = *(undefined4 *)(*(int *)(local_18 + 0x380) + 0x90);
          *(undefined4 *)(*(int *)(local_18 + 0x380) + 0x90) = 0;
          local_4c = local_78;
          FUN_009196fd(local_78);
          local_4c = 0;
        }
/* ... 52 more lines truncated by --max-lines */
```

### decoy: 0x005a7e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a7e70: 16-byte aligned: YES
  005a7e70  55             PUSH EBP
  005a7e71  8B EC          MOV EBP,ESP
  005a7e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a7e70
string literals (0):
direct callees (15):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E4E60  CGameEffect::GetInteger
  0x005E4EA0  CGameEffect::GetFloat
  0x005143C0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00543F10  CSWSObject::SetOrientation
  0x0052BE60  CSWSArea::Constructor_2
  0x005453C0  CSWSObject::GetArea
  0x0051C910  (unnamed)
  0x00919723  operator new
  0x009196FD  operator delete
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x90 [dec 144]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX
  +0x10 [dec 16]  x2  via EDX
  +0x8C [dec 140]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

## Card B0-9-29

GOG function: CSWSEffectListHandler::OnApplySetState  gog 0x0073f490  callconv=(blank)  param_size_bytes=12  gog_size=3989  gog_retn=12
Candidate Steam address: 0x0059cba0 (size 3989, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059db50:STR0/CALLEE0/SIG0.321/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073f340 CSWSEffectListHandler::OnRemoveArcaneSpellFailure -> 0x0059ca50  gate_class=REVIEW
- GOG-order neighbour after:  0x00740440 CSWSEffectListHandler::OnRemoveSetState -> 0x0059db50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (17), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x006f4870 CSWSCreatureStats::HasFeat -> Steam ?  in candidate callees: n/a
  ... 9 more GOG callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0xa*,0x18*,0x1c*,0x20*,0x30*,0x145*,0x150*,0xabc*,0xff5*,0x10dc*,0x1198*} | Steam {0x4*,0x8*,0xa*,0x18*,0x1c*,0x20*,0x30*,0x145*,0x150*,0xabc*,0xff5*,0x10dc*,0x1198*}
imm sets (* = in both, 40 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+26)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+26)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059cba0: 16-byte aligned: YES
  0059cba0  55             PUSH EBP
  0059cba1  8B EC          MOV EBP,ESP
  0059cba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059cba0
string literals (0):
direct callees (17):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC9A0  CSWSCreatureStats::GetEffectLinkImmunity
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x00544790  CSWSObject::RemoveEffectById
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
struct offsets touched (top 10):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x30 [dec 48]  x10  via EDX
  +0x150 [dec 336]  x9  via ECX,EDX
  +0x8 [dec 8]  x5  via EAX,EDX
  +0xA [dec 10]  x2  via EDX
  +0x18 [dec 24]  x2  via ECX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0xFF5 [dec 4085]  x2  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x145 [dec 325]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059cba0 FUN_0059cba0 size=3989 ret=12 callers=0 callees=17

undefined4 FUN_0059cba0(int *param_1,int param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  undefined4 uVar8;
  int local_198;
  int local_194;
  undefined4 local_190;
  undefined4 local_18c;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  undefined4 local_168;
  undefined4 local_164;
  undefined4 local_160;
  undefined4 local_15c;
  undefined4 local_158;
  undefined4 local_154;
  undefined4 local_150;
  undefined4 local_14c;
  int local_3c;
  undefined1 local_20;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952230;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_14 = 0;
  if ((iVar2 != 0) && (iVar4 = *(int *)(iVar2 + 0x1198), param_2 != 0)) {
    iVar3 = FUN_005e4e60(1);
    if ((iVar3 == 0) && ((iVar4 != 0 && (iVar4 = FUN_006bc9a0(param_2), iVar4 != 0)))) {
      ExceptionList = local_10;
      return 1;
    }
    iVar4 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar5 = FUN_005e4e60(0);
    local_20 = (undefined1)iVar5;
    if (((iVar4 != 0) && (*(int *)(iVar4 + 0x10dc) != 0)) &&
       (local_14 = FUN_005908c0(*(undefined1 *)(*(int *)(iVar4 + 0x10dc) + 0xabc)), local_14 != 0))
    {
      *(undefined1 *)(local_14 + 0x145) = local_20;
    }
    if (iVar5 == 6) {
      iVar3 = FUN_006bc570(0xd,iVar4);
      if (iVar3 == 1) {
        iVar2 = FUN_00919723(0x34);
        local_8 = 0;
        if (iVar2 == 0) {
          local_14c = 0;
        }
        else {
          local_14c = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e1c70(0,param_1[1 = byte +0x4]);
        if (iVar4 != 0) {
          iVar2 = FUN_00919723(0x34);
          local_8 = 1;
          if (iVar2 == 0) {
            local_150 = 0;
          }
          else {
            local_150 = FUN_005e2540();
/* ... 335 more lines truncated by --max-lines */
```

### decoy: 0x0059db50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059db50: 16-byte aligned: YES
  0059db50  55             PUSH EBP
  0059db51  8B EC          MOV EBP,ESP
  0059db53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059db50
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 8):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EDX
  +0xEC [dec 236]  x2  via EAX,ECX
  +0xFF5 [dec 4085]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x20 [dec 32]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-30

GOG function: CSWSEffectListHandler::OnRemoveEnemyAttackBonus  gog 0x0073f180  callconv=(blank)  param_size_bytes=8  gog_size=279  gog_retn=8
Candidate Steam address: 0x0059c890 (size 279, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ca50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073f0e0 CSWSEffectListHandler::OnApplyEnemyAttackBonus -> 0x0059c7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f2a0 CSWSEffectListHandler::OnApplyArcaneSpellFailure -> 0x0059c9b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x107*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x107*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x8*,0x18*,0x2c*,0x148*} | Steam {0x8*,0x18*,0x2c*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059c890: 16-byte aligned: YES
  0059c890  55             PUSH EBP
  0059c891  8B EC          MOV EBP,ESP
  0059c893  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c890
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x107 [dec 263]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c890 FUN_0059c890 size=279 ret=8 callers=0 callees=1

undefined4 __thiscall FUN_0059c890(undefined4 param_1,int *param_2,int param_3)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  char local_c;
  int local_8;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    iVar2 = (**(code **)(*param_2 + 0x30))();
    piVar3 = (int *)(iVar2 + 0x148);
    local_8 = 0;
    local_c = '\0';
    if ((param_3 != 0) && (cVar1 = '\0', 0 < *(int *)(iVar2 + 0x14c))) {
      for (; local_c = cVar1, local_8 < *(int *)(iVar2 + 0x14c); local_8 = local_8 + 1) {
        if (0x18 < *(ushort *)(*(int *)(*piVar3 + local_8 * 4) + 8)) break;
        if ((*(short *)(*(int *)(*piVar3 + local_8 * 4) + 8) == 0x18) &&
           (*(int *)(*piVar3 + local_8 * 4) != param_3)) {
          cVar1 = FUN_005e4e60(0);
          local_c = cVar1 + local_c;
        }
        cVar1 = local_c;
      }
    }
    *(char *)(*(int *)(iVar2 + 0x1198) + 0x107) = local_c;
  }
  return 1;
}
```

### decoy: 0x0059ca50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ca50: 16-byte aligned: YES
  0059ca50  55             PUSH EBP
  0059ca51  8B EC          MOV EBP,ESP
  0059ca53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ca50
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x0040E770  (unnamed)
struct offsets touched (top 5):
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-9-31

GOG function: CSWSEffectListHandler::OnApplyMissChance  gog 0x00748790  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=12
Candidate Steam address: 0x005a5ea0 (size 52, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a5ee0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748650 CSWSEffectListHandler::OnRemoveBlindness -> 0x005a5d60  gate_class=AUTO
- GOG-order neighbour after:  0x007487d0 CSWSEffectListHandler::OnApplyDarkness -> 0x005a5ee0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x64*} | Steam {0xc*,0x64*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x005a5ea0: 16-byte aligned: YES
  005a5ea0  55             PUSH EBP
  005a5ea1  8B EC          MOV EBP,ESP
  005a5ea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a5ea0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005a5ea0 FUN_005a5ea0 size=52 ret=12 callers=0 callees=1

undefined4 FUN_005a5ea0(void)

{
  int iVar1;
  
  iVar1 = FUN_005e4e60(0);
  if ((iVar1 < 0x65) && (iVar1 = FUN_005e4e60(0), 0 < iVar1)) {
    return 0;
  }
  return 1;
}
```

### decoy: 0x005a5ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a5ee0: 16-byte aligned: YES
  005a5ee0  55             PUSH EBP
  005a5ee1  8B EC          MOV EBP,ESP
  005a5ee3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a5ee0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x122 [dec 290]  x1  via EAX
```

## Card B0-9-32

GOG function: CSWPartyTable::SetSoloMode  gog 0x007048a0  callconv=(blank)  param_size_bytes=8  gog_size=47  gog_retn=8
Candidate Steam address: 0x005fed30 (size 41, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::SetPlanetAvailable
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005fed00:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00704810 CSWPartyTable::GetTotalPlayedSeconds -> 0x005fee60  gate_class=AUTO
- GOG-order neighbour after:  0x007048d0 CSWPartyTable::UnstealthParty -> 0x005fef20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007048d0 CSWPartyTable::UnstealthParty -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x238} | Steam {0x88}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005fed30: 16-byte aligned: YES
  005fed30  55             PUSH EBP
  005fed31  8B EC          MOV EBP,ESP
  005fed33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005fed30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005fed30 FUN_005fed30 size=41 ret=8 callers=1 callees=0

void __thiscall FUN_005fed30(int param_1,uint param_2,undefined4 param_3)

{
  if (param_2 < 0x10) {
    *(undefined4 *)(param_1 + 0x88 + param_2 * 4) = param_3;
  }
  return;
}
```

### decoy: 0x005fed00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005fed00: 16-byte aligned: YES
  005fed00  55             PUSH EBP
  005fed01  8B EC          MOV EBP,ESP
  005fed03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005fed00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-9-33

GOG function: CSWSEffectListHandler::OnRemoveEntangled  gog 0x0073e160  callconv=(blank)  param_size_bytes=4  gog_size=95  gog_retn=8
Candidate Steam address: 0x0059b3e0 (size 313, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnRemoveDamageImmunityDecrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059b2e0:STR0/CALLEE0/SIG0.196/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073de10 CSWSEffectListHandler::OnApplyEntangled -> 0x0059b520  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e1c0 CSWSEffectListHandler::OnApplyKnockdown -> 0x0059b8d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x30,0x10e8} | Steam {0x4,0x8,0xb0}
imm sets (* = in both, 1 common): GOG {0x8*,0xc} | Steam {0x8*,0x10,0x11,0x2c,0x148}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0059b3e0: 16-byte aligned: YES
  0059b3e0  55             PUSH EBP
  0059b3e1  8B EC          MOV EBP,ESP
  0059b3e3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b3e0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB0 [dec 176]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b3e0 FUN_0059b3e0 size=313 ret=8 callers=0 callees=2

undefined4 FUN_0059b3e0(int *param_1,int param_2)

{
  int iVar1;
  char cVar2;
  char cVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  undefined4 extraout_var;
  undefined4 uVar7;
  int local_8;
  
  if (param_2 != 0) {
    uVar5 = FUN_005e4e60(0);
    uVar7 = extraout_var;
    cVar2 = FUN_005e4e60(1);
    (**(code **)(*param_1 + 0xb0))(uVar5 & 0xff,uVar7);
    cVar4 = '\0';
    for (local_8 = 0; local_8 < param_1[0x53 = byte +0x14c]; local_8 = local_8 + 1) {
      iVar1 = *(int *)(param_1[0x52 = byte +0x148] + local_8 * 4);
      if (*(short *)(iVar1 + 8) == 0x10) {
        uVar6 = FUN_005e4e60(0);
        if (uVar6 != uVar5) goto LAB_0059b493;
LAB_0059b4b6:
        if (*(short *)(iVar1 + 8) == 0x10) {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar3 + cVar4;
        }
        else {
          cVar3 = FUN_005e4e60(1);
          cVar4 = cVar4 - cVar3;
        }
      }
      else {
LAB_0059b493:
        if (*(short *)(iVar1 + 8) == 0x11) {
          uVar6 = FUN_005e4e60(0);
          if (uVar6 == uVar5) goto LAB_0059b4b6;
        }
      }
    }
    FUN_005431f0(uVar5 & 0xffff,cVar4 - cVar2);
  }
  return 1;
}
```

### decoy: 0x0059b2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b2e0: 16-byte aligned: YES
  0059b2e0  55             PUSH EBP
  0059b2e1  8B EC          MOV EBP,ESP
  0059b2e3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b2e0
string literals (0):
direct callees (4):
  0x005E4E60  CGameEffect::GetInteger
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x005431F0  (unnamed)
struct offsets touched (top 6):
  +0x30 [dec 48]  x2  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xFC [dec 252]  x1  via EAX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-9-34

GOG function: CSWSEffectListHandler::OnApplyForceJump  gog 0x00749ec0  callconv=(blank)  param_size_bytes=8  gog_size=513  gog_retn=12
Candidate Steam address: 0x005a7610 (size 513, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a7820:STR0/CALLEE0/SIG0.254/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00749e00 CSWSEffectListHandler::OnApplyForceDrain -> 0x005a7520  gate_class=AUTO
- GOG-order neighbour after:  0x0074a0d0 CSWSEffectListHandler::OnApplyForceJumpDelayed -> 0x005a7820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00695ea0 CSWSCreature::SetLockOrientationToObject -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00734ba0 CGameEffect::GetObjectID -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xa*,0x30*} | Steam {0x4*,0x8*,0xa*,0x30*}
imm sets (* = in both, 9 common): GOG {0x4*,0xc*,0x1e*,0x4c*,0x67*,0x8c*,0x12c*,0x3ea*,0xfffffff8*} | Steam {0x4*,0xc*,0x1e*,0x4c*,0x67*,0x8c*,0x12c*,0x3ea*,0xfffffff8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a7610: 16-byte aligned: YES
  005a7610  55             PUSH EBP
  005a7611  8B EC          MOV EBP,ESP
  005a7613  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a7610
string literals (0):
direct callees (10):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4F00  CGameEffect::SetObjectID
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x4  via EAX,EDX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a7610 FUN_005a7610 size=513 ret=12 callers=0 callees=10

undefined4 FUN_005a7610(int *param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  int local_5c;
  int local_58;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952a06;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    uVar2 = FUN_005e4ee0(0);
    iVar3 = FUN_0051c100(uVar2);
    if (iVar3 != 0) {
      FUN_0057d370(*(undefined4 *)(iVar3 + 4),0);
      iVar4 = FUN_00919723(0x8c);
      local_8 = 0;
      if (iVar4 == 0) {
        local_58 = 0;
      }
      else {
        local_58 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_58 + 8) = 0x67;
      *(ushort *)(local_58 + 0xa [dec 10]) = *(ushort *)(local_58 + 0xa [dec 10]) & 0xfff8 | DAT_009967b4;
      FUN_005e4f00(0,*(undefined4 *)(iVar3 + 4));
      uVar2 = *(undefined4 *)(iVar1 + 4);
      uVar7 = *(undefined4 *)(iVar1 + 4);
      uVar6 = 300;
      uVar5 = 0;
      uVar8 = DAT_00992408;
      FUN_0051c280(0,300,uVar7,uVar2,DAT_00992408,local_58);
      FUN_0051e170(uVar5,uVar6,uVar7,uVar2,uVar8,local_58);
      iVar1 = FUN_00919723(0x8c);
      local_8 = 1;
      if (iVar1 == 0) {
        local_5c = 0;
      }
      else {
        local_5c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_5c + 8) = 0x1e;
      *(ushort *)(local_5c + 0xa [dec 10]) = *(ushort *)(local_5c + 0xa [dec 10]) & 0xfff8 | (ushort)PTR_009967ac;
      FUN_005e4e80(0,0x3ea);
      FUN_00544210(local_5c,0,0);
    }
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005a7820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a7820: 16-byte aligned: YES
  005a7820  55             PUSH EBP
  005a7821  8B EC          MOV EBP,ESP
  005a7823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a7820
string literals (0):
direct callees (15):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x005143C0  (unnamed)
  0x005716A0  CSWSCreature::DesiredAttackRange
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0051CD90  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x009196FD  operator delete
  0x00919723  operator new
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x90 [dec 144]  x7  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x94 [dec 148]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,EDX
```

## Card B0-9-35

GOG function: CResGFF::GetField_2  gog 0x00623b60  callconv=(blank)  param_size_bytes=8  gog_size=156  gog_retn=8
Candidate Steam address: 0x007178e0 (size 269, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::GetFieldByLabel
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.45 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00717a00:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00623a40 CResGFF::GetFieldByLabel -> 0x007178e0  gate_class=AUTO
- GOG-order neighbour after:  0x00623c00 CResGFF::InitializeForWriting -> 0x00717aa0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x14,0x2c,0x40*,0x4c,0x64} | Steam {0x4*,0x8*,0xc,0x1c,0x40*,0x54}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*,0x10,0x34}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007178e0: 16-byte aligned: YES
  007178e0  55             PUSH EBP
  007178e1  8B EC          MOV EBP,ESP
  007178e3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007178e0
string literals (0):
direct callees (6):
  0x0091C630  (unnamed)
  0x0091D0F0  (unnamed)
  0x00717720  CResGFF::GetStruct
  0x007186C0  CResGFF::GetFieldCount
  0x00717A00  CResGFF::GetField_2
  0x009196EE  __security_check_cookie
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x54 [dec 84]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x40 [dec 64]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007178e0 FUN_007178e0 size=269 ret=8 callers=20 callees=6

void __thiscall FUN_007178e0(int param_1,undefined4 param_2,char *param_3)

{
  undefined4 uVar1;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  uint local_1c;
  uint local_18;
  uint local_14;
  int *local_10;
  int local_c;
  int *local_8;
  
  local_1c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if ((param_3 != (char *)0x0) && (*(int *)(param_1 + 0x54) != 0)) {
    _memset [GHIDRA-AUTO-LABEL: unreliable](&local_2c,0,0x10);
    _strncpy [GHIDRA-AUTO-LABEL: unreliable]((char *)&local_2c,param_3,0x10);
    uVar1 = FUN_00717720(param_2);
    local_18 = FUN_007186c0(uVar1);
    local_14 = 0;
    while ((((local_14 < local_18 && (local_c = FUN_00717a00(uVar1,local_14), local_c != 0)) &&
            (*(uint *)(local_c + 4) < *(uint *)(*(int *)(param_1 + 0x40) + 0x1c))) &&
           (((local_10 = (int *)(*(int *)(local_c + 4) * 0x10 + *(int *)(param_1 + 0x54)),
             local_8 = local_10, local_2c != *local_10 || (local_28 != local_10[1 = byte +0x4])) ||
            ((local_24 != local_10[2 = byte +0x8] || (local_20 != local_10[3 = byte +0xc]))))))) {
      local_14 = local_14 + 1;
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_1c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x00717a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00717a00: 16-byte aligned: YES
  00717a00  55             PUSH EBP
  00717a01  8B EC          MOV EBP,ESP
  00717a03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00717a00
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x4C [dec 76]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via ECX,EDX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B0-9-36

GOG function: CSWSEffectListHandler::OnRemoveArcaneSpellFailure  gog 0x0073f340  callconv=(blank)  param_size_bytes=8  gog_size=326  gog_retn=8
Candidate Steam address: 0x0059ca50 (size 326, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c890:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0073f2a0 CSWSEffectListHandler::OnApplyArcaneSpellFailure -> 0x0059c9b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f490 CSWSEffectListHandler::OnApplySetState -> 0x0059cba0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x0084bb80 Global::return_void_0x4 -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x384*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x384*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x8*,0x19*,0x30*,0x148*} | Steam {0x8*,0x19*,0x30*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059ca50: 16-byte aligned: YES
  0059ca50  55             PUSH EBP
  0059ca51  8B EC          MOV EBP,ESP
  0059ca53  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ca50
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x0040E770  (unnamed)
struct offsets touched (top 5):
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ca50 FUN_0059ca50 size=326 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059ca50(undefined4 param_1,int *param_2,int param_3)

{
  char cVar1;
  char cVar2;
  int iVar3;
  int *piVar4;
  int local_8;
  
  iVar3 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar3 != 0) {
    iVar3 = (**(code **)(*param_2 + 0x30))();
    piVar4 = (int *)(iVar3 + 0x148);
    local_8 = 0;
    cVar2 = '\0';
    if ((param_3 != 0) && (0 < *(int *)(iVar3 + 0x14c))) {
      for (; local_8 < *(int *)(iVar3 + 0x14c); local_8 = local_8 + 1) {
        if (0x19 < *(ushort *)(*(int *)(*piVar4 + local_8 * 4) + 8)) break;
        if ((*(short *)(*(int *)(*piVar4 + local_8 * 4) + 8) == 0x19) &&
           (*(int *)(*piVar4 + local_8 * 4) != param_3)) {
          cVar1 = FUN_005e4e60(0);
          cVar2 = cVar1 + cVar2;
        }
      }
    }
    iVar3 = (**(code **)(*param_2 + 0x30))();
    if (iVar3 != 0) {
      iVar3 = (**(code **)(*param_2 + 0x30))();
      *(undefined4 *)(iVar3 + 0x384 [dec 900]) = 1;
    }
    FUN_0040e770(cVar2);
  }
  return 1;
}
```

### decoy: 0x0059c890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c890: 16-byte aligned: YES
  0059c890  55             PUSH EBP
  0059c891  8B EC          MOV EBP,ESP
  0059c893  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c890
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x107 [dec 263]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-37

GOG function: CSWSEffectListHandler::OnApplySetStateInternal  gog 0x00740720  callconv=(blank)  param_size_bytes=12  gog_size=2536  gog_retn=12
Candidate Steam address: 0x0059de30 (size 2536, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059db50:STR0/CALLEE0/SIG0.244/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00740440 CSWSEffectListHandler::OnRemoveSetState -> 0x0059db50  gate_class=REVIEW
- GOG-order neighbour after:  0x00741150 CSWSEffectListHandler::OnRemoveSetStateInternal -> 0x0059e860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x00663170 CSWSCreature::SetAttackSummaryFeedbackEffectData -> Steam ?  in candidate callees: n/a
  0x006934c0 CSWSCreature::RecomputeAmbientAnimationState -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0x18*,0x30*,0xec*,0x122*,0x145*,0x154*,0x390*,0x394*,0xabc*,0x10dc*} | Steam {0x4*,0x8*,0x18*,0x30*,0xec*,0x122*,0x145*,0x154*,0x390*,0x394*,0xabc*,0x10dc*}
imm sets (* = in both, 22 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x11*,0x17*,0x1b*,...(+8)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0x11*,0x17*,0x1b*,...(+8)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0059de30: 16-byte aligned: YES
  0059de30  55             PUSH EBP
  0059de31  8B EC          MOV EBP,ESP
  0059de33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059de30
string literals (0):
direct callees (13):
  0x005E4E60  CGameEffect::GetInteger
  0x00541080  CSWSObject::ClearAllActions
  0x00590430  CSWSCombatRound::RemoveAllActions
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4FA0  CGameEffect::GetExpiryTime
  0x005E4F80  CGameEffect::SetExpiryTime
  0x00544210  CSWSObject::ApplyEffect
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005908C0  CSWSCombatRound::GetAttack
  0x006E6E70  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x14  via EAX,ECX,EDX
  +0x10DC [dec 4316]  x3  via EAX,ECX,EDX
  +0xEC [dec 236]  x2  via EAX
  +0x122 [dec 290]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x145 [dec 325]  x1  via ECX
  +0x154 [dec 340]  x1  via EAX
  +0x390 [dec 912]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059de30 FUN_0059de30 size=2536 ret=12 callers=0 callees=13

undefined4 FUN_0059de30(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  bool bVar2;
  int local_12c;
  int local_128;
  int local_124;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  int local_100;
  int local_fc;
  undefined4 local_38;
  int local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095232d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_20 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_18 = 0;
  if ((local_20 != 0) && (param_2 != 0)) {
    local_14 = FUN_005e4e60(0);
    local_28 = 0;
    *(undefined4 *)(local_20 + 0x390) = 0;
    *(undefined4 *)(local_20 + 0x394) = 1;
    local_18 = 0;
    bVar2 = local_14 != 9;
    if (bVar2) {
      *(undefined4 *)(local_20 + 0xec) = 1;
      FUN_00541080(1);
      FUN_00590430();
      *(undefined4 *)(local_20 + 0xec) = 0;
    }
    local_18 = (uint)bVar2;
    switch(local_14) {
    case 2:
      iVar1 = FUN_00919723(0x8c);
      local_8 = 4;
      if (iVar1 == 0) {
        local_10c = 0;
      }
      else {
        local_10c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      *(undefined2 *)(local_10c + 8) = 0x1b;
      FUN_005e4e80(0,2);
      FUN_005e4e80(1,0);
      FUN_005e4e80(3,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
      FUN_00544210(local_10c,param_3,0);
      break;
    case 3:
      iVar1 = FUN_00919723(0x8c);
      local_8 = 5;
      if (iVar1 == 0) {
        local_110 = 0;
      }
      else {
        local_110 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
/* ... 187 more lines truncated by --max-lines */
```

### decoy: 0x0059db50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059db50: 16-byte aligned: YES
  0059db50  55             PUSH EBP
  0059db51  8B EC          MOV EBP,ESP
  0059db53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0059db50
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 8):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX,EDX
  +0xA [dec 10]  x2  via EDX
  +0xEC [dec 236]  x2  via EAX,ECX
  +0xFF5 [dec 4085]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x20 [dec 32]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-38

GOG function: CSWSEffectListHandler::OnRemoveForceResistanceIncrease  gog 0x007420a0  callconv=(blank)  param_size_bytes=8  gog_size=503  gog_retn=8
Candidate Steam address: 0x0059f7d0 (size 503, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059fae0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741fe0 CSWSEffectListHandler::OnApplyForceResistanceIncrease -> 0x0059f710  gate_class=AUTO
- GOG-order neighbour after:  0x007422a0 CSWSEffectListHandler::OnApplyForceResistanceDecrease -> 0x0059f9d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006f8850 CSWSCreatureStats::SetForceResistance -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x30*,0x384*,0x1198*} | Steam {0x4*,0x8*,0x30*,0x384*,0x1198*}
imm sets (* = in both, 5 common): GOG {0x8*,0x21*,0x22*,0x64*,0x148*} | Steam {0x8*,0x21*,0x22*,0x64*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059f7d0: 16-byte aligned: YES
  0059f7d0  55             PUSH EBP
  0059f7d1  8B EC          MOV EBP,ESP
  0059f7d3  83 EC 64       SUB ESP,0x64
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059f7d0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC3D0  CSWSCreatureStats::SetForceResistance
struct offsets touched (top 5):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EDX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059f7d0 FUN_0059f7d0 size=503 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059f7d0(undefined4 param_1,int *param_2,int *param_3)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int local_18;
  int local_c;
  int local_8;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    (**(code **)(*param_2 + 0x30))();
    piVar3 = param_2 + 0x52;
    local_8 = 0;
    local_c = 0;
    for (local_18 = 0; local_18 < param_2[0x53 = byte +0x14c]; local_18 = local_18 + 1) {
      piVar1 = *(int **)(*piVar3 + local_18 * 4);
      if ((*piVar1 != *param_3) || (piVar1[1 = byte +0x4] != param_3[1 = byte +0x4])) {
        if ((*(short *)(*(int *)(*piVar3 + local_18 * 4) + 8) == 0x21) ||
           (*(short *)(*(int *)(*piVar3 + local_18 * 4) + 8) == 0x22)) {
          if (*(short *)(*(int *)(*piVar3 + local_18 * 4) + 8) == 0x22) {
            local_c = FUN_005e4e60(0);
          }
          else {
            iVar2 = FUN_005e4e60(0);
            if (local_8 < iVar2) {
              local_8 = FUN_005e4e60(0);
            }
          }
        }
        else if (0x22 < *(ushort *)(*(int *)(*piVar3 + local_18 * 4) + 8)) break;
      }
    }
    iVar2 = (**(code **)(*param_2 + 0x30))();
    if (iVar2 != 0) {
      iVar2 = (**(code **)(*param_2 + 0x30))();
      *(undefined4 *)(iVar2 + 0x384 [dec 900]) = 1;
    }
    FUN_006bc3d0(local_8 - local_c);
  }
  return 1;
}
```

### decoy: 0x0059fae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059fae0: 16-byte aligned: YES
  0059fae0  55             PUSH EBP
  0059fae1  8B EC          MOV EBP,ESP
  0059fae3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059fae0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x006BC400  CSWSCreatureStats::SetForceResistancePenalty
struct offsets touched (top 5):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x384 [dec 900]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-39

GOG function: CSWSEffectListHandler::OnRemoveNegativeLevel  gog 0x00749540  callconv=(blank)  param_size_bytes=8  gog_size=138  gog_retn=8
Candidate Steam address: 0x005a6c60 (size 138, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a6d60:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00748eb0 CSWSEffectListHandler::OnApplyNegativeLevel -> 0x005a65d0  gate_class=AUTO
- GOG-order neighbour after:  0x007495d0 CSWSEffectListHandler::OnApplyBonusFeat -> 0x005a6cf0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006e97f0 CSWSCreatureStats::GetClassNegativeLevels -> Steam ?  in candidate callees: n/a
  0x006f2f30 CSWSCreatureStats::SetClassNegativeLevels -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x1198*} | Steam {0x30*,0x1198*}
imm sets (* = in both, 2 common): GOG {0x8*,0x18*} | Steam {0x8*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005a6c60: 16-byte aligned: YES
  005a6c60  55             PUSH EBP
  005a6c61  8B EC          MOV EBP,ESP
  005a6c63  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6c60
string literals (0):
direct callees (3):
  0x005E4E60  CGameEffect::GetInteger
  0x006AD370  CSWSCreatureStats::GetClassNegativeLevels
  0x006B6AB0  CSWSCreatureStats::SetClassNegativeLevels
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6c60 FUN_005a6c60 size=138 ret=8 callers=0 callees=3

undefined4 __thiscall FUN_005a6c60(undefined4 param_1,int *param_2)

{
  undefined1 uVar1;
  int iVar2;
  uint uVar3;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    (**(code **)(*param_2 + 0x30))();
    uVar1 = FUN_005e4e60(1);
    uVar3 = FUN_006ad370(uVar1);
    iVar2 = FUN_005e4e60(0);
    FUN_006b6ab0(uVar1,(uVar3 & 0xff) - iVar2);
  }
  return 1;
}
```

### decoy: 0x005a6d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a6d60: 16-byte aligned: YES
  005a6d60  55             PUSH EBP
  005a6d61  8B EC          MOV EBP,ESP
  005a6d63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a6d60
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005AA100  (unnamed)
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-40

GOG function: CSWSEffectListHandler::OnRemoveModifyNumAttacks  gog 0x00744f70  callconv=(blank)  param_size_bytes=8  gog_size=99  gog_retn=8
Candidate Steam address: 0x005a26a0 (size 99, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2640:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744f10 CSWSEffectListHandler::OnApplyModifyNumAttacks -> 0x005a2640  gate_class=REVIEW
- GOG-order neighbour after:  0x00744fe0 CSWSEffectListHandler::OnApplyCurse -> 0x005a2710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00776930 CSWSCombatRound::SetBonusEffectAttacks -> Steam ?  in candidate callees: n/a
  0x00776950 CSWSCombatRound::GetActualBonusEffectAttacks -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x10dc*} | Steam {0x30*,0x10dc*}
imm sets (* = in both, 2 common): GOG {0x8*,0x10*} | Steam {0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005a26a0: 16-byte aligned: YES
  005a26a0  55             PUSH EBP
  005a26a1  8B EC          MOV EBP,ESP
  005a26a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a26a0
string literals (0):
direct callees (3):
  0x005922A0  CSWSCombatRound::GetActualBonusEffectAttacks
  0x005E4E60  CGameEffect::GetInteger
  0x00592280  CSWSCombatRound::SetBonusEffectAttacks
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a26a0 FUN_005a26a0 size=99 ret=8 callers=0 callees=3

undefined4 FUN_005a26a0(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if (iVar1 != 0) {
    iVar1 = FUN_005922a0();
    iVar2 = FUN_005e4e60(0);
    FUN_00592280(iVar1 - iVar2);
  }
  return 1;
}
```

### decoy: 0x005a2640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2640: 16-byte aligned: YES
  005a2640  55             PUSH EBP
  005a2641  8B EC          MOV EBP,ESP
  005a2643  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a2640
string literals (0):
direct callees (3):
  0x005922A0  CSWSCombatRound::GetActualBonusEffectAttacks
  0x005E4E60  CGameEffect::GetInteger
  0x00592280  CSWSCombatRound::SetBonusEffectAttacks
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-41

GOG function: CSWSEffectListHandler::OnRemoveForcePush  gog 0x0074acc0  callconv=(blank)  param_size_bytes=4  gog_size=282  gog_retn=8
Candidate Steam address: 0x005a8410 (size 282, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a7e70:STR0/CALLEE0/SIG0.284/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074a720 CSWSEffectListHandler::OnApplyForcePush -> 0x005a7e70  gate_class=REVIEW
- GOG-order neighbour after:  0x0074ade0 CSWSEffectListHandler::OnRemoveHorrified -> 0x005a8530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00695ea0 CSWSCreature::SetLockOrientationToObject -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (1): '33#@EffectType'
disp sets (* = in both, 5 common): GOG {0x8*,0xa*,0xc*,0x18*,0x30*} | Steam {0x8*,0xa*,0xc*,0x18*,0x30*}
imm sets (* = in both, 8 common): GOG {0x4*,0x8*,0x17*,0x20*,0x8c*,0x7f000000*,0xfffffea1*,0xfffffff8*} | Steam {0x4*,0x8*,0x17*,0x20*,0x8c*,0x7f000000*,0xfffffea1*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a8410: 16-byte aligned: YES
  005a8410  55             PUSH EBP
  005a8411  8B EC          MOV EBP,ESP
  005a8413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a8410
string literals (0):
direct callees (6):
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8410 FUN_005a8410 size=282 ret=8 callers=0 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005a8410(int *param_1)

{
  int iVar1;
  int local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f0fb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    FUN_0057d370(0x7f000000,0);
    iVar1 = FUN_00919723(0x8c);
    local_8 = 0;
    if (iVar1 == 0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    *(ushort *)(local_30 + 0xa [dec 10]) = *(ushort *)(local_30 + 0xa [dec 10]) & 0xfff8 | DAT_009967b4;
    *(undefined4 *)(local_30 + 0xc) = _DAT_009963dc;
    FUN_005e5ea0(*(undefined4 *)(local_30 + 0x18));
    *(undefined2 *)(local_30 + 8) = 0x17;
    FUN_005e4e80(0,0xfffffea1);
    FUN_00544210(local_30,0,0);
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005a7e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a7e70: 16-byte aligned: YES
  005a7e70  55             PUSH EBP
  005a7e71  8B EC          MOV EBP,ESP
  005a7e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a7e70
string literals (0):
direct callees (15):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E4E60  CGameEffect::GetInteger
  0x005E4EA0  CGameEffect::GetFloat
  0x005143C0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00543F10  CSWSObject::SetOrientation
  0x0052BE60  CSWSArea::Constructor_2
  0x005453C0  CSWSObject::GetArea
  0x0051C910  (unnamed)
  0x00919723  operator new
  0x009196FD  operator delete
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x90 [dec 144]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX
  +0x10 [dec 16]  x2  via EDX
  +0x8C [dec 140]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

## Card B0-9-42

GOG function: CSWSEffectListHandler::OnApplyBodyFuel  gog 0x0074b090  callconv=(blank)  param_size_bytes=4  gog_size=52  gog_retn=12
Candidate Steam address: 0x005a87e0 (size 52, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a8820:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074af10 CSWSEffectListHandler::OnApplyHealForcePoints -> 0x005a8660  gate_class=REVIEW
- GOG-order neighbour after:  0x0074b0d0 CSWSEffectListHandler::OnRemoveBodyFuel -> 0x005a8820  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x11c4*} | Steam {0x30*,0x11c4*}
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
prologue for 0x005a87e0: 16-byte aligned: YES
  005a87e0  55             PUSH EBP
  005a87e1  8B EC          MOV EBP,ESP
  005a87e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a87e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a87e0 FUN_005a87e0 size=52 ret=12 callers=0 callees=0

undefined4 __thiscall FUN_005a87e0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    *(undefined4 *)(iVar1 + 0x11c4) = 1;
  }
  return 0;
}
```

### decoy: 0x005a8820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8820: 16-byte aligned: YES
  005a8820  55             PUSH EBP
  005a8821  8B EC          MOV EBP,ESP
  005a8823  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a8820
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x1  via EDX
  +0x11C4 [dec 4548]  x1  via ECX
```

## Card B0-9-43

GOG function: CSWSEffectListHandler::OnApplyDamageForcePoints  gog 0x0074ae10  callconv=(blank)  param_size_bytes=8  gog_size=245  gog_retn=12
Candidate Steam address: 0x005a8560 (size 245, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a8660:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074ade0 CSWSEffectListHandler::OnRemoveHorrified -> 0x005a8530  gate_class=REVIEW
- GOG-order neighbour after:  0x0074af10 CSWSEffectListHandler::OnApplyHealForcePoints -> 0x005a8660  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x30*,0x130*,0x132*,0x1198*} | Steam {0x30*,0x130*,0x132*,0x1198*}
imm sets (* = in both, 2 common): GOG {0xc*,0x28*} | Steam {0xc*,0x28*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a8560: 16-byte aligned: YES
  005a8560  55             PUSH EBP
  005a8561  8B EC          MOV EBP,ESP
  005a8563  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a8560
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 4):
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x4  via EAX,ECX,EDX
  +0x132 [dec 306]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8560 FUN_005a8560 size=245 ret=12 callers=0 callees=1

/* WARNING: Removing unreachable block (ram,0x005a85bd) */
/* WARNING: Removing unreachable block (ram,0x005a85fe) */

undefined4 FUN_005a8560(int *param_1)

{
  int iVar1;
  int iVar2;
  undefined2 local_c;
  
  local_c = 0;
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if ((iVar1 != 0) && (*(int *)(iVar1 + 0x1198) != 0)) {
    iVar2 = FUN_005e4e60(0);
    if (iVar2 < (short)(*(short *)(*(int *)(iVar1 + 0x1198) + 0x130) +
                       *(short *)(*(int *)(iVar1 + 0x1198) + 0x132))) {
      local_c = (*(short *)(*(int *)(iVar1 + 0x1198) + 0x130) +
                *(short *)(*(int *)(iVar1 + 0x1198) + 0x132)) - (short)iVar2;
    }
    *(short *)(*(int *)(iVar1 + 0x1198) + 0x130) = local_c;
  }
  return 1;
}
```

### decoy: 0x005a8660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8660: 16-byte aligned: YES
  005a8660  55             PUSH EBP
  005a8661  8B EC          MOV EBP,ESP
  005a8663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a8660
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 5):
  +0x130 [dec 304]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x132 [dec 306]  x1  via ECX
```

## Card B0-9-44

GOG function: CSWSEffectListHandler::OnApplyEnemyAttackBonus  gog 0x0073f0e0  callconv=(blank)  param_size_bytes=8  gog_size=152  gog_retn=12
Candidate Steam address: 0x0059c7f0 (size 152, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c890:STR0/CALLEE0/SIG0.35/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073ee40 CSWSEffectListHandler::OnApplyDeaf -> 0x0059c550  gate_class=REVIEW
- GOG-order neighbour after:  0x0073f180 CSWSEffectListHandler::OnRemoveEnemyAttackBonus -> 0x0059c890  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x30*,0xfc*,0x107*,0x1198*} | Steam {0x30*,0xfc*,0x107*,0x1198*}
imm sets (* = in both, 2 common): GOG {0xc*,0x18*} | Steam {0xc*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059c7f0: 16-byte aligned: YES
  0059c7f0  55             PUSH EBP
  0059c7f1  8B EC          MOV EBP,ESP
  0059c7f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059c7f0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 4):
  +0x30 [dec 48]  x3  via EDX
  +0x107 [dec 263]  x2  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059c7f0 FUN_0059c7f0 size=152 ret=12 callers=0 callees=1

undefined4 __thiscall FUN_0059c7f0(undefined4 param_1,int *param_2,int param_3)

{
  char cVar1;
  char cVar2;
  int iVar3;
  
  iVar3 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar3 != 0) {
    iVar3 = (**(code **)(*param_2 + 0x30))();
    if (*(int *)(iVar3 + 0xfc) != 0) {
      return 1;
    }
    iVar3 = (**(code **)(*param_2 + 0x30))();
    iVar3 = *(int *)(iVar3 + 0x1198);
    if (param_3 != 0) {
      cVar1 = *(char *)(iVar3 + 0x107);
      cVar2 = FUN_005e4e60(0);
      *(char *)(iVar3 + 0x107) = cVar1 + cVar2;
    }
  }
  return 0;
}
```

### decoy: 0x0059c890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c890: 16-byte aligned: YES
  0059c890  55             PUSH EBP
  0059c891  8B EC          MOV EBP,ESP
  0059c893  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c890
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x107 [dec 263]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-9-45

GOG function: CSWSEffectListHandler::OnApplyLight  gog 0x00744d20  callconv=(blank)  param_size_bytes=12  gog_size=215  gog_retn=12
Candidate Steam address: 0x005a2450 (size 215, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2530:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744940 CSWSEffectListHandler::OnRemoveSanctuary -> 0x005a2070  gate_class=AUTO
- GOG-order neighbour after:  0x00744e00 CSWSEffectListHandler::OnApplyVision -> 0x005a2530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x00734780 CGameEffect::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x30*} | Steam {0x8*,0x30*}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0x14*,0x1e*,0x8c*} | Steam {0x4*,0xc*,0x14*,0x1e*,0x8c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a2450: 16-byte aligned: YES
  005a2450  55             PUSH EBP
  005a2451  8B EC          MOV EBP,ESP
  005a2453  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2450
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E60  CGameEffect::GetInteger
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2450 FUN_005a2450 size=215 ret=12 callers=0 callees=5

undefined4 FUN_005a2450(int *param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  int local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949bbb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar1 = FUN_00919723(0x8c);
    local_8 = 0;
    if (iVar1 == 0) {
      local_24 = 0;
    }
    else {
      local_24 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    *(undefined2 *)(local_24 + 8) = 0x1e;
    uVar2 = FUN_005e4e60(0);
    FUN_005e4e80(0,uVar2);
    uVar2 = 0;
    (**(code **)(*param_1 + 0x30))(local_24,param_3,0);
    FUN_00544210(local_24,param_3,uVar2);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005a2530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2530: 16-byte aligned: YES
  005a2530  55             PUSH EBP
  005a2531  8B EC          MOV EBP,ESP
  005a2533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2530
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-46

GOG function: CSWSEffectListHandler::OnRemoveBonusFeat  gog 0x00749640  callconv=(blank)  param_size_bytes=8  gog_size=101  gog_retn=8
Candidate Steam address: 0x005a6d60 (size 101, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a6c60:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007495d0 CSWSEffectListHandler::OnApplyBonusFeat -> 0x005a6cf0  gate_class=AUTO
- GOG-order neighbour after:  0x007496b0 CSWSEffectListHandler::OnApplyWounding -> 0x005a6dd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x004c75e0 unnamed -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x1198*} | Steam {0x30*,0x1198*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0x18*} | Steam {0x8*,0x10*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005a6d60: 16-byte aligned: YES
  005a6d60  55             PUSH EBP
  005a6d61  8B EC          MOV EBP,ESP
  005a6d63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6d60
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005AA100  (unnamed)
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6d60 FUN_005a6d60 size=101 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_005a6d60(undefined4 param_1,int *param_2)

{
  undefined2 uVar1;
  int iVar2;
  
  iVar2 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar2 != 0) {
    uVar1 = FUN_005e4e60(0);
    (**(code **)(*param_2 + 0x30))();
    FUN_005aa100(uVar1);
  }
  return 1;
}
```

### decoy: 0x005a6c60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a6c60: 16-byte aligned: YES
  005a6c60  55             PUSH EBP
  005a6c61  8B EC          MOV EBP,ESP
  005a6c63  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a6c60
string literals (0):
direct callees (3):
  0x005E4E60  CGameEffect::GetInteger
  0x006AD370  CSWSCreatureStats::GetClassNegativeLevels
  0x006B6AB0  CSWSCreatureStats::SetClassNegativeLevels
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via ECX,EDX
```

## Card B0-9-47

GOG function: CSWSEffectListHandler::OnRemoveEntangled  gog 0x0073e160  callconv=(blank)  param_size_bytes=4  gog_size=95  gog_retn=8
Candidate Steam address: 0x0059be80 (size 250, retn 8)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnRemoveKnockdown
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059bf80:STR0/CALLEE0/SIG0.095/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073de10 CSWSEffectListHandler::OnApplyEntangled -> 0x0059b520  gate_class=REVIEW
- GOG-order neighbour after:  0x0073e1c0 CSWSEffectListHandler::OnApplyKnockdown -> 0x0059b8d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006dbb50 CSWSCreature::UpdateVisibleList -> Steam ?  in candidate callees: n/a
  0x006dc130 CSWSCreature::ClearVisibleList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*,0x10e8} | Steam {0x30*,0x7c,0x94,0x9c,0xec,0xaa8,0x10dc,0x11ac}
imm sets (* = in both, 1 common): GOG {0x8*,0xc} | Steam {0x8*,0x1c,0x2711,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x0059be80: 16-byte aligned: YES
  0059be80  55             PUSH EBP
  0059be81  8B EC          MOV EBP,ESP
  0059be83  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059be80
string literals (0):
direct callees (2):
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
struct offsets touched (top 8):
  +0x10DC [dec 4316]  x3  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
  +0x7C [dec 124]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0xEC [dec 236]  x1  via ECX
  +0xAA8 [dec 2728]  x1  via EAX
  +0x11AC [dec 4524]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059be80 FUN_0059be80 size=250 ret=8 callers=0 callees=2

undefined4 __thiscall FUN_0059be80(undefined4 param_1,int *param_2,int param_3)

{
  bool bVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  
  piVar3 = (int *)(**(code **)(*param_2 + 0x30))(param_1);
  if ((piVar3 != (int *)0x0) && (param_3 != 0)) {
    if (*(int *)(piVar3[0x437 = byte +0x10dc] + 0xaa8) != 0) {
      FUN_0058fc60(0,0x7f000000);
      FUN_0058fcd0(0,0);
    }
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
        (**(code **)(*piVar3 + 0x7c))(0x2711);
        param_2[0x3b = byte +0xec] = 1;
      }
    }
  }
  return 1;
}
```

### decoy: 0x0059bf80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059bf80: 16-byte aligned: YES
  0059bf80  55             PUSH EBP
  0059bf81  8B EC          MOV EBP,ESP
  0059bf83  81 EC 88 00 00 00 SUB ESP,0x88
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059bf80
string literals (0):
direct callees (14):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x006D0620  (unnamed)
  0x005BDE40  CSWSCreature::RemoveItem
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00605090  CItemRepository::AddItem
  0x005453C0  CSWSObject::GetArea
  0x00600EB0  CSWSItem::AddToArea
  0x00600380  CSWSItem::SetPossessor
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x0091EB8B  (unnamed)
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x3  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
  +0x100 [dec 256]  x2  via EDX
  +0x2C8 [dec 712]  x2  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x380 [dec 896]  x1  via EDX
  +0x538 [dec 1336]  x1  via ECX
```

## Card B0-9-48

GOG function: CSWSEffectListHandler::OnApplyModifyNumAttacks  gog 0x00744f10  callconv=(blank)  param_size_bytes=8  gog_size=96  gog_retn=12
Candidate Steam address: 0x005a2640 (size 96, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a26a0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00744e00 CSWSEffectListHandler::OnApplyVision -> 0x005a2530  gate_class=REVIEW
- GOG-order neighbour after:  0x00744f70 CSWSEffectListHandler::OnRemoveModifyNumAttacks -> 0x005a26a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00776930 CSWSCombatRound::SetBonusEffectAttacks -> Steam ?  in candidate callees: n/a
  0x00776950 CSWSCombatRound::GetActualBonusEffectAttacks -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0x10dc*} | Steam {0x30*,0x10dc*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a2640: 16-byte aligned: YES
  005a2640  55             PUSH EBP
  005a2641  8B EC          MOV EBP,ESP
  005a2643  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a2640
string literals (0):
direct callees (3):
  0x005922A0  CSWSCombatRound::GetActualBonusEffectAttacks
  0x005E4E60  CGameEffect::GetInteger
  0x00592280  CSWSCombatRound::SetBonusEffectAttacks
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2640 FUN_005a2640 size=96 ret=12 callers=0 callees=3

undefined4 FUN_005a2640(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if (iVar1 != 0) {
    iVar1 = FUN_005922a0();
    iVar2 = FUN_005e4e60(0);
    FUN_00592280(iVar1 + iVar2);
  }
  return 0;
}
```

### decoy: 0x005a26a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a26a0: 16-byte aligned: YES
  005a26a0  55             PUSH EBP
  005a26a1  8B EC          MOV EBP,ESP
  005a26a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a26a0
string literals (0):
direct callees (3):
  0x005922A0  CSWSCombatRound::GetActualBonusEffectAttacks
  0x005E4E60  CGameEffect::GetInteger
  0x00592280  CSWSCombatRound::SetBonusEffectAttacks
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-49

GOG function: CSWSEffectListHandler::OnApplyVisualEffect  gog 0x00741560  callconv=(blank)  param_size_bytes=8  gog_size=1163  gog_retn=12
Candidate Steam address: 0x0059ec70 (size 1163, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059ea10:STR0/CALLEE0/SIG0.153/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741470 CSWSEffectListHandler::OnApplyMovementSpeedDecrease -> 0x0059eb80  gate_class=REVIEW
- GOG-order neighbour after:  0x00741a10 CSWSEffectListHandler::OnRemoveVisualEffect -> 0x0059f120  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00605b70 CExoString::operator==_2 -> Steam ?  in candidate callees: n/a
  0x00620b50 C2DA::GetCExoStringEntry_2 -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  ... 11 more GOG callees
GOG strings (1): 'Type_FD'
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xa*,0xc*,0x18*,0x1c*,0x9c*,0x130*} | Steam {0x4*,0x8*,0xa*,0xc*,0x18*,0x1c*,0x9c*,0x130*}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x7*,0xc*,0x44*,0x94*,0xd4*,0x7f000000*} | Steam {0x2*,0x4*,0x7*,0xc*,0x44*,0x94*,0xd4*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059ec70: 16-byte aligned: YES
  0059ec70  55             PUSH EBP
  0059ec71  8B EC          MOV EBP,ESP
  0059ec73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059ec70
string literals (0):
direct callees (19):
  0x005E4EE0  CGameEffect::GetObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x00548DB0  CSWSObject::CalculateSpellRangedMissTarget
  0x00733540  CExoString::CExoString(empty)
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x0071B0B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733A60  CExoString::operator==_2
  0x005453C0  CSWSObject::GetArea
  0x0053E2D0  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0051C8F0  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x00632A20  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0051C2E0  (unnamed)
  0x0064B280  (unnamed)
  0x00544130  (unnamed)
struct offsets touched (top 8):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0xA [dec 10]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x1C [dec 28]  x1  via ECX
  +0x9C [dec 156]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ec70 FUN_0059ec70 size=1163 ret=12 callers=0 callees=19

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059ec70(int param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  undefined1 local_9c [11];
  char local_91;
  undefined1 local_90 [12];
  undefined4 local_84;
  float local_80;
  float local_7c;
  float local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  int local_68;
  undefined4 local_64;
  undefined4 local_60;
  int local_5c;
  float local_58;
  float local_54;
  undefined4 local_50;
  undefined4 local_4c;
  float local_48;
  undefined4 local_44;
  int local_40;
  undefined1 local_3c [8];
  int local_34;
  undefined4 local_30;
  undefined1 local_2a;
  undefined1 local_29;
  undefined4 local_28;
  undefined4 local_24;
  uint local_20;
  undefined4 local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095236b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((param_1 == 0) || (param_2 == 0)) {
    uVar3 = 1;
  }
  else {
    local_40 = 0;
    local_50 = 0;
    local_4c = 0;
    local_48 = 0.0;
    local_28 = *(undefined4 *)(param_1 + 4);
    iVar1 = FUN_005e4ee0(0);
    if (iVar1 == 0x7f000000) {
      local_44 = *(undefined4 *)(param_2 + 0x18);
    }
    else {
      local_44 = FUN_005e4ee0(0);
    }
    local_30 = FUN_005e4ee0(1);
    local_20 = FUN_005e4e60(0);
    local_29 = FUN_005e4e60(1);
    local_2a = 1;
    local_34 = FUN_005e4e60(2);
    local_1c = *(undefined4 *)(param_2 + 0xc);
    if (local_34 == 1) {
      local_28 = 0x7f000000;
      puVar2 = (undefined4 *)
               FUN_00548db0(local_90,*(undefined4 *)(param_2 + 0x18),*(undefined4 *)(param_1 + 4));
      local_50 = *puVar2;
      local_4c = puVar2[1 = byte +0x4];
      local_48 = (float)puVar2[2 = byte +0x8];
      if (local_48 == *(float *)(param_1 + 0x9c) + (float)_DAT_00986b50) {
        local_2a = 1;
      }
/* ... 77 more lines truncated by --max-lines */
```

### decoy: 0x0059ea10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

## Card B0-9-50

GOG function: CGuiInGame::GetCanHeadFollowInDialog  gog 0x004dfff0  callconv=(blank)  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x007cf960 (size 117, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x007cf8f0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf8f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004e0060 CGuiInGame::TutorialReasonWillShowWindow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x1c,0xb0,0xcc}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0xf28,0x7f000000} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf960: 16-byte aligned: YES
  007cf960  55             PUSH EBP
  007cf961  8B EC          MOV EBP,ESP
  007cf963  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf960
string literals (0):
direct callees (3):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x007CFA10  (unnamed)
  0x00898EB0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0xB0 [dec 176]  x1  via EDX
  +0xCC [dec 204]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf960 FUN_007cf960 size=117 ret=4 callers=1 callees=3

undefined4 __thiscall FUN_007cf960(int param_1,undefined1 param_2)

{
  int iVar1;
  
  iVar1 = FUN_0072fb00();
  if (((((*(uint *)(iVar1 + 0x1c) >> 1 & 1) != 0) && (iVar1 = FUN_007cfa10(param_2), iVar1 == 0)) &&
      (*(int *)(param_1 + 0xcc) == 0)) && (iVar1 = FUN_00898eb0(param_2), iVar1 != 0)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x007cf8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf8f0: 16-byte aligned: YES
  007cf8f0  55             PUSH EBP
  007cf8f1  8B EC          MOV EBP,ESP
  007cf8f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf8f0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B0-9-51

GOG function: CSWSEffectListHandler::OnApplyDisarm  gog 0x0073e870  callconv=(blank)  param_size_bytes=8  gog_size=1072  gog_retn=12
Candidate Steam address: 0x0059bf80 (size 1072, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059c3b0:STR0/CALLEE0/SIG0.101/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073e770 CSWSEffectListHandler::OnRemoveKnockdown -> 0x0059be80  gate_class=REVIEW
- GOG-order neighbour after:  0x0073eca0 CSWSEffectListHandler::OnApplySetAIState -> 0x0059c3b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x00663bb0 CSWSItem::SetPossessor -> Steam ?  in candidate callees: n/a
  0x006646e0 CSWSItem::AddToArea -> Steam ?  in candidate callees: n/a
  0x006688c0 CItemRepository::AddItem -> Steam ?  in candidate callees: n/a
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam ?  in candidate callees: n/a
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0x14*,0x18*,0x30*,0x100*,0x2c8*,0x380*,0x538*,0x1150*} | Steam {0x4*,0x8*,0x14*,0x18*,0x30*,0x100*,0x2c8*,0x380*,0x538*,0x1150*}
imm sets (* = in both, 9 common): GOG {0x3*,0xc*,0x10*,0x19*,0x88*,0x94*,0x7f000000*,0x80000001*,0xfffffffe*} | Steam {0x3*,0xc*,0x10*,0x19*,0x88*,0x94*,0x7f000000*,0x80000001*,0xfffffffe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059bf80: 16-byte aligned: YES
  0059bf80  55             PUSH EBP
  0059bf81  8B EC          MOV EBP,ESP
  0059bf83  81 EC 88 00 00 00 SUB ESP,0x88
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059bf80
string literals (0):
direct callees (14):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x005E4E60  CGameEffect::GetInteger
  0x006D0620  (unnamed)
  0x005BDE40  CSWSCreature::RemoveItem
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00605090  CItemRepository::AddItem
  0x005453C0  CSWSObject::GetArea
  0x00600EB0  CSWSItem::AddToArea
  0x00600380  CSWSItem::SetPossessor
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x0091EB8B  (unnamed)
  0x0054BDC0  CSWSArea::ComputeSafeLocation
  0x0054B130  CSWSArea::ComputeHeight
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x3  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
  +0x100 [dec 256]  x2  via EDX
  +0x2C8 [dec 712]  x2  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x380 [dec 896]  x1  via EDX
  +0x538 [dec 1336]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0059bf80 FUN_0059bf80 size=1072 ret=12 callers=0 callees=14

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059bf80(int *param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  float10 fVar3;
  undefined4 uVar4;
  float *pfVar5;
  int *piVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  float fVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  int local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  float local_10;
  int local_c;
  int local_8;
  
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if (iVar1 != 0) {
    local_8 = (**(code **)(*param_1 + 0x30))();
    local_24 = 0.0;
    local_20 = 0.0;
    local_1c = 0;
    local_18 = 0;
    local_14 = 0;
    local_10 = 0.0;
    if (*(int *)(local_8 + 0x538) != 0) {
      local_c = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
      iVar1 = FUN_005e4e60(0);
      if ((((iVar1 == 1) && (local_c != 0)) && (iVar1 = FUN_006d0620(0x10), iVar1 == 0)) &&
         (local_28 = FUN_006d0620(0x10), local_28 != 0)) {
        FUN_005bde40(local_28,0,1,0);
        if ((*(uint *)(local_28 + 0x2c8) >> 3 & 1) == 0) {
          if (*(int *)(local_28 + 0x100) == 1) {
            uVar8 = *(undefined4 *)(local_28 + 0x14);
            uVar9 = *(undefined4 *)(local_c + 4);
            uVar13 = 0;
            uVar4 = 0;
            uVar12 = 0;
            uVar11 = DAT_00992420;
            FUN_0051c280(0,0,uVar9,uVar8,DAT_00992420,0);
            FUN_0051e170(uVar12,uVar4,uVar9,uVar8,uVar11,uVar13);
          }
        }
        else {
          uVar11 = 1;
          uVar9 = 0;
          uVar8 = 1;
          piVar6 = &local_28;
          FUN_0058a2c0(1);
          iVar1 = FUN_00605090(piVar6,uVar8,uVar9,uVar11);
          if ((iVar1 == 0) && (local_28 != 0)) {
            uVar8 = *(undefined4 *)(local_c + 0x94);
            uVar9 = *(undefined4 *)(local_c + 0x98);
            fVar10 = *(float *)(local_c + 0x9c) + (float)_DAT_00987df0;
            uVar12 = 1;
            local_18 = uVar8;
            local_14 = uVar9;
            local_10 = fVar10;
            uVar11 = FUN_005453c0(uVar8,uVar9,fVar10,1);
            FUN_00600eb0(uVar11,uVar8,uVar9,fVar10,uVar12);
          }
          if (local_28 != 0) {
            FUN_00600380(*(undefined4 *)(local_c + 4),1,1,0x7f000000);
          }
        }
      }
      else {
/* ... 73 more lines truncated by --max-lines */
```

### decoy: 0x0059c3b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059c3b0: 16-byte aligned: YES
  0059c3b0  55             PUSH EBP
  0059c3b1  8B EC          MOV EBP,ESP
  0059c3b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059c3b0
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x1114 [dec 4372]  x2  via ECX,EDX
```

## Card B0-9-52

GOG function: CSWSEffectListHandler::OnApplyForcePush  gog 0x0074a720  callconv=(blank)  param_size_bytes=8  gog_size=1440  gog_retn=12
Candidate Steam address: 0x005a7e70 (size 1440, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a7820:STR0/CALLEE0/SIG0.537/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074a0d0 CSWSEffectListHandler::OnApplyForceJumpDelayed -> 0x005a7820  gate_class=REVIEW
- GOG-order neighbour after:  0x0074acc0 CSWSEffectListHandler::OnRemoveForcePush -> 0x005a8410  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (15), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00422ca0 CWalkHitInfo::Constructor -> Steam ?  in candidate callees: n/a
  0x0064bac0 CServerExoApp::GetGameObject -> Steam 0x0051c0a0  in candidate callees: yes
  0x0064c330 CServerExoApp::TestDirectLine -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ced30 CSWSCreature::WalkUpdateLocation_QuickWalk -> Steam ?  in candidate callees: n/a
  ... 7 more GOG callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x30*,0x8c*,0x90*,0x94*,0x98*,0x9c*,0x380*,...(+1)} | Steam {0x4*,0x8*,0xa*,0xc*,0x10*,0x14*,0x18*,0x30*,0x8c*,0x90*,0x94*,0x98*,0x9c*,0x380*,...(+1)}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x8*,0x9*,0xc*,0x10*,0x44*,0x8c*,0x94*,0x104*,0x164*,0xfffffff8*} | Steam {0x2*,0x3*,0x4*,0x8*,0x9*,0xc*,0x10*,0x44*,0x8c*,0x94*,0x104*,0x164*,0xfffffff8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a7e70: 16-byte aligned: YES
  005a7e70  55             PUSH EBP
  005a7e71  8B EC          MOV EBP,ESP
  005a7e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a7e70
string literals (0):
direct callees (15):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x005E4E60  CGameEffect::GetInteger
  0x005E4EA0  CGameEffect::GetFloat
  0x005143C0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00543F10  CSWSObject::SetOrientation
  0x0052BE60  CSWSArea::Constructor_2
  0x005453C0  CSWSObject::GetArea
  0x0051C910  (unnamed)
  0x00919723  operator new
  0x009196FD  operator delete
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x90 [dec 144]  x3  via EAX,ECX
  +0xA [dec 10]  x2  via EAX
  +0x10 [dec 16]  x2  via EDX
  +0x8C [dec 140]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a7e70 FUN_005a7e70 size=1440 ret=12 callers=0 callees=15

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005a7e70(int *param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  int local_174;
  undefined4 local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  undefined4 local_a4;
  undefined1 local_a0 [4];
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  int local_54;
  int local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  float local_3c;
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  float local_20;
  undefined4 *local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952a6e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (local_14 != 0) {
    local_18 = (int *)FUN_0051c0a0(*(undefined4 *)(param_2 + 0x18));
    if (local_18 != (int *)0x0) {
      iVar2 = (**(code **)(*local_18 + 0x10))();
      if (iVar2 != 0) {
        local_28 = FUN_005e4e60(0);
        local_4c = 0.0;
        local_48 = 0.0;
        local_44 = 0.0;
        local_50 = 0;
        if (local_28 == 0) {
          iVar2 = (**(code **)(*local_18 + 0x10))();
          local_4c = *(float *)(iVar2 + 0x94);
          local_48 = *(float *)(iVar2 + 0x98);
          local_44 = *(float *)(iVar2 + 0x9c);
        }
        else {
          fVar3 = (float10)FUN_005e4ea0(0);
          local_4c = (float)fVar3;
          fVar3 = (float10)FUN_005e4ea0(1);
          local_48 = (float)fVar3;
          fVar3 = (float10)FUN_005e4ea0(2);
          local_44 = (float)fVar3;
          local_50 = FUN_005e4e60(1);
        }
        fVar3 = (float10)FUN_005e4ea0(3);
        local_20 = (float)fVar3;
        if (local_20 == (float)_DAT_00986b38) {
          local_20 = _DAT_00986b68;
/* ... 69 more lines truncated by --max-lines */
```

### decoy: 0x005a7820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a7820: 16-byte aligned: YES
  005a7820  55             PUSH EBP
  005a7821  8B EC          MOV EBP,ESP
  005a7823  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a7820
string literals (0):
direct callees (15):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x005143C0  (unnamed)
  0x005716A0  CSWSCreature::DesiredAttackRange
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0051CD90  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x009196FD  operator delete
  0x00919723  operator new
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005C3F70  CSWSCreature::WalkUpdateLocation_QuickWalk
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 10):
  +0x380 [dec 896]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x90 [dec 144]  x7  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX,ECX,EDX
  +0xA [dec 10]  x2  via ECX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x94 [dec 148]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x9C [dec 156]  x2  via EAX,EDX
```

## Card B0-9-53

GOG function: CSWSEffectListHandler::OnApplyAreaOfEffect  gog 0x00741a40  callconv=(blank)  param_size_bytes=8  gog_size=915  gog_retn=12
Candidate Steam address: 0x0059f150 (size 915, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059f4f0:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00741a10 CSWSEffectListHandler::OnRemoveVisualEffect -> 0x0059f120  gate_class=REVIEW
- GOG-order neighbour after:  0x00741de0 CSWSEffectListHandler::OnRemoveAreaOfEffect -> 0x0059f4f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605c70 CExoString::operator!=_2 -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x00734bc0 CGameEffect::SetObjectID -> Steam ?  in candidate callees: n/a
  0x00734be0 CGameEffect::GetString -> Steam ?  in candidate callees: n/a
  0x007e6190 CSWSAreaOfEffectObject::Constructor -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x18*,0x1c*,0x30*,0xbc*,0x2a0*} | Steam {0x4*,0x8*,0x18*,0x1c*,0x30*,0xbc*,0x2a0*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0xc*,0x94*,0xa8*,0x2d0*,0x7f000000*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0xc*,0x94*,0xa8*,0x2d0*,0x7f000000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059f150: 16-byte aligned: YES
  0059f150  55             PUSH EBP
  0059f151  8B EC          MOV EBP,ESP
  0059f153  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059f150
string literals (0):
direct callees (13):
  0x00919723  operator new
  0x0062BFC0  CSWSAreaOfEffectObject::Constructor
  0x0062EE30  CSWSAreaOfEffectObject::SetCreator
  0x005E4E60  CGameEffect::GetInteger
  0x0062D750  CSWSAreaOfEffectObject::LoadAreaEffect
  0x005E4F20  CGameEffect::GetString
  0x00733B60  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007337C0  CExoString::operator=
  0x005453C0  CSWSObject::GetArea
  0x0062D540  CSWSAreaOfEffectObject::SetTargetObjID
  0x0062C800  CSWSAreaOfEffectObject::AddToArea
  0x005E4F00  CGameEffect::SetObjectID
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0xBC [dec 188]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059f150 FUN_0059f150 size=915 ret=12 callers=0 callees=13

undefined4 FUN_0059f150(int *param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int *local_94;
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  int local_60;
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  int local_4c;
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  int local_38;
  int local_34;
  int *local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  undefined1 local_15;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009523cb;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_1c = (**(code **)(*param_1 + 0x30))(uVar1);
  if (local_1c == 0) {
    uVar2 = 1;
  }
  else {
    local_34 = FUN_00919723(0x2d0);
    local_8 = 0;
    if (local_34 == 0) {
      local_94 = (int *)0x0;
    }
    else {
      local_94 = (int *)FUN_0062bfc0(0x7f000000);
    }
    local_30 = local_94;
    local_8 = 0xffffffff;
    local_14 = local_94;
    FUN_0062ee30(*(undefined4 *)(param_2 + 0x18));
    local_15 = FUN_005e4e60(0);
    FUN_0062d750(local_15);
    FUN_005e4f20(local_40,0);
    local_8 = 1;
    local_38 = FUN_00733b60(&DAT_00986925);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_38 != 0) {
      uVar2 = FUN_005e4f20(local_48,0);
      local_8 = 2;
      FUN_007337c0(uVar2);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    FUN_005e4f20(local_54,1);
    local_8 = 3;
    local_4c = FUN_00733b60(&DAT_00986925);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_4c != 0) {
      uVar2 = FUN_005e4f20(local_5c,1);
      local_8 = 4;
      FUN_007337c0(uVar2);
      local_8 = 0xffffffff;
      FUN_00733780();
    }
    uVar2 = FUN_005e4f20(local_68,2);
    local_8 = 5;
/* ... 26 more lines truncated by --max-lines */
```

### decoy: 0x0059f4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059f4f0: 16-byte aligned: YES
  0059f4f0  55             PUSH EBP
  0059f4f1  8B EC          MOV EBP,ESP
  0059f4f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059f4f0
string literals (0):
direct callees (5):
  0x005E4EE0  CGameEffect::GetObjectID
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX
  +0x30 [dec 48]  x2  via EDX
```

## Card B0-9-54

GOG function: CSWSEffectListHandler::OnRemoveMovementSpeedIncrease  gog 0x00741300  callconv=(blank)  param_size_bytes=8  gog_size=353  gog_retn=8
Candidate Steam address: 0x0059ea10 (size 353, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0059e860:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00741260 CSWSEffectListHandler::OnApplyMovementSpeedIncrease -> 0x0059e970  gate_class=REVIEW
- GOG-order neighbour after:  0x00741470 CSWSEffectListHandler::OnApplyMovementSpeedDecrease -> 0x0059eb80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0068f1d0 CSWSCreature::SetMovementRateFactor -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x30*} | Steam {0x4*,0x8*,0x30*}
imm sets (* = in both, 5 common): GOG {0x8*,0x1c*,0x1d*,0x44*,0x148*} | Steam {0x8*,0x1c*,0x1d*,0x44*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0059ea10: 16-byte aligned: YES
  0059ea10  55             PUSH EBP
  0059ea11  8B EC          MOV EBP,ESP
  0059ea13  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059ea10
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005766A0  CSWSCreature::SetMovementRateFactor
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x3  via ECX
  +0x30 [dec 48]  x3  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059ea10 FUN_0059ea10 size=353 ret=8 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_0059ea10(int *param_1,int *param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int local_1c;
  float local_10;
  
  iVar2 = (**(code **)(*param_1 + 0x30))();
  if (iVar2 != 0) {
    (**(code **)(*param_1 + 0x30))();
    if (param_2 != (int *)0x0) {
      local_10 = 1.0;
      iVar2 = (**(code **)(*param_1 + 0x30))();
      local_1c = 0;
      while( true ) {
        if (*(int *)(iVar2 + 0x14c) <= local_1c) break;
        if (0x1d < *(ushort *)(*(int *)(*(int *)(iVar2 + 0x148) + local_1c * 4) + 8)) break;
        piVar1 = *(int **)(*(int *)(iVar2 + 0x148) + local_1c * 4);
        if ((*piVar1 != *param_2) || (piVar1[1 = byte +0x4] != param_2[1 = byte +0x4])) {
          if ((short)piVar1[2 = byte +0x8] == 0x1c) {
            iVar3 = FUN_005e4e60(0);
            local_10 = (float)iVar3 / (float)_DAT_0098ba38 + local_10;
          }
          else if ((short)piVar1[2 = byte +0x8] == 0x1d) {
            iVar3 = FUN_005e4e60(0);
            local_10 = local_10 - (float)iVar3 / (float)_DAT_0098ba38;
          }
        }
        local_1c = local_1c + 1;
      }
      FUN_005766a0(local_10);
    }
  }
  return 1;
}
```

### decoy: 0x0059e860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059e860: 16-byte aligned: YES
  0059e860  55             PUSH EBP
  0059e861  8B EC          MOV EBP,ESP
  0059e863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059e860
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x006FD8D0  CVirtualMachine::RunScript
  0x0057A990  CSWSCreature::RecomputeAmbientAnimationState
  0x005D1510  CSWSCreature::ClearVisibleList
  0x005D0F30  CSWSObject::UpdatePerception
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x10E8 [dec 4328]  x1  via ECX
  +0x59E958 [dec 5892440]  x1  via EDX
```

## Card B0-9-55

GOG function: CSWSEffectListHandler::OnApplyAppear  gog 0x00748e60  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=12
Candidate Steam address: 0x005a6570 (size 72, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a64c0:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00748db0 CSWSEffectListHandler::OnApplyDisappear -> 0x005a64c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00748eb0 CSWSEffectListHandler::OnApplyNegativeLevel -> 0x005a65d0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00696230 CSWSCreature::AddAppearActions -> Steam ?  in candidate callees: n/a
  0x006a7d60 CSWSObject::ClearAllActions -> Steam 0x00541080  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x30*} | Steam {0x8*,0x30*}
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
prologue for 0x005a6570: 16-byte aligned: YES
  005a6570  55             PUSH EBP
  005a6571  8B EC          MOV EBP,ESP
  005a6573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6570
string literals (0):
direct callees (2):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D700  CSWSCreature::AddAppearActions
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6570 FUN_005a6570 size=72 ret=12 callers=0 callees=2

undefined4 __thiscall FUN_005a6570(undefined4 param_1,int *param_2)

{
  undefined4 uVar1;
  
  if ((char)param_2[2 = byte +0x8] == DAT_00994474) {
    uVar1 = (**(code **)(*param_2 + 0x30))(param_1);
    FUN_00541080(1);
    FUN_0057d700(param_1,uVar1);
  }
  return 1;
}
```

### decoy: 0x005a64c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a64c0: 16-byte aligned: YES
  005a64c0  55             PUSH EBP
  005a64c1  8B EC          MOV EBP,ESP
  005a64c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a64c0
string literals (0):
direct callees (4):
  0x00541080  CSWSObject::ClearAllActions
  0x0057D790  CSWSCreature::AddDisappearActions
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-56

GOG function: CSWSEffectListHandler::OnRemoveHorrified  gog 0x0074ade0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=8
Candidate Steam address: 0x005a8530 (size 36, retn 8)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a8410:STR0/CALLEE0/SIG0.163/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074acc0 CSWSEffectListHandler::OnRemoveForcePush -> 0x005a8410  gate_class=REVIEW
- GOG-order neighbour after:  0x0074ae10 CSWSEffectListHandler::OnApplyDamageForcePoints -> 0x005a8560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a8530: 16-byte aligned: YES
  005a8530  55             PUSH EBP
  005a8531  8B EC          MOV EBP,ESP
  005a8533  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a8530
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8530 FUN_005a8530 size=36 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005a8530(undefined4 param_1,int *param_2)

{
  (**(code **)(*param_2 + 0x30))(param_1);
  return 1;
}
```

### decoy: 0x005a8410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8410: 16-byte aligned: YES
  005a8410  55             PUSH EBP
  005a8411  8B EC          MOV EBP,ESP
  005a8413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a8410
string literals (0):
direct callees (6):
  0x0057D370  CSWSCreature::SetLockOrientationToObject
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-9-57

GOG function: CSWSEffectListHandler::OnApplyTurnResistance  gog 0x00748940  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=12
Candidate Steam address: 0x005a6050 (size 90, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a5ee0:STR0/CALLEE0/SIG0.156/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007487d0 CSWSEffectListHandler::OnApplyDarkness -> 0x005a5ee0  gate_class=AUTO
- GOG-order neighbour after:  0x007489a0 CSWSEffectListHandler::OnApplyDisappearAppear -> 0x005a60b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x30*,0x6c*,0x1198*} | Steam {0x30*,0x6c*,0x1198*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x005a6050: 16-byte aligned: YES
  005a6050  55             PUSH EBP
  005a6051  8B EC          MOV EBP,ESP
  005a6053  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a6050
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x30 [dec 48]  x2  via EDX
  +0x6C [dec 108]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a6050 FUN_005a6050 size=90 ret=12 callers=0 callees=0

undefined4 __thiscall FUN_005a6050(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (((param_2 != (int *)0x0) && (iVar1 = (**(code **)(*param_2 + 0x30))(param_1), iVar1 != 0)) &&
     (iVar1 = (**(code **)(*param_2 + 0x30))(), *(int *)(*(int *)(iVar1 + 0x1198) + 0x6c) == 0)) {
    return 0;
  }
  return 1;
}
```

### decoy: 0x005a5ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a5ee0: 16-byte aligned: YES
  005a5ee0  55             PUSH EBP
  005a5ee1  8B EC          MOV EBP,ESP
  005a5ee3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a5ee0
string literals (0):
direct callees (4):
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x122 [dec 290]  x1  via EAX
```

## Card B0-9-58

GOG function: CSWSEffectListHandler::OnApplyDispelAllMagic  gog 0x007457d0  callconv=(blank)  param_size_bytes=8  gog_size=1199  gog_retn=12
Candidate Steam address: 0x005a2ee0 (size 1199, retn 12)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a3390:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007457b0 Global::sub_007457B0_tail_wing_return_one  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00745c80 CSWSEffectListHandler::OnApplyDispelBestMagic -> 0x005a3390  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x0050e640 CSWRules::RollDice -> Steam ?  in candidate callees: n/a
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549cb0 CSWCCMessageData::Destructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xa*,0x18*,0x1c*,0x30*,0x1198*} | Steam {0x4*,0x8*,0xa*,0x18*,0x1c*,0x30*,0x1198*}
imm sets (* = in both, 11 common): GOG {0x4*,0x7*,0x8*,0xb*,0xc*,0xf*,0x14*,0x18*,0x34*,0xc0*,0x148*} | Steam {0x4*,0x7*,0x8*,0xb*,0xc*,0xf*,0x14*,0x18*,0x34*,0xc0*,0x148*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a2ee0: 16-byte aligned: YES
  005a2ee0  55             PUSH EBP
  005a2ee1  8B EC          MOV EBP,ESP
  005a2ee3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2ee0
string literals (0):
direct callees (19):
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005AA160  (unnamed)
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005AA250  (unnamed)
  0x006A2000  CSWRules::RollDice(n,sides)
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005E1BD0  (unnamed)
  0x005AA210  (unnamed)
  0x0051C600  CServerExoApp::GetPlayerByControlledObjectId
  0x0051C2E0  (unnamed)
  0x00648CC0  (unnamed)
  0x005E25D0  (unnamed)
  0x009196FD  operator delete
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 7):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EDX
  +0xA [dec 10]  x3  via ECX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x1198 [dec 4504]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2ee0 FUN_005a2ee0 size=1199 ret=12 callers=0 callees=19

undefined4 FUN_005a2ee0(int param_1,int param_2)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  int local_cc;
  int local_54;
  int *local_50;
  undefined4 local_4c;
  int *local_48[4 = byte +0x10];
  int local_38;
  int local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_24;
  undefined4 *local_20;
  uint local_1c;
  uint local_18;
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009526d3;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_24 = FUN_0051c080(uVar2);
  local_14 = (int *)(param_1 + 0x148);
  cVar1 = FUN_0053dfb0(*(undefined4 *)(param_2 + 0x18),local_48);
  if (((cVar1 == DAT_0099447f) && (local_48[0] != (int *)0x0)) &&
     (iVar3 = (**(code **)(*local_48[0] + 0x30))(), iVar3 != 0)) {
    (**(code **)(*local_48[0] + 0x30))();
    local_1c = FUN_006a4e00(0);
    local_1c = local_1c & 0xff;
    FUN_005aa160(0);
    local_8 = 0;
    iVar3 = FUN_00919723(0x34);
    local_8._0_1_ = 1;
    if (iVar3 == 0) {
      local_cc = 0;
    }
    else {
      local_cc = FUN_005e2540();
    }
    local_8 = (uint)local_8._1_3_ << 8;
    local_38 = local_cc;
    FUN_005e1c70(0,*(undefined4 *)(param_1 + 4));
    local_34 = 0;
    for (local_54 = 0; local_54 < local_14[1 = byte +0x4]; local_54 = local_54 + 1) {
      local_20 = *(undefined4 **)(*local_14 + local_54 * 4);
      if (((*(ushort *)((int)local_20 + 0xa [dec 10]) & 0x18) == 8) &&
         (((*(ushort *)((int)local_20 + 0xa [dec 10]) & 7) == DAT_009967b4 ||
          ((*(ushort *)((int)local_20 + 0xa [dec 10]) & 7) == DAT_009967b8)))) {
        local_30 = *local_20;
        local_2c = local_20[1 = byte +0x4];
        local_4c = local_20[7 = byte +0x1c];
        iVar3 = FUN_005aa250(local_30,local_2c);
        if (iVar3 == 0) {
          cVar1 = FUN_0053dfb0(local_20[6 = byte +0x18],&local_50);
          if ((cVar1 == DAT_0099447f) && (iVar3 = (**(code **)(*local_50 + 0x30))(), iVar3 != 0)) {
            (**(code **)(*local_50 + 0x30))();
            local_18 = FUN_006a4e00(0);
            local_18 = local_18 & 0xff;
          }
          else {
            local_18 = 0;
          }
          uVar4 = FUN_006a2000(1,0x14);
          if ((int)(local_18 + 0xb) < (int)((uVar4 & 0xffff) + local_1c)) {
/* ... 45 more lines truncated by --max-lines */
```

### decoy: 0x005a3390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a3390: 16-byte aligned: YES
  005a3390  55             PUSH EBP
  005a3391  8B EC          MOV EBP,ESP
  005a3393  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a3390
string literals (0):
direct callees (20):
  0x0051C080  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005AA160  (unnamed)
  0x005AA250  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
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
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x005AA1A0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 7):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0xA [dec 10]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,EDX
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x1198 [dec 4504]  x2  via ECX,EDX
```

## Card B0-9-59

GOG function: CSWSEffectListHandler::OnRemoveSilence  gog 0x00745760  callconv=(blank)  param_size_bytes=4  gog_size=66  gog_retn=8
Candidate Steam address: 0x005a2e90 (size 66, retn 8)
Candidate sources: branch+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyDispelAllMagic
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a2c70:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00745540 CSWSEffectListHandler::OnApplySilence -> 0x005a2c70  gate_class=REVIEW
- GOG-order neighbour after:  0x007457b0 Global::sub_007457B0_tail_wing_return_one  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x30*,0xfd0*} | Steam {0x30*,0xfd0*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x005a2e90: 16-byte aligned: YES
  005a2e90  55             PUSH EBP
  005a2e91  8B EC          MOV EBP,ESP
  005a2e93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a2e90
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0xFD0 [dec 4048]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2e90 FUN_005a2e90 size=66 ret=8 callers=0 callees=0

undefined4 __thiscall FUN_005a2e90(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  iVar1 = (**(code **)(*param_2 + 0x30))(param_1);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_2 + 0x30))();
    *(undefined4 *)(iVar1 + 0xfd0) = 0;
  }
  return 1;
}
```

### decoy: 0x005a2c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2c70: 16-byte aligned: YES
  005a2c70  55             PUSH EBP
  005a2c71  8B EC          MOV EBP,ESP
  005a2c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2c70
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0xFD0 [dec 4048]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-9-60

GOG function: CSWSEffectListHandler::OnApplyHealForcePoints  gog 0x0074af10  callconv=(blank)  param_size_bytes=8  gog_size=371  gog_retn=12
Candidate Steam address: 0x005a8660 (size 371, retn 12)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005a8560:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0074ae10 CSWSEffectListHandler::OnApplyDamageForcePoints -> 0x005a8560  gate_class=REVIEW
- GOG-order neighbour after:  0x0074b090 CSWSEffectListHandler::OnApplyBodyFuel -> 0x005a87e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x006977d0 CSWSCreature::GetMaxForcePoints -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x30*,0x130*,0x132*,0x1198*} | Steam {0x4*,0x30*,0x130*,0x132*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x34*,0xe4*} | Steam {0x4*,0xc*,0x34*,0xe4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x005a8660: 16-byte aligned: YES
  005a8660  55             PUSH EBP
  005a8661  8B EC          MOV EBP,ESP
  005a8663  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a8660
string literals (0):
direct callees (7):
  0x005E4E60  CGameEffect::GetInteger
  0x0057ECA0  CSWSCreature::GetMaxForcePoints
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 5):
  +0x130 [dec 304]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x132 [dec 306]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005a8660 FUN_005a8660 size=371 ret=12 callers=0 callees=7

/* WARNING: Removing unreachable block (ram,0x005a86dc) */

undefined4 FUN_005a8660(int *param_1)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 local_44;
  short local_40;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0bb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if ((iVar2 != 0) && (*(int *)(iVar2 + 0x1198) != 0)) {
    iVar3 = FUN_005e4e60(0);
    local_40 = *(short *)(*(int *)(iVar2 + 0x1198) + 0x130) +
               *(short *)(*(int *)(iVar2 + 0x1198) + 0x132);
    local_14 = local_40 + iVar3;
    sVar1 = FUN_0057eca0();
    if (sVar1 < local_14) {
      sVar1 = FUN_0057eca0();
      local_14 = (int)sVar1;
    }
    *(undefined2 *)(*(int *)(iVar2 + 0x1198) + 0x130) = (undefined2)local_14;
    iVar4 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar4 == 0) {
      local_44 = 0;
    }
    else {
      local_44 = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1c70(0,*(undefined4 *)(iVar2 + 4));
    FUN_005e1bd0(0,iVar3);
    FUN_00584da0(0xe4,local_44);
  }
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x005a8560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a8560: 16-byte aligned: YES
  005a8560  55             PUSH EBP
  005a8561  8B EC          MOV EBP,ESP
  005a8563  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005a8560
string literals (0):
direct callees (1):
  0x005E4E60  CGameEffect::GetInteger
struct offsets touched (top 4):
  +0x130 [dec 304]  x5  via EAX,ECX,EDX
  +0x1198 [dec 4504]  x4  via EAX,ECX,EDX
  +0x132 [dec 306]  x2  via ECX,EDX
  +0x30 [dec 48]  x1  via EDX
```
