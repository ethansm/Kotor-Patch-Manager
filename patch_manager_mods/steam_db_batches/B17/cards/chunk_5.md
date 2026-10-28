# Review cards B17 chunk 5

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-5-01

GOG function: CExoTimers::Constructor  gog 0x00608260  callconv=(blank)  param_size_bytes=0  gog_size=125  gog_retn=0
Candidate Steam address: 0x00736130 (size 125, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007361b0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00608230 CExoBase::WcsToMbs  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006082e0 CExoTimers::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0060dda0 CExoTimersInternal::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x20*} | Steam {0x4*,0x10*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00736130: 16-byte aligned: YES
  00736130  55             PUSH EBP
  00736131  8B EC          MOV EBP,ESP
  00736133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00736130
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0073B720  CExoTimersInternal::Constructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00736130 FUN_00736130 size=125 ret=0 callers=1 callees=2

undefined4 * __fastcall FUN_00736130(undefined4 *param_1)

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
  iVar1 = FUN_00919723(0x20,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_20 = 0;
  }
  else {
    local_20 = FUN_0073b720();
  }
  *param_1 = local_20;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x007361b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007361b0: 16-byte aligned: YES
  007361b0  55             PUSH EBP
  007361b1  8B EC          MOV EBP,ESP
  007361b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007361b0
string literals (0):
direct callees (2):
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 125 B, RET 0
- callers 1: 0x00735c60
- callees: 0x00919723 operator new (CALL), 0x0073b720 (CALL)
- twins: none

## Card B17-5-02

GOG function: CSWGuiKeyMapButton::HitCheckMouse  gog 0x005f1910  callconv=(blank)  param_size_bytes=8  gog_size=76  gog_retn=8
Candidate Steam address: 0x00902b60 (size 76, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00902c70:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f18a0 CSWGuiKeyMapButton::SetCaptureVisual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f1960 CSWGuiLevelUpPanel::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00916fc0 CSWGuiControl::HitCheckMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x44*,0x1d4*} | Steam {0x44*,0x1d4*}
imm sets (* = in both, 2 common): GOG {0x8*,0x1d4*} | Steam {0x8*,0x1d4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00902b60: 16-byte aligned: YES
  00902b60  55             PUSH EBP
  00902b61  8B EC          MOV EBP,ESP
  00902b63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00902b60
string literals (0):
direct callees (1):
  0x00418D20  (unnamed)
struct offsets touched (top 2):
  +0x44 [dec 68]  x1  via EAX
  +0x1D4 [dec 468]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00902b60 FUN_00902b60 size=76 ret=8 callers=0 callees=1

int __thiscall FUN_00902b60(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_00418d20(param_2,param_3);
  if (iVar1 == 0) {
    iVar1 = (**(code **)(*(int *)(param_1 + 0x1d4) + 0x44))(param_2,param_3);
  }
  return iVar1;
}
```

### decoy: 0x00902c70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00902c70: 16-byte aligned: YES
  00902c70  55             PUSH EBP
  00902c71  8B EC          MOV EBP,ESP
  00902c73  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00902c70
string literals (1):
  0x009AA134  at 00902c93  'projectile'
direct callees (4):
  0x00710980  (unnamed)
  0x00462320  (unnamed)
  0x00710CA0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xB8 [dec 184]  x2  via EAX,EDX
  +0x3C [dec 60]  x1  via EAX
```

### Static facts (script output, report-only)
- size 76 B, RET 8
- callees: 0x00418d20 (CALL)
- indirect: 0x00902ba4 slot 0x44
- twins: none

## Card B17-5-03

GOG function: CExoKeyTable::GetTableIndex  gog 0x00630410  callconv=(blank)  param_size_bytes=6  gog_size=142  gog_retn=8
Candidate Steam address: 0x00726670 (size 287, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x00630390:1.00>0.57
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.23 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00726790:STR0/CALLEE0/SIG0.317/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00630390 CExoKeyTable::FindKey_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006304a0 CExoKeyTable::RebuildTable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*,0x10,0x1a}
imm sets (* = in both, 1 common): GOG {0x4,0x8*,0x10,0x14,0xff,0xffffff} | Steam {0x8*,0x1c,0x24}

### abi
```
GOG param bytes: 6; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(6/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 6 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00726670: 16-byte aligned: YES
  00726670  55             PUSH EBP
  00726671  8B EC          MOV EBP,ESP
  00726673  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00726670
string literals (0):
direct callees (3):
  0x00710C20  (unnamed)
  0x007269C0  (unnamed)
  0x00710B40  (unnamed)
struct offsets touched (top 2):
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00726670 FUN_00726670 size=287 ret=8 callers=1 callees=3

int __thiscall FUN_00726670(int param_1,undefined4 param_2,short param_3)

{
  bool bVar1;
  bool bVar2;
  int iVar3;
  uint uVar4;
  int local_14;
  uint local_c;
  
  local_14 = 0;
  bVar1 = false;
  bVar2 = false;
  iVar3 = FUN_00710c20(&DAT_00986925);
  if (iVar3 == 0) {
    if (*(int *)(param_1 + 0xc) == 0) {
      local_14 = 0;
    }
    else {
      uVar4 = FUN_007269c0(param_2,param_3);
      local_c = uVar4;
      while ((!bVar1 && (!bVar2))) {
        iVar3 = FUN_00710b40(param_2);
        if ((iVar3 == 0) ||
           (*(short *)(*(int *)(param_1 + 0x10) + 0x1a + local_c * 0x24) != param_3)) {
          iVar3 = FUN_00710c20(&DAT_00986925);
          if (iVar3 == 0) {
            local_c = local_c + 1;
            if (*(uint *)(param_1 + 0xc) <= local_c) {
              local_c = 0;
            }
            if (local_c == uVar4) {
              bVar2 = true;
            }
          }
          else {
            bVar1 = true;
          }
        }
        else {
          local_14 = local_c * 0x24 + *(int *)(param_1 + 0x10);
          bVar1 = true;
        }
      }
    }
  }
  else {
    local_14 = 0;
  }
  return local_14;
}
```

### decoy: 0x00726790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00726790: 16-byte aligned: YES
  00726790  55             PUSH EBP
  00726791  8B EC          MOV EBP,ESP
  00726793  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00726790
string literals (0):
direct callees (9):
  0x00919723  operator new
  0x0073A9E0  CExoArrayList ctor(capacity)
  0x00710980  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0073AAD0  (unnamed)
  0x008289A0  (unnamed)
  0x0073AA30  (unnamed)
  0x009196FD  operator delete
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xC [dec 12]  x2  via ECX
  +0x10 [dec 16]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 287 B, RET 8
- callers 1: 0x007122c0 CExoResMan::GetKeyEntryFromTable
- callees: 0x00710c20 (CALL), 0x007269c0 (CALL), 0x00710b40 (CALL)
- twins: none

## Card B17-5-04

GOG function: CSWCModule::TurnCamera  gog 0x0046b0c0  callconv=(blank)  param_size_bytes=8  gog_size=885  gog_retn=8
Candidate Steam address: 0x007f3770 (size 1196, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.55) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.26 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetCombatMode 0x740860)
- best decoy (neighbour/other candidate) scores: 0x007f3640:STR0/CALLEE0/SIG0.156/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b000 CSWCModule::AcclTiltCamera  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: no
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: no
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x00574940 CSWBehaviorCameraCombat::SetTurn -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam 0x00543f10  in candidate callees: no
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x0080d740 Global::Pitch -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x14,0x1c*,0x28,0x2c*,0x40*,0x80*,0x88,0x98*,0x100,0x10c,0x460} | Steam {0x4*,0x8*,0xc*,0x1c*,0x2c*,0x30,0x38,0x3c,0x40*,0x44,0x58,0x5c,0x60,0x64,...(+12)}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x5,0x7,0x8*,0xc*,0x10,0x30,0x41,0x44*,0x9c,0x106a*,0x106e,0x1070,...(+1)} | Steam {0x3*,0x4*,0x8*,0xc*,0x24,0x44*,0x50,0x58,0x64,0x90,0xff,0x198,0x106a*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f3770: 16-byte aligned: YES
  007f3770  55             PUSH EBP
  007f3771  8B EC          MOV EBP,ESP
  007f3773  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007f3770
string literals (0):
direct callees (10):
  0x00740860  CClientExoApp::GetCombatMode
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x00919723  operator new
  0x007DD380  (unnamed)
  0x007F3C20  (unnamed)
  0x00775990  CSWCCreature::GetHeadHeightOffset
  0x007E2090  (unnamed)
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007AC3A0  (unnamed)
  0x00774620  CSWCCreature::UpdateShadowBlob
struct offsets touched (top 10):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x40 [dec 64]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via EAX,EDX
  +0x5C [dec 92]  x3  via EDX
  +0x70 [dec 112]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x44 [dec 68]  x2  via ECX,EDX
  +0x64 [dec 100]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f3770 FUN_007f3770 size=1196 ret=8 callers=5 callees=10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f3770(int param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  float10 fVar5;
  float10 fVar6;
  int local_98;
  undefined1 local_70 [12];
  float local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined1 local_54 [12];
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  int *local_38;
  int *local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int local_24;
  int *local_20;
  int *local_1c;
  int *local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096bbcb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_58 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  iVar1 = FUN_00740860(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0xc) == 3) {
      local_5c = *(undefined4 *)(DAT_00a1b4a4 + 4);
      local_34 = (int *)FUN_0073f550(param_2);
      if (local_34 != (int *)0x0) {
        local_24 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
        if (local_24 == 0) {
          local_48 = FUN_00919723(0x198);
          local_8 = 0;
          if (local_48 == 0) {
            local_98 = 0;
          }
          else {
            uVar2 = (**(code **)(*local_34 + 0xd0))();
            uVar3 = (**(code **)(**(int **)(param_1 + 0x40) + 4))(uVar2);
            local_98 = FUN_007dd380(uVar3,uVar2);
          }
          local_44 = local_98;
          local_8 = 0xffffffff;
          local_24 = local_98;
          (**(code **)(**(int **)(param_1 + 0x40) + 0x7c))(local_98);
        }
        FUN_007f3c20(&local_30);
        local_60 = (**(code **)(*local_34 + 0xd4))();
        *(undefined4 *)(local_24 + 0x68) = local_60;
        fVar5 = (float10)FUN_00775990();
        fVar6 = (float10)(**(code **)(*local_34 + 0xd8))();
        local_64 = (float)(fVar6 + (float10)(double)fVar5);
        *(float *)(local_24 + 0x6c) = local_64;
        iVar1 = param_3;
        uVar2 = (**(code **)(*local_34 + 0xd0))(param_3);
        FUN_007e2090(uVar2,iVar1);
        *(undefined4 *)(param_1 + 0x70) = param_2;
        if (param_3 == 0) {
          if (*(int *)(local_24 + 0x1c) != 0) {
            (**(code **)(**(int **)(local_24 + 0x1c) + 0x5c))(local_70,local_30,local_2c,local_28);
          }
/* ... 58 more lines truncated by --max-lines */
```

### decoy: 0x007f3640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3640: 16-byte aligned: YES
  007f3640  55             PUSH EBP
  007f3641  8B EC          MOV EBP,ESP
  007f3643  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f3640
string literals (0):
direct callees (4):
  0x007F51C0  (unnamed)
  0x00784440  CClientExoAppInternal::GetActivePauseState
  0x007A6760  CSWCArea::Update
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 10):
  +0x40 [dec 64]  x5  via ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x8C [dec 140]  x2  via EAX,EDX
  +0xB8 [dec 184]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
  +0x44 [dec 68]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
```

### Static facts (script output, report-only)
- size 1196 B, RET 8
- callers 5: 0x00774900, 0x0078c170 CClientExoAppInternal::AttachPlayerGameObjectToArea, 0x00797f30 CClientExoAppInternal::ChangeCharacter, 0x00804f10, 0x008074e0
- callees: 0x00740860 CClientExoApp::GetCombatMode (CALL), 0x0073f550 CClientExoApp::GetCreatureByGameObjectID (CALL), 0x00919723 operator new (CALL), 0x007dd380 (CALL), 0x007f3c20 (CALL), 0x00775990 (CALL), ...
- indirect: 0x007f3810 slot 0x80, 0x007f384a slot 0xd0, 0x007f3864 slot 0x4, 0x007f38b2 slot 0x7c, 0x007f38ce slot 0xd4, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-5-05

GOG function: Global::AnimateHierarchy  gog 0x008fa000  callconv=(blank)  param_size_bytes=21  gog_size=614  gog_retn=0
Candidate Steam address: 0x004e31c0 (size 107, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004e3160:STR0/CALLEE0/SIG0.033/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008f9cd0 Global::ProceduralAnimateHierarchy -> 0x004dfba0  gate_class=REVIEW
- GOG-order neighbour after:  0x008fa270 Part::Animate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00845e70 MdlNode::ControlDispatch -> Steam ?  in candidate callees: n/a
  0x0084c400 Gob::IsInCutscene -> Steam 0x00459e20  in candidate callees: no
  0x008fa000 Global::AnimateHierarchy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2,0x4,0x8*,0x16,0x28,0x2c,0x30,0x34,0x40,0x48,0x58,0xa8,0x164} | Steam {0x8*,0xdc,0xe0}
imm sets (* = in both, 0 common): GOG {0x5,0x18,0x28,0x41,0x64} | Steam {0x8,0x20}

### abi
```
GOG param bytes: 21; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(21/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 21 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x004e31c0: 16-byte aligned: YES
  004e31c0  55             PUSH EBP
  004e31c1  8B EC          MOV EBP,ESP
  004e31c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004e31c0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0xDC [dec 220]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xE0 [dec 224]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004e31c0 FUN_004e31c0 size=107 ret=0 callers=1 callees=0

undefined4 FUN_004e31c0(int param_1)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0xe0) <= local_8) {
      return 0;
    }
    if (((*(uint *)(*(int *)(*(int *)(param_1 + 0xdc) + local_8 * 4) + 8) & 0x20) == 0) &&
       ((*(uint *)(*(int *)(*(int *)(param_1 + 0xdc) + local_8 * 4) + 8) & 8) == 0)) break;
    local_8 = local_8 + 1;
  }
  return *(undefined4 *)(*(int *)(param_1 + 0xdc) + local_8 * 4);
}
```

### decoy: 0x004e3160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004e3160: 16-byte aligned: YES
  004e3160  55             PUSH EBP
  004e3161  8B EC          MOV EBP,ESP
  004e3163  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004e3160
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xDC [dec 220]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

### Static facts (script output, report-only)
- size 107 B, RET 0
- callers 1: 0x004e3270
- twins: none

## Card B17-5-06

GOG function: CSWGuiFeatsCharGen::AddChosenFeat  gog 0x005fafb0  callconv=(blank)  param_size_bytes=2  gog_size=271  gog_retn=4
Candidate Steam address: 0x0090c120 (size 271, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0090bf90:STR0/CALLEE0/SIG0.6/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005fae20 CSWGuiFeatsCharGen::RemoveChosenFeat -> 0x0090bf90  gate_class=REVIEW
- GOG-order neighbour after:  0x005fb0c0 CSWGuiFeatsCharGen::BuildAvailableList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00491a90 CSWCLevelUpStats::FeatAcquired -> Steam ?  in candidate callees: n/a
  0x004953b0 unnamed -> Steam ?  in candidate callees: n/a
  0x0050e730 CSWRules::GetFeat -> Steam ?  in candidate callees: n/a
  0x005fafb0 CSWGuiFeatsCharGen::AddChosenFeat -> Steam ?  in candidate callees: n/a
  0x005fb0c0 CSWGuiFeatsCharGen::BuildAvailableList -> Steam ?  in candidate callees: n/a
  0x005fb4b0 unnamed -> Steam ?  in candidate callees: n/a
  0x005fb500 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x68*,0x310*,0x1bcc*,0x1bd0*,0x1bde*} | Steam {0x68*,0x310*,0x1bcc*,0x1bd0*,0x1bde*}
imm sets (* = in both, 4 common): GOG {0x4*,0x18*,0x1bc0*,0x1bcc*} | Steam {0x4*,0x18*,0x1bc0*,0x1bcc*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0090c120: 16-byte aligned: YES
  0090c120  55             PUSH EBP
  0090c121  8B EC          MOV EBP,ESP
  0090c123  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0090c120
string literals (0):
direct callees (7):
  0x0090C620  (unnamed)
  0x005AA2C0  (unnamed)
  0x006A20F0  (unnamed)
  0x00846F00  (unnamed)
  0x0090C120  (unnamed)
  0x0090C670  (unnamed)
  0x0090C230  CSWGuiFeatsCharGen::BuildChoosableFeatVector
struct offsets touched (top 5):
  +0x1BCC [dec 7116]  x2  via EAX,ECX
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x68 [dec 104]  x1  via EDX
  +0x310 [dec 784]  x1  via EAX
  +0x1BD0 [dec 7120]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0090c120 FUN_0090c120 size=271 ret=4 callers=3 callees=7

void __thiscall FUN_0090c120(int param_1,undefined2 param_2)

{
  bool bVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_10;
  
  iVar2 = FUN_0090c620(param_2);
  if (iVar2 != -1) {
    FUN_005aa2c0(iVar2);
    *(char *)(param_1 + 0x1bde) = *(char *)(param_1 + 0x1bde) + '\x01';
    bVar1 = true;
    for (local_10 = 0; local_10 < *(int *)(param_1 + 0x1bd0); local_10 = local_10 + 1) {
      iVar2 = param_1 + 0x1bcc;
      uVar3 = FUN_006a20f0(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_10 * 2));
      iVar2 = FUN_00846f00(uVar3,iVar2);
      if (iVar2 == 0) {
        FUN_0090c120(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_10 * 2));
        bVar1 = false;
        break;
      }
    }
    if (bVar1) {
      FUN_0090c670(0);
      FUN_0090c230();
    }
  }
  return;
}
```

### decoy: 0x0090bf90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0090bf90: 16-byte aligned: YES
  0090bf90  55             PUSH EBP
  0090bf91  8B EC          MOV EBP,ESP
  0090bf93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0090bf90
string literals (0):
direct callees (9):
  0x0090C620  (unnamed)
  0x005AA2C0  (unnamed)
  0x006A20F0  (unnamed)
  0x006D6FE0  (unnamed)
  0x00733C40  (unnamed)
  0x00733780  CExoString::~CExoString
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0090C230  CSWGuiFeatsCharGen::BuildChoosableFeatVector
struct offsets touched (top 3):
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD0 [dec 7120]  x1  via ECX
```

### Static facts (script output, report-only)
- size 271 B, RET 4
- callers 3: 0x0090b6b0 CSWGuiFeatsCharGen::OnRecommendedButton, 0x0090b890 CSWGuiFeatsCharGen::OnSlotClicked, 0x0090c120
- callees: 0x0090c620 (CALL), 0x005aa2c0 (CALL), 0x006a20f0 (CALL), 0x00846f00 (CALL), 0x0090c120 (CALL), 0x0090c670 (CALL), ...
- globals: 0x00a1b4d0 READ
- twins: none

## Card B17-5-07

GOG function: CSWGuiNameChargen::Destructor  gog 0x00604f00  callconv=(blank)  param_size_bytes=0  gog_size=317  gog_retn=0
Candidate Steam address: 0x00918f00 (size 317, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00919040:STR0/CALLEE0/SIG0.065/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00604ed0 CSWGuiNameChargen::scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00605040 CSWGuiNameChargen::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: yes
  0x005d7d00 CSWGuiEditbox::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x008115a0 CSWNameGen::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 21 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0x10*,0x60*,0x70*,0xd8*,...(+7)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0x10*,0x60*,0x70*,0xd8*,...(+7)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00918f00: 16-byte aligned: YES
  00918f00  55             PUSH EBP
  00918f01  8B EC          MOV EBP,ESP
  00918f03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00918f00
string literals (0):
direct callees (8):
  0x00942280  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0041A3D0  CSWGuiButton::Destructor
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x00421060  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00918f00 FUN_00918f00 size=317 ret=0 callers=1 callees=8

void __fastcall FUN_00918f00(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009829fd;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 7;
  FID_conflict__ctype [GHIDRA-AUTO-LABEL: unreliable]<char>(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8._0_1_ = 6;
  FUN_00733780();
  local_8._0_1_ = 5;
  FUN_0041a3d0();
  local_8._0_1_ = 4;
  FUN_0041a3d0();
  *(undefined ***)(param_1 + 0x4f0) = CSWGuiLabel::vftable;
  local_8._0_1_ = 9;
  FUN_00417290();
  local_8._0_1_ = 8;
  FUN_00414c60();
  local_8._0_1_ = 3;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x3a8) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xb;
  FUN_00417290();
  local_8._0_1_ = 10;
  FUN_00414c60();
  local_8._0_1_ = 2;
  FUN_00418640();
  local_8._0_1_ = 1;
  FUN_00421060();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0041a3d0();
  local_8 = 0xffffffff;
  FUN_0040e860();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00919040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00919040: 16-byte aligned: YES
  00919040  55             PUSH EBP
  00919041  8B EC          MOV EBP,ESP
  00919043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00919040
string literals (0):
direct callees (10):
  0x00733540  CExoString::Constructor
  0x004215E0  (unnamed)
  0x0073EEE0  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x00916600  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00919F10  _strlen
  0x00919300  (unnamed)
  0x00417DD0  (unnamed)
struct offsets touched (top 2):
  +0x6C [dec 108]  x2  via ECX
  +0x9D8 [dec 2520]  x2  via ECX
```

### Static facts (script output, report-only)
- size 317 B, RET 0
- callers 1: 0x00918ed0
- callees: 0x00942280 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0041a3d0 CSWGuiButton::Destructor (CALL), 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), ...
- twins: none

## Card B17-5-08

GOG function: CExoFileInternal::Read  gog 0x00610130  callconv=(blank)  param_size_bytes=8  gog_size=231  gog_retn=8
Candidate Steam address: 0x0073dc30 (size 231, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0073dd20:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006100a0 CExoFileInternal::GetSize -> 0x0073dba0  gate_class=REVIEW
- GOG-order neighbour after:  0x00610220 CExoFileInternal::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00927e9f unnamed -> Steam ?  in candidate callees: n/a
  0x00927ed3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092866d unnamed -> Steam ?  in candidate callees: n/a
  0x00928942 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x18*} | Steam {0x4*,0x8*,0x10*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0073dc30: 16-byte aligned: YES
  0073dc30  55             PUSH EBP
  0073dc31  8B EC          MOV EBP,ESP
  0073dc33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073dc30
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x0091F3EF  (unnamed)
  0x00921563  (unnamed)
  0x00734010  CExoString::CStr
  0x0092178F  (unnamed)
  0x0092152F  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073dc30 FUN_0073dc30 size=231 ret=8 callers=1 callees=8

size_t __thiscall FUN_0073dc30(int *param_1,undefined4 param_2,size_t param_3)

{
  void *_DstBuf [GHIDRA-AUTO-LABEL: unreliable];
  size_t sVar1;
  int iVar2;
  
  if (*param_1 == 0) {
    return 0;
  }
  _DstBuf [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(param_3 + 1);
  sVar1 = _fread [GHIDRA-AUTO-LABEL: unreliable](_DstBuf [GHIDRA-AUTO-LABEL: unreliable],1,param_3,(FILE *)*param_1);
  if (sVar1 != param_3) {
    iVar2 = _ferror [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    if (iVar2 != 0) {
      FUN_00734010();
      FUN_0092178f(*param_1);
      return 0;
    }
    iVar2 = _feof [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    if (iVar2 != 0) {
      FUN_00734010();
      FUN_0092178f(*param_1);
      return 0;
    }
  }
  *(undefined1 *)((int)_DstBuf [GHIDRA-AUTO-LABEL: unreliable] + sVar1) = 0;
  FUN_007338d0(_DstBuf [GHIDRA-AUTO-LABEL: unreliable]);
  FUN_009196fd(_DstBuf [GHIDRA-AUTO-LABEL: unreliable]);
  return sVar1;
}
```

### decoy: 0x0073dd20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073dd20: 16-byte aligned: YES
  0073dd20  55             PUSH EBP
  0073dd21  8B EC          MOV EBP,ESP
  0073dd23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073dd20
string literals (0):
direct callees (5):
  0x0091F3EF  (unnamed)
  0x00921563  (unnamed)
  0x00734010  CExoString::CStr
  0x0092178F  (unnamed)
  0x0092152F  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 231 B, RET 8
- callers 1: 0x007399c0
- callees: 0x00919723 operator new (CALL), 0x0091f3ef (CALL), 0x00921563 (CALL), 0x00734010 (CALL), 0x0092178f (CALL), 0x0092152f (CALL), ...
- twins: none

## Card B17-5-09

GOG function: CSWGuiExamine::HandleInputEvent  gog 0x005b3580  callconv=(blank)  param_size_bytes=8  gog_size=117  gog_retn=8
Candidate Steam address: 0x008bf0e0 (size 372, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008bf280:STR0/CALLEE0/SIG0.121/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005b3550 CSWGuiTutorialBox::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005b3610 CSWGuiCreateItemSubMenu::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004db310 CGuiInGame::HideExamineBox -> Steam 0x007cac10  in candidate callees: no
  0x0052ced0 CSWGuiMessageBox::HandleInputEvent -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x1c*} | Steam {0x4*,0x1c*,0x4a0,0x4a8}
imm sets (* = in both, 1 common): GOG {0x7,0x8*,0xc,0x27} | Steam {0x8*,0x16,0x18,0x28,0xffffff9d}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x008bf0e0: 16-byte aligned: YES
  008bf0e0  55             PUSH EBP
  008bf0e1  8B EC          MOV EBP,ESP
  008bf0e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008bf0e0
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CAB60  CGuiInGame::HideSubItemCreateMenu
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 5):
  +0x4A0 [dec 1184]  x14  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x4A8 [dec 1192]  x3  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8BF264 [dec 9171556]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008bf0e0 FUN_008bf0e0 size=372 ret=8 callers=0 callees=4

void __thiscall FUN_008bf0e0(int param_1,int param_2,int param_3)

{
  if (param_3 != 0) {
    switch(param_2) {
    case 0x28:
    case 0x2e:
      FUN_0073f750(param_2 + -0x28,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
      FUN_007cab60();
      break;
    case 0x31:
    case 0x3d:
      if (*(int *)(param_1 + 0x4a0) == -99) {
        *(undefined4 *)(param_1 + 0x4a0) = 0xffffffff;
      }
      if (*(int *)(param_1 + 0x4a0) < 1) {
        if (*(int *)(param_1 + 0x4a0) == 0) {
          *(int *)(param_1 + 0x4a0) = *(int *)(param_1 + 0x4a8) + -1;
        }
      }
      else {
        *(int *)(param_1 + 0x4a0) = *(int *)(param_1 + 0x4a0) + -1;
      }
      if (*(int *)(param_1 + 0x1c) != 0) {
        FUN_004122a0(1);
      }
      break;
    case 0x32:
    case 0x3e:
      if (*(int *)(param_1 + 0x4a0) == -99) {
        *(undefined4 *)(param_1 + 0x4a0) = 0xffffffff;
      }
      if (*(int *)(param_1 + 0x4a0) < *(int *)(param_1 + 0x4a8)) {
        *(int *)(param_1 + 0x4a0) = *(int *)(param_1 + 0x4a0) + 1;
      }
      else if (*(int *)(param_1 + 0x4a0) == *(int *)(param_1 + 0x4a8)) {
        *(undefined4 *)(param_1 + 0x4a0) = 0;
      }
      if (*(int *)(param_1 + 0x1c) != 0) {
        FUN_004122a0(1);
      }
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

### decoy: 0x008bf280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008bf280: 16-byte aligned: YES
  008bf280  55             PUSH EBP
  008bf281  8B EC          MOV EBP,ESP
  008bf283  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008bf280
string literals (4):
  0x009A8284  at 008bf441  'BAD_BLUEPRINT'
  0x0099364C  at 008bf45c  'TemplateResRef'
  0x00997578  at 008bf48a  'BaseItem'
  0x0099754C  at 008bf507  'LocalizedName'
direct callees (38):
  0x007125B0  CExoResMan::GetResOfType
  0x008289A0  (unnamed)
  0x0073AE20  (unnamed)
  0x00734010  CExoString::CStr
  0x00710840  (unnamed)
  0x00712970  (unnamed)
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0091D0F0  (unnamed)
  0x00712B60  CExoResMan::SetResObject
  0x007111E0  CRes::Demand
  0x007107F0  (unnamed)
  0x007189D0  CResGFF::GetTopLevelStruct
  0x00718E40  CResGFF::ReadFieldCResRef
  0x00710CA0  (unnamed)
  0x00718C80  CResGFF::ReadFieldINT
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00735240  (unnamed)
  0x007190E0  CResGFF::ReadFieldCExoLocString
struct offsets touched (top 8):
  +0x2CC [dec 716]  x2  via ECX,EDX
  +0x4A4 [dec 1188]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x344 [dec 836]  x1  via EAX
  +0x4A0 [dec 1184]  x1  via EDX
  +0x4BC [dec 1212]  x1  via EDX
  +0x4D0 [dec 1232]  x1  via EAX
```

### Static facts (script output, report-only)
- size 372 B, RET 8
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cab60 CGuiInGame::HideSubItemCreateMenu (CALL), 0x0040eaa0 CSWGuiPanel::HandleInputEvent (CALL)
- twins: none

## Card B17-5-10

GOG function: Global::PartOutside_2  gog 0x0086bb10  callconv=(blank)  param_size_bytes=8  gog_size=162  gog_retn=0
Candidate Steam address: 0x00494170 (size 138, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00494200:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ba80 Global::PartOutside  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086bbc0 Global::PartOutside_3  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080be00 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0xc,0xd0} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x24,0x41*} | Steam {0x4*,0x8,0xc*,0x41*,0x4c,0x58}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00494170: 16-byte aligned: YES
  00494170  55             PUSH EBP
  00494171  8B EC          MOV EBP,ESP
  00494173  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00494170
string literals (0):
direct callees (1):
  0x00513FA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00494170 FUN_00494170 size=138 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00494170(int param_1,int *param_2)

{
  float10 fVar1;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (param_2[1 = byte +0x4] <= local_8) {
      return 0;
    }
    fVar1 = (float10)FUN_00513fa0(param_1 + 0x4c,param_1 + 0x58,
                                  ((local_8 + _DAT_00a733c0) % param_2[1 = byte +0x4]) * 0x10 + *param_2);
    if ((float10)_DAT_00986b38 < fVar1) break;
    local_8 = local_8 + 1;
  }
  _DAT_00a733c0 = (local_8 + _DAT_00a733c0) % param_2[1 = byte +0x4];
  return 1;
}
```

### decoy: 0x00494200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00494200: 16-byte aligned: YES
  00494200  55             PUSH EBP
  00494201  8B EC          MOV EBP,ESP
  00494203  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00494200
string literals (0):
direct callees (1):
  0x00514050  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via EAX
  +0xD0 [dec 208]  x1  via EAX
```

### Static facts (script output, report-only)
- size 138 B, RET 0
- callers 1: 0x0049ae30
- callees: 0x00513fa0 (CALL)
- twins: none

## Card B17-5-11

GOG function: Global::AnimateHierarchy  gog 0x008fa000  callconv=(blank)  param_size_bytes=21  gog_size=614  gog_retn=0
Candidate Steam address: 0x004dfba0 (size 815, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.16 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004dfed0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008f9cd0 Global::ProceduralAnimateHierarchy -> 0x004dfba0  gate_class=REVIEW
- GOG-order neighbour after:  0x008fa270 Part::Animate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00845e70 MdlNode::ControlDispatch -> Steam ?  in candidate callees: n/a
  0x0084c400 Gob::IsInCutscene -> Steam 0x00459e20  in candidate callees: no
  0x008fa000 Global::AnimateHierarchy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2,0x4*,0x8*,0x16,0x28,0x2c*,0x30*,0x34*,0x40,0x48,0x58,0xa8,0x164} | Steam {0x4*,0x8*,0x2c*,0x30*,0x34*,0x78,0x90,0xac,0x138,0x13c,0x140,0x144,0x148,0x14c,...(+3)}
imm sets (* = in both, 1 common): GOG {0x5*,0x18,0x28,0x41,0x64} | Steam {0x4,0x5*,0x10,0x44}

### abi
```
GOG param bytes: 21; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(21/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 21 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x004dfba0: 16-byte aligned: YES
  004dfba0  55             PUSH EBP
  004dfba1  8B EC          MOV EBP,ESP
  004dfba3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dfba0
string literals (0):
direct callees (7):
  0x004545A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00432EE0  (unnamed)
  0x0091EB8B  (unnamed)
  0x00452830  (unnamed)
  0x00432F10  (unnamed)
  0x004DFBA0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x78 [dec 120]  x4  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x144 [dec 324]  x2  via EAX,ECX
  +0x148 [dec 328]  x2  via EAX
  +0x160 [dec 352]  x2  via EAX,ECX
  +0x180 [dec 384]  x2  via EAX,ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004dfba0 FUN_004dfba0 size=815 ret=0 callers=2 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004dfba0(int *param_1)

{
  float *pfVar1;
  float *pfVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  float fVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int local_8;
  
  iVar6 = (**(code **)(*param_1 + 0x2c))();
  if ((iVar6 != 0) && (*(int *)(iVar6 + 0x78) != -1)) {
    iVar7 = FUN_004545a0();
    if ((*(int *)(iVar7 + 0x138) != 0) &&
       ((*(int *)(iVar7 + 0x160) != -1 && (*(int *)(iVar6 + 0x78) != -1)))) {
      iVar8 = FUN_0091c860();
      fVar5 = *(float *)(iVar7 + 0x144) / (float)_DAT_00987490;
      iVar9 = FUN_00432ee0(*(undefined4 *)(iVar6 + 0x78),0,0,0x10);
      for (local_8 = 0; local_8 < *(short *)(iVar7 + 0x180); local_8 = local_8 + 1) {
        pfVar1 = (float *)(iVar9 + local_8 * 8);
        pfVar2 = (float *)(*(int *)(iVar6 + 0x90) + local_8 * 8);
        *pfVar2 = *pfVar2 + *(float *)(iVar7 + 0x13c);
        pfVar2[1 = byte +0x4] = pfVar2[1 = byte +0x4] + *(float *)(iVar7 + 0x140);
        if (iVar8 != 0) {
          iVar10 = _rand [GHIDRA-AUTO-LABEL: unreliable]();
          *pfVar1 = *(float *)(iVar7 + 0x148) *
                    ((float)(iVar10 % iVar8) * (float)_DAT_00986b70 - fVar5) + *pfVar1;
          if (*pfVar1 - *pfVar2 <= fVar5) {
            if (*pfVar1 - *pfVar2 < -fVar5) {
              *pfVar1 = *pfVar2 - fVar5;
            }
          }
          else {
            *pfVar1 = *pfVar2 + fVar5;
          }
          iVar10 = _rand [GHIDRA-AUTO-LABEL: unreliable]();
          pfVar1[1 = byte +0x4] = *(float *)(iVar7 + 0x148) *
                      ((float)(iVar10 % iVar8) * (float)_DAT_00986b70 - fVar5) + pfVar1[1 = byte +0x4];
          if (pfVar1[1 = byte +0x4] - pfVar2[1 = byte +0x4] <= fVar5) {
            if (pfVar1[1 = byte +0x4] - pfVar2[1 = byte +0x4] < -fVar5) {
              pfVar1[1 = byte +0x4] = pfVar2[1 = byte +0x4] - fVar5;
            }
          }
          else {
            pfVar1[1 = byte +0x4] = pfVar2[1 = byte +0x4] + fVar5;
          }
        }
      }
      iVar8 = FUN_00452830();
      iVar8 = FUN_00432ee0(*(undefined4 *)(iVar8 + 0xac),0,0,1);
      for (local_8 = 0; local_8 < *(short *)(iVar7 + 0x180); local_8 = local_8 + 1) {
        puVar3 = (undefined4 *)(iVar9 + local_8 * 8);
        uVar4 = puVar3[1 = byte +0x4];
        iVar11 = iVar8 + *(int *)(iVar7 + 0x19c) + local_8 * *(int *)(iVar7 + 0x14c);
        iVar10 = *(int *)(iVar7 + 0x160);
        *(undefined4 *)(iVar11 + iVar10) = *puVar3;
        *(undefined4 *)(iVar11 + 4 + iVar10) = uVar4;
      }
      iVar7 = FUN_00452830();
      FUN_00432f10(*(undefined4 *)(iVar7 + 0xac));
      FUN_00432f10(*(undefined4 *)(iVar6 + 0x78));
    }
  }
  for (local_8 = 0; local_8 < param_1[0xd = byte +0x34]; local_8 = local_8 + 1) {
    FUN_004dfba0(*(undefined4 *)(param_1[0xc = byte +0x30] + local_8 * 4));
  }
  return;
}
```

### decoy: 0x004dfed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dfed0: 16-byte aligned: YES
  004dfed0  55             PUSH EBP
  004dfed1  8B EC          MOV EBP,ESP
  004dfed3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004dfed0
string literals (0):
direct callees (4):
  0x00452830  (unnamed)
  0x00453860  MdlNode::ControlDispatch
  0x00459E20  Gob::IsInCutscene
  0x004DFED0  (unnamed)
struct offsets touched (top 10):
  +0x30 [dec 48]  x5  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX
  +0x28 [dec 40]  x2  via ECX,EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x58 [dec 88]  x2  via ECX,EDX
  +0x164 [dec 356]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x16 [dec 22]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
```

### Static facts (script output, report-only)
- size 815 B, RET 0
- callers 2: 0x004dfba0, 0x004e1d40
- callees: 0x004545a0 (CALL), 0x0091c860 (CALL), 0x00432ee0 (CALL), 0x0091eb8b (CALL), 0x00452830 (CALL), 0x00432f10 (CALL), ...
- indirect: 0x004dfbc1 slot 0x2c
- twins: none

## Card B17-5-12

GOG function: CSWGuiFeatsCharGen::RemoveChosenFeat  gog 0x005fae20  callconv=(blank)  param_size_bytes=2  gog_size=395  gog_retn=4
Candidate Steam address: 0x0090bf90 (size 395, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x005fafb0:0.57>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0090c120:STR0/CALLEE0/SIG0.6/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005fabe0 CSWGuiFeatsCharGen::BuildButtons  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005fafb0 CSWGuiFeatsCharGen::AddChosenFeat -> 0x0090c120  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x004952b0 unnamed -> Steam ?  in candidate callees: n/a
  0x004953b0 unnamed -> Steam ?  in candidate callees: n/a
  0x004f33a0 CSWRace::GetConverNameLowerText -> Steam ?  in candidate callees: n/a
  0x0050e730 CSWRules::GetFeat -> Steam ?  in candidate callees: n/a
  0x0058a840 unnamed -> Steam ?  in candidate callees: n/a
  0x005fb0c0 CSWGuiFeatsCharGen::BuildAvailableList -> Steam ?  in candidate callees: n/a
  0x005fb4b0 unnamed -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1bcc*,0x1bd0*,0x1bde*} | Steam {0x1bcc*,0x1bd0*,0x1bde*}
imm sets (* = in both, 4 common): GOG {0x4*,0x30*,0x1bc0*,0x1bcc*} | Steam {0x4*,0x30*,0x1bc0*,0x1bcc*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0090bf90: 16-byte aligned: YES
  0090bf90  55             PUSH EBP
  0090bf91  8B EC          MOV EBP,ESP
  0090bf93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0090bf90
string literals (0):
direct callees (9):
  0x0090C620  (unnamed)
  0x005AA2C0  (unnamed)
  0x006A20F0  (unnamed)
  0x006D6FE0  (unnamed)
  0x00733C40  (unnamed)
  0x00733780  CExoString::~CExoString
  0x005934C0  (unnamed)
  0x00593450  CExoArrayList<uint16>::Add
  0x0090C230  CSWGuiFeatsCharGen::BuildChoosableFeatVector
struct offsets touched (top 3):
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x1BCC [dec 7116]  x1  via ECX
  +0x1BD0 [dec 7120]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0090bf90 FUN_0090bf90 size=395 ret=4 callers=2 callees=9

void __thiscall FUN_0090bf90(int param_1,undefined2 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 *puVar3;
  undefined1 local_2c [8];
  int local_24;
  int local_20;
  int local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00981180;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_20 = FUN_0090c620(param_2);
  if (local_20 != -1) {
    FUN_005aa2c0(local_20);
    local_1c = 0;
    puVar3 = local_18;
    FUN_006a20f0(param_2);
    FUN_006d6fe0(puVar3);
    local_8 = 0;
    local_20 = 0;
    while ((local_1c == 0 && (local_20 < *(int *)(param_1 + 0x1bd0)))) {
      puVar3 = local_2c;
      FUN_006a20f0(*(undefined2 *)(*(int *)(param_1 + 0x1bcc) + local_20 * 2));
      uVar2 = FUN_006d6fe0(puVar3);
      local_8._0_1_ = 1;
      local_24 = FUN_00733c40(local_18);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780(uVar1,uVar2);
      if (local_24 == 0) {
        local_20 = local_20 + 1;
      }
      else {
        local_1c = 1;
      }
    }
    *(char *)(param_1 + 0x1bde) = *(char *)(param_1 + 0x1bde) + -1;
    if (local_1c == 0) {
      FUN_00593450(param_2);
    }
    else {
      FUN_005934c0(param_2,local_20);
    }
    FUN_0090c230();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0090c120 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0090c120: 16-byte aligned: YES
  0090c120  55             PUSH EBP
  0090c121  8B EC          MOV EBP,ESP
  0090c123  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0090c120
string literals (0):
direct callees (7):
  0x0090C620  (unnamed)
  0x005AA2C0  (unnamed)
  0x006A20F0  (unnamed)
  0x00846F00  (unnamed)
  0x0090C120  (unnamed)
  0x0090C670  (unnamed)
  0x0090C230  CSWGuiFeatsCharGen::BuildChoosableFeatVector
struct offsets touched (top 5):
  +0x1BCC [dec 7116]  x2  via EAX,ECX
  +0x1BDE [dec 7134]  x2  via ECX,EDX
  +0x68 [dec 104]  x1  via EDX
  +0x310 [dec 784]  x1  via EAX
  +0x1BD0 [dec 7120]  x1  via EAX
```

### Static facts (script output, report-only)
- size 395 B, RET 4
- callers 2: 0x0090b6b0 CSWGuiFeatsCharGen::OnRecommendedButton, 0x0090b890 CSWGuiFeatsCharGen::OnSlotClicked
- callees: 0x0090c620 (CALL), 0x005aa2c0 (CALL), 0x006a20f0 (CALL), 0x006d6fe0 (CALL), 0x00733c40 (CALL), 0x00733780 CExoString::Destructor (CALL), ...
- globals: 0x00a1b4d0 READ
- twins: none

## Card B17-5-13

GOG function: CExoFileInternal::GetSize  gog 0x006100a0  callconv=(blank)  param_size_bytes=0  gog_size=133  gog_retn=0
Candidate Steam address: 0x0073dba0 (size 133, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0073db70:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00610070 CExoFileInternal::GetOffset  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00610130 CExoFileInternal::Read -> 0x0073dc30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00922455 unnamed -> Steam ?  in candidate callees: n/a
  0x009227a7 unnamed -> Steam ?  in candidate callees: n/a
  0x00928827 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0xc*} | Steam {0x2*,0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073dba0: 16-byte aligned: YES
  0073dba0  55             PUSH EBP
  0073dba1  8B EC          MOV EBP,ESP
  0073dba3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073dba0
string literals (0):
direct callees (3):
  0x0091AB77  (unnamed)
  0x009205FC  (unnamed)
  0x0091A825  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073dba0 FUN_0073dba0 size=133 ret=0 callers=1 callees=3

long __fastcall FUN_0073dba0(int *param_1)

{
  long _Offset [GHIDRA-AUTO-LABEL: unreliable];
  long lVar1;
  
  if (*param_1 == 0) {
    lVar1 = -1;
  }
  else {
    _fflush [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    _Offset [GHIDRA-AUTO-LABEL: unreliable] = _ftell [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    if (_Offset [GHIDRA-AUTO-LABEL: unreliable] < 0) {
      lVar1 = -1;
    }
    else {
      _fseek [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1,0,2);
      lVar1 = _ftell [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
      _fseek [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1,_Offset [GHIDRA-AUTO-LABEL: unreliable],0);
    }
  }
  return lVar1;
}
```

### decoy: 0x0073db70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073db70: 16-byte aligned: YES
  0073db70  55             PUSH EBP
  0073db71  8B EC          MOV EBP,ESP
  0073db73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073db70
string literals (0):
direct callees (1):
  0x009205FC  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 133 B, RET 0
- callers 1: 0x00739970
- callees: 0x0091ab77 (CALL), 0x009205fc (CALL), 0x0091a825 (CALL)
- twins: none

## Card B17-5-14

GOG function: CSWGuiCustomPanel::OnSelectFeatsButton  gog 0x005f42f0  callconv=(blank)  param_size_bytes=4  gog_size=203  gog_retn=4
Candidate Steam address: 0x00914e70 (size 203, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00915010:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f4220 CSWGuiCustomPanel::OnSelectNameButton  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f43c0 CSWGuiCustomPanel::OnSelectSkillsButton  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005f8c90 CSWGuiFeatsCharGen::Constructor -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x1c*,0x50*,0x68*,0x6c*,0x2120*} | Steam {0x1c*,0x50*,0x68*,0x6c*,0x2120*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x18*,0x1c08*} | Steam {0x3*,0x4*,0x18*,0x1c08*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00914e70: 16-byte aligned: YES
  00914e70  55             PUSH EBP
  00914e71  8B EC          MOV EBP,ESP
  00914e73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00914e70
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x00909E00  CSWGuiFeatsCharGen::Constructor
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00914e70 FUN_00914e70 size=203 ret=4 callers=0 callees=3

void __thiscall FUN_00914e70(int param_1,int param_2)

{
  int iVar1;
  int local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009821bb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((2 < *(int *)(param_1 + 0x2120)) && (*(int *)(param_2 + 0x50) != 0)) {
    iVar1 = FUN_00919723(0x1c08,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_28 = 0;
    }
    else {
      local_28 = FUN_00909e00(*(undefined4 *)(param_1 + 0x1c),*(undefined4 *)(param_1 + 0x68),0,0);
    }
    local_8 = 0xffffffff;
    *(int *)(local_28 + 0x6c) = param_1;
    FUN_00410530(local_28,3,1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00915010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00915010: 16-byte aligned: YES
  00915010  55             PUSH EBP
  00915011  8B EC          MOV EBP,ESP
  00915013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00915010
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x00910490  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 5):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x2120 [dec 8480]  x1  via EAX
```

### Static facts (script output, report-only)
- size 203 B, RET 4
- callees: 0x00919723 operator new (CALL), 0x00909e00 (CALL), 0x00410530 CSWGuiManager::AddPanel (CALL)
- twins: none

## Card B17-5-15

GOG function: CSWGuiSkillsCharGen::Destructor_2  gog 0x005fd330  callconv=(blank)  param_size_bytes=0  gog_size=974  gog_retn=0
Candidate Steam address: 0x0090e440 (size 974, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0090e810:STR0/CALLEE0/SIG0.016/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005fd300 CSWGuiSkillsCharGen::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005fd700 CSWGuiSkillsCharGen::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: yes
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0091b1a0 CSWGuiListBox::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0091b2a0 CSWGuiListBox::ClearItems -> Steam ?  in candidate callees: n/a
  0x009217f0 Global::`eh_vector_destructor_iterator' -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 60 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+46)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+46)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0090e440: 16-byte aligned: YES
  0090e440  55             PUSH EBP
  0090e441  8B EC          MOV EBP,ESP
  0090e443  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0090e440
string literals (0):
direct callees (8):
  0x0041D290  CSWGuiListBox::ClearItems
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0041A3D0  CSWGuiButton::Destructor
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0041D190  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0090e440 FUN_0090e440 size=974 ret=0 callers=1 callees=8

void __fastcall FUN_0090e440(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009817f5;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiSkillsCharGen::vftable;
  local_8 = 0x13;
  FUN_0041d290(uVar1);
  local_8._0_1_ = 0x12;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x132a,8,8,FUN_00733780);
  local_8._0_1_ = 0x11;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x1286,0x148,2,CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._0_1_ = 0x10;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xee6,0x1d0,8,FUN_0041a3d0);
  local_8._0_1_ = 0xf;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xb46,0x1d0,8,FUN_0041a3d0);
  local_8._0_1_ = 0xe;
  FUN_0041a3d0();
  local_8._0_1_ = 0xd;
  FUN_0041a3d0();
  local_8._0_1_ = 0xc;
  FUN_0041a3d0();
  local_8._0_1_ = 0xb;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x64a,0x1d0,8,FUN_0041a3d0);
  local_8._0_1_ = 10;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            (param_1 + 0x3ba,0x148,8,CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  param_1[0x368 = byte +0xda0] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x15;
  FUN_00417290();
  local_8._0_1_ = 0x14;
  FUN_00414c60();
  local_8._0_1_ = 9;
  FUN_00418640();
  param_1[0x316 = byte +0xc58] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x17;
  FUN_00417290();
  local_8._0_1_ = 0x16;
  FUN_00414c60();
  local_8._0_1_ = 8;
  FUN_00418640();
  param_1[0x2c4 = byte +0xb10] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x19;
  FUN_00417290();
  local_8._0_1_ = 0x18;
  FUN_00414c60();
  local_8._0_1_ = 7;
  FUN_00418640();
  param_1[0x272 = byte +0x9c8] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x1b;
  FUN_00417290();
  local_8._0_1_ = 0x1a;
  FUN_00414c60();
  local_8._0_1_ = 6;
  FUN_00418640();
  param_1[0x220 = byte +0x880] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x1d;
  FUN_00417290();
  local_8._0_1_ = 0x1c;
  FUN_00414c60();
  local_8._0_1_ = 5;
  FUN_00418640();
  param_1[0x1ce = byte +0x738] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x1f;
  FUN_00417290();
  local_8._0_1_ = 0x1e;
  FUN_00414c60();
  local_8._0_1_ = 4;
  FUN_00418640();
  param_1[0x17c = byte +0x5f0] = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x21;
/* ... 26 more lines truncated by --max-lines */
```

### decoy: 0x0090e810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0090e810: 16-byte aligned: YES
  0090e810  55             PUSH EBP
  0090e811  8B EC          MOV EBP,ESP
  0090e813  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0090e810
string literals (0):
direct callees (2):
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x00740A20  CClientExoApp::ShowTutorialWindow
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 974 B, RET 0
- callers 1: 0x0090e410
- callees: 0x0041d290 (CALL), 0x00919bcc (CALL), 0x0041a3d0 CSWGuiButton::Destructor (CALL), 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), ...
- twins: none

## Card B17-5-16

GOG function: CSWGuiSwoopUpgrade::SetDescription  gog 0x005cf920  callconv=(blank)  param_size_bytes=0  gog_size=93  gog_retn=4
Candidate Steam address: 0x008df550 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008df5b0:STR0/CALLEE0/SIG0.211/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005cf840 CSWGuiSwoopUpgrade::HandleInputEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005cf980 CSWGuiSwoopUpgrade::OnAssemble -> 0x008df5b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004de610 unnamed -> Steam ?  in candidate callees: n/a
  0x005ccf50 CSWGuiSwoopUpgrade::OnCloseSwoopUpgrade -> Steam ?  in candidate callees: n/a
  0x005cfa50 CSWGuiSwoopUpgrade::OnUpgradeSelected -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x1c*,0x4340*} | Steam {0x4*,0x1c*,0x4340*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008df550: 16-byte aligned: YES
  008df550  55             PUSH EBP
  008df551  8B EC          MOV EBP,ESP
  008df553  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008df550
string literals (0):
direct callees (5):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008DCB80  (unnamed)
  0x008DF680  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CDF10  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
  +0x4340 [dec 17216]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008df550 FUN_008df550 size=93 ret=4 callers=0 callees=5

void __fastcall FUN_008df550(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = *(undefined4 *)(param_1 + 0x1c);
  FUN_004122a0(0);
  if ((*(uint *)(param_1 + 0x4340) & 1) == 0) {
    FUN_008df680();
    FUN_0073f750(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar1);
    FUN_007cdf10();
  }
  else {
    FUN_008dcb80();
  }
  return;
}
```

### decoy: 0x008df5b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008df5b0: 16-byte aligned: YES
  008df5b0  55             PUSH EBP
  008df5b1  8B EC          MOV EBP,ESP
  008df5b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008df5b0
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CDF10  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via ECX
```

### Static facts (script output, report-only)
- size 93 B, RET 4
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x008dcb80 (CALL), 0x008df680 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cdf10 (CALL)
- twins: none

## Card B17-5-17

GOG function: CSWCModule::TurnCamera  gog 0x0046b0c0  callconv=(blank)  param_size_bytes=8  gog_size=885  gog_retn=8
Candidate Steam address: 0x007f4220 (size 643, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.74 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f3ea0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b000 CSWCModule::AcclTiltCamera  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046b440 CSWCModule::AcclTurnCamera -> 0x007f4220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: no
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x00574940 CSWBehaviorCameraCombat::SetTurn -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam 0x00543f10  in candidate callees: no
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x0080d740 Global::Pitch -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x88,0x98*,0x100*,0x10c*,0x460*} | Steam {0x4*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x98*,0x100*,0x10c*,0x460*}
imm sets (* = in both, 10 common): GOG {0x3*,0x4,0x5*,0x7*,0x8*,0xc,0x10,0x30,0x41*,0x44*,0x9c,0x106a*,0x106e*,0x1070*,...(+1)} | Steam {0x3*,0x5*,0x7*,0x8*,0x41*,0x44*,0x48,0x106a*,0x106e*,0x1070*,0x1071*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f4220: 16-byte aligned: YES
  007f4220  55             PUSH EBP
  007f4221  8B EC          MOV EBP,ESP
  007f4223  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f4220
string literals (0):
direct callees (3):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f4220 FUN_007f4220 size=643 ret=8 callers=2 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f4220(int param_1,float param_2)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = FUN_00740860();
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0xc) == 3) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
      if (iVar1 != 0) {
        *(undefined4 *)(iVar1 + 0x100) = 0;
        *(float *)(iVar1 + 0x10c) = param_2;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 5) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
      if ((iVar1 != 0) && (iVar1 = FUN_0073f450(), iVar1 != 0)) {
        *(float *)(iVar1 + 0x460) = -param_2;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 7) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1071);
      if (iVar1 != 0) {
        *(float *)(iVar1 + 0x14) = -param_2 * (float)_DAT_00991358;
      }
    }
    else {
      if (((*(int *)(param_1 + 0x40) != 0) &&
          (piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0xffffffff),
          piVar2 != (int *)0x0)) && (iVar1 = (**(code **)(*piVar2 + 0x1c))(), iVar1 != 0)) {
        *(float *)(iVar1 + 0x40) = *(float *)(iVar1 + 0x40) + param_2;
        *(undefined4 *)(param_1 + 0x98) = *(undefined4 *)(iVar1 + 0x40);
      }
      if (param_2 != (float)_DAT_00986b38) {
        *(undefined4 *)(param_1 + 0x100) = 1;
        *(uint *)(param_1 + 0x80) = *(uint *)(param_1 + 0x80) | 1;
      }
    }
  }
  else {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x1070);
    if (iVar1 != 0) {
      FUN_008e5480(0);
      *(float *)(iVar1 + 0x2c) = -param_2;
    }
  }
  return;
}
```

### decoy: 0x007f3ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3ea0: 16-byte aligned: YES
  007f3ea0  55             PUSH EBP
  007f3ea1  8B EC          MOV EBP,ESP
  007f3ea3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f3ea0
string literals (0):
direct callees (11):
  0x00740860  CClientExoApp::GetCombatMode
  0x008E5480  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x00515E60  (unnamed)
  0x00515A70  (unnamed)
  0x00515990  (unnamed)
  0x005158B0  (unnamed)
  0x00515620  (unnamed)
  0x00463120  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 10):
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x80 [dec 128]  x7  via EAX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x100 [dec 256]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via ECX
```

### Static facts (script output, report-only)
- size 643 B, RET 8
- callers 2: 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x0078cea0
- callees: 0x00740860 CClientExoApp::GetCombatMode (CALL), 0x008e5480 (CALL), 0x0073f450 CClientExoApp::GetPlayerCreature (CALL)
- indirect: 0x007f4259 slot 0x80, 0x007f42a6 slot 0x80, 0x007f42f3 slot 0x80, 0x007f43ea slot 0x80, 0x007f442c slot 0x80, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-5-18

GOG function: CSWGuiSwoopUpgrade::OnAssemble  gog 0x005cf980  callconv=(blank)  param_size_bytes=4  gog_size=198  gog_retn=4
Candidate Steam address: 0x008df5b0 (size 198, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008df550:STR0/CALLEE0/SIG0.211/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005cf920 CSWGuiSwoopUpgrade::SetDescription -> 0x008df550  gate_class=REVIEW
- GOG-order neighbour after:  0x005cfa50 CSWGuiSwoopUpgrade::OnUpgradeSelected  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004de610 unnamed -> Steam ?  in candidate callees: n/a
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x1c*,0x4c*,0x50*,0x58*,0x17c*,0x43cc*} | Steam {0x4*,0x8*,0x1c*,0x4c*,0x50*,0x58*,0x17c*,0x43cc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x6*,0x24*,0x2d*} | Steam {0x4*,0x6*,0x24*,0x2d*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008df5b0: 16-byte aligned: YES
  008df5b0  55             PUSH EBP
  008df5b1  8B EC          MOV EBP,ESP
  008df5b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008df5b0
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CDF10  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008df5b0 FUN_008df5b0 size=198 ret=4 callers=0 callees=4

void __thiscall FUN_008df5b0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 local_8;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    if (*(int *)(param_2 + 0x4c) == 0x2d) {
      FUN_004122a0(*(undefined1 *)(param_2 + 0x58));
    }
    for (local_8 = 0; local_8 < 6; local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 0x43cc + local_8 * 4) != 0) {
        uVar1 = *(undefined4 *)(param_1 + 0x43cc + local_8 * 4);
        iVar2 = FUN_0051c8d0();
        *(undefined4 *)(iVar2 + 0x17c + local_8 * 4) = uVar1;
      }
    }
    FUN_0073f750(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
    FUN_007cdf10();
  }
  return;
}
```

### decoy: 0x008df550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008df550: 16-byte aligned: YES
  008df550  55             PUSH EBP
  008df551  8B EC          MOV EBP,ESP
  008df553  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008df550
string literals (0):
direct callees (5):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008DCB80  (unnamed)
  0x008DF680  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CDF10  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x1C [dec 28]  x1  via EAX
  +0x4340 [dec 17216]  x1  via EDX
```

### Static facts (script output, report-only)
- size 198 B, RET 4
- callees: 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x0051c8d0 (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007cdf10 (CALL)
- twins: none

## Card B17-5-19

GOG function: Global::PartOutside_2  gog 0x0086bb10  callconv=(blank)  param_size_bytes=8  gog_size=162  gog_retn=0
Candidate Steam address: 0x004942b0 (size 125, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00494200:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ba80 Global::PartOutside  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086bbc0 Global::PartOutside_3  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080be00 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0xc,0xd0} | Steam {0x4*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x24,0x41*} | Steam {0x4*,0x8,0xc*,0x41*,0x54,0x60}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x004942b0: 16-byte aligned: YES
  004942b0  55             PUSH EBP
  004942b1  8B EC          MOV EBP,ESP
  004942b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004942b0
string literals (0):
direct callees (1):
  0x00513FA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004942b0 FUN_004942b0 size=125 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004942b0(int param_1,int *param_2)

{
  float10 fVar1;
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (param_2[1 = byte +0x4] <= local_8) {
      return 0;
    }
    fVar1 = (float10)FUN_00513fa0(param_1 + 0x54,param_1 + 0x60,
                                  ((local_8 + _DAT_00a733c4) % param_2[1 = byte +0x4]) * 0x10 + *param_2);
    if ((float10)_DAT_00986b38 < fVar1) break;
    local_8 = local_8 + 1;
  }
  _DAT_00a733c4 = local_8;
  return 1;
}
```

### decoy: 0x00494200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00494200: 16-byte aligned: YES
  00494200  55             PUSH EBP
  00494201  8B EC          MOV EBP,ESP
  00494203  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00494200
string literals (0):
direct callees (1):
  0x00514050  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via EAX
  +0xD0 [dec 208]  x1  via EAX
```

### Static facts (script output, report-only)
- size 125 B, RET 0
- callers 1: 0x00499eb0
- callees: 0x00513fa0 (CALL)
- twins: none

## Card B17-5-20

GOG function: CSWGuiMainInterface::Quicksave  gog 0x0044ccb0  callconv=(blank)  param_size_bytes=0  gog_size=148  gog_retn=0
Candidate Steam address: 0x007a0630 (size 126, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoAppInternal::BusyWaitMuteSound | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a06b0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044cc30 CClientExoAppInternal::BusyWaitMuteSound -> 0x007a0630  trusted
- GOG-order neighbour after:  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> 0x007a0750  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cbb0 CClientExoApp::SendSaveGameRequest -> Steam 0x0073f6f0  in candidate callees: no
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: no
GOG strings (1): 'QUICKSAVE'
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {0x18} | Steam {0xc,0x1f4,0x3e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a0630: 16-byte aligned: YES
  007a0630  55             PUSH EBP
  007a0631  8B EC          MOV EBP,ESP
  007a0633  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a0630
string literals (0):
direct callees (4):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
  0x0070BC60  (unnamed)
  0x0070B8F0  CExoSound::Render wrapper
struct offsets touched (top 1):
  +0x4 [dec 4]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a0630 FUN_007a0630 size=126 ret=0 callers=1 callees=4

void __fastcall FUN_007a0630(undefined4 param_1)

{
  int iVar1;
  undefined8 uVar2;
  int local_c;
  
  uVar2 = FUN_00736220(param_1);
  iVar1 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar2,1000,0);
  FUN_0070bc60(DAT_0099c2ac,0);
  local_c = iVar1;
  while ((uint)(local_c - iVar1) < 500) {
    FUN_0070b8f0();
    uVar2 = FUN_00736220();
    local_c = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar2,1000,0);
  }
  return;
}
```

### decoy: 0x007a06b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a06b0: 16-byte aligned: YES
  007a06b0  55             PUSH EBP
  007a06b1  8B EC          MOV EBP,ESP
  007a06b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 126 B, RET 0
- callers 1: 0x00740f00 CClientExoApp::BusyWaitMuteSound
- callees: 0x00736220 (CALL), 0x0091d220 (CALL), 0x0070bc60 (CALL), 0x0070b8f0 (CALL)
- globals: 0x00a1b488 READ, 0x00a1b488 READ, 0x00a1b494 READ
- twins: none

## Card B17-5-21

GOG function: CSWGuiMainInterface::Quicksave  gog 0x0044ccb0  callconv=(blank)  param_size_bytes=0  gog_size=148  gog_retn=0
Candidate Steam address: 0x007a0830 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a0750:STR0/CALLEE1/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044cc30 CClientExoAppInternal::BusyWaitMuteSound -> 0x007a0630  trusted
- GOG-order neighbour after:  0x0044cd50 CClientExoAppInternal::SetToolTipInstantMode -> 0x007a0750  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cbb0 CClientExoApp::SendSaveGameRequest -> Steam 0x0073f6f0  in candidate callees: no
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: no
GOG strings (1): 'QUICKSAVE'
disp sets (* = in both, 0 common): GOG {0x4} | Steam {0x130}
imm sets (* = in both, 0 common): GOG {0x18} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a0830: 16-byte aligned: YES
  007a0830  55             PUSH EBP
  007a0831  8B EC          MOV EBP,ESP
  007a0833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a0830
string literals (0):
direct callees (1):
  0x0093B5E0  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007a0830 FUN_007a0830 size=37 ret=0 callers=1 callees=1

void __fastcall FUN_007a0830(int param_1)

{
  if (*(int *)(param_1 + 0x130) != 0) {
    FUN_0093b5e0();
  }
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
- size 37 B, RET 0
- callers 1: 0x0040bf40
- callees: 0x0093b5e0 (CALL)
- twins: none

## Card B17-5-22

GOG function: CSWGuiSaveLoad::UnpackGame  gog 0x005d6e10  callconv=(blank)  param_size_bytes=8  gog_size=1040  gog_retn=12
Candidate Steam address: 0x00857490 (size 1040, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: HideLoadScreen 0x73ff50, NukeDirectory 0x715710; callers: LoadGame 0x853a00); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008578b0:STR0/CALLEE0/SIG0.274/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005d6a10 CSWGuiSaveLoadEntry::SetXboxTitle  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d7230 CSWGuiSaveLoad::HandleSaveButton  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x0040d430 CClientExoApp::HideLoadScreen -> Steam 0x0073ff50  in candidate callees: yes
  0x004e2040 CSWGuiSaveLoad::CopyGameToFutureGame -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  ... 12 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
GOG strings (5): 'SAVES:'; 'CORRUPT'; '%06d - %s'; 'FUTUREGAME:' ...
disp sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}
imm sets (* = in both, 23 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+9)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+9)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 12)
```

### prologue
```
prologue for 0x00857490: 16-byte aligned: YES
  00857490  55             PUSH EBP
  00857491  8B EC          MOV EBP,ESP
  00857493  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00857490
string literals (7):
  0x00993E84  at 008574d9  'GAMEINPROGRESS:'
  0x009A178C  at 00857514  'FUTUREGAME:'
  0x00993E84  at 0085754c  'GAMEINPROGRESS:'
  0x00993E38  at 008576cd  '%06d - %s'
  0x00993E30  at 008576ec  'SAVES:'
  0x009A1784  at 0085777b  'CORRUPT'
  0x009A1784  at 0085784c  'CORRUPT'
direct callees (20):
  0x007D1940  ExtractSaveToFUTUREGAME
  0x00733570  CExoString::CExoString_char
  0x007397B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00715710  CExoResMan::NukeDirectory
  0x00919F10  _strlen
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x00922AED  (unnamed)
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x0073FF50  CClientExoApp::HideLoadScreen
  0x0040E770  (unnamed)
  0x007397F0  (unnamed)
  0x00739A80  (unnamed)
  0x00739900  (unnamed)
  0x00739880  (unnamed)
struct offsets touched (top 2):
  +0xC [dec 12]  x2  via ECX
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00857490 FUN_00857490 size=1040 ret=12 callers=1 callees=20

undefined4 FUN_00857490(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  size_t local_9c;
  size_t local_98;
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined4 local_60;
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined1 local_3c [12];
  undefined1 local_30 [16];
  char *local_20 [2];
  char *local_18 [2];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972b96;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_007d1940(param_1,param_2);
  if (iVar2 == 0) {
    FUN_00733540(uVar1);
    local_8 = 9;
    FUN_00733540();
    local_8 = CONCAT31(local_8._1_3_,10);
    if (param_1 == -1) {
      FUN_007337c0(param_2);
    }
    else {
      uVar3 = FUN_00734010();
      FUN_00734270(local_30,"%06d - %s",param_1,uVar3);
    }
    FUN_007338d0("SAVES:");
    uVar3 = FUN_00733df0(local_68,local_30);
    local_8._0_1_ = 0xb;
    FUN_007337c0(uVar3);
    local_8._0_1_ = 10;
    FUN_00733780();
    Concurrency::details::SchedulerResourceManagement::Statistics
              (*(SchedulerResourceManagement **)(DAT_00a1b4a4 + 4),(uint *)0x0,(uint *)0x1,
               (uint *)0x1);
    FUN_0040e770(0xbc51);
    FUN_00733570(&DAT_009969b8);
    local_8._0_1_ = 0xc;
    FUN_00733570("CORRUPT");
    local_8._0_1_ = 0xd;
    FUN_00733570(&DAT_00993e10);
    local_8._0_1_ = 0xe;
    FUN_00733df0(local_80,local_78);
    local_8._0_1_ = 0xf;
    uVar3 = FUN_00733df0(local_90,local_88);
    local_8._0_1_ = 0x10;
    FUN_007397f0(uVar3,0,local_70);
    local_8._0_1_ = 0x12;
    FUN_00733780();
    local_8._0_1_ = 0x13;
    FUN_00733780();
    local_8._0_1_ = 0x14;
    FUN_00733780();
    local_8._0_1_ = 0x15;
    FUN_00733780();
    local_8._0_1_ = 0x16;
    FUN_00733780();
    FUN_00739a80("CORRUPT");
    FUN_00739900();
    local_8._0_1_ = 10;
/* ... 57 more lines truncated by --max-lines */
```

### decoy: 0x008578b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008578b0: 16-byte aligned: YES
  008578b0  55             PUSH EBP
  008578b1  8B EC          MOV EBP,ESP
  008578b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008578b0
string literals (5):
  0x009A41E4  at 008578fe  'OPTIONS:'
  0x00993E30  at 008579f9  'SAVES:'
  0x009A41C8  at 00857b33  '%2.3f'
  0x00993E30  at 00857cbf  'SAVES:'
  0x009A41C8  at 00857dd8  '%2.3f'
direct callees (23):
  0x00733570  CExoString::CExoString_char
  0x00711690  CExoResMan::AddResourceDirectory
  0x00733780  CExoString::~CExoString
  0x007397F0  (unnamed)
  0x007398E0  (unnamed)
  0x00739970  (unnamed)
  0x00739880  (unnamed)
  0x0041FFB0  (unnamed)
  0x00714DE0  CExoResMan::GetFreeDiskSpace
  0x0091D220  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x0071E910  (unnamed)
  0x0071DEE0  (unnamed)
  0x007337C0  CExoString::operator=
  0x0071EBD0  (unnamed)
  0x0075BB00  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x0075CA30  (unnamed)
struct offsets touched (top 9):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x1C [dec 28]  x3  via EAX,EDX
  +0xA0 [dec 160]  x3  via ECX,EDX
  +0x74 [dec 116]  x2  via EDX
  +0x1DC [dec 476]  x2  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x1D0 [dec 464]  x1  via EDX
  +0x11BC [dec 4540]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1040 B, RET 12
- callers 1: 0x00853a00 CSWGuiSaveLoad::LoadGame
- callees: 0x007d1940 (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x007397b0 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x00715710 CExoResMan::NukeDirectory (CALL), 0x00919f10 _strlen (CALL), ...
- globals: 0x00a1b488 READ, 0x00a1b490 READ
- twins: none
