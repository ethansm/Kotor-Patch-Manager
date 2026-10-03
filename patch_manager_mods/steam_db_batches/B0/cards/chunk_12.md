# Review cards B0 chunk 12

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-12-01

GOG function: CWorldTimer::GetWorldTimeMinute  gog 0x0080b620  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x0051b140 (size 49, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b100:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b5e0 CWorldTimer::GetWorldTimeHour -> 0x0051b100  gate_class=REVIEW
- GOG-order neighbour after:  0x0080b660 CWorldTimer::GetWorldTimeSecond -> 0x0051b180  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080b020 CWorldTimer::ConvertFromTimeOfDay -> Steam 0x0051ab40  in candidate callees: yes
  0x0080b5c0 Global::GetWorldTimeTimeOfDay -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0051b140: 16-byte aligned: YES
  0051b140  55             PUSH EBP
  0051b141  8B EC          MOV EBP,ESP
  0051b143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b140
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051b140 FUN_0051b140 size=49 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0051b140(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined4 *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined4 local_c;
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = &local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_c;
}
```

### decoy: 0x0051b100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b100: 16-byte aligned: YES
  0051b100  55             PUSH EBP
  0051b101  8B EC          MOV EBP,ESP
  0051b103  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b100
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

## Card B0-12-02

GOG function: CSWSScriptVarTable::GetObjectA  gog 0x007f0180  callconv=(blank)  param_size_bytes=4  gog_size=60  gog_retn=4
Candidate Steam address: 0x005e68f0 (size 60, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e67d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f00e0 CSWSScriptVarTable::GetString -> 0x005e6850  gate_class=AUTO
- GOG-order neighbour after:  0x007f01c0 CSWSScriptVarTable::GetLocation -> 0x005e6930  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007efe10 CSWSScriptVarTable::MatchIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}
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
prologue for 0x005e68f0: 16-byte aligned: YES
  005e68f0  55             PUSH EBP
  005e68f1  8B EC          MOV EBP,ESP
  005e68f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e68f0
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005e68f0 FUN_005e68f0 size=60 ret=4 callers=2 callees=1

undefined4 FUN_005e68f0(undefined4 param_1)

{
  int iVar1;
  undefined4 local_c;
  
  local_c = DAT_00994484;
  iVar1 = FUN_005e6580(param_1,4,0);
  if (iVar1 != 0) {
    local_c = *(undefined4 *)(iVar1 + 0xc);
  }
  return local_c;
}
```

### decoy: 0x005e67d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e67d0: 16-byte aligned: YES
  005e67d0  55             PUSH EBP
  005e67d1  8B EC          MOV EBP,ESP
  005e67d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e67d0
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via ECX
```

## Card B0-12-03

GOG function: Gob::GetName  gog 0x0084bcd0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x004596f0 (size 17, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459670:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bc50 CAurObjectLightContainer::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bcf0 Gob::GetPosition -> 0x00459710  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x64*} | Steam {0x64*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004596f0: 16-byte aligned: YES
  004596f0  55             PUSH EBP
  004596f1  8B EC          MOV EBP,ESP
  004596f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004596f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004596f0 FUN_004596f0 size=17 ret=0 callers=0 callees=0

int __fastcall FUN_004596f0(int param_1)

{
  return param_1 + 0x64 [dec 100];
}
```

### decoy: 0x00459670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459670: 16-byte aligned: YES
  00459670  55             PUSH EBP
  00459671  8B EC          MOV EBP,ESP
  00459673  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00459670
string literals (0):
direct callees (3):
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00463B20  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-12-04

GOG function: CSWSScriptVarTable::DestroyFloat  gog 0x007f07b0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x005e6f20 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e6f60:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f0670 CSWSScriptVarTable::Destructor -> 0x005e6de0  gate_class=AUTO
- GOG-order neighbour after:  0x007f07d0 CSWSScriptVarTable::DestroyString -> 0x005e6f40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007effe0 CSWSScriptVarTable::DeleteIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*} | Steam {0x2*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e6f20: 16-byte aligned: YES
  005e6f20  55             PUSH EBP
  005e6f21  8B EC          MOV EBP,ESP
  005e6f23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6f20
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e6f20 FUN_005e6f20 size=27 ret=4 callers=1 callees=1
THUNK -> 0x005E6750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e6f20(undefined4 param_1)

{
  FUN_005e6750(param_1,2);
  return;
}
```

### decoy: 0x005e6f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e6f60: 16-byte aligned: YES
  005e6f60  55             PUSH EBP
  005e6f61  8B EC          MOV EBP,ESP
  005e6f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e6f60
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

## Card B0-12-05

GOG function: CSWSItemPropertyHandler::ApplySkillBonus  gog 0x007c9340  callconv=__thiscall  param_size_bytes=12  gog_size=323  gog_retn=20
Candidate Steam address: 0x005b1f30 (size 323, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b21c0:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9110 CSWSItemPropertyHandler::RemoveImmunity -> 0x005b1d00  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9490 CSWSItemPropertyHandler::RemoveSkillBonus -> 0x005b2080  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*,0x122*}
imm sets (* = in both, 6 common): GOG {0x2*,0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*} | Steam {0x2*,0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b1f30: 16-byte aligned: YES
  005b1f30  55             PUSH EBP
  005b1f31  8B EC          MOV EBP,ESP
  005b1f33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b1f30
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x122 [dec 290]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1f30 FUN_005b1f30 size=323 ret=20 callers=0 callees=5

undefined4
FUN_005b1f30(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_3c;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  sVar1 = *(short *)(param_2 + 6);
  if (sVar1 != 0) {
    iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_28 = (undefined2)DAT_00995ff8;
    *(undefined2 *)(local_3c + 8) = local_28;
    *(ushort *)(local_3c + 0xa [dec 10]) = *(ushort *)(local_3c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,sVar1);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    FUN_00544210(local_3c,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b21c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b21c0: 16-byte aligned: YES
  005b21c0  55             PUSH EBP
  005b21c1  8B EC          MOV EBP,ESP
  005b21c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b21c0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

## Card B0-12-06

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityDecrease  gog 0x0073dbd0  callconv=(blank)  param_size_bytes=8  gog_size=241  gog_retn=12
Candidate Steam address: 0x0059b520 (size 836, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyEntangled
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.35 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059b870:STR0/CALLEE0/SIG0.196/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073dcd0 CSWSEffectListHandler::OnRemoveDamageImmunityDecrease -> 0x0059b3e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x8*,0x18*,0x30*,0xb4,0xfc*,0x1198*} | Steam {0x4,0x8*,0x18*,0x30*,0xfc*,0x122,0x1198*}
imm sets (* = in both, 1 common): GOG {0xc*,0x16,0x24} | Steam {0x2,0x3,0x4,0xa,0xb,0xc*,0x17,0x25,0x34,0x74,0x8c,0x90,0xfffffffd}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059b520: 16-byte aligned: YES
  0059b520  55             PUSH EBP
  0059b521  8B EC          MOV EBP,ESP
  0059b523  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0059b520
string literals (0):
direct callees (11):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x00541080  CSWSObject::ClearAllActions
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0x122 [dec 290]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b520 FUN_0059b520 size=836 ret=12 callers=0 callees=11

undefined4 FUN_0059b520(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_84;
  int local_80;
  int local_7c;
  undefined4 local_78;
  undefined4 local_74;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952067;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar1 = (**(code **)(*param_1 + 0x30))();
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(10,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_74 = 0;
      }
      else {
        local_74 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_78 = 0;
        }
        else {
          local_78 = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_78);
        FUN_00584da0(0x90,local_78);
      }
      uVar4 = 0x90;
      (**(code **)(*param_1 + 0x30))(0x90,local_74);
      FUN_00584da0(uVar4,local_74);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    if (param_2 != 0) {
      FUN_00541080(1);
      iVar1 = FUN_00919723(0x8c);
      local_8 = 2;
      if (iVar1 == 0) {
        local_7c = 0;
      }
      else {
        local_7c = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
      iVar1 = FUN_00919723(0x8c);
      local_8 = 3;
      if (iVar1 == 0) {
        local_80 = 0;
      }
      else {
        local_80 = FUN_005e4ac0(param_2);
      }
      local_8 = 0xffffffff;
/* ... 26 more lines truncated by --max-lines */
```

### decoy: 0x0059b870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b870: 16-byte aligned: YES
  0059b870  55             PUSH EBP
  0059b871  8B EC          MOV EBP,ESP
  0059b873  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b870
string literals (0):
direct callees (2):
  0x005D1510  CSWSCreature::ClearVisibleList
  0x005D0F30  CSWSObject::UpdatePerception
struct offsets touched (top 2):
  +0x30 [dec 48]  x2  via EDX
  +0x10E8 [dec 4328]  x1  via ECX
```

## Card B0-12-07

GOG function: CSWSItemPropertyHandler::RemoveSkillBonus  gog 0x007c9490  callconv=__thiscall  param_size_bytes=12  gog_size=320  gog_retn=16
Candidate Steam address: 0x005b2080 (size 320, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2340:STR0/CALLEE0/SIG0.633/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9340 CSWSItemPropertyHandler::ApplySkillBonus -> 0x005b1f30  gate_class=REVIEW
- GOG-order neighbour after:  0x007c95d0 CSWSItemPropertyHandler::ApplyRegeneration -> 0x005b21c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x13c*,0x1198*} | Steam {0x2*,0x4*,0x6*,0x8*,0xa*,0x14*,0x18*,0x13c*,0x1198*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x38*,0x148*} | Steam {0x7*,0x10*,0x38*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b2080: 16-byte aligned: YES
  005b2080  55             PUSH EBP
  005b2081  8B EC          MOV EBP,ESP
  005b2083  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2080
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 9):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x13C [dec 316]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2080 FUN_005b2080 size=320 ret=16 callers=0 callees=2

undefined4 FUN_005b2080(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  uint local_10;
  
  local_10 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x13c);
  while( true ) {
    if (*(int *)(param_3 + 0x14c) <= (int)local_10) {
      return 0;
    }
    if ((int)DAT_00995ff8 <
        (int)(uint)*(ushort *)(*(int *)(*(int *)(param_3 + 0x148) + local_10 * 4) + 8)) break;
    puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_10 * 4);
    if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995ff8) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
        (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
       ((uVar1 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar1 &&
        (uVar1 = *(ushort *)(param_2 + 6), uVar3 = FUN_005e4e60(1), uVar3 == uVar1)))) {
      FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
      return 0;
    }
    local_10 = local_10 + 1;
  }
  return 0;
}
```

### decoy: 0x005b2340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2340: 16-byte aligned: YES
  005b2340  55             PUSH EBP
  005b2341  8B EC          MOV EBP,ESP
  005b2343  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2340
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-12-08

GOG function: CSWSEffectListHandler::OnApplyDamageImmunityDecrease  gog 0x0073dbd0  callconv=(blank)  param_size_bytes=8  gog_size=241  gog_retn=12
Candidate Steam address: 0x0059b130 (size 110, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyDamageImmunityIncrease
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0059b1a0:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0073da90 CSWSEffectListHandler::OnRemoveDamageImmunityIncrease -> 0x0059b1a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0073dcd0 CSWSEffectListHandler::OnRemoveDamageImmunityDecrease -> 0x0059b3e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x006a9ed0 CSWSObject::SetDamageImmunity -> Steam ?  in candidate callees: n/a
  0x006f89f0 CSWSCreatureStats::GetEffectImmunity -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8,0x18,0x30,0xb4*,0xfc,0x1198} | Steam {0xb4*}
imm sets (* = in both, 1 common): GOG {0xc*,0x16,0x24} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x0059b130: 16-byte aligned: YES
  0059b130  55             PUSH EBP
  0059b131  8B EC          MOV EBP,ESP
  0059b133  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0059b130
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 1):
  +0xB4 [dec 180]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0059b130 FUN_0059b130 size=110 ret=12 callers=0 callees=2

undefined4 FUN_0059b130(int *param_1,int param_2)

{
  char cVar1;
  undefined2 uVar2;
  int iVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  
  if (param_2 != 0) {
    uVar2 = FUN_005e4e60(0);
    uVar4 = extraout_var;
    iVar3 = FUN_005e4e60(1);
    if (iVar3 < 0) {
      return 1;
    }
    cVar1 = (**(code **)(*param_1 + 0xb4))(uVar2,uVar4);
    FUN_005431f0(uVar2,cVar1 + iVar3 & 0xff);
  }
  return 0;
}
```

### decoy: 0x0059b1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0059b1a0: 16-byte aligned: YES
  0059b1a0  55             PUSH EBP
  0059b1a1  8B EC          MOV EBP,ESP
  0059b1a3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0059b1a0
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x005431F0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0xB4 [dec 180]  x1  via EDX
```

## Card B0-12-09

GOG function: CWorldTimer::GetWorldTimeMillisecond  gog 0x0080b6a0  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x0051b1c0 (size 49, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b180:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b660 CWorldTimer::GetWorldTimeSecond -> 0x0051b180  gate_class=REVIEW
- GOG-order neighbour after:  0x0080b6e0 CWorldTimer::SetFixedSnapshotRate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080b020 CWorldTimer::ConvertFromTimeOfDay -> Steam 0x0051ab40  in candidate callees: yes
  0x0080b5c0 Global::GetWorldTimeTimeOfDay -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0051b1c0: 16-byte aligned: YES
  0051b1c0  55             PUSH EBP
  0051b1c1  8B EC          MOV EBP,ESP
  0051b1c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b1c0
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051b1c0 FUN_0051b1c0 size=49 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0051b1c0(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined4 *puVar5;
  undefined1 local_14 [4];
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined4 local_8;
  
  puVar5 = &local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_8;
}
```

### decoy: 0x0051b180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b180: 16-byte aligned: YES
  0051b180  55             PUSH EBP
  0051b181  8B EC          MOV EBP,ESP
  0051b183  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b180
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

## Card B0-12-10

GOG function: CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEvent  gog 0x007f98e0  callconv=__thiscall  param_size_bytes=8  gog_size=249  gog_retn=8
Candidate Steam address: 0x006f6320 (size 249, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006f6420:STR0/CALLEE0/SIG0.45/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f97c0 Global::ExecuteCommandAurPostString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007f99e0 CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEventModelName -> 0x006f6420  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040d550 CClientExoApp::GetMGOArray -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00539d40 CSWMiniGameObjectArray::GetMiniGameObject -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00640cb0 CVirtualMachine::StackPushString -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 4 common): GOG {0x8*,0x10*,0x28*,0xfffff830*} | Steam {0x8*,0x10*,0x28*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006f6320: 16-byte aligned: YES
  006f6320  55             PUSH EBP
  006f6321  8B EC          MOV EBP,ESP
  006f6323  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006f6320
string literals (0):
direct callees (8):
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006f6320 FUN_006f6320 size=249 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_006f6320(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_28 [11];
  char local_1d;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec40;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar3 = 0;
  uVar1 = *(undefined4 *)(param_1 + 8);
  FUN_00740070(uVar1,0,DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1,
               *(undefined4 *)(DAT_00a1b4a4 + 4));
  local_14 = FUN_00832d90(uVar1,uVar3);
  FUN_00733540();
  local_8 = 0;
  if (local_14 != 0) {
    FUN_007337c0(local_14 + 0x10);
  }
  uVar1 = FUN_00734010();
  FUN_00733570(uVar1);
  local_8._0_1_ = 1;
  iVar2 = FUN_006fda90(local_28);
  local_1d = '\x01' - (iVar2 != 0);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  if (local_1d == '\0') {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0;
  }
  else {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0xfffff830;
  }
  ExceptionList = local_10;
  return uVar1;
}
```

### decoy: 0x006f6420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f6420: 16-byte aligned: YES
  006f6420  55             PUSH EBP
  006f6421  8B EC          MOV EBP,ESP
  006f6423  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006f6420
string literals (0):
direct callees (8):
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

## Card B0-12-11

GOG function: CSWSAreaOfEffectObject::SetEffectSpellId  gog 0x007e6370  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x0062c1a0 (size 25, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0062c180:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e6350 CSWSAreaOfEffectObject::GetEffectSpellId -> 0x0062c180  gate_class=REVIEW
- GOG-order neighbour after:  0x007e6390 CSWSAreaOfEffectObject::Destructor -> 0x0062c1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x274*} | Steam {0x274*}
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
prologue for 0x0062c1a0: 16-byte aligned: YES
  0062c1a0  55             PUSH EBP
  0062c1a1  8B EC          MOV EBP,ESP
  0062c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0062c1a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x274 [dec 628]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0062c1a0 FUN_0062c1a0 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_0062c1a0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x274) = param_2;
  return;
}
```

### decoy: 0x0062c180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0062c180: 16-byte aligned: YES
  0062c180  55             PUSH EBP
  0062c181  8B EC          MOV EBP,ESP
  0062c183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0062c180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x274 [dec 628]  x1  via EAX
```

## Card B0-12-12

GOG function: CSWSEncounter::Constructor  gog 0x007e9150  callconv=(blank)  param_size_bytes=4  gog_size=894  gog_retn=4
Candidate Steam address: 0x00622c90 (size 894, retn 4)
Candidate sources: branch+mapper:drift-0x10+name_map:high:batch7_reverse_engineering+near    mapper tag: drift-0x10

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (AddObject 0x51e040); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00622c10:STR0/CALLEE0/SIG0.078/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e9000 CSWSAreaOfEffectObject::SetCreator -> 0x0062ee30  gate_class=AUTO
- GOG-order neighbour after:  0x007e94d0 CSWSEncounter::GetFirstName -> 0x00623010  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00607350 CExoLocString::Constructor -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064bd90 CServerExoApp::GetWorldTimer -> Steam 0x0051c370  in candidate callees: yes
  0x0064da60 CServerAIMaster::AddObject -> Steam 0x0051e040  in candidate callees: yes
  0x006a4d30 CSWSCreature::UseLoreOnItem -> Steam ?  in candidate callees: n/a
  0x006a55e0 CSWSObject::Constructor -> Steam ?  in candidate callees: n/a
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 41 common): GOG {0x8*,0xac*,0xb0*,0x26c*,0x27c*,0x280*,0x284*,0x288*,0x290*,0x294*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,...(+27)} | Steam {0x8*,0xac*,0xb0*,0x26c*,0x27c*,0x280*,0x284*,0x288*,0x290*,0x294*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,...(+27)}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xd*,0x10*,0x30*,0x32*,0x3c*,0x270*,0x298*,0x328*,0x7f000000*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xd*,0x10*,0x30*,0x32*,0x3c*,0x270*,0x298*,0x328*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00622c90: 16-byte aligned: YES
  00622c90  55             PUSH EBP
  00622c91  8B EC          MOV EBP,ESP
  00622c93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00622c90
string literals (0):
direct callees (14):
  0x0053E830  CSWSObject::Constructor
  0x00735240  (unnamed)
  0x0084A640  (unnamed)
  0x0091D67E  (unnamed)
  0x0051C280  (unnamed)
  0x0051E040  CServerAIMaster::AddObject
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x00919723  operator new
  0x006288F0  (unnamed)
  0x0058C290  (unnamed)
  0x00627E50  CSWSEncounter::LoadFractionalCRData
  0x00627F60  CSWSEncounter::SetActive
  0x00543F50  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x2  via EAX
  +0x2C0 [dec 704]  x2  via EDX
  +0xAC [dec 172]  x1  via ECX
  +0xB0 [dec 176]  x1  via EAX
  +0x26C [dec 620]  x1  via ECX
  +0x27C [dec 636]  x1  via EDX
  +0x280 [dec 640]  x1  via EAX
  +0x284 [dec 644]  x1  via ECX
  +0x288 [dec 648]  x1  via EDX
  +0x290 [dec 656]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00622c90 FUN_00622c90 size=894 ret=4 callers=2 callees=14

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_00622c90(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  float10 fVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009576cc;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0053e830(0xd,param_2,0);
  local_8 = 0;
  *param_1 = CSWSEncounter::vftable;
  FUN_00735240(uVar1);
  local_8._0_1_ = 1;
  FUN_0084a640(0);
  local_8._0_1_ = 2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xca,8,5,FUN_00733540,FUN_00733780);
  local_8 = CONCAT31(local_8._1_3_,3);
  uVar5 = 0;
  puVar3 = param_1;
  FUN_0051c280(param_1,0);
  FUN_0051e040(puVar3,uVar5);
  puVar3 = &local_14;
  puVar4 = &local_18;
  FUN_0051c370(puVar4,puVar3);
  FUN_0051ad20(puVar4,puVar3);
  param_1[0x2b = byte +0xac] = local_18;
  param_1[0x2c = byte +0xb0] = local_14;
  param_1[0xac = byte +0x2b0] = 0;
  param_1[0xad = byte +0x2b4] = 0;
  param_1[0xab = byte +0x2ac] = 0;
  param_1[0xaa = byte +0x2a8] = 0;
  param_1[0xbd = byte +0x2f4] = 0;
  param_1[0xbc = byte +0x2f0] = 0;
  param_1[0xb0 = byte +0x2c0] = 0x10;
  local_20 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[0xb0 = byte +0x2c0] * 4 >> 0x20) != 0) |
                          (uint)((ulonglong)(uint)param_1[0xb0 = byte +0x2c0] * 4));
  param_1[0xb1 = byte +0x2c4] = local_20;
  param_1[0xb2 = byte +0x2c8] = 0;
  param_1[0xb3 = byte +0x2cc] = 0;
  param_1[0xb4 = byte +0x2d0] = 0;
  param_1[0xb6 = byte +0x2d8] = 0;
  param_1[0xb5 = byte +0x2d4] = 0;
  param_1[0xb7 = byte +0x2dc] = 0;
  param_1[200 = byte +0x320] = 0x7f000000;
  param_1[0xc0 = byte +0x300] = 0;
  param_1[0xbf = byte +0x2fc] = 0;
  param_1[0xc2 = byte +0x308] = 0;
  param_1[0xa9 = byte +0x2a4] = 0;
  param_1[0xc6 = byte +0x318] = 0;
  param_1[199 = byte +0x31c] = 0;
  param_1[0xc3 = byte +0x30c] = 0;
  param_1[0xc4 = byte +0x310] = 0;
  param_1[0xc9 = byte +0x324] = 0x7f000000;
  param_1[0xbe = byte +0x2f8] = 0;
  param_1[0xae = byte +0x2b8] = 0;
  for (local_1c = 0; local_1c < 0x33; local_1c = local_1c + 1) {
    fVar2 = (float10)FUN_006288f0(_DAT_0098b5c0,local_1c);
    FUN_0058c290((float)fVar2);
  }
  FUN_00627e50();
  FUN_00627f60(1);
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x00622c10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00622c10: 16-byte aligned: YES
  00622c10  55             PUSH EBP
  00622c11  8B EC          MOV EBP,ESP
  00622c13  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00622c10
string literals (0):
direct callees (3):
  0x0051C8F0  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FB070  CSWPartyTable::GetIsMember_2
struct offsets touched (top 3):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x354 [dec 852]  x2  via ECX,EDX
  +0x328 [dec 808]  x1  via EAX
```

## Card B0-12-13

GOG function: CSWSItemPropertyHandler::ApplyMaxDexBonus  gog 0x007c9f80  callconv=__thiscall  param_size_bytes=8  gog_size=68  gog_retn=20
Candidate Steam address: 0x005b2b70 (size 68, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2bc0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9f50 CSWSItemPropertyHandler::ApplyRapidShotBonus -> 0x005b2b40  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9fd0 CSWSItemPropertyHandler::RemoveMaxDexBonus -> 0x005b2bc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x6*,0x2ec*} | Steam {0x6*,0x2ec*}
imm sets (* = in both, 2 common): GOG {0xc*,0x14*} | Steam {0xc*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b70: 16-byte aligned: YES
  005b2b70  55             PUSH EBP
  005b2b71  8B EC          MOV EBP,ESP
  005b2b73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b70 FUN_005b2b70 size=68 ret=20 callers=0 callees=0

undefined4 FUN_005b2b70(int param_1,int param_2)

{
  if (param_1 != 0) {
    *(uint *)(param_1 + 0x2ec) = (uint)*(ushort *)(param_2 + 6) + *(int *)(param_1 + 0x2ec);
  }
  return 0;
}
```

### decoy: 0x005b2bc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2bc0: 16-byte aligned: YES
  005b2bc0  55             PUSH EBP
  005b2bc1  8B EC          MOV EBP,ESP
  005b2bc3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2bc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
```

## Card B0-12-14

GOG function: CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEventModelName  gog 0x007f99e0  callconv=__thiscall  param_size_bytes=8  gog_size=277  gog_retn=8
Candidate Steam address: 0x006f6420 (size 277, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006f6320:STR0/CALLEE0/SIG0.45/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f98e0 CSWVirtualMachineCommands::ExecuteCommandSWMG_GetLastEvent -> 0x006f6320  gate_class=REVIEW
- GOG-order neighbour after:  0x007f9b00 Global::ExecuteCommandSWMG_GetObjectByName  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040d550 CClientExoApp::GetMGOArray -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x00539d40 CSWMiniGameObjectArray::GetMiniGameObject -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00640cb0 CVirtualMachine::StackPushString -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x18*} | Steam {0x4*,0x8*,0xc*,0x18*}
imm sets (* = in both, 3 common): GOG {0x8*,0x30*,0xfffff830*} | Steam {0x8*,0x30*,0xfffff830*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006f6420: 16-byte aligned: YES
  006f6420  55             PUSH EBP
  006f6421  8B EC          MOV EBP,ESP
  006f6423  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006f6420
string literals (0):
direct callees (8):
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006f6420 FUN_006f6420 size=277 ret=8 callers=0 callees=8

undefined4 __fastcall FUN_006f6420(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined1 local_2c [11];
  char local_21;
  int *local_20;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095ec70;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar3 = 0;
  uVar1 = *(undefined4 *)(param_1 + 8);
  FUN_00740070(uVar1,0,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_14 = FUN_00832d90(uVar1,uVar3);
  FUN_00733540();
  local_8 = 0;
  if (local_14 != 0) {
    local_20 = *(int **)(local_14 + 0x18);
    if (local_20 != (int *)0x0) {
      uVar1 = (**(code **)(*local_20 + 0xc))();
      FUN_007338d0(uVar1);
    }
  }
  uVar1 = FUN_00734010();
  FUN_00733570(uVar1);
  local_8._0_1_ = 1;
  iVar2 = FUN_006fda90(local_2c);
  local_21 = '\x01' - (iVar2 != 0);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
  if (local_21 == '\0') {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0;
  }
  else {
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar1 = 0xfffff830;
  }
  ExceptionList = local_10;
  return uVar1;
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
  0x00733540  CExoString::CExoString(empty)
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FDA90  CVirtualMachine::StackPushString
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
```

## Card B0-12-15

GOG function: CSWPartyTable::ClearInventory  gog 0x006fe490  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x005f9220 (size 2034, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWPartyTable::CreateParty
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005f9190:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006fe280 CSWPartyTable::UpdateInventory -> 0x005f8710  gate_class=AUTO
- GOG-order neighbour after:  0x006fe4d0 CSWPartyTable::TransferInventory -> 0x005f8960  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00669a30 CItemRepository::ClearItems -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1a4*,0x1ac*} | Steam {0x4,0x8,0x10,0x1c,0x8c,0x90,0x118,0x1a4*,0x1ac*,0x46c}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0xc,0x18,0xa0,0x11c,0x240,0x30c,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005f9220: 16-byte aligned: YES
  005f9220  55             PUSH EBP
  005f9221  8B EC          MOV EBP,ESP
  005f9223  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005f9220
string literals (2):
  0x00996ED4  at 005f94b5  'INVENTORY'
  0x00995348  at 005f95b1  'ItemList'
direct callees (38):
  0x005FDCA0  CSWPartyTable::AddGameInProgress
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x0051CDE0  CServerExoApp::GetLoadFromSaveGame
  0x0051C8F0  (unnamed)
  0x005FD480  CSWPartyTable::SpawnNPC
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00775860  (unnamed)
  0x0058A330  CSWSCreature::SetPartyMemberFlag
  0x005FD810  CSWPartyTable::SpawnPUP
  0x0058A470  CSWSCreature::SetIsPuppet
  0x00919723  operator new
  0x00710840  (unnamed)
  0x00716FD0  CResGFF::Constructor_2
  0x00414BA0  (unnamed)
  0x00604FB0  CItemRepository::Constructor
  0x005F8920  CSWPartyTable::ClearInventory
  0x007189D0  CResGFF::GetTopLevelStruct
  0x00718760  CResGFF::GetList
struct offsets touched (top 8):
  +0x1AC [dec 428]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x118 [dec 280]  x2  via ECX,EDX
  +0x1A4 [dec 420]  x2  via EAX,EDX
  +0x8C [dec 140]  x1  via EDX
  +0x90 [dec 144]  x1  via EDX
  +0x46C [dec 1132]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005f9220 FUN_005f9220 size=2034 ret=0 callers=2 callees=38

void __fastcall FUN_005f9220(uint *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  char **ppcVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 *local_114;
  uint local_110;
  undefined4 *local_10c;
  size_t local_104;
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  undefined1 local_ec [8];
  undefined4 *local_e4;
  undefined4 *local_e0;
  undefined4 *local_dc;
  undefined4 *local_d8;
  int local_d4;
  undefined4 *local_d0;
  int local_cc;
  uint local_c8;
  undefined1 local_c4 [16];
  int local_b4;
  undefined4 *local_b0;
  undefined4 local_ac;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined1 local_7c [8];
  char *local_74 [2];
  undefined1 local_6c [8];
  undefined4 local_64;
  undefined4 *local_60;
  undefined4 local_5c;
  int local_58;
  int local_54;
  int local_50;
  int *local_4c;
  undefined4 local_48;
  undefined4 *local_44;
  undefined1 local_40 [4];
  uint local_3c;
  undefined1 local_38 [4];
  undefined1 local_34 [20];
  uint local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009548d3;
  local_10 = ExceptionList;
  local_20 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (param_1[0x69 = byte +0x1a4] == 0) {
    FUN_005fdca0(local_20);
    local_48 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    local_14 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    local_18 = FUN_0073fb90();
    local_58 = FUN_0051cde0();
    if (param_1[0x46 = byte +0x118] != 0xffffffff) {
      uVar1 = FUN_0051c8f0();
      param_1[param_1[0x46 = byte +0x118] + 7] = uVar1;
/* ... 194 more lines truncated by --max-lines */
```

### decoy: 0x005f9190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f9190: 16-byte aligned: YES
  005f9190  55             PUSH EBP
  005f9191  8B EC          MOV EBP,ESP
  005f9193  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f9190
string literals (1):
  0x00996EC8  at 005f91d2  'AVAILPUP%d'
direct callees (4):
  0x00733540  CExoString::CExoString(empty)
  0x00734270  CExoString::Format
  0x007335F0  CExoString::CExoString_copy
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B0-12-16

GOG function: CSWSItemPropertyHandler::ApplyRapidShotBonus  gog 0x007c9f50  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2b40 (size 34, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2b10:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9f20 CSWSItemPropertyHandler::ApplySniperBonus -> 0x005b2b10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f80 CSWSItemPropertyHandler::ApplyMaxDexBonus -> 0x005b2b70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2e4*} | Steam {0x2e4*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b40: 16-byte aligned: YES
  005b2b40  55             PUSH EBP
  005b2b41  8B EC          MOV EBP,ESP
  005b2b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b40
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E4 [dec 740]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b40 FUN_005b2b40 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2b40(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e4) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2b10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b10: 16-byte aligned: YES
  005b2b10  55             PUSH EBP
  005b2b11  8B EC          MOV EBP,ESP
  005b2b13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E8 [dec 744]  x1  via EAX
```

## Card B0-12-17

GOG function: CSWSAreaOfEffectObject::GetEffectSpellId  gog 0x007e6350  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0062c180 (size 20, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0062c1a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e6190 CSWSAreaOfEffectObject::Constructor -> 0x0062bfc0  gate_class=AUTO
- GOG-order neighbour after:  0x007e6370 CSWSAreaOfEffectObject::SetEffectSpellId -> 0x0062c1a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x274*} | Steam {0x274*}
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
prologue for 0x0062c180: 16-byte aligned: YES
  0062c180  55             PUSH EBP
  0062c181  8B EC          MOV EBP,ESP
  0062c183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0062c180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x274 [dec 628]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0062c180 FUN_0062c180 size=20 ret=0 callers=0 callees=0

undefined4 __fastcall FUN_0062c180(int param_1)

{
  return *(undefined4 *)(param_1 + 0x274);
}
```

### decoy: 0x0062c1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0062c1a0: 16-byte aligned: YES
  0062c1a0  55             PUSH EBP
  0062c1a1  8B EC          MOV EBP,ESP
  0062c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0062c1a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x274 [dec 628]  x1  via EAX
```

## Card B0-12-18

GOG function: CGuiInGame::RemoveGUIPopUps  gog 0x004e0ca0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x007d0620 (size 306, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::ResetInterfaceFont
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d05a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0d20 CGuiInGame::ResetInterfaceFont  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam ?  in candidate callees: n/a
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x1c,0x98,0x59b4} | Steam {0x38,0x3c,0x54,0x94,0xa0,0xa4,0xb0}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {0x20,0x28}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0620: 16-byte aligned: YES
  007d0620  55             PUSH EBP
  007d0621  8B EC          MOV EBP,ESP
  007d0623  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0620
string literals (0):
direct callees (1):
  0x00410800  GuiManager::Contains
struct offsets touched (top 7):
  +0x38 [dec 56]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x5  via EAX,EDX
  +0x54 [dec 84]  x3  via EAX,ECX
  +0x94 [dec 148]  x3  via EAX,EDX
  +0xA4 [dec 164]  x3  via ECX,EDX
  +0xA0 [dec 160]  x2  via EAX,EDX
  +0xB0 [dec 176]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0620 FUN_007d0620 size=306 ret=0 callers=1 callees=1

void __fastcall FUN_007d0620(int param_1)

{
  int iVar1;
  
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xa0))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xa0) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xb0))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xb0) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x94))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0x94) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0x54))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0x54) + 0x3c))(0x28,1);
  }
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),*(CSize **)(param_1 + 0xa4))
  ;
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0xa4) + 0x3c))(0x28,1);
  }
  return;
}
```

### decoy: 0x007d05a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d05a0: 16-byte aligned: YES
  007d05a0  55             PUSH EBP
  007d05a1  8B EC          MOV EBP,ESP
  007d05a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d05a0
string literals (0):
direct callees (2):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x007400A0  CClientExoApp::GetMiniGame
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x59B4 [dec 22964]  x1  via EDX
```

## Card B0-12-19

GOG function: CSWSItemPropertyHandler::ApplyImmunity  gog 0x007c8e40  callconv=__thiscall  param_size_bytes=12  gog_size=672  gog_retn=20
Candidate Steam address: 0x005b1a30 (size 672, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b16b0:STR0/CALLEE0/SIG0.557/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8c50 CSWSItemPropertyHandler::RemoveDecreaseSkillModifier -> 0x005b1840  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9110 CSWSItemPropertyHandler::RemoveImmunity -> 0x005b1d00  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x8*,0xa*,0x14*,0x122*} | Steam {0x2*,0x8*,0xa*,0x14*,0x122*}
imm sets (* = in both, 7 common): GOG {0x3*,0x4*,0x9*,0x14*,0x4c*,0x8c*,0xfffffff8*} | Steam {0x3*,0x4*,0x9*,0x14*,0x4c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b1a30: 16-byte aligned: YES
  005b1a30  55             PUSH EBP
  005b1a31  8B EC          MOV EBP,ESP
  005b1a33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b1a30
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E4E80  CGameEffect::SetInteger
  0x005E5EA0  CGameEffect::SetCreator
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x4  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x14 [dec 20]  x2  via EAX
  +0x122 [dec 290]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1a30 FUN_005b1a30 size=672 ret=20 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_005b1a30(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  bool bVar1;
  int iVar2;
  int local_5c;
  int local_58;
  undefined2 local_44;
  undefined2 local_38;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952a06;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  bVar1 = false;
  switch(*(undefined2 *)(param_2 + 2)) {
  case 0:
    local_18 = _DAT_0099622c;
    break;
  case 1:
    local_18 = DAT_00996228;
    local_14 = DAT_00996200;
    bVar1 = true;
    break;
  case 2:
    local_18 = _DAT_009961c0;
    break;
  case 3:
    local_18 = _DAT_009961c4;
    break;
  case 4:
    local_18 = _DAT_009961c8;
    break;
  case 5:
    local_18 = _DAT_009961cc;
    break;
  case 6:
    local_18 = _DAT_00996224;
    break;
  case 7:
    local_18 = _DAT_009961d4;
    break;
  case 8:
    local_18 = _DAT_00996230;
    break;
  case 9:
    local_18 = _DAT_00996234;
    break;
  default:
    goto switchD_005b1a7e_default;
  }
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_58 = 0;
  }
  else {
    local_58 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  if (local_58 != 0) {
    local_38 = (undefined2)DAT_00995f74;
    *(undefined2 *)(local_58 + 8) = local_38;
    *(ushort *)(local_58 + 0xa [dec 10]) = *(ushort *)(local_58 + 0xa [dec 10]) & 0xfff8 | 3;
    FUN_005e4e80(0,local_18);
    FUN_005e4e80(1,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_00544210(local_58,param_5,param_5);
    if (bVar1) {
      iVar2 = FUN_00919723(0x8c);
      local_8 = 1;
/* ... 22 more lines truncated by --max-lines */
```

### decoy: 0x005b16b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b16b0: 16-byte aligned: YES
  005b16b0  55             PUSH EBP
  005b16b1  8B EC          MOV EBP,ESP
  005b16b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b16b0
string literals (1):
  0x00992E8C  at 005b16d8  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0xA [dec 10]  x2  via EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B0-12-20

GOG function: CWorldTimer::GetWorldTimeSecond  gog 0x0080b660  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x0051b180 (size 49, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b140:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b620 CWorldTimer::GetWorldTimeMinute -> 0x0051b140  gate_class=REVIEW
- GOG-order neighbour after:  0x0080b6a0 CWorldTimer::GetWorldTimeMillisecond -> 0x0051b1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080b020 CWorldTimer::ConvertFromTimeOfDay -> Steam 0x0051ab40  in candidate callees: yes
  0x0080b5c0 Global::GetWorldTimeTimeOfDay -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0051b180: 16-byte aligned: YES
  0051b180  55             PUSH EBP
  0051b181  8B EC          MOV EBP,ESP
  0051b183  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b180
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051b180 FUN_0051b180 size=49 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0051b180(undefined4 param_1)

{
  undefined4 uVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  undefined4 *puVar4;
  undefined1 *puVar5;
  undefined1 local_14 [4];
  undefined4 local_10;
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = &local_10;
  puVar3 = local_c;
  puVar2 = local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_10;
}
```

### decoy: 0x0051b140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b140: 16-byte aligned: YES
  0051b140  55             PUSH EBP
  0051b141  8B EC          MOV EBP,ESP
  0051b143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b140
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

## Card B0-12-21

GOG function: CWorldTimer::SubtractWorldTimes  gog 0x0080b440  callconv=(blank)  param_size_bytes=24  gog_size=115  gog_retn=24
Candidate Steam address: 0x0051af60 (size 115, retn 24)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (CompareWorldTimes 0x51afe0,FadeObject 0x7f12d0,GetCanClick 0x7d0b90)
- best decoy (neighbour/other candidate) scores: 0x0051aef0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b3d0 CWorldTimer::AddWorldTimes -> 0x0051aef0  gate_class=REVIEW
- GOG-order neighbour after:  0x0080b4c0 CWorldTimer::CompareWorldTimes -> 0x0051afe0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080b4c0 CWorldTimer::CompareWorldTimes -> Steam 0x0051afe0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
imm sets (* = in both, 2 common): GOG {0x18*,0xfffffffe*} | Steam {0x18*,0xfffffffe*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: TRUE (GOG 24 bytes -> x86 slots 24; ASM RET 24)
```

### prologue
```
prologue for 0x0051af60: 16-byte aligned: YES
  0051af60  55             PUSH EBP
  0051af61  8B EC          MOV EBP,ESP
  0051af63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051af60
string literals (0):
direct callees (1):
  0x0051AFE0  CWorldTimer::CompareWorldTimes
struct offsets touched (top 1):
  +0x3C [dec 60]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051af60 FUN_0051af60 size=115 ret=24 callers=39 callees=1

undefined4 __thiscall
FUN_0051af60(int param_1,int param_2,int param_3,int param_4,int param_5,int *param_6,uint *param_7)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_0051afe0(param_2,param_3,param_4,param_5);
  if (iVar1 == -1) {
    uVar2 = 0xfffffffe;
  }
  else {
    *param_6 = param_2 - param_4;
    *param_7 = param_3 - param_5;
    if (*(uint *)(param_1 + 0x3c) <= *param_7) {
      *param_6 = *param_6 + -0x1 [dec -1] {int*: byte -0x4};
      *param_7 = *param_7 + *(int *)(param_1 + 0x3c);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0051aef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051aef0: 16-byte aligned: YES
  0051aef0  55             PUSH EBP
  0051aef1  8B EC          MOV EBP,ESP
  0051aef3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051aef0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C [dec 60]  x4  via EAX,EDX
```

## Card B0-12-22

GOG function: CSWSItemPropertyHandler::ApplyDoorSabering  gog 0x007c9ef0  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2ae0 (size 34, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2ab0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9ec0 CSWSItemPropertyHandler::ApplyDoorCutting -> 0x005b2ab0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f20 CSWSItemPropertyHandler::ApplySniperBonus -> 0x005b2b10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2e0*} | Steam {0x2e0*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2ae0: 16-byte aligned: YES
  005b2ae0  55             PUSH EBP
  005b2ae1  8B EC          MOV EBP,ESP
  005b2ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2ae0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E0 [dec 736]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2ae0 FUN_005b2ae0 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2ae0(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e0) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2ab0: 16-byte aligned: YES
  005b2ab0  55             PUSH EBP
  005b2ab1  8B EC          MOV EBP,ESP
  005b2ab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2ab0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2DC [dec 732]  x1  via EAX
```

## Card B0-12-23

GOG function: CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionDecrease  gog 0x007c9b90  callconv=__thiscall  param_size_bytes=12  gog_size=233  gog_retn=16
Candidate Steam address: 0x005b2780 (size 233, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2560:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9a60 CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease -> 0x005b2650  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9c80 CSWSItemPropertyHandler::ApplyDisguise -> 0x005b2870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x30*,0x148*} | Steam {0x7*,0x10*,0x30*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b2780: 16-byte aligned: YES
  005b2780  55             PUSH EBP
  005b2781  8B EC          MOV EBP,ESP
  005b2783  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2780
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2780 FUN_005b2780 size=233 ret=16 callers=0 callees=2

undefined4 FUN_005b2780(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int local_14;
  
  uVar1 = *(ushort *)(param_2 + 6);
  if ((int *)(param_3 + 0x148) != (int *)0x0) {
    for (local_14 = 0; local_14 < *(int *)(param_3 + 0x14c); local_14 = local_14 + 1) {
      puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_14 * 4);
      if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_0099608c) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
          && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
        FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
        return 0;
      }
    }
  }
  return 0;
}
```

### decoy: 0x005b2560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2560: 16-byte aligned: YES
  005b2560  55             PUSH EBP
  005b2561  8B EC          MOV EBP,ESP
  005b2563  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2560
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-12-24

GOG function: CSWVirtualMachineCommands::ExecuteCommandSWMG_SetFollowerIntInfo  gog 0x007facb0  callconv=__thiscall  param_size_bytes=8  gog_size=168  gog_retn=8
Candidate Steam address: 0x006f76f0 (size 168, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006f77a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007faba0 Global::ExecuteCommandSWMG_GetFollowerPosition  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007fad60 Global::ExecuteCommandSWMG_SetFollowerFloatInfo  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d550 CClientExoApp::GetMGOArray -> Steam ?  in candidate callees: n/a
  0x00539d40 CSWMiniGameObjectArray::GetMiniGameObject -> Steam ?  in candidate callees: n/a
  0x005423d0 CSWTrackFollower::SetHitPoints -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x38*} | Steam {0x4*,0x38*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x18*,0x26a*,0x26e*,0xfffff82f*} | Steam {0x4*,0x8*,0x18*,0x26a*,0x26e*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006f76f0: 16-byte aligned: YES
  006f76f0  55             PUSH EBP
  006f76f1  8B EC          MOV EBP,ESP
  006f76f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f76f0
string literals (0):
direct callees (5):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00835690  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x38 [dec 56]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006f76f0 FUN_006f76f0 size=168 ret=8 callers=0 callees=5

undefined4 FUN_006f76f0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_006fdaf0(&local_c);
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
      uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 4);
      uVar4 = 4;
      FUN_00740070(local_c,4);
      piVar3 = (int *)FUN_00832d90(local_c,uVar4);
      if (piVar3 != (int *)0x0) {
        if (param_1 == 0x26a) {
          FUN_00835690(local_8);
        }
        else if (param_1 == 0x26e) {
          (**(code **)(*piVar3 + 0x38))(local_8,0x26e,uVar2,uVar5);
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006f77a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f77a0: 16-byte aligned: YES
  006f77a0  55             PUSH EBP
  006f77a1  8B EC          MOV EBP,ESP
  006f77a3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f77a0
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9E0  CVirtualMachine::StackPopFloat
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x008352B0  (unnamed)
  0x00835650  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-12-25

GOG function: CExoResMan::AddResourceImageFile  gog 0x0061af40  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x00711670 (size 29, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoResMan::AddFixedKeyTableFile
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00711690:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061af20 CExoResMan::AddEncapsulatedResourceFile -> 0x007115c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0061af60 CExoResMan::AddResourceImageFilesAsync -> 0x00711600  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061b0b0 CExoResMan::AddKeyTable -> Steam ?  in candidate callees: n/a
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
prologue for 0x00711670: 16-byte aligned: YES
  00711670  55             PUSH EBP
  00711671  8B EC          MOV EBP,ESP
  00711673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711670
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711670 FUN_00711670 size=29 ret=4 callers=3 callees=1
THUNK -> 0x00711750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00711670(undefined4 param_1)

{
  FUN_00711750(param_1,1,0);
  return;
}
```

### decoy: 0x00711690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00711690: 16-byte aligned: YES
  00711690  55             PUSH EBP
  00711691  8B EC          MOV EBP,ESP
  00711693  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00711690
string literals (0):
direct callees (1):
  0x00711750  CExoResMan::AddKeyTable
struct offsets touched (top 0):
```

## Card B0-12-26

GOG function: Gob::SetScene  gog 0x0084b0b0  callconv=(blank)  param_size_bytes=4  gog_size=130  gog_retn=0
Candidate Steam address: 0x00458ae0 (size 130, retn 0)
Candidate sources: near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084b050 Global::FindGob  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b140 Gob::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x8*,0x14*,0x64*} | Steam {0x8*,0x14*,0x64*}

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

## Card B0-12-27

GOG function: CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease  gog 0x007c9970  callconv=__thiscall  param_size_bytes=12  gog_size=233  gog_retn=16
Candidate Steam address: 0x005b2560 (size 233, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2340:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9840 CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease -> 0x005b2430  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9a60 CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease -> 0x005b2650  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x30*,0x148*} | Steam {0x7*,0x10*,0x30*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b2560: 16-byte aligned: YES
  005b2560  55             PUSH EBP
  005b2561  8B EC          MOV EBP,ESP
  005b2563  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2560
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2560 FUN_005b2560 size=233 ret=16 callers=0 callees=2

undefined4 FUN_005b2560(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int local_14;
  
  uVar1 = *(ushort *)(param_2 + 6);
  if ((int *)(param_3 + 0x148) != (int *)0x0) {
    for (local_14 = 0; local_14 < *(int *)(param_3 + 0x14c); local_14 = local_14 + 1) {
      puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_14 * 4);
      if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996088) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
          && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
        FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
        return 0;
      }
    }
  }
  return 0;
}
```

### decoy: 0x005b2340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2340: 16-byte aligned: YES
  005b2340  55             PUSH EBP
  005b2341  8B EC          MOV EBP,ESP
  005b2343  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2340
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-12-28

GOG function: CSWSEncounter::AddToArea  gog 0x007e9df0  callconv=(blank)  param_size_bytes=20  gog_size=141  gog_retn=20
Candidate Steam address: 0x00623940 (size 141, retn 20)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006239d0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e9b40 CSWSEncounter::EventHandler -> 0x00623690  gate_class=AUTO
- GOG-order neighbour after:  0x007e9e80 CSWSEncounter::RemoveFromArea -> 0x006239d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ac100 CSWSObject::SetArea -> Steam ?  in candidate callees: n/a
  0x00717da0 CSWSArea::AddObjectToArea -> Steam ?  in candidate callees: n/a
  0x007e9e80 CSWSEncounter::RemoveFromArea -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x14*,0x18*,0x1a4*} | Steam {0x14*,0x18*,0x1a4*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00623940: 16-byte aligned: YES
  00623940  55             PUSH EBP
  00623941  8B EC          MOV EBP,ESP
  00623943  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00623940
string literals (0):
direct callees (6):
  0x006239D0  CSWSEncounter::RemoveFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
  0x005453C0  CSWSObject::GetArea
  0x00522BF0  CSWSArea::AddObjectToArea
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00623940 FUN_00623940 size=141 ret=20 callers=2 callees=6

void __thiscall
FUN_00623940(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  undefined4 uVar1;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  FUN_006239d0();
  if (param_2 != 0) {
    FUN_00545420(param_2);
    local_10 = param_3;
    local_c = param_4;
    local_8 = param_5;
    FUN_00543f50(&local_10,0,1,0);
    uVar1 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar1,param_6);
    FUN_00522bf0(uVar1,param_6);
    FUN_005453c0();
    FUN_0083ea60(*(undefined4 *)(param_1 + 4));
  }
  return;
}
```

### decoy: 0x006239d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006239d0: 16-byte aligned: YES
  006239d0  55             PUSH EBP
  006239d1  8B EC          MOV EBP,ESP
  006239d3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006239d0
string literals (0):
direct callees (5):
  0x005453C0  CSWSObject::GetArea
  0x00523090  CSWSArea::RemoveObjectFromArea
  0x0052E4E0  (unnamed)
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B0-12-29

GOG function: CSWSAreaOfEffectObject::Destructor  gog 0x007e6390  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0062c1c0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch8_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0062c1a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e6370 CSWSAreaOfEffectObject::SetEffectSpellId -> 0x0062c1a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007e63c0 CSWSAreaOfEffectObject::Destructor_2 -> 0x0062c1f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e63c0 CSWSAreaOfEffectObject::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x0062c1c0: 16-byte aligned: YES
  0062c1c0  55             PUSH EBP
  0062c1c1  8B EC          MOV EBP,ESP
  0062c1c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0062c1c0
string literals (0):
direct callees (2):
  0x0062C1F0  CSWSAreaOfEffectObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0062c1c0 FUN_0062c1c0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0062c1c0(undefined4 param_1,uint param_2)

{
  FUN_0062c1f0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0062c1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0062c1a0: 16-byte aligned: YES
  0062c1a0  55             PUSH EBP
  0062c1a1  8B EC          MOV EBP,ESP
  0062c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0062c1a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x274 [dec 628]  x1  via EAX
```

## Card B0-12-30

GOG function: CWorldTimer::AddWorldTimes  gog 0x0080b3d0  callconv=(blank)  param_size_bytes=24  gog_size=108  gog_retn=24
Candidate Steam address: 0x0051aef0 (size 108, retn 24)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (SetDialogDelay 0x545470)
- best decoy (neighbour/other candidate) scores: 0x0051af60:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b310 CWorldTimer::ResetTimer -> 0x0051ae30  gate_class=AUTO
- GOG-order neighbour after:  0x0080b440 CWorldTimer::SubtractWorldTimes -> 0x0051af60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
imm sets (* = in both, 2 common): GOG {0x18*,0xfffffffe*} | Steam {0x18*,0xfffffffe*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: TRUE (GOG 24 bytes -> x86 slots 24; ASM RET 24)
```

### prologue
```
prologue for 0x0051aef0: 16-byte aligned: YES
  0051aef0  55             PUSH EBP
  0051aef1  8B EC          MOV EBP,ESP
  0051aef3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051aef0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C [dec 60]  x4  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051aef0 FUN_0051aef0 size=108 ret=24 callers=10 callees=0

undefined4 __thiscall
FUN_0051aef0(int param_1,int param_2,uint param_3,int param_4,uint param_5,int *param_6,
            uint *param_7)

{
  undefined4 uVar1;
  
  if ((param_3 < *(uint *)(param_1 + 0x3c)) && (param_5 < *(uint *)(param_1 + 0x3c))) {
    *param_6 = param_2 + param_4;
    *param_7 = param_3 + param_5;
    if (*(uint *)(param_1 + 0x3c) <= *param_7) {
      *param_6 = *param_6 + 0x1 [dec 1] {int*: byte +0x4};
      *param_7 = *param_7 - *(int *)(param_1 + 0x3c);
    }
    uVar1 = 0;
  }
  else {
    uVar1 = 0xfffffffe;
  }
  return uVar1;
}
```

### decoy: 0x0051af60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051af60: 16-byte aligned: YES
  0051af60  55             PUSH EBP
  0051af61  8B EC          MOV EBP,ESP
  0051af63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051af60
string literals (0):
direct callees (1):
  0x0051AFE0  CWorldTimer::CompareWorldTimes
struct offsets touched (top 1):
  +0x3C [dec 60]  x2  via EAX,ECX
```

## Card B0-12-31

GOG function: CWorldTimer::UnpauseWorldTimer  gog 0x0080b580  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x0051b0a0 (size 54, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b050:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b530 CWorldTimer::PauseWorldTimer -> 0x0051b050  gate_class=AUTO
- GOG-order neighbour after:  0x0080b5c0 Global::GetWorldTimeTimeOfDay  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080b110 CWorldTimer::SetWorldTime -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x24*,0x28*,0x2c*} | Steam {0x24*,0x28*,0x2c*}
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
prologue for 0x0051b0a0: 16-byte aligned: YES
  0051b0a0  55             PUSH EBP
  0051b0a1  8B EC          MOV EBP,ESP
  0051b0a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b0a0
string literals (0):
direct callees (1):
  0x0051AC30  (unnamed)
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x28 [dec 40]  x1  via EAX
  +0x2C [dec 44]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051b0a0 FUN_0051b0a0 size=54 ret=0 callers=4 callees=1

void __fastcall FUN_0051b0a0(int param_1)

{
  if (*(int *)(param_1 + 0x24) == 1) {
    FUN_0051ac30(*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c),0);
    *(undefined4 *)(param_1 + 0x24) = 0;
  }
  return;
}
```

### decoy: 0x0051b050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b050: 16-byte aligned: YES
  0051b050  55             PUSH EBP
  0051b051  8B EC          MOV EBP,ESP
  0051b053  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b050
string literals (0):
direct callees (1):
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 3):
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x28 [dec 40]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B0-12-32

GOG function: CSWSScriptVarTable::DestroyString  gog 0x007f07d0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x005e6f40 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e6f60:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f07b0 CSWSScriptVarTable::DestroyFloat -> 0x005e6f20  gate_class=REVIEW
- GOG-order neighbour after:  0x007f07f0 CSWSScriptVarTable::DestroyObject -> 0x005e6f60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007effe0 CSWSScriptVarTable::DeleteIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x3*,0x4*} | Steam {0x3*,0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e6f40: 16-byte aligned: YES
  005e6f40  55             PUSH EBP
  005e6f41  8B EC          MOV EBP,ESP
  005e6f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6f40
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e6f40 FUN_005e6f40 size=27 ret=4 callers=1 callees=1
THUNK -> 0x005E6750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e6f40(undefined4 param_1)

{
  FUN_005e6750(param_1,3);
  return;
}
```

### decoy: 0x005e6f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e6f60: 16-byte aligned: YES
  005e6f60  55             PUSH EBP
  005e6f61  8B EC          MOV EBP,ESP
  005e6f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e6f60
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

## Card B0-12-33

GOG function: CSWSItemPropertyHandler::RemoveRegeneration  gog 0x007c9750  callconv=__thiscall  param_size_bytes=12  gog_size=233  gog_retn=16
Candidate Steam address: 0x005b2340 (size 233, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2560:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c95d0 CSWSItemPropertyHandler::ApplyRegeneration -> 0x005b21c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9840 CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease -> 0x005b2430  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*} | Steam {0x4*,0x6*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x30*,0x148*} | Steam {0x7*,0x10*,0x30*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b2340: 16-byte aligned: YES
  005b2340  55             PUSH EBP
  005b2341  8B EC          MOV EBP,ESP
  005b2343  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2340
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2340 FUN_005b2340 size=233 ret=16 callers=0 callees=2

undefined4 FUN_005b2340(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int local_c;
  
  uVar1 = *(ushort *)(param_2 + 6);
  if ((int *)(param_3 + 0x148) != (int *)0x0) {
    for (local_c = 0; local_c < *(int *)(param_3 + 0x14c); local_c = local_c + 1) {
      puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_c * 4);
      if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f38) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
          && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
        FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
        return 0;
      }
    }
  }
  return 0;
}
```

### decoy: 0x005b2560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2560: 16-byte aligned: YES
  005b2560  55             PUSH EBP
  005b2561  8B EC          MOV EBP,ESP
  005b2563  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2560
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-12-34

GOG function: CSWSEncounter::Destructor  gog 0x007e94f0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00623030 (size 44, retn 4)
Candidate sources: branch+mapper:drift+0x0+name_map:high:batch7_reverse_engineering+near    mapper tag: drift

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006232b0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e94d0 CSWSEncounter::GetFirstName -> 0x00623010  gate_class=REVIEW
- GOG-order neighbour after:  0x007e9520 CSWSEncounter::Destructor_2 -> 0x00623060  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e9520 CSWSEncounter::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x00623030: 16-byte aligned: YES
  00623030  55             PUSH EBP
  00623031  8B EC          MOV EBP,ESP
  00623033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00623030
string literals (0):
direct callees (2):
  0x00623060  CSWSEncounter::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00623030 FUN_00623030 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00623030(undefined4 param_1,uint param_2)

{
  FUN_00623060();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x006232b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006232b0: 16-byte aligned: YES
  006232b0  55             PUSH EBP
  006232b1  8B EC          MOV EBP,ESP
  006232b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006232b0
string literals (0):
direct callees (2):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-12-35

GOG function: CSWSEncounter::Destructor_2  gog 0x007e9520  callconv=(blank)  param_size_bytes=0  gog_size=590  gog_retn=0
Candidate Steam address: 0x00623060 (size 590, retn 0)
Candidate sources: branch+mapper:drift+0x0+name_map:high:batch7_reverse_engineering+near    mapper tag: drift

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (RemoveObject 0x51e0c0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006232b0:STR0/CALLEE0/SIG0.091/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e94f0 CSWSEncounter::Destructor -> 0x00623030  gate_class=REVIEW
- GOG-order neighbour after:  0x007e97e0 CSWSEncounter::AIUpdate -> 0x00623330  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00607470 CExoLocString::Destructor -> Steam 0x00735360  in candidate callees: yes
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064dae0 CServerAIMaster::RemoveObject -> Steam 0x0051e0c0  in candidate callees: yes
  0x006a5d30 CSWSObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00731230 CSWSModule::RemoveObjectFromLookupTable -> Steam ?  in candidate callees: n/a
  0x007e9770 unnamed -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x2c4*,0x2d0*,0x2d8*,0x2f0*,0x2f8*,0x350*} | Steam {0x4*,0x8*,0x2c4*,0x2d0*,0x2d8*,0x2f0*,0x2f8*,0x350*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x18*,0x3c*,0x270*,0x298*,0x328*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x18*,0x3c*,0x270*,0x298*,0x328*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00623060: 16-byte aligned: YES
  00623060  55             PUSH EBP
  00623061  8B EC          MOV EBP,ESP
  00623063  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00623060
string literals (0):
direct callees (12):
  0x006239D0  CSWSEncounter::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x006232B0  (unnamed)
  0x009196FD  operator delete
  0x00628840  CSWSEncounter::ClearSpawnList
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00735360  CExoLocString::Destructor
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 8):
  +0x2C4 [dec 708]  x3  via EAX,ECX
  +0x2D0 [dec 720]  x3  via EAX,EDX
  +0x2D8 [dec 728]  x3  via ECX,EDX
  +0x2F0 [dec 752]  x3  via ECX,EDX
  +0x2F8 [dec 760]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x350 [dec 848]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00623060 FUN_00623060 size=590 ret=0 callers=1 callees=12

void __fastcall FUN_00623060(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *puVar3;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0095772c;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWSEncounter::vftable;
  local_8 = 3;
  FUN_006239d0(uVar1);
  puVar3 = param_1;
  FUN_0051c280(param_1);
  FUN_0051e0c0(puVar3);
  iVar2 = FUN_0051bd90();
  if (iVar2 != 0) {
    FUN_0055ea80(param_1 + 0x6 [dec 6] {undefined4*: byte +0x18},param_1[1 = byte +0x4]);
  }
  if (param_1[0xbc = byte +0x2f0] != 0) {
    if (param_1[0xbc = byte +0x2f0] != 0) {
      FUN_006232b0(3);
    }
    param_1[0xbc = byte +0x2f0] = 0;
  }
  if (param_1[0xb1 = byte +0x2c4] != 0) {
    FUN_009196fd(param_1[0xb1 = byte +0x2c4]);
    param_1[0xb1 = byte +0x2c4] = 0;
  }
  if (param_1[0xb4 = byte +0x2d0] != 0) {
    FUN_009196fd(param_1[0xb4 = byte +0x2d0]);
    param_1[0xb4 = byte +0x2d0] = 0;
  }
  if (param_1[0xb6 = byte +0x2d8] != 0) {
    FUN_009196fd(param_1[0xb6 = byte +0x2d8]);
    param_1[0xb6 = byte +0x2d8] = 0;
  }
  if (param_1[0xbe = byte +0x2f8] != 0) {
    FUN_009196fd(param_1[0xbe = byte +0x2f8]);
    param_1[0xbe = byte +0x2f8] = 0;
  }
  FUN_00628840();
  iVar2 = param_1[0xd4 = byte +0x350];
  if (iVar2 == 0) {
    iVar2 = 0;
  }
  else {
    Free();
    FUN_009196fd(iVar2);
  }
  local_8._0_1_ = 2;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xca,8,5,FUN_00733780);
  local_8._0_1_ = 1;
  Free(uVar1,iVar2);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00735360();
  local_8 = 0xffffffff;
  FUN_0053f050();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x006232b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006232b0: 16-byte aligned: YES
  006232b0  55             PUSH EBP
  006232b1  8B EC          MOV EBP,ESP
  006232b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006232b0
string literals (0):
direct callees (2):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-12-36

GOG function: CSWSItemPropertyHandler::RemoveMaxDexBonus  gog 0x007c9fd0  callconv=__thiscall  param_size_bytes=8  gog_size=70  gog_retn=16
Candidate Steam address: 0x005b2bc0 (size 70, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2b70:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9f80 CSWSItemPropertyHandler::ApplyMaxDexBonus -> 0x005b2b70  gate_class=REVIEW
- GOG-order neighbour after:  0x007ca020 CSWSForcedAction::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x6*,0x2ec*} | Steam {0x6*,0x2ec*}
imm sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005b2bc0: 16-byte aligned: YES
  005b2bc0  55             PUSH EBP
  005b2bc1  8B EC          MOV EBP,ESP
  005b2bc3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2bc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2bc0 FUN_005b2bc0 size=70 ret=16 callers=0 callees=0

undefined4 FUN_005b2bc0(int param_1,int param_2)

{
  if (param_1 != 0) {
    *(uint *)(param_1 + 0x2ec) = *(int *)(param_1 + 0x2ec) - (uint)*(ushort *)(param_2 + 6);
  }
  return 0;
}
```

### decoy: 0x005b2b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2b70: 16-byte aligned: YES
  005b2b70  55             PUSH EBP
  005b2b71  8B EC          MOV EBP,ESP
  005b2b73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2b70
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2EC [dec 748]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EDX
```

## Card B0-12-37

GOG function: CSWVirtualMachineCommands::ExecuteCommandSWMG_AdjustFollowerHitPoints  gog 0x007f9fb0  callconv=__thiscall  param_size_bytes=8  gog_size=163  gog_retn=8
Candidate Steam address: 0x006f69f0 (size 163, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006f6970:STR0/CALLEE0/SIG0.306/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f9f30 Global::ExecuteCommandSWMG_GetLastBulletShooter  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007fa060 Global::ExecuteCommandSWMG_DefaultImplementations  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040d550 CClientExoApp::GetMGOArray -> Steam ?  in candidate callees: n/a
  0x00539d40 CSWMiniGameObjectArray::GetMiniGameObject -> Steam ?  in candidate callees: n/a
  0x00542360 CSWTrackFollower::SetMaxHitPoints -> Steam ?  in candidate callees: n/a
  0x00542560 CSWTrackFollower::AdjustHitPoints -> Steam ?  in candidate callees: n/a
  0x00640bc0 CVirtualMachine::StackPopInteger -> Steam ?  in candidate callees: n/a
  0x00640d10 CVirtualMachine::StackPopObject -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x18*,0x24e*,0x25c*,0xfffff82f*} | Steam {0x4*,0x8*,0x18*,0x24e*,0x25c*,0xfffff82f*}

### abi
```
GOG param bytes: 8; callconv: thiscall
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006f69f0: 16-byte aligned: YES
  006f69f0  55             PUSH EBP
  006f69f1  8B EC          MOV EBP,ESP
  006f69f3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f69f0
string literals (0):
direct callees (6):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00835820  (unnamed)
  0x00835620  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006f69f0 FUN_006f69f0 size=163 ret=8 callers=0 callees=6

undefined4 FUN_006f69f0(int param_1)

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
      uVar2 = 4;
      FUN_00740070(local_c,4);
      iVar1 = FUN_00832d90(local_c,uVar2);
      if (iVar1 != 0) {
        if (param_1 == 0x24e) {
          FUN_00835820(local_8);
        }
        else if (param_1 == 0x25c) {
          FUN_00835620(local_8);
        }
      }
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x006f6970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f6970: 16-byte aligned: YES
  006f6970  55             PUSH EBP
  006f6971  8B EC          MOV EBP,ESP
  006f6973  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f6970
string literals (0):
direct callees (4):
  0x00740070  CClientExoApp::GetMGOArray
  0x00832D90  (unnamed)
  0x00833280  (unnamed)
  0x006FDB10  CVirtualMachine::StackPushObject
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-12-38

GOG function: CSWSScriptVarTable::GetFloat  gog 0x007f00a0  callconv=(blank)  param_size_bytes=4  gog_size=57  gog_retn=4
Candidate Steam address: 0x005e6810 (size 57, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005e67d0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f0060 CSWSScriptVarTable::GetInt -> 0x005e67d0  gate_class=REVIEW
- GOG-order neighbour after:  0x007f00e0 CSWSScriptVarTable::GetString -> 0x005e6850  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007efe10 CSWSScriptVarTable::MatchIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0xc*} | Steam {0x2*,0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005e6810: 16-byte aligned: YES
  005e6810  55             PUSH EBP
  005e6811  8B EC          MOV EBP,ESP
  005e6813  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6810
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005e6810 FUN_005e6810 size=57 ret=4 callers=2 callees=1

float10 FUN_005e6810(undefined4 param_1)

{
  int iVar1;
  undefined4 local_c;
  
  local_c = 0.0;
  iVar1 = FUN_005e6580(param_1,2,0);
  if (iVar1 != 0) {
    local_c = *(float *)(iVar1 + 0xc);
  }
  return (float10)local_c;
}
```

### decoy: 0x005e67d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e67d0: 16-byte aligned: YES
  005e67d0  55             PUSH EBP
  005e67d1  8B EC          MOV EBP,ESP
  005e67d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e67d0
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via ECX
```

## Card B0-12-39

GOG function: CSWSItemPropertyHandler::RemoveDampenSound  gog 0x007c9ea0  callconv=__thiscall  param_size_bytes=0  gog_size=25  gog_retn=16
Candidate Steam address: 0x005b2a90 (size 25, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2a70:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9e80 CSWSItemPropertyHandler::ApplyDampenSound -> 0x005b2a70  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9ec0 CSWSItemPropertyHandler::ApplyDoorCutting -> 0x005b2ab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0067efe0 CSWSCreature::SetSilentMove -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 16)
```

### prologue
```
prologue for 0x005b2a90: 16-byte aligned: YES
  005b2a90  55             PUSH EBP
  005b2a91  8B EC          MOV EBP,ESP
  005b2a93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2a90
string literals (0):
direct callees (1):
  0x005663B0  CSWSCreature::SetSilentMove
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005b2a90 FUN_005b2a90 size=25 ret=16 callers=0 callees=1

undefined4 FUN_005b2a90(void)

{
  FUN_005663b0(0);
  return 0;
}
```

### decoy: 0x005b2a70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2a70: 16-byte aligned: YES
  005b2a70  55             PUSH EBP
  005b2a71  8B EC          MOV EBP,ESP
  005b2a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2a70
string literals (0):
direct callees (1):
  0x005663B0  CSWSCreature::SetSilentMove
struct offsets touched (top 0):
```

## Card B0-12-40

GOG function: CResGFF::WriteFieldWORD  gog 0x00625c10  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719a40 (size 97, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::WriteFieldCHAR
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719ab0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  gate_class=REVIEW
- GOG-order neighbour after:  0x00625c80 CResGFF::WriteFieldSHORT -> 0x00719b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x2,0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719a40: 16-byte aligned: YES
  00719a40  55             PUSH EBP
  00719a41  8B EC          MOV EBP,ESP
  00719a43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719a40
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719a40 FUN_00719a40 size=97 ret=12 callers=6 callees=2

undefined4 __thiscall FUN_00719a40(int param_1,int param_2,char param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,1);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719ab0: 16-byte aligned: YES
  00719ab0  55             PUSH EBP
  00719ab1  8B EC          MOV EBP,ESP
  00719ab3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719ab0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-12-41

GOG function: CSWSItemPropertyHandler::ApplyDisguise  gog 0x007c9c80  callconv=__thiscall  param_size_bytes=12  gog_size=266  gog_retn=20
Candidate Steam address: 0x005b2870 (size 266, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2650:STR0/CALLEE0/SIG0.733/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9b90 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionDecrease -> 0x005b2780  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9d90 CSWSItemPropertyHandler::RemoveDisguise -> 0x005b2980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x2*,0x8*,0xa*,0x14*} | Steam {0x2*,0x8*,0xa*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x14*,0x28*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x28*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b2870: 16-byte aligned: YES
  005b2870  55             PUSH EBP
  005b2871  8B EC          MOV EBP,ESP
  005b2873  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b2870
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2870 FUN_005b2870 size=266 ret=20 callers=0 callees=5

undefined4
FUN_005b2870(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined2 uVar1;
  int iVar2;
  int local_38;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f4eb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar1 = *(undefined2 *)(param_2 + 2);
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_38 = 0;
  }
  else {
    local_38 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  local_28 = (undefined2)DAT_00996014;
  *(undefined2 *)(local_38 + 8) = local_28;
  *(ushort *)(local_38 + 0xa [dec 10]) = *(ushort *)(local_38 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
  FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
  FUN_005e4e80(0,uVar1);
  FUN_005e4e80(1,0);
  FUN_00544210(local_38,param_5,0);
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2650: 16-byte aligned: YES
  005b2650  55             PUSH EBP
  005b2651  8B EC          MOV EBP,ESP
  005b2653  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b2650
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-12-42

GOG function: CSWSItemPropertyHandler::RemoveDisguise  gog 0x007c9d90  callconv=__thiscall  param_size_bytes=12  gog_size=233  gog_retn=16
Candidate Steam address: 0x005b2980 (size 233, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2780:STR0/CALLEE0/SIG0.857/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9c80 CSWSItemPropertyHandler::ApplyDisguise -> 0x005b2870  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9e80 CSWSItemPropertyHandler::ApplyDampenSound -> 0x005b2a70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 4 common): GOG {0x7*,0x10*,0x30*,0x148*} | Steam {0x7*,0x10*,0x30*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b2980: 16-byte aligned: YES
  005b2980  55             PUSH EBP
  005b2981  8B EC          MOV EBP,ESP
  005b2983  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2980
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2980 FUN_005b2980 size=233 ret=16 callers=0 callees=2

undefined4 FUN_005b2980(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  int local_14;
  
  uVar1 = *(ushort *)(param_2 + 2);
  if ((int *)(param_3 + 0x148) != (int *)0x0) {
    for (local_14 = 0; local_14 < *(int *)(param_3 + 0x14c); local_14 = local_14 + 1) {
      puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_14 * 4);
      if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996014) && (uVar3 = FUN_005e4e60(0), uVar3 == uVar1))
          && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
         ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
        FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
        return 0;
      }
    }
  }
  return 0;
}
```

### decoy: 0x005b2780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2780: 16-byte aligned: YES
  005b2780  55             PUSH EBP
  005b2781  8B EC          MOV EBP,ESP
  005b2783  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2780
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

## Card B0-12-43

GOG function: Gob::SetProcessFlag  gog 0x0084bb30  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00461ed0 (size 21, retn 0)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00461df0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bb00 CAurPathInfo::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bb80 Global::return_void_0x4  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00855f80 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x193}
imm sets (* = in both, 0 common): GOG {0x4} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00461ed0: 16-byte aligned: YES
  00461ed0  55             PUSH EBP
  00461ed1  8B EC          MOV EBP,ESP
  00461ed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00461ed0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x193 [dec 403]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00461ed0 FUN_00461ed0 size=21 ret=0 callers=7 callees=0

void __fastcall FUN_00461ed0(int param_1)

{
  *(undefined1 *)(param_1 + 0x193) = 1;
  return;
}
```

### decoy: 0x00461df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00461df0: 16-byte aligned: YES
  00461df0  55             PUSH EBP
  00461df1  8B EC          MOV EBP,ESP
  00461df3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00461df0
string literals (0):
direct callees (4):
  0x00461B70  (unnamed)
  0x00461CF0  (unnamed)
  0x00461D70  (unnamed)
  0x00461C60  (unnamed)
struct offsets touched (top 7):
  +0x98 [dec 152]  x4  via EAX,ECX,EDX
  +0x190 [dec 400]  x3  via EAX,ECX,EDX
  +0x194 [dec 404]  x3  via EAX,EDX
  +0x193 [dec 403]  x2  via EAX
  +0x192 [dec 402]  x1  via ECX
  +0x1AB [dec 427]  x1  via ECX
  +0x1D0 [dec 464]  x1  via EAX
```

## Card B0-12-44

GOG function: CWorldTimer::GetWorldTimeHour  gog 0x0080b5e0  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x0051b100 (size 49, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051b140:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080b5c0 Global::GetWorldTimeTimeOfDay  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080b620 CWorldTimer::GetWorldTimeMinute -> 0x0051b140  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080b020 CWorldTimer::ConvertFromTimeOfDay -> Steam 0x0051ab40  in candidate callees: yes
  0x0080b5c0 Global::GetWorldTimeTimeOfDay -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0051b100: 16-byte aligned: YES
  0051b100  55             PUSH EBP
  0051b101  8B EC          MOV EBP,ESP
  0051b103  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051b100
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051b100 FUN_0051b100 size=49 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0051b100(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined1 *puVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  undefined4 local_14;
  undefined1 local_10 [4];
  undefined1 local_c [4];
  undefined1 local_8 [4];
  
  puVar5 = local_8;
  puVar4 = local_10;
  puVar3 = local_c;
  puVar2 = &local_14;
  uVar1 = FUN_0051b0e0(puVar2,puVar3,puVar4,puVar5,param_1);
  FUN_0051ab40(uVar1,puVar2,puVar3,puVar4,puVar5);
  return local_14;
}
```

### decoy: 0x0051b140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051b140: 16-byte aligned: YES
  0051b140  55             PUSH EBP
  0051b141  8B EC          MOV EBP,ESP
  0051b143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051b140
string literals (0):
direct callees (2):
  0x0051B0E0  Global::GetWorldTimeTimeOfDay
  0x0051AB40  CWorldTimer::ConvertFromTimeOfDay
struct offsets touched (top 0):
```

## Card B0-12-45

GOG function: CSWSItemPropertyHandler::ApplySniperBonus  gog 0x007c9f20  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2b10 (size 34, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2ae0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9ef0 CSWSItemPropertyHandler::ApplyDoorSabering -> 0x005b2ae0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9f50 CSWSItemPropertyHandler::ApplyRapidShotBonus -> 0x005b2b40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2e8*} | Steam {0x2e8*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2b10: 16-byte aligned: YES
  005b2b10  55             PUSH EBP
  005b2b11  8B EC          MOV EBP,ESP
  005b2b13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2b10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E8 [dec 744]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2b10 FUN_005b2b10 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2b10(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2e8) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2ae0: 16-byte aligned: YES
  005b2ae0  55             PUSH EBP
  005b2ae1  8B EC          MOV EBP,ESP
  005b2ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2ae0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E0 [dec 736]  x1  via EAX
```

## Card B0-12-46

GOG function: CSWSItemPropertyHandler::ApplyDampenSound  gog 0x007c9e80  callconv=__thiscall  param_size_bytes=0  gog_size=25  gog_retn=20
Candidate Steam address: 0x005b2a70 (size 25, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2ab0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9d90 CSWSItemPropertyHandler::RemoveDisguise -> 0x005b2980  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9ea0 CSWSItemPropertyHandler::RemoveDampenSound -> 0x005b2a90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0067efe0 CSWSCreature::SetSilentMove -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 20)
```

### prologue
```
prologue for 0x005b2a70: 16-byte aligned: YES
  005b2a70  55             PUSH EBP
  005b2a71  8B EC          MOV EBP,ESP
  005b2a73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2a70
string literals (0):
direct callees (1):
  0x005663B0  CSWSCreature::SetSilentMove
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005b2a70 FUN_005b2a70 size=25 ret=20 callers=0 callees=1

undefined4 FUN_005b2a70(void)

{
  FUN_005663b0(1);
  return 0;
}
```

### decoy: 0x005b2ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2ab0: 16-byte aligned: YES
  005b2ab0  55             PUSH EBP
  005b2ab1  8B EC          MOV EBP,ESP
  005b2ab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2ab0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2DC [dec 732]  x1  via EAX
```

## Card B0-12-47

GOG function: CResGFF::WriteFieldWORD  gog 0x00625c10  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x00719b20 (size 97, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CResGFF::WriteFieldSHORT
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719ab0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  gate_class=REVIEW
- GOG-order neighbour after:  0x00625c80 CResGFF::WriteFieldSHORT -> 0x00719b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam 0x00717aa0  in candidate callees: yes
  0x00624190 CResGFF::AddField -> Steam 0x00718030  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x2,0x8*,0xc*} | Steam {0x3,0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00719b20: 16-byte aligned: YES
  00719b20  55             PUSH EBP
  00719b21  8B EC          MOV EBP,ESP
  00719b23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00719b20
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00719b20 FUN_00719b20 size=97 ret=12 callers=6 callees=2

undefined4 __thiscall FUN_00719b20(int param_1,int param_2,short param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,3);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(int *)(iVar2 + 8) = (int)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719ab0: 16-byte aligned: YES
  00719ab0  55             PUSH EBP
  00719ab1  8B EC          MOV EBP,ESP
  00719ab3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719ab0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card B0-12-48

GOG function: CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionDecrease  gog 0x007c9a60  callconv=__thiscall  param_size_bytes=12  gog_size=293  gog_retn=20
Candidate Steam address: 0x005b2650 (size 293, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2430:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9970 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease -> 0x005b2560  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9b90 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionDecrease -> 0x005b2780  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b2650: 16-byte aligned: YES
  005b2650  55             PUSH EBP
  005b2651  8B EC          MOV EBP,ESP
  005b2653  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b2650
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2650 FUN_005b2650 size=293 ret=20 callers=0 callees=5

undefined4
FUN_005b2650(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_3c;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  sVar1 = *(short *)(param_2 + 6);
  if (sVar1 != 0) {
    iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_28 = (undefined2)DAT_0099608c;
    *(undefined2 *)(local_3c + 8) = local_28;
    *(ushort *)(local_3c + 0xa [dec 10]) = *(ushort *)(local_3c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,sVar1);
    FUN_00544210(local_3c,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2430: 16-byte aligned: YES
  005b2430  55             PUSH EBP
  005b2431  8B EC          MOV EBP,ESP
  005b2433  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b2430
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-12-49

GOG function: CSWSItemPropertyHandler::ApplyRegeneration  gog 0x007c95d0  callconv=__thiscall  param_size_bytes=12  gog_size=369  gog_retn=20
Candidate Steam address: 0x005b21c0 (size 369, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2430:STR0/CALLEE0/SIG0.686/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9490 CSWSItemPropertyHandler::RemoveSkillBonus -> 0x005b2080  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9750 CSWSItemPropertyHandler::RemoveRegeneration -> 0x005b2340  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734960 CGameEffect::Destructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x6*,0x8*,0xa*,0x14*} | Steam {0x6*,0x8*,0xa*,0x14*}
imm sets (* = in both, 6 common): GOG {0x4*,0x14*,0x34*,0x8c*,0x1770*,0xfffffff8*} | Steam {0x4*,0x14*,0x34*,0x8c*,0x1770*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b21c0: 16-byte aligned: YES
  005b21c0  55             PUSH EBP
  005b21c1  8B EC          MOV EBP,ESP
  005b21c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b21c0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0xA [dec 10]  x2  via EAX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005b21c0 FUN_005b21c0 size=369 ret=20 callers=0 callees=7

undefined4
FUN_005b21c0(int param_1,undefined2 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_40;
  undefined2 local_34;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095a0eb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar2 == 0) {
    local_40 = 0;
  }
  else {
    local_40 = FUN_005e4920(1);
  }
  local_8 = 0xffffffff;
  sVar1 = param_2[3];
  if (sVar1 == 0) {
    if (local_40 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(local_40);
    }
  }
  else {
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    local_34 = (undefined2)DAT_00995f38;
    *(undefined2 *)(local_40 + 8) = local_34;
    *(ushort *)(local_40 + 0xa [dec 10]) = *(ushort *)(local_40 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e4e80(0,sVar1);
    FUN_005e4e80(1,6000);
    FUN_005e4e80(4,*param_2);
    FUN_00544210(local_40,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2430: 16-byte aligned: YES
  005b2430  55             PUSH EBP
  005b2431  8B EC          MOV EBP,ESP
  005b2433  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b2430
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-12-50

GOG function: CSWSItemPropertyHandler::RemoveImmunity  gog 0x007c9110  callconv=__thiscall  param_size_bytes=12  gog_size=518  gog_retn=16
Candidate Steam address: 0x005b1d00 (size 518, retn 16)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005b2080:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c8e40 CSWSItemPropertyHandler::ApplyImmunity -> 0x005b1a30  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9340 CSWSItemPropertyHandler::ApplySkillBonus -> 0x005b1f30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*}
imm sets (* = in both, 5 common): GOG {0x7*,0x9*,0x10*,0x50*,0x148*} | Steam {0x7*,0x9*,0x10*,0x50*,0x148*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005b1d00: 16-byte aligned: YES
  005b1d00  55             PUSH EBP
  005b1d01  8B EC          MOV EBP,ESP
  005b1d03  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b1d00
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b1d00 FUN_005b1d00 size=518 ret=16 callers=0 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_005b1d00(int param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  bool bVar2;
  int iVar3;
  int local_14;
  int local_10;
  int local_8;
  
  bVar2 = false;
  if ((int *)(param_3 + 0x148) != (int *)0x0) {
    switch(*(undefined2 *)(param_2 + 2)) {
    case 0:
      local_14 = _DAT_0099622c;
      break;
    case 1:
      local_14 = DAT_00996228;
      local_10 = DAT_00996200;
      bVar2 = true;
      break;
    case 2:
      local_14 = _DAT_009961c0;
      break;
    case 3:
      local_14 = _DAT_009961c4;
      break;
    case 4:
      local_14 = _DAT_009961c8;
      break;
    case 5:
      local_14 = _DAT_009961cc;
      break;
    case 6:
      local_14 = _DAT_00996224;
      break;
    case 7:
      local_14 = _DAT_009961d4;
      break;
    case 8:
      local_14 = _DAT_00996230;
      break;
    case 9:
      local_14 = _DAT_00996234;
      break;
    default:
      goto switchD_005b1d43_default;
    }
    for (local_8 = 0; local_8 < *(int *)(param_3 + 0x14c); local_8 = local_8 + 1) {
      puVar1 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_8 * 4);
      if ((((*(ushort *)(puVar1 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f74) &&
           (iVar3 = FUN_005e4e60(0), iVar3 == local_14)) &&
          ((*(ushort *)((int)puVar1 + 0xa [dec 10]) & 7) == DAT_009967b0)) &&
         (puVar1[6 = byte +0x18] == *(int *)(param_1 + 0x14))) {
        FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
        return 0;
      }
      if (((bVar2) && (*(ushort *)(puVar1 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00995f74)) &&
         ((iVar3 = FUN_005e4e60(0), iVar3 == local_10 &&
          (((*(ushort *)((int)puVar1 + 0xa [dec 10]) & 7) == DAT_009967b0 &&
           (puVar1[6 = byte +0x18] == *(int *)(param_1 + 0x14))))))) {
        FUN_00544790(*puVar1,puVar1[1 = byte +0x4]);
        return 0;
      }
    }
  }
switchD_005b1d43_default:
  return 0;
}
```

### decoy: 0x005b2080 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2080: 16-byte aligned: YES
  005b2080  55             PUSH EBP
  005b2081  8B EC          MOV EBP,ESP
  005b2083  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2080
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 9):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EAX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x13C [dec 316]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

## Card B0-12-51

GOG function: CSWSEncounter::GetFirstName  gog 0x007e94d0  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00623010 (size 19, retn 0)
Candidate sources: branch+mapper:drift+0x0+name_map:high:batch7_reverse_engineering+near    mapper tag: drift

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00623060:STR0/CALLEE0/SIG0.05/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e9150 CSWSEncounter::Constructor -> 0x00622c90  gate_class=REVIEW
- GOG-order neighbour after:  0x007e94f0 CSWSEncounter::Destructor -> 0x00623030  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x270*} | Steam {0x270*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00623010: 16-byte aligned: YES
  00623010  55             PUSH EBP
  00623011  8B EC          MOV EBP,ESP
  00623013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00623010
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00623010 FUN_00623010 size=19 ret=0 callers=0 callees=0

int __fastcall FUN_00623010(int param_1)

{
  return param_1 + 0x270;
}
```

### decoy: 0x00623060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00623060: 16-byte aligned: YES
  00623060  55             PUSH EBP
  00623061  8B EC          MOV EBP,ESP
  00623063  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00623060
string literals (0):
direct callees (12):
  0x006239D0  CSWSEncounter::RemoveFromArea
  0x0051C280  (unnamed)
  0x0051E0C0  CServerAIMaster::RemoveObject
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EA80  (unnamed)
  0x006232B0  (unnamed)
  0x009196FD  operator delete
  0x00628840  CSWSEncounter::ClearSpawnList
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00735360  CExoLocString::Destructor
  0x0053F050  CSWSObject::Destructor_2
struct offsets touched (top 8):
  +0x2C4 [dec 708]  x3  via EAX,ECX
  +0x2D0 [dec 720]  x3  via EAX,EDX
  +0x2D8 [dec 728]  x3  via ECX,EDX
  +0x2F0 [dec 752]  x3  via ECX,EDX
  +0x2F8 [dec 760]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x350 [dec 848]  x1  via EDX
```

## Card B0-12-52

GOG function: CSWSScriptVarTable::DestroyObject  gog 0x007f07f0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x005e6f60 (size 27, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e6f40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f07d0 CSWSScriptVarTable::DestroyString -> 0x005e6f40  gate_class=REVIEW
- GOG-order neighbour after:  0x007f0810 CSWSScriptVarTable::LoadVarTable -> 0x005e6f80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007effe0 CSWSScriptVarTable::DeleteIndex -> Steam ?  in candidate callees: n/a
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
prologue for 0x005e6f60: 16-byte aligned: YES
  005e6f60  55             PUSH EBP
  005e6f61  8B EC          MOV EBP,ESP
  005e6f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e6f60
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005e6f60 FUN_005e6f60 size=27 ret=4 callers=1 callees=1
THUNK -> 0x005E6750 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005e6f60(undefined4 param_1)

{
  FUN_005e6750(param_1,4);
  return;
}
```

### decoy: 0x005e6f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e6f40: 16-byte aligned: YES
  005e6f40  55             PUSH EBP
  005e6f41  8B EC          MOV EBP,ESP
  005e6f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e6f40
string literals (0):
direct callees (1):
  0x005E6750  CSWSScriptVarTable::DeleteIndex
struct offsets touched (top 0):
```

## Card B0-12-53

GOG function: CSWSEncounter::RemoveFromArea  gog 0x007e9e80  callconv=(blank)  param_size_bytes=0  gog_size=128  gog_retn=0
Candidate Steam address: 0x006239d0 (size 128, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch7_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00623940:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e9df0 CSWSEncounter::AddToArea -> 0x00623940  gate_class=REVIEW
- GOG-order neighbour after:  0x007e9f00 CSWSEncounter::LineSegmentIntersectActivateArea -> 0x00623a50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ac100 CSWSObject::SetArea -> Steam ?  in candidate callees: n/a
  0x006fcfa0 unnamed -> Steam ?  in candidate callees: n/a
  0x00718240 CSWSArea::RemoveObjectFromArea -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x18*,0x1a4*} | Steam {0x18*,0x1a4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006239d0: 16-byte aligned: YES
  006239d0  55             PUSH EBP
  006239d1  8B EC          MOV EBP,ESP
  006239d3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006239d0
string literals (0):
direct callees (5):
  0x005453C0  CSWSObject::GetArea
  0x00523090  CSWSArea::RemoveObjectFromArea
  0x0052E4E0  (unnamed)
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006239d0 FUN_006239d0 size=128 ret=0 callers=2 callees=5

void __fastcall FUN_006239d0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
    FUN_005453c0();
    FUN_0052e4e0(*(undefined4 *)(param_1 + 4));
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### decoy: 0x00623940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00623940: 16-byte aligned: YES
  00623940  55             PUSH EBP
  00623941  8B EC          MOV EBP,ESP
  00623943  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00623940
string literals (0):
direct callees (6):
  0x006239D0  CSWSEncounter::RemoveFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
  0x005453C0  CSWSObject::GetArea
  0x00522BF0  CSWSArea::AddObjectToArea
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B0-12-54

GOG function: CSWSItemPropertyHandler::ApplyDoorCutting  gog 0x007c9ec0  callconv=__thiscall  param_size_bytes=4  gog_size=34  gog_retn=20
Candidate Steam address: 0x005b2ab0 (size 34, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2ae0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9ea0 CSWSItemPropertyHandler::RemoveDampenSound -> 0x005b2a90  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9ef0 CSWSItemPropertyHandler::ApplyDoorSabering -> 0x005b2ae0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2dc*} | Steam {0x2dc*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005b2ab0: 16-byte aligned: YES
  005b2ab0  55             PUSH EBP
  005b2ab1  8B EC          MOV EBP,ESP
  005b2ab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005b2ab0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2DC [dec 732]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2ab0 FUN_005b2ab0 size=34 ret=20 callers=0 callees=0

undefined4 FUN_005b2ab0(int param_1)

{
  if (param_1 != 0) {
    *(undefined4 *)(param_1 + 0x2dc) = 1;
  }
  return 0;
}
```

### decoy: 0x005b2ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2ae0: 16-byte aligned: YES
  005b2ae0  55             PUSH EBP
  005b2ae1  8B EC          MOV EBP,ESP
  005b2ae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005b2ae0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2E0 [dec 736]  x1  via EAX
```

## Card B0-12-55

GOG function: CSWSItemPropertyHandler::ApplyLight  gog 0x007c4dc0  callconv=__thiscall  param_size_bytes=4  gog_size=276  gog_retn=20
Candidate Steam address: 0x005adaf0 (size 479, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyAbilityBonus
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.47 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005adcf0:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4ad0 CSWSItemPropertyHandler::RemoveAttackPenalty -> 0x005ad6c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4ee0 CSWSItemPropertyHandler::RemoveLight -> 0x005adad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x14*} | Steam {0x2,0x6,0x8*,0xa*,0x14*,0x130}
imm sets (* = in both, 4 common): GOG {0x4*,0x14*,0x1d,0x24,0x8c*,0x1388,0xfffffff8*} | Steam {0x4*,0x5,0x14*,0x3c,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005adaf0: 16-byte aligned: YES
  005adaf0  55             PUSH EBP
  005adaf1  8B EC          MOV EBP,ESP
  005adaf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005adaf0
string literals (1):
  0x00992E8C  at 005adb18  'Value'
direct callees (9):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 6):
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005adaf0 FUN_005adaf0 size=479 ret=20 callers=0 callees=9

undefined4
FUN_005adaf0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  undefined1 *puVar4;
  int *piVar5;
  int local_48;
  undefined2 local_34;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952e03;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  piVar5 = &local_14;
  puVar4 = local_20;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(1);
  FUN_0071bcf0(uVar2,puVar4,piVar5);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_14 != 0) {
    iVar3 = FUN_00919723(0x8c);
    local_8 = 1;
    if (iVar3 == 0) {
      local_48 = 0;
    }
    else {
      local_48 = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_18 = local_48;
    local_34 = (undefined2)DAT_00995fac;
    *(undefined2 *)(local_48 + 8) = local_34;
    *(ushort *)(local_48 + 0xa [dec 10]) = *(ushort *)(local_48 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    switch(*(undefined2 *)(param_2 + 2)) {
    case 0:
      FUN_005e4e80(0,DAT_009960f8);
      break;
    case 1:
      FUN_005e4e80(0,DAT_009960fc);
      break;
    case 2:
      FUN_005e4e80(0,DAT_00996100);
      break;
    case 3:
      FUN_005e4e80(0,DAT_00996104);
      break;
    case 4:
      FUN_005e4e80(0,DAT_00996108);
      break;
    case 5:
      FUN_005e4e80(0,DAT_0099610c);
    }
    FUN_005e4e80(1,local_14);
    FUN_00544210(local_18,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005adcf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005adcf0: 16-byte aligned: YES
  005adcf0  55             PUSH EBP
  005adcf1  8B EC          MOV EBP,ESP
  005adcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005adcf0
string literals (1):
  0x00992E8C  at 005add18  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x150 [dec 336]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-12-56

GOG function: CSWSItemPropertyHandler::ApplyLight  gog 0x007c4dc0  callconv=__thiscall  param_size_bytes=4  gog_size=276  gog_retn=20
Candidate Steam address: 0x005ad3c0 (size 765, retn 20)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::ApplyAttackPenalty
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ad6c0:STR0/CALLEE0/SIG0.179/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c4ad0 CSWSItemPropertyHandler::RemoveAttackPenalty -> 0x005ad6c0  gate_class=REVIEW
- GOG-order neighbour after:  0x007c4ee0 CSWSItemPropertyHandler::RemoveLight -> 0x005adad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x8*,0xa*,0x14*} | Steam {0x6,0x8*,0xa*,0xc,0x14*,0x34,0x122,0x130}
imm sets (* = in both, 4 common): GOG {0x4*,0x14*,0x1d,0x24,0x8c*,0x1388,0xfffffff8*} | Steam {0x2,0x3,0x4*,0x8,0x10,0x14*,0x20,0x64,0x8c*,0x4000,0x8000,0x10000,0xfffffff8*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 20)
```

### prologue
```
prologue for 0x005ad3c0: 16-byte aligned: YES
  005ad3c0  55             PUSH EBP
  005ad3c1  8B EC          MOV EBP,ESP
  005ad3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ad3c0
string literals (1):
  0x00992E8C  at 005ad42e  'Value'
direct callees (13):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x006D5200  (unnamed)
  0x005E4FD0  CGameEffect::CopyEffect
  0x00544210  CSWSObject::ApplyEffect
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 8):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xA [dec 10]  x2  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ad3c0 FUN_005ad3c0 size=765 ret=20 callers=0 callees=13

undefined4 FUN_005ad3c0(int param_1,int param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 local_70;
  int local_6c;
  undefined2 local_48;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952dce;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == 0) {
    local_6c = 0;
  }
  else {
    local_6c = FUN_005e4920(1);
  }
  local_20 = local_6c;
  local_8 = 0xffffffff;
  local_14 = local_6c;
  FUN_00733570("Value");
  local_8 = 1;
  piVar4 = &local_18;
  puVar3 = local_2c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780();
  iVar1 = local_14;
  local_18 = -local_18;
  if (local_18 == 0) {
    if (local_14 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar1);
    }
  }
  else {
    local_48 = (undefined2)DAT_00995f48;
    *(undefined2 *)(local_14 + 8) = local_48;
    *(ushort *)(local_14 + 0xa [dec 10]) = *(ushort *)(local_14 + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,local_18);
    FUN_005e4e80(2,*(undefined1 *)(DAT_00a1b4d0 + 0x122));
    if (param_4 == 0x10) {
      FUN_005e4e80(1,DAT_00996188);
    }
    else if (param_4 == 0x20) {
      FUN_005e4e80(1,DAT_0099618c);
    }
    else if (param_4 == 0x4000) {
      FUN_005e4e80(1,DAT_00996190);
    }
    else if (param_4 == 0x8000) {
      FUN_005e4e80(1,DAT_00996194);
    }
    else if (param_4 == 0x10000) {
      FUN_005e4e80(1,DAT_00996198);
    }
    else if (param_4 == 8) {
      FUN_005e4e80(1,DAT_0099619c);
    }
    iVar1 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if (*(char *)(iVar1 + 8) == '\x03') {
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x005ad6c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ad6c0: 16-byte aligned: YES
  005ad6c0  55             PUSH EBP
  005ad6c1  8B EC          MOV EBP,ESP
  005ad6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ad6c0
string literals (1):
  0x00992E8C  at 005ad6e8  'Value'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x6 [dec 6]  x1  via EAX
  +0xA [dec 10]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x122 [dec 290]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x13C [dec 316]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
```

## Card B0-12-57

GOG function: CSWVirtualMachineCommands::InitializeSWMGCommands  gog 0x007f9140  callconv=__thiscall  param_size_bytes=0  gog_size=1659  gog_retn=0
Candidate Steam address: 0x006f5b80 (size 1659, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006e3a40:STR0/CALLEE0/SIG0.004/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f9120 CSWSClient::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007f97c0 Global::ExecuteCommandAurPostString  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 104 common): GOG {0xc*,0x820*,0x824*,0x918*,0x91c*,0x920*,0x924*,0x928*,0x92c*,0x930*,0x934*,0x938*,0x93c*,0x940*,...(+90)} | Steam {0xc*,0x820*,0x824*,0x918*,0x91c*,0x920*,0x924*,0x928*,0x92c*,0x930*,0x934*,0x938*,0x93c*,0x940*,...(+90)}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006f5b80: 16-byte aligned: YES
  006f5b80  55             PUSH EBP
  006f5b81  8B EC          MOV EBP,ESP
  006f5b83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f5b80
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0xC [dec 12]  x103  via EAX,ECX,EDX
  +0x820 [dec 2080]  x1  via ECX
  +0x824 [dec 2084]  x1  via EDX
  +0x918 [dec 2328]  x1  via ECX
  +0x91C [dec 2332]  x1  via EAX
  +0x920 [dec 2336]  x1  via EDX
  +0x924 [dec 2340]  x1  via ECX
  +0x928 [dec 2344]  x1  via EAX
  +0x92C [dec 2348]  x1  via EDX
  +0x930 [dec 2352]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006f5b80 FUN_006f5b80 size=1659 ret=0 callers=1 callees=0

void __fastcall FUN_006f5b80(int param_1)

{
  *(code **)(*(int *)(param_1 + 0xc) + 0x918) = FUN_006f6200;
  *(code **)(*(int *)(param_1 + 0xc) + 0x91c) = FUN_006f6320;
  *(code **)(*(int *)(param_1 + 0xc) + 0x920) = FUN_006f6420;
  *(code **)(*(int *)(param_1 + 0xc) + 0x924) = FUN_006f6540;
  *(code **)(*(int *)(param_1 + 0xc) + 0x928) = FUN_006f6640;
  *(code **)(*(int *)(param_1 + 0xc) + 0x92c) = FUN_006f6840;
  *(code **)(*(int *)(param_1 + 0xc) + 0x930) = FUN_006f6840;
  *(code **)(*(int *)(param_1 + 0xc) + 0x934) = FUN_006f6970;
  *(code **)(*(int *)(param_1 + 0xc) + 0x938) = FUN_006f69f0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x93c) = FUN_006f6aa0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x940) = FUN_006f6aa0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x944) = FUN_006f6bc0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x948) = FUN_006f6bc0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x94c) = FUN_006f6840;
  *(code **)(*(int *)(param_1 + 0xc) + 0x950) = FUN_006f6840;
  *(code **)(*(int *)(param_1 + 0xc) + 0x954) = FUN_006f6c90;
  *(code **)(*(int *)(param_1 + 0xc) + 0x958) = FUN_006f6aa0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x95c) = FUN_006f6db0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x960) = FUN_006f6db0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x964) = FUN_006f6db0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x968) = FUN_006f6db0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x96c) = FUN_006f6db0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x970) = FUN_006f69f0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x974) = FUN_006f6aa0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x978) = FUN_006f6f10;
  *(code **)(*(int *)(param_1 + 0xc) + 0x97c) = FUN_006f6f80;
  *(code **)(*(int *)(param_1 + 0xc) + 0x980) = FUN_006f7090;
  *(code **)(*(int *)(param_1 + 0xc) + 0x984) = FUN_006f7090;
  *(code **)(*(int *)(param_1 + 0xc) + 0x988) = FUN_006f7120;
  *(code **)(*(int *)(param_1 + 0xc) + 0x98c) = FUN_006f71a0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x990) = FUN_006f7210;
  *(code **)(*(int *)(param_1 + 0xc) + 0x994) = FUN_006f72a0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x998) = FUN_006f7210;
  *(code **)(*(int *)(param_1 + 0xc) + 0x99c) = FUN_006f72a0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9a0) = FUN_006f7360;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9a4) = FUN_006f7360;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9a8) = FUN_006f76f0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9ac) = FUN_006f7530;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9b0) = FUN_006f77a0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9b4) = FUN_006f7360;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9b8) = FUN_006f76f0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9bc) = FUN_006f75e0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9c0) = FUN_006f7360;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9c4) = FUN_006f7850;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9c8) = FUN_006f7850;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9cc) = FUN_006f79c0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9d0) = FUN_006f7ac0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9d4) = FUN_006f7ac0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9d8) = FUN_006f7ac0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9dc) = FUN_006f79c0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9e0) = FUN_006f7cd0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9e4) = FUN_006f7cd0;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9e8) = FUN_006f7e60;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9ec) = FUN_006f7f40;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9f0) = FUN_006f7f40;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9f4) = FUN_006f7f40;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9f8) = FUN_006f7e60;
  *(code **)(*(int *)(param_1 + 0xc) + 0x9fc) = FUN_006f8180;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa00) = FUN_006f79c0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa04) = FUN_006f8340;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa08) = FUN_006f7530;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa0c) = FUN_006f8250;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa10) = FUN_006f8250;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa6c) = FUN_006f8250;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa14) = FUN_006f8250;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa18) = FUN_006f8340;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa1c) = FUN_006f85d0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa20) = FUN_006f77a0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa24) = FUN_006f84e0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa28) = FUN_006f84e0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa70) = FUN_006f84e0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa2c) = FUN_006f84e0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa30) = FUN_006f85d0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa34) = FUN_006f8340;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa38) = FUN_006f85d0;
  *(code **)(*(int *)(param_1 + 0xc) + 0xa3c) = FUN_006f8340;
/* ... 29 more lines truncated by --max-lines */
```

### decoy: 0x006e3a40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006e3a40: 16-byte aligned: YES
  006e3a40  55             PUSH EBP
  006e3a41  8B EC          MOV EBP,ESP
  006e3a43  81 EC 30 02 00 00 SUB ESP,0x230
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006e3a40
string literals (0):
direct callees (20):
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0058FC60  CSWSCombatRound::SetRoundPaused
  0x0058FCD0  CSWSCombatRound::SetPauseTimer
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x005143C0  (unnamed)
  0x005716A0  CSWSCreature::DesiredAttackRange
  0x0056E1E0  CSWSCreature::RangeWeaponEquipped
  0x0091EB8B  (unnamed)
  0x0091FC10  (unnamed)
  0x0091FA70  (unnamed)
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0045CFE0  (unnamed)
  0x00550980  CSWSArea::TestDirectLine
  0x005C25E0  CSWSCreature::UpdateSubareasOnMoveTo
  0x00543F50  (unnamed)
struct offsets touched (top 10):
  +0x10DC [dec 4316]  x16  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX
  +0x7C [dec 124]  x8  via EAX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX
  +0x30 [dec 48]  x6  via ECX,EDX
  +0x380 [dec 896]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x1B0 [dec 432]  x1  via EAX
```

## Card B0-12-58

GOG function: CSWSItemPropertyHandler::ApplyBlasterBoltDeflectionIncrease  gog 0x007c9840  callconv=__thiscall  param_size_bytes=12  gog_size=293  gog_retn=20
Candidate Steam address: 0x005b2430 (size 293, retn 20)
Candidate sources: branch+name_map:high:batch8_reverse_engineering+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005b2650:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x007c9750 CSWSItemPropertyHandler::RemoveRegeneration -> 0x005b2340  gate_class=REVIEW
- GOG-order neighbour after:  0x007c9970 CSWSItemPropertyHandler::RemoveBlasterBoltDeflectionIncrease -> 0x005b2560  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  0x007345e0 CGameEffect::Constructor -> Steam ?  in candidate callees: n/a
  0x00734b40 CGameEffect::SetInteger -> Steam ?  in candidate callees: n/a
  0x00735b60 CGameEffect::SetCreator -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x6*,0x8*,0xa*,0x14*} | Steam {0x2*,0x6*,0x8*,0xa*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*} | Steam {0x4*,0x14*,0x2c*,0x8c*,0xfffffff8*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 20)
```

### prologue
```
prologue for 0x005b2430: 16-byte aligned: YES
  005b2430  55             PUSH EBP
  005b2431  8B EC          MOV EBP,ESP
  005b2433  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005b2430
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005b2430 FUN_005b2430 size=293 ret=20 callers=0 callees=5

undefined4
FUN_005b2430(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  short sVar1;
  int iVar2;
  int local_3c;
  undefined2 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095315b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  sVar1 = *(short *)(param_2 + 6);
  if (sVar1 != 0) {
    iVar2 = FUN_00919723(0x8c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar2 == 0) {
      local_3c = 0;
    }
    else {
      local_3c = FUN_005e4920(1);
    }
    local_8 = 0xffffffff;
    local_28 = (undefined2)DAT_00996088;
    *(undefined2 *)(local_3c + 8) = local_28;
    *(ushort *)(local_3c + 0xa [dec 10]) = *(ushort *)(local_3c + 0xa [dec 10]) & 0xfff8 | DAT_009967b0;
    FUN_005e5ea0(*(undefined4 *)(param_1 + 0x14));
    FUN_005e4e80(0,*(undefined2 *)(param_2 + 2));
    FUN_005e4e80(1,sVar1);
    FUN_00544210(local_3c,param_5,0);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005b2650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005b2650: 16-byte aligned: YES
  005b2650  55             PUSH EBP
  005b2651  8B EC          MOV EBP,ESP
  005b2653  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005b2650
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0xA [dec 10]  x2  via EAX
  +0x2 [dec 2]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
```

## Card B0-12-59

GOG function: CSWSScriptVarTable::GetInt  gog 0x007f0060  callconv=(blank)  param_size_bytes=4  gog_size=59  gog_retn=4
Candidate Steam address: 0x005e67d0 (size 59, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:gog_cswsscriptvartable_cluster+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005e68f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007effe0 CSWSScriptVarTable::DeleteIndex -> 0x005e6750  gate_class=AUTO
- GOG-order neighbour after:  0x007f00a0 CSWSScriptVarTable::GetFloat -> 0x005e6810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007efe10 CSWSScriptVarTable::MatchIndex -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}
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
prologue for 0x005e67d0: 16-byte aligned: YES
  005e67d0  55             PUSH EBP
  005e67d1  8B EC          MOV EBP,ESP
  005e67d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005e67d0
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005e67d0 FUN_005e67d0 size=59 ret=4 callers=3 callees=1

undefined4 FUN_005e67d0(undefined4 param_1)

{
  int iVar1;
  undefined4 local_c;
  
  local_c = 0;
  iVar1 = FUN_005e6580(param_1,1,0);
  if (iVar1 != 0) {
    local_c = *(undefined4 *)(iVar1 + 0xc);
  }
  return local_c;
}
```

### decoy: 0x005e68f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005e68f0: 16-byte aligned: YES
  005e68f0  55             PUSH EBP
  005e68f1  8B EC          MOV EBP,ESP
  005e68f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005e68f0
string literals (0):
direct callees (1):
  0x005E6580  CSWSScriptVarTable::MatchIndex
struct offsets touched (top 1):
  +0xC [dec 12]  x1  via EDX
```

## Card B0-12-60

GOG function: CGuiInGame::RemoveGUIPopUps  gog 0x004e0ca0  callconv=(blank)  param_size_bytes=0  gog_size=120  gog_retn=0
Candidate Steam address: 0x007d0560 (size 49, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007d05a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004e0c60 CGuiInGame::GetMiniMapVisible -> 0x007d0560  gate_class=REVIEW
- GOG-order neighbour after:  0x004e0d20 CGuiInGame::ResetInterfaceFont  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam ?  in candidate callees: n/a
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x1c,0x98,0x59b4} | Steam {0x3c,0x40,0x84}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007d0560: 16-byte aligned: YES
  007d0560  55             PUSH EBP
  007d0561  8B EC          MOV EBP,ESP
  007d0563  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007d0560
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x3C [dec 60]  x3  via EAX,EDX
  +0x40 [dec 64]  x1  via ECX
  +0x84 [dec 132]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007d0560 FUN_007d0560 size=49 ret=0 callers=1 callees=0

void __fastcall FUN_007d0560(int param_1)

{
  if (*(int *)(param_1 + 0x3c) == *(int *)(param_1 + 0x40)) {
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x84))();
  }
  return;
}
```

### decoy: 0x007d05a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007d05a0: 16-byte aligned: YES
  007d05a0  55             PUSH EBP
  007d05a1  8B EC          MOV EBP,ESP
  007d05a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007d05a0
string literals (0):
direct callees (2):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
  0x007400A0  CClientExoApp::GetMiniGame
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x59B4 [dec 22964]  x1  via EDX
```
