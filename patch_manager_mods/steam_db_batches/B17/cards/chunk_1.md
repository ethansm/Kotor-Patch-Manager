# Review cards B17 chunk 1

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-1-01

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458980 (size 253, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084af50 PartLight::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b0b0 Gob::SetScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0084b0b0 Gob::SetScene -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1c,0x20,0x1a4}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0x64} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458980: 16-byte aligned: YES
  00458980  55             PUSH EBP
  00458981  8B EC          MOV EBP,ESP
  00458983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00458980
string literals (0):
direct callees (5):
  0x00476620  (unnamed)
  0x004142E0  (unnamed)
  0x0049B840  (unnamed)
  0x00463B50  (unnamed)
  0x00456EA0  (unnamed)
struct offsets touched (top 3):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x1A4 [dec 420]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00458980 FUN_00458980 size=253 ret=0 callers=1 callees=5

void __fastcall FUN_00458980(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00949d08;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = PartLight::vftable;
  local_8 = 0;
  for (local_14 = 0; local_14 < DAT_00a1ffa0; local_14 = local_14 + 1) {
    iVar1 = *(int *)(*(int *)(DAT_00a1ff9c + local_14 * 4) + 0x1a4);
    if (iVar1 != 0) {
      iVar3 = FUN_00476620(param_1);
      if (iVar3 != 0) {
        iVar3 = FUN_004142e0(param_1);
        if (-1 < iVar3) {
          FUN_0049b840(iVar3);
          FUN_00463b50(iVar3);
        }
      }
    }
    if (*(undefined4 **)(iVar1 + 0x1c) == param_1) {
      *(undefined4 *)(iVar1 + 0x1c) = 0;
      *(undefined4 *)(iVar1 + 0x20) = 0;
    }
  }
  local_8 = 0xffffffff;
  FUN_00456ea0(uVar2);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00458a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 253 B, RET 0
- callers 1: 0x00458950
- callees: 0x00476620 (CALL), 0x004142e0 (CALL), 0x0049b840 (CALL), 0x00463b50 (CALL), 0x00456ea0 (CALL)
- twins: none

## Card B17-1-02

GOG function: Achievements::EvaluateAchievement  gog 0x004010f0  callconv=(blank)  param_size_bytes=4  gog_size=1382  gog_retn=0
Candidate Steam address: 0x004010e0 (size 1382, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004010a0:STR0/CALLEE0/SIG0.013/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004010b0 Achievements::UnlockAchievement  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00401660 Achievements::EvaluateAchievementMovie  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004010b0 Achievements::UnlockAchievement -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 79 common): GOG {0x3*,0x4*,0x6*,0x7*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,0x10*,0x11*,0x12*,0x13*,...(+65)} | Steam {0x3*,0x4*,0x6*,0x7*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,0x10*,0x11*,0x12*,0x13*,...(+65)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004010e0: 16-byte aligned: YES
  004010e0  55             PUSH EBP
  004010e1  8B EC          MOV EBP,ESP
  004010e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004010e0
string literals (0):
direct callees (1):
  0x004010A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004010e0 FUN_004010e0 size=1382 ret=0 callers=1 callees=1

void FUN_004010e0(uint param_1)

{
  if (param_1 < 0x1bcc9) {
    if (param_1 == 0x1bcc8) {
      FUN_004010a0(0xd);
      return;
    }
    if (param_1 < 0x1a74e) {
      if (param_1 == 0x1a74d) {
        FUN_004010a0(0x19);
        return;
      }
      if (param_1 < 0x1685a) {
        if (param_1 == 0x16859) {
          FUN_004010a0(0xb);
          return;
        }
        if (0x14f48 < param_1) {
          if (param_1 == 0x15a5b) {
            FUN_004010a0(0xe);
            return;
          }
          if (param_1 == 0x16839) {
            FUN_004010a0(0xc);
            return;
          }
          return;
        }
        if (param_1 != 0x14f48) {
          if (param_1 == 0x12f66) {
            DAT_00a13c21 = 1;
            return;
          }
          if (param_1 != 0x14f46) {
            return;
          }
        }
        FUN_004010a0(0x32);
        return;
      }
      if (param_1 < 0x189a9) {
        if (param_1 == 0x189a8) {
          DAT_00a13c24 = 1;
          return;
        }
        if (param_1 == 0x169a1) {
          FUN_004010a0(0x1a);
          return;
        }
        if (param_1 == 0x189a5) {
          FUN_004010a0(6);
          return;
        }
        return;
      }
      if (param_1 == 0x18e1a) {
        FUN_004010a0(0x38);
        return;
      }
      if (param_1 == 0x19ed0) {
        FUN_004010a0(7);
        return;
      }
      return;
    }
    if (0x1ab40 < param_1) {
      if (0x1b84d < param_1) {
        if (param_1 == 0x1b851) {
          DAT_00a13c20 = 1;
          return;
        }
        return;
      }
      if (param_1 == 0x1b84d) {
        if ((DAT_00a13c21 != '\0') && (DAT_00a13c20 == '\0')) {
          FUN_004010a0(0x23);
        }
      }
/* ... 153 more lines truncated by --max-lines */
```

### decoy: 0x004010a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004010a0: 16-byte aligned: YES
  004010a0  55             PUSH EBP
  004010a1  8B EC          MOV EBP,ESP
  004010a3  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004010a0
string literals (1):
  0x009868A0  at 004010b4  'achievement_%d'
direct callees (3):
  0x00919671  (unnamed)
  0x004016E0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1382 B, RET 0
- callers 1: 0x0071df90
- callees: 0x004010a0 (CALL)
- twins: none

## Card B17-1-03

GOG function: CSWSpellArray::GetSpell  gog 0x004ed450  callconv=(blank)  param_size_bytes=4  gog_size=57  gog_retn=4
Candidate Steam address: 0x006c13e0 (size 109, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006c1450:STR0/CALLEE35/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004ed3e0 CSWSpell::`scalar/vector  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ed490 CSWSpellArray::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (35 of 62): the card's A-CALLEE credit may come from these, not from callees
  0x004138f0 CSWCCreature::HandleConjuringCreature -> Steam 0x00767b20  in candidate callers: no
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: no
  0x004275f0 CSWCCreature::IsLeaderUsableItem -> Steam 0x0077b670  in candidate callers: no
  0x00445390 CClientExoAppInternal::ShowObject -> Steam 0x00798d70  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1a8*} | Steam {0x2,0x4*,0x1a8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006c13e0: 16-byte aligned: YES
  006c13e0  55             PUSH EBP
  006c13e1  8B EC          MOV EBP,ESP
  006c13e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006c13e0
string literals (0):
direct callees (3):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
  0x006C0FD0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006c13e0 FUN_006c13e0 size=109 ret=4 callers=1 callees=3

void * __thiscall FUN_006c13e0(void *param_1,uint param_2)

{
  if ((param_2 & 2) == 0) {
    FUN_006c0fd0();
    if ((param_2 & 1) != 0) {
      FUN_009196fd(param_1);
    }
  }
  else {
    _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1,0x1a8,*(int *)((int)param_1 + -4),FUN_006c0fd0);
    if ((param_2 & 1) != 0) {
      FUN_009196fd((int)param_1 + -4);
    }
    param_1 = (void *)((int)param_1 + -4);
  }
  return param_1;
}
```

### decoy: 0x006c1450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006c1450: 16-byte aligned: YES
  006c1450  55             PUSH EBP
  006c1451  8B EC          MOV EBP,ESP
  006c1453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006c1450
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 109 B, RET 4
- callers 1: 0x006c1380
- callees: 0x00919bcc (CALL), 0x009196fd operator delete (CALL), 0x006c0fd0 (CALL)
- twins: none

## Card B17-1-04

GOG function: CClientOptions::SetFrameBuffer  gog 0x00469000  callconv=(blank)  param_size_bytes=4  gog_size=448  gog_retn=4
Candidate Steam address: 0x007bc330 (size 448, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.57) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007bc2f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00468fc0 CClientOptions::SetTexQual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004691c0 CClientOptions::SetAnisotropy  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x0086ac00 Global::AurEnableFrameBufferEffects -> Steam ?  in candidate callees: n/a
  0x0086ac10 Global::AurDisableFrameBufferEffects -> Steam ?  in candidate callees: n/a
  0x0086ba10 Global::AurCheckFrameBufferModificationsAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x38*,0xb4*,0xb8*,0xcc*} | Steam {0x4*,0x38*,0xb4*,0xb8*,0xcc*}
imm sets (* = in both, 5 common): GOG {0x4*,0x34*,0x1f40*,0x1f42*,0x7f000000*} | Steam {0x4*,0x34*,0x1f40*,0x1f42*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007bc330: 16-byte aligned: YES
  007bc330  55             PUSH EBP
  007bc331  8B EC          MOV EBP,ESP
  007bc333  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc330
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x00478D20  Global::AurEnableFrameBufferEffects
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x00484C80  (unnamed)
  0x00478D30  (unnamed)
struct offsets touched (top 5):
  +0x38 [dec 56]  x2  via ECX,EDX
  +0xB4 [dec 180]  x2  via EDX
  +0xB8 [dec 184]  x2  via EAX
  +0xCC [dec 204]  x2  via EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc330 FUN_007bc330 size=448 ret=4 callers=3 callees=5

void __thiscall FUN_007bc330(int param_1,undefined4 param_2)

{
  char cVar1;
  int iVar2;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int *local_10;
  byte local_9;
  int *local_8;
  
  local_8 = (int *)FUN_0073fb90();
  *(undefined4 *)(param_1 + 0x38) = param_2;
  if (*(int *)(param_1 + 0x38) == 0) {
    FUN_00478d30();
    if (local_8 != (int *)0x0) {
      for (local_9 = 0; (int)(uint)local_9 < *local_8; local_9 = local_9 + 1) {
        local_10 = (int *)FUN_007e5da0(local_9);
        if (((local_10 != (int *)0x0) && (iVar2 = (**(code **)(*local_10 + 0xcc))(8000), iVar2 != 0)
            ) && (cVar1 = FUN_00484c80(), cVar1 != '\0')) {
          (**(code **)(*local_10 + 0xb8))(8000);
          local_28 = 0;
          local_24 = 0;
          local_20 = 0;
          (**(code **)(*local_10 + 0xb4))(0x1f42,0,0x7f000000,0x7f000000,0,0,&local_28);
        }
      }
    }
  }
  else {
    FUN_00478d20();
    if (local_8 != (int *)0x0) {
      for (local_9 = 0; (int)(uint)local_9 < *local_8; local_9 = local_9 + 1) {
        local_10 = (int *)FUN_007e5da0(local_9);
        if (((local_10 != (int *)0x0) &&
            (iVar2 = (**(code **)(*local_10 + 0xcc))(0x1f42), iVar2 != 0)) &&
           (cVar1 = FUN_00484c80(), cVar1 != '\0')) {
          (**(code **)(*local_10 + 0xb8))(0x1f42);
          local_1c = 0;
          local_18 = 0;
          local_14 = 0;
          (**(code **)(*local_10 + 0xb4))(8000,0,0x7f000000,0x7f000000,0,0,&local_1c);
        }
      }
    }
  }
  return;
}
```

### decoy: 0x007bc2f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc2f0: 16-byte aligned: YES
  007bc2f0  55             PUSH EBP
  007bc2f1  8B EC          MOV EBP,ESP
  007bc2f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc2f0
string literals (0):
direct callees (1):
  0x0073FBF0  CClientExoApp::SetTexturePack
struct offsets touched (top 2):
  +0x44 [dec 68]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 448 B, RET 4
- callers 3: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d5cc0, 0x007d75f0
- callees: 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x00478d20 (CALL), 0x007e5da0 Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0) (CALL), 0x00484c80 (CALL), 0x00478d30 (CALL)
- indirect: 0x007bc3c1 slot 0xcc, 0x007bc3e6 slot 0xb8, 0x007bc41e slot 0xb4, 0x007bc486 slot 0xcc, 0x007bc4ab slot 0xb8, ...
- globals: 0x00a1b4a4 READ
- twins: none

## Card B17-1-05

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458ae0 (size 130, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084af50 PartLight::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b0b0 Gob::SetScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0084b0b0 Gob::SetScene -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4,0x8*,0x64*} | Steam {0x8*,0x14,0x64*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458ae0: 16-byte aligned: YES
  00458ae0  55             PUSH EBP
  00458ae1  8B EC          MOV EBP,ESP
  00458ae3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00458ae0
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00458ae0 FUN_00458ae0 size=130 ret=0 callers=2 callees=1

int FUN_00458ae0(char *param_1)

{
  int iVar1;
  int iVar2;
  int local_c;
  int local_8;
  
  local_c = 0;
  local_8 = DAT_00a1ffa0 + -1;
  while( true ) {
    while( true ) {
      if (local_8 < local_c) {
        return local_c;
      }
      iVar2 = local_8 + local_c >> 1;
      iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(DAT_00a1ff9c + iVar2 * 4) + 0x64 [dec 100]),param_1);
      if (iVar1 < 1) break;
      local_8 = iVar2 + -1;
    }
    if (-1 < iVar1) break;
    local_c = iVar2 + 1;
  }
  return iVar2;
}
```

### decoy: 0x00458a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 130 B, RET 0
- callers 2: 0x00458a80, 0x00458b70
- callees: 0x009370c7 _stricmp (CALL)
- twins: none

## Card B17-1-06

GOG function: CAurInternal::RestoreDesktopDisplayMode  gog 0x008689c0  callconv=(blank)  param_size_bytes=0  gog_size=56  gog_retn=0
Candidate Steam address: 0x00476b20 (size 89, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurInternalGL::RestoreDesktopMode | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00476ae0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00868790 CAurInternal::GetValidMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00868a00 CAurInternal::RestoreAuroraDisplayMode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x24*,0xa8*} | Steam {0x24*,0xa8*,0xb0}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00476b20: 16-byte aligned: YES
  00476b20  55             PUSH EBP
  00476b21  8B EC          MOV EBP,ESP
  00476b23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00476b20
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x24 [dec 36]  x1  via EDX
  +0xA8 [dec 168]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00476b20 FUN_00476b20 size=89 ret=0 callers=0 callees=1

void __fastcall FUN_00476b20(int *param_1)

{
  LONG LVar1;
  
  (**(code **)(*param_1 + 0x24))(param_1[0x2c = byte +0xb0],1);
  if ((param_1[0x2a = byte +0xa8] != 0) &&
     (LVar1 = ChangeDisplaySettingsA((DEVMODEA *)(param_1 + 0x1 [dec 1] {int*: byte +0x4}),2), LVar1 == 0)) {
    ChangeDisplaySettingsA((DEVMODEA *)(param_1 + 0x1 [dec 1] {int*: byte +0x4}),4);
  }
  return;
}
```

### decoy: 0x00476ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00476ae0: 16-byte aligned: YES
  00476ae0  55             PUSH EBP
  00476ae1  8B EC          MOV EBP,ESP
  00476ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00476ae0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x24 [dec 36]  x1  via EDX
  +0xA8 [dec 168]  x1  via ECX
```

### Static facts (script output, report-only)
- size 89 B, RET 0
- callees: 0x00000124 (CALL)
- indirect: 0x00476b41 slot 0x24
- imports: ChangeDisplaySettingsA (USER32.DLL)
- twins: none

## Card B17-1-07

GOG function: CSWCModule::ZoomCamera  gog 0x0046b6d0  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x007f3de0 (size 177, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.62) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.24 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f3c70:STR0/CALLEE1/SIG0.282/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW
- GOG-order neighbour after:  0x0046b770 CSWCModule::ScrollCamera  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c,0x34,0x40*,0x80*,0x9c,0x104*} | Steam {0xc,0x24,0x40*,0x80*,0x104*}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x44} | Steam {0x3,0x4*,0x5,0x7,0x10,0x106a,0x106e,0x1071}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f3de0: 16-byte aligned: YES
  007f3de0  55             PUSH EBP
  007f3de1  8B EC          MOV EBP,ESP
  007f3de3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f3de0
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x40 [dec 64]  x6  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x80 [dec 128]  x3  via EDX
  +0x24 [dec 36]  x1  via ECX
  +0x104 [dec 260]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f3de0 FUN_007f3de0 size=177 ret=4 callers=2 callees=0

void __thiscall FUN_007f3de0(int param_1,float param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xc) == 3) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
    if (iVar1 != 0) {
      *(float *)(iVar1 + 0x104) = -param_2;
    }
  }
  else if (*(int *)(param_1 + 0xc) == 5) {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
    if (iVar1 != 0) {
      *(float *)(iVar1 + 0x24) = -param_2;
    }
  }
  else if (*(int *)(param_1 + 0xc) == 7) {
    (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1071);
  }
  return;
}
```

### decoy: 0x007f3c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3c70: 16-byte aligned: YES
  007f3c70  55             PUSH EBP
  007f3c71  8B EC          MOV EBP,ESP
  007f3c73  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f3c70
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x40 [dec 64]  x9  via EAX,ECX,EDX
  +0x80 [dec 128]  x6  via ECX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x48 [dec 72]  x3  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x38 [dec 56]  x1  via ECX
  +0x94 [dec 148]  x1  via EAX
  +0x104 [dec 260]  x1  via ECX
```

### Static facts (script output, report-only)
- size 177 B, RET 4
- callers 2: 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x007ac470 CClientExoAppInternal::ProcessInput
- indirect: 0x007f3e0b slot 0x80, 0x007f3e4a slot 0x80, 0x007f3e86 slot 0x80
- twins: none

## Card B17-1-08

GOG function: CAurGUIStringInternal::SetSizeBoundingRect  gog 0x008d0ae0  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x0047a1a0 (size 35, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a1d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d0ab0 CAurGUIStringInternal::GetSizeBoundingRect  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d0b00 CAurGUIStringInternal::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
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
prologue for 0x0047a1a0: 16-byte aligned: YES
  0047a1a0  55             PUSH EBP
  0047a1a1  8B EC          MOV EBP,ESP
  0047a1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a1a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a1a0 FUN_0047a1a0 size=35 ret=8 callers=0 callees=0

void __thiscall FUN_0047a1a0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  *param_2 = *(undefined4 *)(param_1 + 0xc);
  *param_3 = *(undefined4 *)(param_1 + 0x10);
  return;
}
```

### decoy: 0x0047a1d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a1d0: 16-byte aligned: YES
  0047a1d0  55             PUSH EBP
  0047a1d1  8B EC          MOV EBP,ESP
  0047a1d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a1d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EDX
```

### Static facts (script output, report-only)
- size 35 B, RET 8
- twins: none

## Card B17-1-09

GOG function: CAurGUIStringInternal::SetSizeBoundingRect  gog 0x008d0ae0  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x0047a680 (size 83, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a600:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d0ab0 CAurGUIStringInternal::GetSizeBoundingRect  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d0b00 CAurGUIStringInternal::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xc,0x10} | Steam {0x4,0x8}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0xa,0xc}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0047a680: 16-byte aligned: YES
  0047a680  55             PUSH EBP
  0047a681  8B EC          MOV EBP,ESP
  0047a683  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a680
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a680 FUN_0047a680 size=83 ret=8 callers=0 callees=0

void __thiscall FUN_0047a680(int param_1,float *param_2,float *param_3)

{
  *param_2 = (float)*(int *)(param_1 + 4) /
             (float)(int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10);
  *param_3 = (float)*(int *)(param_1 + 8) / (float)(int)(short)(&DAT_00a309ce)[DAT_00a309c0 * 5];
  return;
}
```

### decoy: 0x0047a600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a600: 16-byte aligned: YES
  0047a600  55             PUSH EBP
  0047a601  8B EC          MOV EBP,ESP
  0047a603  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a600
string literals (0):
direct callees (4):
  0x00919F10  _strlen
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091CFF0  (unnamed)
struct offsets touched (top 1):
  +0x14 [dec 20]  x4  via EAX,ECX
```

### Static facts (script output, report-only)
- size 83 B, RET 8
- globals: 0x00a309c0 READ, 0x00a309c0 READ
- twins: none

## Card B17-1-10

GOG function: CSWCModule::ZoomCamera  gog 0x0046b6d0  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x007f4610 (size 1533, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.23) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f4550:STR0/CALLEE1/SIG0.183/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW
- GOG-order neighbour after:  0x0046b770 CSWCModule::ScrollCamera  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x34,0x40*,0x80*,0x9c,0x104} | Steam {0x4,0xc,0x1c*,0x2c,0x40*,0x48,0x5c,0x64,0x68,0x6c,0x70,0x74,0x75,0x78,...(+9)}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x44} | Steam {0x2,0x3,0x4*,0x5,0x7,0x30,0xc0,0xc4,0xff,0x198}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f4610: 16-byte aligned: YES
  007f4610  55             PUSH EBP
  007f4611  8B EC          MOV EBP,ESP
  007f4613  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007f4610
string literals (0):
direct callees (11):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073F4D0  CClientExoApp::GetGameObject
  0x00919723  operator new
  0x007DD380  (unnamed)
  0x00775990  CSWCCreature::GetHeadHeightOffset
  0x0077F160  CSWCCreature::GetFreeLookVideoEffect
  0x007E24D0  (unnamed)
  0x007E3C10  (unnamed)
  0x007F5B20  (unnamed)
  0x007F6030  (unnamed)
  0x007F61B0  (unnamed)
struct offsets touched (top 10):
  +0x40 [dec 64]  x25  via EAX,ECX,EDX
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x75 [dec 117]  x6  via EAX,ECX,EDX
  +0x7C [dec 124]  x5  via ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x68 [dec 104]  x2  via ECX,EDX
  +0x6C [dec 108]  x2  via ECX,EDX
  +0xD0 [dec 208]  x2  via EDX
  +0xD5 [dec 213]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f4610 FUN_007f4610 size=1533 ret=4 callers=1 callees=11

void __thiscall FUN_007f4610(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  uint uVar8;
  float10 fVar9;
  float10 fVar10;
  undefined4 local_cc;
  undefined4 local_c8;
  int local_bc;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096bc11;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_0072fb00(uVar1);
  uVar8 = (uint)*(byte *)(iVar2 + 0x75);
  if (((uVar8 != 4) && (*(int *)(param_1 + 0x8c) == 0)) && (*(int *)(param_1 + 0x40) != 0)) {
    if (uVar8 != *(uint *)(param_1 + 0xc)) {
      piVar3 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x70));
      if (piVar3 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      piVar4 = (int *)(**(code **)(*piVar3 + 0xc))();
      if (piVar4 == (int *)0x0) {
        ExceptionList = local_10;
        return;
      }
      iVar2 = (**(code **)(*piVar4 + 0x98))(0xff,1);
      if (iVar2 == 0) {
        ExceptionList = local_10;
        return;
      }
      if (*(int *)(param_1 + 0xc) == 3) {
        (**(code **)(**(int **)(param_1 + 0x40) + 0x7c))(0);
      }
      if (uVar8 == 3) {
        piVar3 = (int *)(**(code **)(*piVar3 + 0x2c))();
        iVar2 = FUN_00919723(0x198);
        local_8 = 0;
        if (iVar2 == 0) {
          local_bc = 0;
        }
        else {
          uVar7 = (**(code **)(*piVar3 + 0xd0))();
          uVar5 = (**(code **)(**(int **)(param_1 + 0x40) + 4))(uVar7);
          local_bc = FUN_007dd380(uVar5,uVar7);
        }
        local_8 = 0xffffffff;
        uVar7 = (**(code **)(*piVar3 + 0xd4))();
        *(undefined4 *)(local_bc + 0x68) = uVar7;
        fVar10 = (float10)FUN_00775990();
        fVar9 = (float10)(**(code **)(*piVar3 + 0xd8))();
        *(float *)(local_bc + 0x6c) = (float)(fVar9 + (float10)(double)fVar10);
        (**(code **)(**(int **)(param_1 + 0x40) + 0x7c))(local_bc);
      }
      else if (uVar8 == 5) {
        piVar3 = (int *)(**(code **)(*piVar3 + 0x2c))();
        iVar2 = FUN_00919723(0xc4);
        local_8 = 1;
        if (iVar2 == 0) {
          local_c8 = 0;
        }
        else {
          uVar7 = FUN_0077f160();
          uVar5 = (**(code **)(*piVar3 + 0xd0))(uVar7);
          uVar6 = (**(code **)(**(int **)(param_1 + 0x40) + 4))(uVar5);
          local_c8 = FUN_007e24d0(uVar6,uVar5,uVar7);
/* ... 61 more lines truncated by --max-lines */
```

### decoy: 0x007f4550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f4550: 16-byte aligned: YES
  007f4550  55             PUSH EBP
  007f4551  8B EC          MOV EBP,ESP
  007f4553  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f4550
string literals (0):
direct callees (1):
  0x0044B8A0  (unnamed)
struct offsets touched (top 6):
  +0x40 [dec 64]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x80 [dec 128]  x1  via ECX
```

### Static facts (script output, report-only)
- size 1533 B, RET 4
- callers 1: 0x0078c350 CClientExoAppInternal::UpdateCamera
- callees: 0x0072fb00 CClientExoApp::GetClientOptions (CALL), 0x0073f4d0 CClientExoApp::GetGameObject (CALL), 0x00919723 operator new (CALL), 0x007dd380 (CALL), 0x00775990 (CALL), 0x0077f160 (CALL), ...
- indirect: 0x007f46dc slot 0xc, 0x007f4701 slot 0x98, 0x007f4736 slot 0x7c, 0x007f4774 slot 0x2c, 0x007f47a4 slot 0xd0, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-1-11

GOG function: CSWGuiMainInterface::Quicksave  gog 0x0044ccb0  callconv=(blank)  param_size_bytes=0  gog_size=148  gog_retn=0
Candidate Steam address: 0x007a06b0 (size 148, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: SendSaveGameRequest 0x73f6f0; callers: HandleInputEvent 0x7b12c0)
- best decoy (neighbour/other candidate) scores: 0x007a0750:STR0/CALLEE1/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044cc30 CClientExoAppInternal::BusyWaitMuteSound -> 0x007a0630  trusted
- GOG-order neighbour after:  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> 0x007a0750  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cbb0 CClientExoApp::SendSaveGameRequest -> Steam 0x0073f6f0  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
GOG strings (1): 'QUICKSAVE'
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x007a06b0: 16-byte aligned: YES
  007a06b0  55             PUSH EBP
  007a06b1  8B EC          MOV EBP,ESP
  007a06b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007a06b0
string literals (2):
  0x0099D9A8  at 007a06d8  'QUICKSAVE'
  0x0099D9A8  at 007a06ec  'QUICKSAVE'
direct callees (3):
  0x00733570  CExoString::CExoString_char
  0x0073F6F0  CClientExoApp::SendSaveGameRequest
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a06b0 FUN_007a06b0 size=148 ret=0 callers=1 callees=3

void FUN_007a06b0(void)

{
  uint uVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  uint local_20[2 = byte +0x8];
  uint local_18[2 = byte +0x8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967c00;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("QUICKSAVE");
  local_8 = 0;
  uVar2 = extraout_var;
  FUN_00733570("QUICKSAVE");
  local_8._0_1_ = 1;
  Concurrency::details::SchedulerResourceManagement::Statistics
            (*(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4),(uint *)0x0,local_20,local_18);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780(uVar1,uVar2);
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007a0750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0750: 16-byte aligned: YES
  007a0750  55             PUSH EBP
  007a0751  8B EC          MOV EBP,ESP
  007a0753  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0750
string literals (0):
direct callees (3):
  0x00752E10  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00752EA0  (unnamed)
struct offsets touched (top 5):
  +0x40 [dec 64]  x5  via EAX,ECX,EDX
  +0x98 [dec 152]  x4  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

### Static facts (script output, report-only)
- size 148 B, RET 0
- callers 1: 0x007b12c0 CClientExoAppInternal::HandleInputEvent
- callees: 0x00733570 CExoString::CStrConstructor (CALL), 0x0073f6f0 CClientExoApp::SendSaveGameRequest (CALL), 0x00733780 CExoString::Destructor (CALL)
- strings: 0x0099d9a8 "QUICKSAVE"
- globals: 0x00a1b4a4 READ
- twins: none

## Card B17-1-12

GOG function: CClientOptions::SetShadows  gog 0x00468f00  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc230 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc260:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00468e80 CClientOptions::SetEnableHardwareMouseCursor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00468f30 CClientOptions::SetSoftShadows -> 0x007bc260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086ab80 Global::AurEnableShadows -> Steam ?  in candidate callees: n/a
  0x0086ab90 Global::AurDisableShadows -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2c*} | Steam {0x2c*}
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
prologue for 0x007bc230: 16-byte aligned: YES
  007bc230  55             PUSH EBP
  007bc231  8B EC          MOV EBP,ESP
  007bc233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc230
string literals (0):
direct callees (2):
  0x00478CA0  (unnamed)
  0x00478CB0  Global::AurDisableShadows
struct offsets touched (top 1):
  +0x2C [dec 44]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc230 FUN_007bc230 size=43 ret=4 callers=4 callees=2

void __thiscall FUN_007bc230(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x2c) = param_2;
  if (*(int *)(param_1 + 0x2c) == 0) {
    FUN_00478cb0();
  }
  else {
    FUN_00478ca0();
  }
  return;
}
```

### decoy: 0x007bc260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc260: 16-byte aligned: YES
  007bc260  55             PUSH EBP
  007bc261  8B EC          MOV EBP,ESP
  007bc263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc260
string literals (0):
direct callees (2):
  0x00478CF0  Global::AurEnableSoftShadows
  0x00478D10  Global::AurDisableSoftShadows
struct offsets touched (top 1):
  +0x30 [dec 48]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 43 B, RET 4
- callers 4: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d5be0, 0x007d6030, 0x007d75f0
- callees: 0x00478ca0 (CALL), 0x00478cb0 (CALL)
- twins: none

## Card B17-1-13

GOG function: Vector::Dot  gog 0x00413780  callconv=(blank)  param_size_bytes=8  gog_size=52  gog_retn=0
Candidate Steam address: 0x0044b8a0 (size 52, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 67 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 67 (callers: SetCombatState 0x768440, RotateCreatureToOrientation 0x768610, SetPrecalculatedPosition 0x775aa0)
- best decoy (neighbour/other candidate) scores: 0x0044b4f0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00413720 Vector::operator-  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004137c0 CSWCCreature::ClearVisibilityList -> 0x007679f0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (67 of 143): the card's A-CALLEE credit may come from these, not from callees
  0x00414210 CSWCCreature::MoveCreatureToPosition -> Steam 0x00768440  in candidate callers: yes
  0x00414430 CSWCCreature::RotateCreatureToOrientation -> Steam 0x00768610  in candidate callers: yes
  0x00421990 CSWCCreature::SetPrecalculatedPosition -> Steam 0x00775aa0  in candidate callers: yes
  0x004264b0 CSWCCreature::GetHostileForcePowers -> Steam 0x0077a4f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x0044b8a0: 16-byte aligned: YES
  0044b8a0  55             PUSH EBP
  0044b8a1  8B EC          MOV EBP,ESP
  0044b8a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0044b8a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0044b8a0 FUN_0044b8a0 size=52 ret=0 callers=143 callees=0
THUNK -> unknown (Ghidra may omit __thiscall on thunks; ECX is passed through)

float10 FUN_0044b8a0(float *param_1,float *param_2)

{
  return (float10)(param_1[2 = byte +0x8] * param_2[2 = byte +0x8] + param_1[1 = byte +0x4] * param_2[1 = byte +0x4] + *param_1 * *param_2);
}
```

### decoy: 0x0044b4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0044b4f0: 16-byte aligned: YES
  0044b4f0  55             PUSH EBP
  0044b4f1  8B EC          MOV EBP,ESP
  0044b4f3  83 EC 6C       SUB ESP,0x6c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0044b4f0
string literals (1):
  0x0098AEB4  at 0044b7ed  'P_HK47'
direct callees (3):
  0x00513EA0  (unnamed)
  0x0044B8A0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 7):
  +0x134 [dec 308]  x13  via EAX,ECX,EDX
  +0x48 [dec 72]  x9  via EAX,ECX,EDX
  +0x58 [dec 88]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via ECX
```

### Static facts (script output, report-only)
- size 52 B, RET 0
- callers 143: 0x0044b4f0, 0x00464470 Scene::SetFogColor, 0x00468b50, 0x0047f320, 0x00481150, ...
- twins: none

## Card B17-1-14

GOG function: Global::StoreMouseTrailsState  gog 0x0040bba0  callconv=(blank)  param_size_bytes=0  gog_size=80  gog_retn=0
Candidate Steam address: 0x0040c090 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0040c0e0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040ba50 Global::RestoreResolutionFromMovie  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040bc10 Global::StoreScreenSaverState  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0xc*,0x5d*,0x5e*} | Steam {0xc*,0x5d*,0x5e*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040c090: 16-byte aligned: YES
  0040c090  55             PUSH EBP
  0040c091  8B EC          MOV EBP,ESP
  0040c093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040c090
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040c090 FUN_0040c090 size=80 ret=0 callers=2 callees=1

void FUN_0040c090(void)

{
  int local_c;
  BOOL local_8;
  
  local_c = 0;
  local_8 = SystemParametersInfoA(0x5e,0,&local_c,0);
  if ((local_8 != 0) && (0 < local_c)) {
    DAT_00a1b754 = local_c;
    SystemParametersInfoA(0x5d,0,(PVOID)0x0,0);
  }
  return;
}
```

### decoy: 0x0040c0e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c0e0: 16-byte aligned: YES
  0040c0e0  55             PUSH EBP
  0040c0e1  8B EC          MOV EBP,ESP
  0040c0e3  83 3D 54 B7 A1 00 01 CMP dword ptr [0x00a1b754],0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040c0e0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 80 B, RET 0
- callers 2: 0x00407920, 0x0040a750 MainWndProc
- callees: 0x00000129 (CALL)
- imports: SystemParametersInfoA (USER32.DLL)
- twins: none

## Card B17-1-15

GOG function: Global::InitOpenGLWindow  gog 0x00408600  callconv=(blank)  param_size_bytes=4  gog_size=320  gog_retn=0
Candidate Steam address: 0x00408af0 (size 320, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'Render Window'@00986c00; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004087a0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004082b0 Global::InitGameApp  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00408740 Global::ProcessCommandLine  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'Render Window'; 'GroupIcon'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 7 common): GOG {0x4*,0xb*,0x48*,0x65*,0x40000*,0x2ca0000*,0x80000000*} | Steam {0x4*,0xb*,0x48*,0x65*,0x40000*,0x2ca0000*,0x80000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00408af0: 16-byte aligned: YES
  00408af0  55             PUSH EBP
  00408af1  8B EC          MOV EBP,ESP
  00408af3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00408af0
string literals (1):
  0x00986C00  at 00408b18  'Render Window'
direct callees (4):
  0x007335F0  CExoString::CExoString_copy
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00408af0 FUN_00408af0 size=320 ret=0 callers=2 callees=7

void FUN_00408af0(HINSTANCE param_1)

{
  LPCSTR lpWindowName;
  DWORD dwStyle;
  int X;
  int Y;
  int nWidth;
  int nHeight;
  HWND pHVar1;
  HMENU hMenu;
  LPVOID lpParam;
  WNDCLASSA local_4c;
  char local_24 [4];
  char local_20 [4];
  char local_1c [4];
  char local_18 [4];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00947f68;
  local_10 = ExceptionList;
  local_14 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_24 = (char  [4])s_Render_Window_00986c00._0_4_;
  local_20 = (char  [4])s_Render_Window_00986c00._4_4_;
  local_1c = (char  [4])s_Render_Window_00986c00._8_4_;
  local_18._0_2_ = s_Render_Window_00986c00._12_2_;
  FUN_007335f0(&DAT_00a1b7a4);
  local_8 = 0;
  local_4c.style = 0xb;
  local_4c.lpfnWndProc = FUN_0040a750;
  local_4c.cbClsExtra = 0;
  local_4c.cbWndExtra = 0;
  local_4c.hInstance = param_1;
  local_4c.hIcon = LoadIconA(param_1,(LPCSTR)0x65);
  local_4c.hCursor = (HCURSOR)0x0;
  local_4c.hbrBackground = GetStockObject(4);
  local_4c.lpszMenuName = (LPCSTR)0x0;
  local_4c.lpszClassName = local_24;
  RegisterClassA(&local_4c);
  lpParam = (LPVOID)0x0;
  hMenu = (HMENU)0x0;
  pHVar1 = (HWND)0x0;
  nHeight = -0x80000000;
  nWidth = -0x80000000;
  Y = -0x80000000;
  X = -0x80000000;
  dwStyle = 0x2ca0000;
  lpWindowName = (LPCSTR)FUN_00734010();
  pHVar1 = CreateWindowExA(0x40000,local_24,lpWindowName,dwStyle,X,Y,nWidth,nHeight,pHVar1,hMenu,
                           param_1,lpParam);
  if (pHVar1 == (HWND)0x0) {
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    local_8 = 0xffffffff;
    DAT_00a1b484 = pHVar1;
    FUN_00733780();
  }
  ExceptionList = local_10;
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_14 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x004087a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004087a0: 16-byte aligned: YES
  004087a0  55             PUSH EBP
  004087a1  8B EC          MOV EBP,ESP
  004087a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004087a0
string literals (3):
  0x00986BF0  at 0040884f  '    v'
  0x009F42C8  at 00408996  'KotOR2'
  0x009F42C8  at 004089d3  'KotOR2'
direct callees (7):
  0x00736390  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x0091C630  (unnamed)
  0x00408AF0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 320 B, RET 0
- callers 2: 0x004087a0, 0x00409750
- callees: 0x007335f0 CExoString::CExoString_copy (CALL), 0x0000013a (CALL), 0x00000134 (CALL), 0x00734010 (CALL), 0x00000133 (CALL), 0x00733780 CExoString::Destructor (CALL), ...
- strings: 0x00986c00 "Render Window"
- globals: 0x00a1b484 WRITE
- imports: CreateWindowExA (USER32.DLL), GetStockObject (GDI32.DLL), LoadIconA (USER32.DLL), ...
- twins: none

## Card B17-1-16

GOG function: PartEmitter::RenderBoundingSphere  gog 0x008b3c00  callconv=(blank)  param_size_bytes=0  gog_size=7939  gog_retn=0
Candidate Steam address: 0x004f5f50 (size 8362, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.13) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.86 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f8000:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b1f40 PartEmitter::RenderStandard  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008b5b40 PartEmitter::StartEmitterLayer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x0080ba90 Global::LookupCosine32Radians -> Steam ?  in candidate callees: n/a
  0x0080bad0 Global::LookupSine32Radians -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x00823860 GLRender::RenderObjectsVTCQuadUP -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 29 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+15)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+17)}
imm sets (* = in both, 15 common): GOG {0x2*,0x3,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0x60,0xb4*,0x11c*,0x17c*,...(+4)} | Steam {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0xb4*,0x11c*,0x17c*,0x188*,0x4f8,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5f50: 16-byte aligned: YES
  004f5f50  55             PUSH EBP
  004f5f51  8B EC          MOV EBP,ESP
  004f5f53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004f5f50
string literals (0):
direct callees (19):
  0x00919723  operator new
  0x0046CDA0  (unnamed)
  0x0046CE40  (unnamed)
  0x00463120  (unnamed)
  0x00428F80  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x004292D0  (unnamed)
  0x004F39B0  (unnamed)
  0x004F5D90  (unnamed)
  0x004DAE60  GUI image texture bind (MakeCurrent via vtbl +0xB8)
  0x004DB1F0  (unnamed)
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x21  via EAX,ECX,EDX
  +0x8 [dec 8]  x20  via EAX,ECX,EDX
  +0x1CC [dec 460]  x11  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x9  via EAX,ECX,EDX
  +0x98 [dec 152]  x8  via EAX,ECX,EDX
  +0x9C [dec 156]  x7  via EAX,ECX,EDX
  +0x40 [dec 64]  x6  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x6  via ECX,EDX
  +0xEC [dec 236]  x5  via EAX,ECX,EDX
  +0x1C8 [dec 456]  x5  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004f5f50 FUN_004f5f50 size=8362 ret=0 callers=1 callees=24

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5f50(int *param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  undefined4 *puVar7;
  float *pfVar8;
  float *pfVar9;
  float10 fVar10;
  int local_4f8;
  int local_4e8;
  int local_4d8;
  uint local_4c8;
  float local_4b0;
  float local_4ac;
  float local_4a8;
  float local_4a4;
  float local_4a0;
  float local_49c;
  float local_498;
  float local_494;
  float local_490;
  float local_48c;
  float local_488;
  float local_484;
  float local_480;
  float local_47c;
  float local_478;
  float local_474;
  int local_458;
  int local_454;
  double local_450;
  undefined1 local_17c [12];
  undefined1 local_170 [12];
  undefined1 local_164 [12];
  int local_158;
  int local_154;
  float local_150;
  float local_14c;
  float local_148;
  uint local_144;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  float local_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  float local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
/* ... 525 more lines truncated by --max-lines */
```

### decoy: 0x004f8000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f8000: 16-byte aligned: YES
  004f8000  55             PUSH EBP
  004f8001  8B EC          MOV EBP,ESP
  004f8003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004f8000
string literals (0):
direct callees (19):
  0x00919723  operator new
  0x0046CDA0  (unnamed)
  0x0046CE40  (unnamed)
  0x004F39B0  (unnamed)
  0x004F5D90  (unnamed)
  0x004DAE60  GUI image texture bind (MakeCurrent via vtbl +0xB8)
  0x004DB1F0  (unnamed)
  0x00463120  (unnamed)
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x00513D20  (unnamed)
  0x005143C0  (unnamed)
  0x00513CE0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00431140  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x11  via EAX,ECX,EDX
  +0x98 [dec 152]  x8  via EAX,ECX,EDX
  +0x1CC [dec 460]  x8  via EAX,ECX,EDX
  +0x9C [dec 156]  x7  via EAX,ECX,EDX
  +0x40 [dec 64]  x6  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x6  via EAX,ECX,EDX
  +0x54 [dec 84]  x4  via EAX,ECX,EDX
  +0x90 [dec 144]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 8362 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x00919723 operator new (CALL), 0x0046cda0 (CALL), 0x0046ce40 (CALL), 0x00463120 (CALL), 0x00428f80 (CALL), 0x00473e50 (CALL), ...
- indirect: 0x004f61ea slot 0x84
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B17-1-17

GOG function: CExoArrayList::IndexOf  gog 0x0044dc90  callconv=__thiscall  param_size_bytes=4  gog_size=70  gog_retn=4
Candidate Steam address: 0x004142e0 (size 70, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 9 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 9 (callers: DoPassiveSelection 0x79a4c0, OnUpgradeSelected 0x8cdb00, OnSlotSelected 0x8ceb00); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00414280:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044db90 CClientExoAppInternal::UnloadSuperModelSet -> 0x007a1590  trusted
- GOG-order neighbour after:  0x0044dce0 CExoArrayList_CExoString_::CExoArrayList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (9 of 36): the card's A-CALLEE credit may come from these, not from callees
  0x00446ae0 CClientExoAppInternal::DoPassiveSelection -> Steam 0x0079a4c0  in candidate callers: yes
  0x005bdec0 CSWGuiUpgrade::OnUpgradeSelected -> Steam 0x008cdb00  in candidate callers: yes
  0x005beec0 CSWGuiUpgrade::OnSlotSelected -> Steam 0x008ceb00  in candidate callers: yes
  0x005bfed0 CSWGuiUpgrade::InsertUpgrade -> Steam 0x008cfb10  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004142e0: 16-byte aligned: YES
  004142e0  55             PUSH EBP
  004142e1  8B EC          MOV EBP,ESP
  004142e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004142e0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004142e0 FUN_004142e0 size=70 ret=4 callers=36 callees=0

int __thiscall FUN_004142e0(int *param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (param_1[1 = byte +0x4] <= local_8) {
      return -1;
    }
    if (*(int *)(*param_1 + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}
```

### decoy: 0x00414280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414280: 16-byte aligned: YES
  00414280  55             PUSH EBP
  00414281  8B EC          MOV EBP,ESP
  00414283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414280
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 70 B, RET 4
- callers 36: 0x0040fd80 CSWGuiPanel::AddControl, 0x0040fdc0, 0x0040fe10, 0x00410780, 0x00410aa0, ...
- twins: none

## Card B17-1-18

GOG function: CAurInternal::RestoreDesktopDisplayMode  gog 0x008689c0  callconv=(blank)  param_size_bytes=0  gog_size=56  gog_retn=0
Candidate Steam address: 0x00476890 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurInternalGL::IsFullscreen | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00476870:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00868790 CAurInternal::GetValidMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00868a00 CAurInternal::RestoreAuroraDisplayMode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x24,0xa8} | Steam {0xac}
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
prologue for 0x00476890: 16-byte aligned: YES
  00476890  55             PUSH EBP
  00476891  8B EC          MOV EBP,ESP
  00476893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00476890
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xAC [dec 172]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00476890 FUN_00476890 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_00476890(int param_1)

{
  return *(undefined4 *)(param_1 + 0xac);
}
```

### decoy: 0x00476870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00476870: 16-byte aligned: YES
  00476870  55             PUSH EBP
  00476871  8B EC          MOV EBP,ESP
  00476873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00476870
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 20 B, RET 0
- twins: none

## Card B17-1-19

GOG function: Global::UpdateScreen  gog 0x004099e0  callconv=__cdecl  param_size_bytes=12  gog_size=646  gog_retn=0
Candidate Steam address: 0x00409ed0 (size 646, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 12 (callees: IsMoviePlaying 0x740160, MainLoop 0x51c470, AurPostString 0x474c00; callers: HideLoadScreen 0x7983e0, LoadArea 0x7a2fb0, Initialize 0x7be4c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00409e10:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00409920 Global::DoConsoleHist  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00409c70 Global::HandleWMKeyDownMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x00408250 Global::glClear_glClear_int_engineMask -> Steam ?  in candidate callees: n/a
  0x00409000 Global::messagepump -> Steam ?  in candidate callees: n/a
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam 0x0073fea0  in candidate callees: yes
  0x0040d620 CClientExoApp::IsMoviePlaying -> Steam 0x00740160  in candidate callees: yes
  0x0063a890 CExoSound::Render -> Steam ?  in candidate callees: n/a
  0x0063fad0 CNetLayer::ProcessReceivedFrames -> Steam ?  in candidate callees: n/a
  0x00640350 CNetLayer::UpdateStatusLoop -> Steam ?  in candidate callees: n/a
  0x0064be90 CServerExoApp::MainLoop -> Steam 0x0051c470  in candidate callees: yes
  ... 8 more GOG callees
GOG callers mapped through trusted rows (9 of 17): the card's A-CALLEE credit may come from these, not from callees
  0x00444a00 CClientExoAppInternal::HideLoadScreen -> Steam 0x007983e0  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004ceb50 CGuiInGame::Initialize -> Steam 0x007be4c0  in candidate callers: yes
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x4*,0xc*,0x10*,0x50*} | Steam {0x2*,0x3*,0x4*,0xc*,0x10*,0x50*}

### abi
```
GOG param bytes: 12; callconv: cdecl
expected_ret = 0 (cdecl: caller cleans the stack)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00409ed0: 16-byte aligned: YES
  00409ed0  55             PUSH EBP
  00409ed1  8B EC          MOV EBP,ESP
  00409ed3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00409ed0
string literals (0):
direct callees (16):
  0x00478A20  Global::AurMakeCurrent
  0x00740160  CClientExoApp::IsMoviePlaying
  0x00408740  (unnamed)
  0x004766B0  Global::AurEnableHeadLight
  0x004766E0  Global::AurDisableHeadLight
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x004113A0  CSWGuiManager::Draw
  0x00474C00  Global::AurPostString
  0x00474BB0  (unnamed)
  0x00436F70  (unnamed)
  0x00436B90  (unnamed)
  0x004094F0  MessagePump
  0x0051C470  CServerExoApp::MainLoop
  0x00703F30  CNetLayer::ProcessReceivedFrames (mapper `vote`, CONFIRMED by body)
  0x0070B8F0  CExoSound::Render wrapper
  0x007047B0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00409ed0 FUN_00409ed0 size=646 ret=0 callers=17 callees=19

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00409ed0(undefined4 param_1,int param_2,int param_3)

{
  char cVar1;
  int iVar2;
  HDC hDC;
  undefined4 uVar3;
  undefined *puVar4;
  int local_c;
  
  FUN_00478a20();
  if ((param_3 == 0) && (iVar2 = FUN_00740160(), iVar2 == 0)) {
    FUN_00408740(3);
    if (DAT_00a1b6f8 == 0) {
      FUN_004766e0();
    }
    else {
      FUN_004766b0();
    }
  }
  FUN_0073fea0(param_1);
  FUN_004113a0(param_1);
  if ((param_3 == 0) && (iVar2 = FUN_00740160(), iVar2 == 0)) {
    if (DAT_00a1b4e8 != 0) {
      FUN_00474c00(&DAT_00986b64,0,0,_DAT_00986b68);
      FUN_00474c00(&DAT_00a1b4f4,2,0,_DAT_00986b68);
    }
    FUN_00474bb0();
    hDC = GetDC(DAT_00a1b484);
    cVar1 = FUN_00436f70();
    if (cVar1 != '\0') {
      FUN_00436b90();
      SwapBuffers(hDC);
    }
    ReleaseDC(DAT_00a1b484,hDC);
  }
  FUN_004094f0();
  if ((param_2 == 1) && (DAT_00a1b4a4 != 0)) {
    if (*(int *)(DAT_00a1b4a4 + 8) != 0) {
      for (local_c = 0; local_c < 1; local_c = local_c + 1) {
        FUN_0051c470();
        FUN_004094f0();
        if (*(int *)(DAT_00a1b4a4 + 4) != 0) {
          uVar3 = 0;
          (**(code **)(**(int **)(DAT_00a1b4a4 + 4) + 0x10))(0);
          FUN_00703f30(uVar3);
        }
      }
    }
    if (*(int *)(DAT_00a1b4a4 + 4) != 0) {
      uVar3 = 0;
      (**(code **)(**(int **)(DAT_00a1b4a4 + 4) + 0x10))(0);
      FUN_00703f30(uVar3);
    }
  }
  if (DAT_00a1b494 != 0) {
    FUN_0070b8f0();
  }
  if (((DAT_00a1b4a4 != 0) && (*(int *)(DAT_00a1b4a4 + 4) != 0)) &&
     (iVar2 = (**(code **)(**(int **)(DAT_00a1b4a4 + 4) + 0x10))(), iVar2 != 0)) {
    puVar4 = PTR_IMAGE_DOS_HEADER_0099b6d0;
    (**(code **)(**(int **)(DAT_00a1b4a4 + 4) + 0x10))(PTR_IMAGE_DOS_HEADER_0099b6d0);
    FUN_007047b0(puVar4);
  }
  uVar3 = 0;
  FUN_0073fea0(0);
  FUN_004113a0(uVar3);
  return;
}
```

### decoy: 0x00409e10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00409e10: 16-byte aligned: YES
  00409e10  55             PUSH EBP
  00409e11  8B EC          MOV EBP,ESP
  00409e13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00409e10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x9F4928 [dec 10438952]  x6  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 646 B, RET 0
- callers 17: 0x0040dac0, 0x0052f610 CServerExoAppInternal::SaveGame, 0x005345b0, 0x00534600 CServerExoAppInternal::StallEventSaveGame, 0x0053af20 CServerExoAppInternal::DoPCAutosave, ...
- callees: 0x00478a20 (CALL), 0x00740160 CClientExoApp::IsMoviePlaying (CALL), 0x00408740 (CALL), 0x004766b0 (CALL), 0x004766e0 (CALL), 0x0073fea0 CClientExoApp::GetSWGuiManager (CALL), ...
- indirect: 0x0040a06e slot 0x10, 0x0040a0aa slot 0x10, 0x0040a0ff slot 0x10, 0x0040a129 slot 0x10
- globals: 0x00a1b484 READ, 0x00a1b484 READ, 0x00a1b494 READ, ...
- imports: GetDC (USER32.DLL), ReleaseDC (USER32.DLL), SwapBuffers (GDI32.DLL)
- twins: none

## Card B17-1-20

GOG function: CClientOptions::SetSoftShadows  gog 0x00468f30  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc260 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc230:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00468f00 CClientOptions::SetShadows -> 0x007bc230  gate_class=REVIEW
- GOG-order neighbour after:  0x00468f60 CClientOptions::SetGrass  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086abd0 Global::AurEnableSoftShadows -> Steam ?  in candidate callees: n/a
  0x0086abf0 Global::AurDisableSoftShadows -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
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
prologue for 0x007bc260: 16-byte aligned: YES
  007bc260  55             PUSH EBP
  007bc261  8B EC          MOV EBP,ESP
  007bc263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc260
string literals (0):
direct callees (2):
  0x00478CF0  Global::AurEnableSoftShadows
  0x00478D10  Global::AurDisableSoftShadows
struct offsets touched (top 1):
  +0x30 [dec 48]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc260 FUN_007bc260 size=43 ret=4 callers=3 callees=2

void __thiscall FUN_007bc260(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x30) = param_2;
  if (*(int *)(param_1 + 0x30) == 0) {
    FUN_00478d10();
  }
  else {
    FUN_00478cf0();
  }
  return;
}
```

### decoy: 0x007bc230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc230: 16-byte aligned: YES
  007bc230  55             PUSH EBP
  007bc231  8B EC          MOV EBP,ESP
  007bc233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc230
string literals (0):
direct callees (2):
  0x00478CA0  (unnamed)
  0x00478CB0  Global::AurDisableShadows
struct offsets touched (top 1):
  +0x2C [dec 44]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 43 B, RET 4
- callers 3: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d5be0, 0x007d75f0
- callees: 0x00478cf0 (CALL), 0x00478d10 (CALL)
- twins: none

## Card B17-1-21

GOG function: Global::glClear_glClear_int_engineMask  gog 0x00408250  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00408740 (size 90, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00407920:STR0/CALLEE0/SIG0.043/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00407450 Global::WinMain  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004082b0 Global::InitGameApp  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x400*,0x500*,0x4000*} | Steam {0x2*,0x4*,0x400*,0x500*,0x4000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00408740: 16-byte aligned: YES
  00408740  55             PUSH EBP
  00408741  8B EC          MOV EBP,ESP
  00408743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00408740
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00408740 FUN_00408740 size=90 ret=0 callers=7 callees=1

void FUN_00408740(uint param_1)

{
  glClear(-(uint)((param_1 & 2) != 0) & 0x500 |
          -(uint)((param_1 & 1) != 0) & 0x4000 | -(uint)((param_1 & 4) != 0) & 0x400);
  return;
}
```

### decoy: 0x00407920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00407920: 16-byte aligned: YES
  00407920  55             PUSH EBP
  00407921  8B EC          MOV EBP,ESP
  00407923  81 EC 98 02 00 00 SUB ESP,0x298
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00407920
string literals (9):
  0x00986BE4  at 0040794f  'swkotor2'
  0x00986BD8  at 004079ad  'config.txt'
  0x00986BC8  at 00407a07  'swKotor2.ini'
  0x00986BB8  at 00407ad9  'Disable Sound'
  0x00986BA8  at 00407ae9  'Sound Options'
  0x00986B98  at 00407af9  '.\\\\swkotor2.ini'
  0x00986BB8  at 00407ba9  'Disable Sound'
  0x00986BA8  at 00407bb9  'Sound Options'
  0x00986B98  at 00407bc9  '.\\\\swkotor2.ini'
direct callees (58):
  0x00402510  (unnamed)
  0x0091C630  (unnamed)
  0x004763B0  (unnamed)
  0x00919723  operator new
  0x00735C60  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x007360E0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00401730  CAppManager::Constructor
  0x004087A0  (unnamed)
  0x004072B0  (unnamed)
  0x004072F0  CSWCObject::TestAndReEquipweapons
  0x004076F0  (unnamed)
  0x0040C090  (unnamed)
  0x0040C100  (unnamed)
  0x00736240  (unnamed)
  0x00733540  CExoString::Constructor
  0x00736320  (unnamed)
  0x0091CF18  (unnamed)
  0x00734270  CExoString::Format
struct offsets touched (top 4):
  +0x4 [dec 4]  x26  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x4C4 [dec 1220]  x1  via EAX
```

### Static facts (script output, report-only)
- size 90 B, RET 0
- callers 7: 0x00407920, 0x00409ed0, 0x004289d0, 0x00469950, 0x00469ac0, ...
- callees: 0x00000011 (CALL)
- imports: glClear (OPENGL32.DLL)
- twins: none

## Card B17-1-22

GOG function: CSWPlayerList::GetFirstPlayer  gog 0x0044e290  callconv=(blank)  param_size_bytes=0  gog_size=52  gog_retn=0
Candidate Steam address: 0x00521060 (size 52, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 40 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 40 (callers: GetCreatureLight 0x774a80, ClearPauseExclusionList 0x7845b0, Uninitialize 0x7856c0)
- best decoy (neighbour/other candidate) scores: 0x00520fc0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044e110 CExoArrayList_CGuiStringStruct_::Insert  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044e390 CSWRumblePattern::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (41 of 84): the card's A-CALLEE credit may come from these, not from callees
  0x00420950 CSWCCreature::GetCreatureLight -> Steam 0x00774a80  in candidate callers: yes
  0x00430bd0 CClientExoAppInternal::ClearPauseExclusionList -> Steam 0x007845b0  in candidate callers: yes
  0x00431ce0 CClientExoAppInternal::Uninitialize -> Steam 0x007856c0  in candidate callers: yes
  0x004382a0 CClientExoAppInternal::GetPlayerByPlayerID -> Steam 0x0078bcc0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x00521060: 16-byte aligned: YES
  00521060  55             PUSH EBP
  00521061  8B EC          MOV EBP,ESP
  00521063  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00521060
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00521060 FUN_00521060 size=52 ret=0 callers=84 callees=0

undefined4 __fastcall FUN_00521060(undefined4 *param_1)

{
  undefined4 local_c;
  
  if (*(int *)*param_1 == 0) {
    local_c = 0;
  }
  else {
    local_c = *(undefined4 *)(*(int *)*param_1 + 0x8 [dec 8] {undefined4*: byte +0x20});
  }
  return local_c;
}
```

### decoy: 0x00520fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00520fc0: 16-byte aligned: YES
  00520fc0  55             PUSH EBP
  00520fc1  8B EC          MOV EBP,ESP
  00520fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00520fc0
string literals (4):
  0x00992EF0  at 00520fcf  'BodyBagId'
  0x00992EE4  at 00520fef  'PositionX'
  0x00992ED8  at 00521010  'PositionY'
  0x00992ECC  at 00521031  'PositionZ'
direct callees (2):
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718D00  CResGFF::ReadFieldFLOAT
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
```

### Static facts (script output, report-only)
- size 52 B, RET 0
- callers 84: 0x0051d790 CServerAIMaster::UpdateState, 0x0051ed10 CServerAIMaster::EventPending, 0x0052c820 CSWSArea::SetCurrentWeather, 0x00533830 CServerExoAppInternal::MainLoop, 0x0053fba0, ...
- twins: none
