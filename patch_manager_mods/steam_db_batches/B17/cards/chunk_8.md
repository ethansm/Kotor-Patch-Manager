# Review cards B17 chunk 8

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-8-01

GOG function: CSWCModule::AcclTurnCamera  gog 0x0046b440  callconv=(blank)  param_size_bytes=8  gog_size=643  gog_retn=8
Candidate Steam address: 0x007f3ea0 (size 885, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.74 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f4220:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0046b0c0 CSWCModule::TurnCamera -> 0x007f3ea0  gate_class=REVIEW
- GOG-order neighbour after:  0x0046b6d0 CSWCModule::ZoomCamera -> 0x007f44b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callees: yes
  0x0040dd20 CClientExoApp::GetCombatMode -> Steam 0x00740860  in candidate callees: yes
  0x00574940 CSWBehaviorCameraCombat::SetTurn -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00438930 CClientExoAppInternal::UpdateCamera -> Steam 0x0078c350  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x98*,0x100*,0x10c*,0x460*} | Steam {0x4*,0x8,0xc*,0x14*,0x1c*,0x28*,0x2c*,0x40*,0x80*,0x88,0x98*,0x100*,0x10c*,0x460*}
imm sets (* = in both, 10 common): GOG {0x3*,0x5*,0x7*,0x8*,0x41*,0x44*,0x48,0x106a*,0x106e*,0x1070*,0x1071*} | Steam {0x3*,0x4,0x5*,0x7*,0x8*,0xc,0x10,0x30,0x41*,0x44*,0x9c,0x106a*,0x106e*,0x1070*,...(+1)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f3ea0: 16-byte aligned: YES
  007f3ea0  55             PUSH EBP
  007f3ea1  8B EC          MOV EBP,ESP
  007f3ea3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007f3ea0 FUN_007f3ea0 size=885 ret=8 callers=2 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_007f3ea0(int param_1,float param_2,float param_3)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  float10 fVar4;
  double local_9c;
  double local_94;
  double local_8c;
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [20];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  float local_20;
  int *local_1c;
  int local_18;
  int local_10;
  int *local_8;
  
  iVar1 = FUN_00740860();
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0xc) == 3) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106a);
      if (iVar1 != 0) {
        *(float *)(iVar1 + 0x100) = param_2;
        *(undefined4 *)(iVar1 + 0x10c) = 0;
      }
    }
    else if (*(int *)(param_1 + 0xc) == 5) {
      local_18 = (**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0x106e);
      if (local_18 != 0) {
        if (param_2 * (float)_DAT_00987f08 <= (float)_DAT_009946f8) {
          local_8c = (double)(param_2 * (float)_DAT_00987f08);
        }
        else {
          local_8c = _DAT_009946f8;
        }
        if (_DAT_009a2420 <= local_8c) {
          if (param_2 * (float)_DAT_00987f08 <= (float)_DAT_009946f8) {
            local_9c = (double)(param_2 * (float)_DAT_00987f08);
          }
          else {
            local_9c = _DAT_009946f8;
          }
          local_94 = local_9c;
        }
        else {
          local_94 = _DAT_009a2420;
        }
        local_20 = -(float)local_94 * *(float *)(local_18 + 0x28) * param_3;
        local_1c = (int *)FUN_0073f450();
        if (local_1c != (int *)0x0) {
          local_1c[0x118 = byte +0x460] = 0;
          FUN_00515e60(&local_30,local_1c + 0xc);
          fVar4 = (float10)FUN_00515a70(local_30,local_2c,local_28,local_24);
          fVar4 = (float10)FUN_00515990(&local_30,(float)fVar4);
          fVar4 = (float10)FUN_005158b0(&local_30,(float)fVar4);
          puVar2 = (undefined4 *)FUN_00515620(local_44,(float)(fVar4 + (float10)local_20));
          local_30 = *puVar2;
          local_2c = puVar2[1 = byte +0x4];
          local_28 = puVar2[2 = byte +0x8];
          local_24 = puVar2[3 = byte +0xc];
          uVar3 = FUN_00463120(local_50);
          (**(code **)(*local_1c + 0x88))(uVar3);
          if ((local_1c != (int *)0x0) && (iVar1 = FUN_0077d800(), iVar1 != 0)) {
            uVar3 = FUN_00463120(local_5c);
            FUN_0077d800(uVar3);
            FUN_00543f10(uVar3);
          }
        }
      }
    }
    else if (*(int *)(param_1 + 0xc) == 7) {
/* ... 27 more lines truncated by --max-lines */
```

### decoy: 0x007f4220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f4220: 16-byte aligned: YES
  007f4220  55             PUSH EBP
  007f4221  8B EC          MOV EBP,ESP
  007f4223  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 885 B, RET 8
- callers 2: 0x0078c350 CClientExoAppInternal::UpdateCamera, 0x0078cea0
- callees: 0x00740860 CClientExoApp::GetCombatMode (CALL), 0x008e5480 (CALL), 0x0073f450 CClientExoApp::GetPlayerCreature (CALL), 0x00515e60 (CALL), 0x00515a70 (CALL), 0x00515990 (CALL), ...
- indirect: 0x007f3edc slot 0x80, 0x007f3f29 slot 0x80, 0x007f3f76 slot 0x80, 0x007f4103 slot 0x88, 0x007f415c slot 0x80, ...
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-8-02

GOG function: Plane::Transform  gog 0x0080cad0  callconv=(blank)  param_size_bytes=8  gog_size=1605  gog_retn=8
Candidate Steam address: 0x00514c40 (size 1605, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00515290:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080ca20 Global::operator*  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080d120 Global::normalize_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 4 common): GOG {0x2*,0x8*,0x44*,0x114*} | Steam {0x2*,0x8*,0x44*,0x114*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00514c40: 16-byte aligned: YES
  00514c40  55             PUSH EBP
  00514c41  8B EC          MOV EBP,ESP
  00514c43  81 EC 14 01 00 00 SUB ESP,0x114
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00514c40
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x11  via ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00514c40 FUN_00514c40 size=1605 ret=8 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __thiscall FUN_00514c40(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float local_60;
  float local_5c;
  float local_58;
  float local_20;
  float local_1c;
  float local_18;
  
  if ((DAT_00a7d7b8 & 1) == 0) {
    DAT_00a7d7b8 = DAT_00a7d7b8 | 1;
  }
  if ((DAT_00a7d7b8 & 2) == 0) {
    DAT_00a7d7b8 = DAT_00a7d7b8 | 2;
  }
  fVar4 = *param_3 * (float)_DAT_00986d98;
  fVar1 = param_3[1 = byte +0x4];
  fVar2 = param_3[2 = byte +0x8];
  fVar3 = param_3[3 = byte +0xc];
  if ((DAT_00a0f190 == 0) || ((fVar4 != (float)_DAT_00986b50 && (fVar4 != (float)_DAT_00986d98)))) {
    local_20 = (1.0 - (fVar3 * fVar3 + fVar2 * fVar2) * (float)_DAT_00987490) * *param_1 +
               (fVar1 * fVar2 - fVar3 * fVar4) * (float)_DAT_00987490 * param_1[1 = byte +0x4] +
               (fVar2 * fVar4 + fVar1 * fVar3) * (float)_DAT_00987490 * param_1[2 = byte +0x8];
    local_1c = (fVar3 * fVar4 + fVar1 * fVar2) * (float)_DAT_00987490 * *param_1 +
               (1.0 - (fVar3 * fVar3 + fVar1 * fVar1) * (float)_DAT_00987490) * param_1[1 = byte +0x4] +
               (fVar2 * fVar3 - fVar1 * fVar4) * (float)_DAT_00987490 * param_1[2 = byte +0x8];
    local_18 = (fVar1 * fVar3 - fVar2 * fVar4) * (float)_DAT_00987490 * *param_1 +
               (fVar1 * fVar4 + fVar2 * fVar3) * (float)_DAT_00987490 * param_1[1 = byte +0x4] +
               (1.0 - (fVar2 * fVar2 + fVar1 * fVar1) * (float)_DAT_00987490) * param_1[2 = byte +0x8];
  }
  else {
    local_20 = *param_1;
    local_1c = param_1[1 = byte +0x4];
    local_18 = param_1[2 = byte +0x8];
  }
  DAT_00a7d7ac = local_20;
  DAT_00a7d7b0 = local_1c;
  DAT_00a7d7b4 = local_18;
  fVar1 = param_1[3 = byte +0xc];
  local_60 = fVar1 * -*param_1 - *param_2;
  local_5c = fVar1 * -param_1[1 = byte +0x4] - param_2[1 = byte +0x4];
  local_58 = fVar1 * -param_1[2 = byte +0x8] - param_2[2 = byte +0x8];
  fVar4 = *param_3 * (float)_DAT_00986d98;
  fVar1 = param_3[1 = byte +0x4];
  fVar2 = param_3[2 = byte +0x8];
  fVar3 = param_3[3 = byte +0xc];
  if ((DAT_00a0f190 == 0) || ((fVar4 != (float)_DAT_00986b50 && (fVar4 != (float)_DAT_00986d98)))) {
    fVar5 = (fVar2 * fVar4 + fVar1 * fVar3) * (float)_DAT_00987490 * local_58;
    fVar6 = (fVar2 * fVar3 - fVar1 * fVar4) * (float)_DAT_00987490 * local_58;
    fVar7 = (fVar3 * fVar4 + fVar1 * fVar2) * (float)_DAT_00987490 * local_60;
    local_58 = (fVar1 * fVar3 - fVar2 * fVar4) * (float)_DAT_00987490 * local_60 +
               (fVar1 * fVar4 + fVar2 * fVar3) * (float)_DAT_00987490 * local_5c +
               (1.0 - (fVar2 * fVar2 + fVar1 * fVar1) * (float)_DAT_00987490) * local_58;
    local_60 = (1.0 - (fVar3 * fVar3 + fVar2 * fVar2) * (float)_DAT_00987490) * local_60 +
               (fVar1 * fVar2 - fVar3 * fVar4) * (float)_DAT_00987490 * local_5c + fVar5;
    local_5c = fVar7 + (1.0 - (fVar3 * fVar3 + fVar1 * fVar1) * (float)_DAT_00987490) * local_5c +
               fVar6;
  }
  _DAT_00a7d7a0 = local_60;
  _DAT_00a7d7a4 = local_5c;
  _DAT_00a7d7a8 = local_58;
  *param_1 = local_20;
  param_1[1 = byte +0x4] = DAT_00a7d7b0;
  param_1[2 = byte +0x8] = DAT_00a7d7b4;
  param_1[3 = byte +0xc] = -(_DAT_00a7d7a0 * DAT_00a7d7ac + _DAT_00a7d7a4 * DAT_00a7d7b0 +
                _DAT_00a7d7a8 * DAT_00a7d7b4);
  return;
}
```

### decoy: 0x00515290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00515290: 16-byte aligned: YES
  00515290  55             PUSH EBP
  00515291  8B EC          MOV EBP,ESP
  00515293  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00515290
string literals (0):
direct callees (1):
  0x0091CB24  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX
```

### Static facts (script output, report-only)
- size 1605 B, RET 8
- callers 2: 0x00469310, 0x004b33e0
- twins: none

## Card B17-8-03

GOG function: CSWCAnimBase::LoadModel  gog 0x004c7ab0  callconv=(blank)  param_size_bytes=5  gog_size=556  gog_retn=8
Candidate Steam address: 0x00860280 (size 56, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCAnimBase::SetOverlayAnimation | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.20 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008601e0:STR0/CALLEE0/SIG0.254/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004c7a80 CSWCAnimBase::GetModel -> 0x0085fbb0  gate_class=REVIEW
- GOG-order neighbour after:  0x004c7ce0 CSWCAnimBase::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: no
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  0x008548b0 Global::NewCAurObject -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): '?headconjure'; 'handconjure'; 'CSWCAnimBase::LoadModel(): The headconju'; 'CSWCAnimBase::LoadModel(): The handconju'
disp sets (* = in both, 1 common): GOG {0x3c,0xa0,0xac,0xb8*} | Steam {0xb8*}
imm sets (* = in both, 1 common): GOG {0x8*,0xc,0x10,0x2c,0x44,0x58,0xff} | Steam {0x8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00860280: 16-byte aligned: YES
  00860280  55             PUSH EBP
  00860281  8B EC          MOV EBP,ESP
  00860283  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00860280
string literals (0):
direct callees (1):
  0x008602C0  CSWCAnimBase::SetOverlayAnimation_2
struct offsets touched (top 1):
  +0xB8 [dec 184]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00860280 FUN_00860280 size=56 ret=8 callers=0 callees=1

undefined4 __thiscall FUN_00860280(int param_1,undefined2 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0xb8) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_008602c0(*(undefined4 *)(param_1 + 0xb8),param_2,param_3);
  }
  return uVar1;
}
```

### decoy: 0x008601e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008601e0: 16-byte aligned: YES
  008601e0  55             PUSH EBP
  008601e1  8B EC          MOV EBP,ESP
  008601e3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008601e0
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 5):
  +0xB8 [dec 184]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
  +0x29 [dec 41]  x1  via EAX
```

### Static facts (script output, report-only)
- size 56 B, RET 8
- callees: 0x008602c0 (CALL)
- twins: none

## Card B17-8-04

GOG function: CSWRoomSurfaceMesh::ClippedLineSegmentWalkable  gog 0x007137e0  callconv=(blank)  param_size_bytes=28  gog_size=2068  gog_retn=28
Candidate Steam address: 0x00651090 (size 2068, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006518b0:STR0/CALLEE0/SIG0.4/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00714000 CSWRoomSurfaceMesh::ComputeClippedLineSegment -> 0x006518b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x00714000 CSWRoomSurfaceMesh::ComputeClippedLineSegment -> Steam ?  in candidate callees: n/a
  0x00714580 CSWRoomSurfaceMesh::FindTriangle -> Steam ?  in candidate callees: n/a
  0x007158e0 CSWRoomSurfaceMesh::GetTriangleAdjacency -> Steam ?  in candidate callees: n/a
  0x00715a00 CSWRoomSurfaceMesh::GetTriangleVertices -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 9 common): GOG {0x2*,0x3*,0xc*,0x14*,0x18*,0x1c*,0x41*,0x138*,0x200*} | Steam {0x2*,0x3*,0xc*,0x14*,0x18*,0x1c*,0x41*,0x138*,0x200*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x00651090: 16-byte aligned: YES
  00651090  55             PUSH EBP
  00651091  8B EC          MOV EBP,ESP
  00651093  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00651090
string literals (0):
direct callees (12):
  0x006518B0  (unnamed)
  0x00651E30  (unnamed)
  0x006532B0  (unnamed)
  0x005E9E90  (unnamed)
  0x00516640  (unnamed)
  0x00653190  (unnamed)
  0x005E9E30  (unnamed)
  0x005188C0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00733780  CExoString::~CExoString
struct offsets touched (top 2):
  +0x8 [dec 8]  x12  via EAX,ECX,EDX
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00651090 FUN_00651090 size=2068 ret=28 callers=1 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00651090(float *param_1,float *param_2,int param_3,int param_4,float *param_5,float *param_6,
            undefined4 param_7)

{
  uint uVar1;
  int iVar2;
  undefined4 extraout_var;
  undefined4 uVar3;
  int local_138;
  undefined4 *local_134;
  undefined1 local_120 [8];
  int local_118;
  int local_114;
  float local_110;
  float local_10c;
  float local_108;
  int local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  undefined4 local_ec;
  float local_e8;
  float local_e4;
  undefined4 local_e0;
  int local_dc;
  float local_d8;
  int local_d4;
  float local_d0;
  float local_cc;
  undefined4 local_c8;
  float local_c4;
  float local_c0;
  undefined4 local_bc;
  undefined4 local_b8;
  undefined1 local_b4 [4];
  undefined1 local_b0 [4];
  float afStack_ac[3 = byte +0xc];
  int aiStack_a0[3 = byte +0xc];
  float afStack_94[4 = byte +0x10];
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_74;
  undefined4 local_70[9 = byte +0x24];
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  float local_34;
  undefined1 auStack_30 [12];
  int local_24;
  int local_20;
  undefined1 local_1c [4];
  undefined1 local_18 [4];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958c4b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar2 = FUN_006518b0(*param_1,param_1[1 = byte +0x4],param_1[2 = byte +0x8],*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],param_3,param_4
                       ,param_7);
  if (iVar2 != 0) {
    local_4c = 0;
    local_138 = 3;
    local_134 = local_70;
    while (local_138 = local_138 + -1, -1 < local_138) {
/* ... 174 more lines truncated by --max-lines */
```

### decoy: 0x006518b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006518b0: 16-byte aligned: YES
  006518b0  55             PUSH EBP
  006518b1  8B EC          MOV EBP,ESP
  006518b3  81 EC 5C 01 00 00 SUB ESP,0x15c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006518b0
string literals (0):
direct callees (4):
  0x00517980  Vec3::Equal
  0x006521B0  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x24  via EAX,ECX,EDX
  +0x8 [dec 8]  x21  via EAX,ECX,EDX
  +0xBC [dec 188]  x10  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 2068 B, RET 28
- callers 1: 0x005ff730
- callees: 0x006518b0 (CALL), 0x00651e30 (CALL), 0x006532b0 (CALL), 0x005e9e90 (CALL), 0x00516640 (CALL), 0x00653190 (CALL), ...
- twins: none

## Card B17-8-05

GOG function: CSWSMessage::WriteGameObjUpdate_UpdateAppearance  gog 0x007d1fb0  callconv=(blank)  param_size_bytes=12  gog_size=2329  gog_retn=12
Candidate Steam address: 0x0063b890 (size 2329, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.76) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0062bf60:STR0/CALLEE0/SIG0.074/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007d1f50 CSWSMessage::WriteGameObjUpdate_WriteInventorySlotDelete  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007d28d0 CSWSMessage::ComputeGameObjectUpdateForYourself  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x0054b810 CSWMessage::WriteBOOL -> Steam ?  in candidate callees: n/a
  0x0054b890 CSWMessage::WriteBYTE -> Steam ?  in candidate callees: n/a
  0x0054b910 CSWMessage::WriteCHAR -> Steam ?  in candidate callees: n/a
  0x0054b990 CSWMessage::WriteWORD -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x0067a5b0 CSWInventory::GetItemInSlot -> Steam 0x006d0620  in candidate callees: yes
  ... 5 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 18 common): GOG {0x4*,0x8*,0xc*,0x10*,0x12*,0x13*,0x14*,0x15*,0x16*,0x17*,0x18*,0x19*,0x1a*,0x1c*,...(+4)} | Steam {0x4*,0x8*,0xc*,0x10*,0x12*,0x13*,0x14*,0x15*,0x16*,0x17*,0x18*,0x19*,0x1a*,0x1c*,...(+4)}
imm sets (* = in both, 21 common): GOG {0x2*,0x4*,0x8*,0xc*,0x10*,0x20*,0x34*,0x3c*,0x40*,0x50*,0x7c*,0x80*,0x200*,0x400*,...(+7)} | Steam {0x2*,0x4*,0x8*,0xc*,0x10*,0x20*,0x34*,0x3c*,0x40*,0x50*,0x7c*,0x80*,0x200*,0x400*,...(+7)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0063b890: 16-byte aligned: YES
  0063b890  55             PUSH EBP
  0063b891  8B EC          MOV EBP,ESP
  0063b893  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0063b890
string literals (0):
direct callees (13):
  0x005E4230  (unnamed)
  0x005E41B0  (unnamed)
  0x00632F60  (unnamed)
  0x005E42B0  (unnamed)
  0x00633190  (unnamed)
  0x005E4130  (unnamed)
  0x006D0620  CSWInventory::GetItemInSlot
  0x0063B830  (unnamed)
  0x0063B7B0  (unnamed)
  0x0058B320  CSWSCreature::HasEffect
  0x0084A640  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 10):
  +0x1150 [dec 4432]  x13  via EAX,ECX,EDX
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via ECX,EDX
  +0xC [dec 12]  x6  via ECX,EDX
  +0x10 [dec 16]  x5  via ECX,EDX
  +0x12 [dec 18]  x5  via EAX,ECX
  +0x13 [dec 19]  x5  via EAX,ECX,EDX
  +0x14 [dec 20]  x5  via ECX,EDX
  +0x15 [dec 21]  x5  via EAX,ECX,EDX
  +0x16 [dec 22]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0063b890 FUN_0063b890 size=2329 ret=12 callers=1 callees=13

void FUN_0063b890(int param_1,int *param_2,uint param_3)

{
  bool bVar1;
  bool bVar2;
  char cVar3;
  int iVar4;
  undefined4 uVar5;
  int *piVar6;
  int local_38;
  int local_34;
  int local_30;
  char local_19;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009586f8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  uVar5 = *(undefined4 *)(param_1 + 4);
  if (*(char *)(param_1 + 8) == DAT_00994474) {
    FUN_005e4230(0x50,8);
    FUN_005e41b0(DAT_00994474,8);
    FUN_00632f60(uVar5);
    FUN_005e42b0(param_3 & 0xffff,0x10);
    piVar6 = (int *)(param_1 + 0x1174);
    if ((param_3 & 0x400) != 0) {
      FUN_00633190(*(int *)(param_1 + 0x1198) + 0x34,0);
      FUN_00633190(*(int *)(param_1 + 0x1198) + 0x3c,0);
    }
    if ((param_3 & 1) != 0) {
      FUN_005e42b0(*(undefined2 *)(param_1 + 0x1184),0x10);
    }
    if ((param_3 & 2) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x1186),8);
    }
    if ((param_3 & 4) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x1187),8);
    }
    if ((param_3 & 0x80) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x118c),8);
    }
    if ((param_3 & 0x4000) != 0) {
      FUN_005e4130(*(undefined4 *)(param_1 + 0x1190));
    }
    if ((param_3 & 0x8000) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x1194),8);
    }
    if ((param_3 & 0x800) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x118d),8);
    }
    if ((param_3 & 0x1000) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x118e),8);
    }
    if ((param_3 & 8) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x1188),8);
    }
    if ((param_3 & 0x10) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x1189),8);
    }
    if ((param_3 & 0x20) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x118a),8);
    }
    if ((param_3 & 0x40) != 0) {
      FUN_005e41b0(*(undefined1 *)(param_1 + 0x118b),8);
    }
    if ((param_3 & 0x2000) != 0) {
      FUN_005e42b0(*(undefined2 *)(param_1 + 0x10ec),0x10);
    }
    local_19 = '\0';
    if (param_3 == 0xffffffff) {
      local_19 = '\x04';
      iVar4 = FUN_006d0620(2);
      if (iVar4 != 0) {
        local_19 = '\x05';
      }
      iVar4 = FUN_006d0620(1);
/* ... 168 more lines truncated by --max-lines */
```

### decoy: 0x0062bf60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0062bf60: 16-byte aligned: YES
  0062bf60  55             PUSH EBP
  0062bf61  8B EC          MOV EBP,ESP
  0062bf63  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0062bf60
string literals (0):
direct callees (2):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00605D80  CItemRepository::GetItemInRepository
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x270 [dec 624]  x1  via ECX
```

### Static facts (script output, report-only)
- size 2329 B, RET 12
- callers 1: 0x006430a0
- callees: 0x005e4230 (CALL), 0x005e41b0 (CALL), 0x00632f60 (CALL), 0x005e42b0 (CALL), 0x00633190 (CALL), 0x005e4130 (CALL), ...
- twins: none

## Card B17-8-06

GOG function: CSWCollisionMesh::NormalLocalToWorld  gog 0x00728b10  callconv=(blank)  param_size_bytes=12  gog_size=331  gog_retn=12
Candidate Steam address: 0x005eebf0 (size 331, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: rotation:0x007286d0:0.80>0.36
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: GetMeshBoundingBox 0x617320, AddToArea 0x5f2a80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ee7b0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007286d0 CSWCollisionMesh::LocalToWorld  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00728c60 Global::AddModuleOverrideResourceDirectories  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00723da0 CSWCollisionMesh::GetVertex -> Steam ?  in candidate callees: n/a
  0x00723e60 CSWCollisionMesh::LoadDefaultMesh -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007627d0 CSWSDoor::GetMeshBoundingBox -> Steam 0x00617320  in candidate callers: yes
  0x0076ce50 CSWSPlaceable::AddToArea -> Steam 0x005f2a80  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x58*,0x64*} | Steam {0x1c*,0x58*,0x64*}
imm sets (* = in both, 3 common): GOG {0xc*,0x50*,0x55ffff00*} | Steam {0xc*,0x50*,0x55ffff00*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005eebf0: 16-byte aligned: YES
  005eebf0  55             PUSH EBP
  005eebf1  8B EC          MOV EBP,ESP
  005eebf3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005eebf0
string literals (0):
direct callees (2):
  0x005E9DD0  (unnamed)
  0x005E9E90  (unnamed)
struct offsets touched (top 3):
  +0x58 [dec 88]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x64 [dec 100]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005eebf0 FUN_005eebf0 size=331 ret=12 callers=2 callees=2

/* WARNING: Type propagation algorithm not settling */

void __thiscall FUN_005eebf0(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int local_4c;
  undefined4 local_48;
  int local_44[16 = byte +0x40];
  
  local_44[7 = byte +0x1c] = 0xffffffff;
  local_4c = -1;
  local_44[0] = -1;
  local_44[8 = byte +0x20] = 0;
  local_44[9 = byte +0x24] = 0;
  local_44[10 = byte +0x28] = 0;
  local_44[1 = byte +0x4] = 0;
  local_44[2 = byte +0x8] = 0;
  local_44[3 = byte +0xc] = 0;
  local_44[0xc = byte +0x30] = 0;
  local_44[0xd = byte +0x34] = 0;
  local_44[0xe = byte +0x38] = 0;
  local_44[4 = byte +0x10] = 0;
  local_44[5 = byte +0x14] = 0;
  local_44[6 = byte +0x18] = 0;
  local_44[0xf = byte +0x3c] = (**(code **)(*param_1 + 0x1c))();
  if (param_1[0x16 = byte +0x58] != 0) {
    for (local_44[0xb = byte +0x2c] = 0; local_44[0xb = byte +0x2c] < param_1[0x16 = byte +0x58]; local_44[0xb = byte +0x2c] = local_44[0xb = byte +0x2c] + 1) {
      local_44[7 = byte +0x1c] = -1;
      local_4c = -1;
      local_44[0] = -1;
      local_48 = 0xffffffff;
      FUN_005e9dd0(local_44[0xb = byte +0x2c],local_44 + 0x7 [dec 7] {int[]: byte +0x1c},&local_4c,local_44);
      if (((local_44[7 = byte +0x1c] != -1) && (local_4c != -1)) && (local_44[0] != -1)) {
        if ((*(uint *)(param_1[0x19 = byte +0x64] + local_44[0xb = byte +0x2c] * 4) & local_44[0xf = byte +0x3c]) == 0) {
          local_48 = param_4;
        }
        else {
          local_48 = param_3;
        }
        FUN_005e9e90(local_44[7 = byte +0x1c],local_44 + 0x8 [dec 8] {int[]: byte +0x20});
        FUN_005e9e90(local_4c,local_44 + 0x1 [dec 1] {int[]: byte +0x4});
        FUN_005e9e90(local_44[0],local_44 + 0xc);
      }
    }
  }
  return;
}
```

### decoy: 0x005ee7b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ee7b0: 16-byte aligned: YES
  005ee7b0  55             PUSH EBP
  005ee7b1  8B EC          MOV EBP,ESP
  005ee7b3  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ee7b0
string literals (0):
direct callees (1):
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 3):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 331 B, RET 12
- callers 2: 0x005f2a80 CSWSPlaceable::AddToArea, 0x00617320 CSWSDoor::GetMeshBoundingBox
- callees: 0x005e9dd0 (CALL), 0x005e9e90 (CALL)
- indirect: 0x005eec55 slot 0x1c
- twins: none

## Card B17-8-07

GOG function: CExoRawInputInternal::Constructor  gog 0x00617350  callconv=(blank)  param_size_bytes=8  gog_size=1847  gog_retn=8
Candidate Steam address: 0x007306c0 (size 24, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007306a0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00617310 CExoDeviceBuffer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00617a90 CExoRawInputInternal::CheckControllerType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x00617a90 CExoRawInputInternal::CheckControllerType -> Steam ?  in candidate callees: n/a
  0x00619e00 CExoRawInputInternal::ShutDownDirectInput -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
  0x00943360 DINPUT8_DLL::DirectInput8Create -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x18,0x1c,0x1d,0x20,0x24,0x28,0x2c,0x30,0x34,...(+5)} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4,0x6,0x8*,0xa,0xc,0x10,0x14,0x18,0x20,0x24,0x74,0x100,0x110,0x134,...(+3)} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007306c0: 16-byte aligned: YES
  007306c0  55             PUSH EBP
  007306c1  8B EC          MOV EBP,ESP
  007306c3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007306c0
string literals (0):
direct callees (1):
  0x00732F80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007306c0 FUN_007306c0 size=24 ret=8 callers=0 callees=1

undefined4 FUN_007306c0(undefined4 param_1)

{
  FUN_00732f80(param_1);
  return 1;
}
```

### decoy: 0x007306a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007306a0: 16-byte aligned: YES
  007306a0  55             PUSH EBP
  007306a1  8B EC          MOV EBP,ESP
  007306a3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007306a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x18 [dec 24]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 24 B, RET 8
- callees: 0x00732f80 (CALL)
- twins: none

## Card B17-8-08

GOG function: CExoRawInputInternal::Constructor  gog 0x00617350  callconv=(blank)  param_size_bytes=8  gog_size=1847  gog_retn=8
Candidate Steam address: 0x00731390 (size 5570, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.55 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.55
- best decoy (neighbour/other candidate) scores: 0x00732960:STR0/CALLEE0/SIG0.348/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00617310 CExoDeviceBuffer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00617a90 CExoRawInputInternal::CheckControllerType  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00608350 CExoTimers::GetHighResolutionTimer -> Steam ?  in candidate callees: n/a
  0x00617a90 CExoRawInputInternal::CheckControllerType -> Steam ?  in candidate callees: n/a
  0x00619e00 CExoRawInputInternal::ShutDownDirectInput -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
  0x00943360 DINPUT8_DLL::DirectInput8Create -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x1d,0x20*,0x24*,0x28,0x2c*,0x30*,0x34*,...(+5)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x2c*,0x30*,0x34*,0x64,0x110*,...(+4)}
imm sets (* = in both, 11 common): GOG {0x4*,0x6,0x8*,0xa,0xc*,0x10*,0x14*,0x18*,0x20*,0x24,0x74,0x100*,0x110*,0x134*,...(+3)} | Steam {0x2,0x3,0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c,0x20*,0x2d,0x30,0x41,0x44,...(+26)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00731390: 16-byte aligned: YES
  00731390  55             PUSH EBP
  00731391  8B EC          MOV EBP,ESP
  00731393  81 EC 90 02 00 00 SUB ESP,0x290
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00731390
string literals (0):
direct callees (9):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091EBAD  (unnamed)
  0x0091DBA0  _memcpy
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x4 [dec 4]  x76  via EAX,ECX,EDX
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x4  via EAX,EDX
  +0x114 [dec 276]  x4  via ECX,EDX
  +0x118 [dec 280]  x4  via EAX,ECX
  +0x1C [dec 28]  x3  via EAX,EDX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x110 [dec 272]  x2  via EAX,EDX
  +0x124 [dec 292]  x2  via EAX
  +0x128 [dec 296]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00731390 FUN_00731390 size=5570 ret=8 callers=1 callees=9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00731390(int param_1,int param_2,int *param_3)

{
  double dVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int *piVar5;
  undefined8 uVar6;
  double dVar7;
  int local_294;
  undefined4 local_284;
  float local_274;
  int local_260;
  undefined4 local_250;
  float local_240;
  int local_22c;
  undefined4 local_21c;
  float local_20c;
  int local_1f8;
  undefined4 local_1e8;
  float local_1d8;
  int local_1c4;
  undefined4 local_1b4;
  float local_1a4;
  int local_190;
  undefined4 local_180;
  float local_170;
  ushort local_144;
  ushort local_134;
  byte local_129;
  int local_120;
  int local_11c;
  int local_118;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  uint auStack_fc[4 = byte +0x10];
  byte abStack_ec [228];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (*param_3 != 0) {
    FUN_009196fd();
    *param_3 = 0;
  }
  param_3[1 = byte +0x4] = 0;
  iVar2 = FUN_00919723();
  *param_3 = iVar2;
  if ((((*param_3 != 0) && (_memset [GHIDRA-AUTO-LABEL: unreliable]((void *)*param_3,0,0x100), *(int *)(param_1 + 0x2c) != 0)) &&
      (*(int *)(*(int *)(param_1 + 0x2c) + param_2 * 4) != 0)) &&
     ((*(int *)(param_1 + 0x30) != 0 && (*(int *)(param_1 + 0x34) != 0)))) {
    local_120 = -0x7ff8ffe2;
    while (local_120 == -0x7ff8ffe2) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x2c) + param_2 * 4) + 0x64 [dec 100]))();
      iVar2 = (**(code **)(**(int **)(*(int *)(param_1 + 0x2c) + param_2 * 4) + 0x24))
                        (*(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4),0x110,&local_11c);
      if (iVar2 == 0) {
        (&DAT_00a7ea70)[param_2] = 1;
        piVar5 = (int *)(param_2 * 0x134 + *(int *)(param_1 + 0x30));
        uVar6 = FUN_00736220();
        iVar2 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar6,1000,0);
        for (local_129 = 0; local_129 < 0x20; local_129 = local_129 + 1) {
          if ((abStack_ec[local_129] & 0x80) != (*(byte *)((int)piVar5 + local_129 + 0x30) & 0x80))
          {
            *(uint *)(param_3[1 = byte +0x4] * 0x14 + *param_3) = local_129 + 0x30;
            *(uint *)(*param_3 + 0x4 [dec 4] {int*: byte +0x10} + param_3[1 = byte +0x4] * 0x14) =
                 (uint)((abStack_ec[local_129] & 0x80) != 0);
            *(undefined4 *)(*param_3 + 0xc + param_3[1 = byte +0x4] * 0x14) = 0;
            *(undefined4 *)(*param_3 + 0x8 [dec 8] {int*: byte +0x20} + param_3[1 = byte +0x4] * 0x14) = 0;
            param_3[1 = byte +0x4] = param_3[1 = byte +0x4] + 1;
          }
        }
        for (local_129 = 0; local_129 < 4; local_129 = local_129 + 1) {
          if (auStack_fc[local_129] != piVar5[local_129 + 8]) {
/* ... 216 more lines truncated by --max-lines */
```

### decoy: 0x00732960 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00732960: 16-byte aligned: YES
  00732960  55             PUSH EBP
  00732961  8B EC          MOV EBP,ESP
  00732963  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00732960
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
```

### Static facts (script output, report-only)
- size 5570 B, RET 8
- callers 1: 0x0072c7e0
- callees: 0x009196fd operator delete (CALL), 0x00919723 operator new (CALL), 0x0091c630 (CALL), 0x00736220 (CALL), 0x0091d220 (CALL), 0x0091c860 (CALL), ...
- indirect: 0x0073149c slot 0x64, 0x007314d4 slot 0x24, 0x0073157d slot 0x1c, 0x007315c2 slot 0x1c
- globals: 0x00a1b488 READ
- twins: none

## Card B17-8-09

GOG function: Global::QueryTimer  gog 0x00713490  callconv=(blank)  param_size_bytes=0  gog_size=158  gog_retn=0
Candidate Steam address: 0x00650d40 (size 158, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650de0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713330 CSWRoomSurfaceMesh::CheckAABBAll  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> 0x00650de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00713530 CSWCollisionMesh::not_GetWalkableMaterialMask -> Steam ?  in candidate callees: n/a
  0x00927050 unnamed -> Steam ?  in candidate callees: n/a
  0x00929ca0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xf4240*} | Steam {0x8*,0xf4240*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00650d40: 16-byte aligned: YES
  00650d40  55             PUSH EBP
  00650d41  8B EC          MOV EBP,ESP
  00650d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00650d40
string literals (0):
direct callees (3):
  0x00650DE0  (unnamed)
  0x0091FC30  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00650d40 FUN_00650d40 size=158 ret=0 callers=1 callees=4

void FUN_00650d40(void)

{
  undefined8 uVar1;
  LARGE_INTEGER local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  uVar1 = CONCAT44(DAT_00a7e104,DAT_00a7e100);
  puStack_c = &LAB_00958c1e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if ((DAT_00a7e108 & 1) == 0) {
    DAT_00a7e108 = DAT_00a7e108 | 1;
    local_8 = 0;
    uVar1 = FUN_00650de0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  }
  DAT_00a7e104 = (undefined4)((ulonglong)uVar1 >> 0x20);
  DAT_00a7e100 = (undefined4)uVar1;
  local_8 = 0xffffffff;
  QueryPerformanceCounter(&local_18);
  uVar1 = __allmul [GHIDRA-AUTO-LABEL: unreliable](local_18.s.LowPart,local_18.s.HighPart,1000000,0);
  __alldiv [GHIDRA-AUTO-LABEL: unreliable](uVar1,DAT_00a7e100,DAT_00a7e104);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00650de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650de0: 16-byte aligned: YES
  00650de0  55             PUSH EBP
  00650de1  8B EC          MOV EBP,ESP
  00650de3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00650de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 158 B, RET 0
- callers 1: 0x00650be0
- callees: 0x00650de0 (CALL), 0x000000d5 (CALL), 0x0091fc30 (CALL), 0x00920670 (CALL)
- imports: QueryPerformanceCounter (KERNEL32.DLL)
- twins: none

## Card B17-8-10

GOG function: CServerAIBodyBagInfo::LoadBodyBag  gog 0x006509e0  callconv=(blank)  param_size_bytes=8  gog_size=147  gog_retn=8
Candidate Steam address: 0x005210d0 (size 31, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005210f0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00650960 CServerAIBodyBagInfo::SaveBodyBag  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00650a80 CSWGlobalVariableTable::Constructor -> 0x00653bc0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00624d60 CResGFF::ReadFieldDWORD -> Steam 0x00718c00  in candidate callees: no
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): 'PositionZ'; 'PositionY'; 'PositionX'; 'BodyBagId'
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
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
prologue for 0x005210d0: 16-byte aligned: YES
  005210d0  55             PUSH EBP
  005210d1  8B EC          MOV EBP,ESP
  005210d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005210d0
string literals (0):
direct callees (1):
  0x00739F20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005210d0 FUN_005210d0 size=31 ret=8 callers=5 callees=1
THUNK -> 0x00739F20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_005210d0(undefined4 param_1,undefined4 param_2)

{
  FUN_00739f20(param_1,param_2);
  return;
}
```

### decoy: 0x005210f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005210f0: 16-byte aligned: YES
  005210f0  55             PUSH EBP
  005210f1  8B EC          MOV EBP,ESP
  005210f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005210f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 31 B, RET 8
- callers 5: 0x0051e220 CServerAIMaster::AddEventAbsoluteTime, 0x00590270 CSWSCombatRound::AddAction, 0x00591b60 CSWSCombatRound::AddEquipAction, 0x00591cb0 CSWSCombatRound::AddUnequipAction, 0x0093c8b0
- callees: 0x00739f20 (CALL)
- twins: none

## Card B17-8-11

GOG function: CWorldTimer::GetSnapshotTime  gog 0x0080ae00  callconv=(blank)  param_size_bytes=0  gog_size=20  gog_retn=0
Candidate Steam address: 0x0051a920 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 11 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 11 (callers: EvaluateNewWayPoints 0x774bc0, HandleMessage 0x781840, MainLoop 0x781be0)
- best decoy (neighbour/other candidate) scores: 0x0051a940:STR0/CALLEE3/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0080ad40 CWorldTimer::SetSnapshotTime -> 0x0051a860  gate_class=AUTO
- GOG-order neighbour after:  0x0080ae20 CWorldTimer::GetSnapshotTimeDifference -> 0x0051a940  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 19): the card's A-CALLEE credit may come from these, not from callees
  0x00420a90 CSWCCreature::EvaluateNewWayPoints -> Steam 0x00774bc0  in candidate callers: yes
  0x0042ddc0 CClientExoAppInternal::HandleMessage -> Steam 0x00781840  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00430d60 CClientExoAppInternal::SetPauseState -> Steam 0x00784740  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
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
prologue for 0x0051a920: 16-byte aligned: YES
  0051a920  55             PUSH EBP
  0051a921  8B EC          MOV EBP,ESP
  0051a923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051a920
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051a920 FUN_0051a920 size=20 ret=0 callers=19 callees=0

undefined8 __fastcall FUN_0051a920(int param_1)

{
  return *(undefined8 *)(param_1 + 0xc);
}
```

### decoy: 0x0051a940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051a940: 16-byte aligned: YES
  0051a940  55             PUSH EBP
  0051a941  8B EC          MOV EBP,ESP
  0051a943  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051a940
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 20 B, RET 0
- callers 19: 0x0040a160, 0x0040a460, 0x0051ac30, 0x0051ad20 CWorldTimer::GetWorldTime, 0x00538ed0 CServerExoAppInternal::SetPauseState, ...
- twins: none

## Card B17-8-12

GOG function: CSWGuiInGameEquip::Draw  gog 0x0059f8a0  callconv=(blank)  param_size_bytes=4  gog_size=209  gog_retn=4
Candidate Steam address: 0x008ab860 (size 494, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008aba50:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0059f6b0 CSWGuiInGameEquip::SetDescription  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0059f980 CSWGuiInGameEquip::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x0059ec10 CSWGuiInGameEquip::SetCharacter -> Steam ?  in candidate callees: n/a
  0x005a1780 CSWGuiInGameEquip::UpdateInventory -> Steam ?  in candidate callees: n/a
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x88,0x4ec4,0x50e4,0x50e8} | Steam {0x4*,0x8,0xc,0x3c,0x58,0x5c,0x27cc}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0x41,0x4ec4} | Steam {0x4*,0x3c,0x3e,0x40,0x27cc,0x27d0,0x28a4,0x28bc,0x3a1c,0xfffffffe}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008ab860: 16-byte aligned: YES
  008ab860  55             PUSH EBP
  008ab861  8B EC          MOV EBP,ESP
  008ab863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008ab860
string literals (0):
direct callees (15):
  0x007335F0  CExoString::CExoString_copy
  0x00734190  CExoString::Find_2
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x007FEEC0  CSWCItem::non_virtual_thunk_to_dtor_CSWCItem
  0x0071EBD0  (unnamed)
  0x00416E30  CSWGuiTextParams::SetText
  0x0041D290  CSWGuiListBox::ClearItems
  0x00919F10  _strlen
  0x004178B0  (unnamed)
  0x00479AB0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0041FD90  CSWGuiListBox::AddControls
  0x0041E9A0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x5C [dec 92]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
  +0x58 [dec 88]  x1  via ECX
  +0x27CC [dec 10188]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008ab860 FUN_008ab860 size=494 ret=4 callers=1 callees=15

void __thiscall FUN_008ab860(int param_1,int *param_2)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  size_t local_48;
  int local_34;
  int local_30;
  float local_2c;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00951198;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007335f0(param_2);
  local_8 = 0;
  iVar3 = FUN_00734190(0x3c,0);
  local_30 = FUN_00734190(0x3e,0);
  if ((iVar3 != -1) || (local_30 != -1)) {
    FUN_0073fb90(uVar2);
    uVar4 = FUN_007e5d80();
    iVar3 = DAT_00a1b4b4;
    *(undefined4 *)(DAT_00a1b4b4 + 0x58) = uVar4;
    uVar1 = FUN_007feec0();
    *(undefined1 *)(iVar3 + 0x3c) = uVar1;
    *(uint *)(iVar3 + 0x5c) = *(uint *)(iVar3 + 0x5c) & 0xfffffffe | 1;
    FUN_0071ebd0(local_28);
  }
  FUN_00416e30(local_28);
  FUN_0041d290();
  local_20 = *(undefined4 *)(param_1 + 0x27d0);
  local_1c = *(undefined4 *)(param_1 + 0x27d4);
  local_18 = *(undefined4 *)(param_1 + 0x27d8);
  local_14 = *(undefined4 *)(param_1 + 0x27dc);
  if (*param_2 == 0) {
    local_48 = 0;
  }
  else {
    local_48 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
  }
  if ((int)local_48 < 1) {
    local_14 = FUN_004178b0();
  }
  else {
    local_14 = (**(code **)(*(int *)(param_1 + 0x28a4) + 8))();
  }
  fVar5 = (float10)FUN_00479ab0();
  local_2c = (float)fVar5;
  local_14 = FUN_0091c860();
  (**(code **)(*(int *)(param_1 + 0x27cc) + 4))(&local_20);
  local_34 = param_1 + 0x27cc;
  FUN_0041fd90(&local_34,1,0,0,0);
  FUN_0041e9a0(param_1 + 0x27cc,0);
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008aba50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008aba50: 16-byte aligned: YES
  008aba50  55             PUSH EBP
  008aba51  8B EC          MOV EBP,ESP
  008aba53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008aba50
string literals (0):
direct callees (5):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x008AADC0  (unnamed)
  0x008AD930  Equipment panel slot refresh (per frame from 0x8ABA50 via 0x8ABA8C): top placeholder resets ctl0/ctl9, slot loop k=[EBP-0x28] 0..10, slot image ctl = this+0x14D0+k*0x148
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 5):
  +0x50E4 [dec 20708]  x4  via EAX,ECX,EDX
  +0x88 [dec 136]  x2  via EAX,EDX
  +0x4EC4 [dec 20164]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x50E8 [dec 20712]  x1  via EAX
```

### Static facts (script output, report-only)
- size 494 B, RET 4
- callers 1: 0x008af2d0
- callees: 0x007335f0 CExoString::CExoString_copy (CALL), 0x00734190 (CALL), 0x0073fb90 CClientExoApp::GetSWParty (CALL), 0x007e5d80 (CALL), 0x007feec0 (CALL), 0x0071ebd0 (CALL), ...
- indirect: 0x008ab9a8 slot 0x8, 0x008ab9ef slot 0x4
- globals: 0x00a1b4b4 READ
- twins: none

## Card B17-8-13

GOG function: CSWGuiContainer::OnPanelAdded  gog 0x005a71d0  callconv=(blank)  param_size_bytes=0  gog_size=243  gog_retn=0
Candidate Steam address: 0x008b3730 (size 30, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b3630:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a7170 CSWGuiContainer::Update -> 0x008b35d0  gate_class=AUTO
- GOG-order neighbour after:  0x005a72d0 CSWGuiContainer::OnPanelRemoved  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0090d840 CSWGuiPanel::OnPanelAdded -> Steam 0x0040f3c0  in candidate callees: no
  0x009125d0 CSWGuiManager::StoreCurrentMousePosition -> Steam ?  in candidate callees: n/a
  0x009126b0 CSWGuiManager::MoveMouseToPosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0x2c,0xb18} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b3730: 16-byte aligned: YES
  008b3730  55             PUSH EBP
  008b3731  8B EC          MOV EBP,ESP
  008b3733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008b3730
string literals (0):
direct callees (2):
  0x00414180  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008b3730 FUN_008b3730 size=30 ret=0 callers=0 callees=2

void __fastcall FUN_008b3730(undefined4 param_1)

{
  FUN_00414180(param_1);
  FUN_0040f4c0();
  return;
}
```

### decoy: 0x008b3630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b3630: 16-byte aligned: YES
  008b3630  55             PUSH EBP
  008b3631  8B EC          MOV EBP,ESP
  008b3633  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b3630
string literals (0):
direct callees (4):
  0x0073EEE0  (unnamed)
  0x00414150  (unnamed)
  0x00414230  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via ECX
  +0xC [dec 12]  x2  via ECX
```

### Static facts (script output, report-only)
- size 30 B, RET 0
- callees: 0x00414180 (CALL), 0x0040f4c0 CSWGuiPanel::OnPanelRemoved (CALL)
- globals: 0x00a1b49c READ
- twins: none

## Card B17-8-14

GOG function: CSWGuiContainer::OnPanelAdded  gog 0x005a71d0  callconv=(blank)  param_size_bytes=0  gog_size=243  gog_retn=0
Candidate Steam address: 0x008b2620 (size 560, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.05) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b25f0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a7170 CSWGuiContainer::Update -> 0x008b35d0  gate_class=AUTO
- GOG-order neighbour after:  0x005a72d0 CSWGuiContainer::OnPanelRemoved  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0090d840 CSWGuiPanel::OnPanelAdded -> Steam 0x0040f3c0  in candidate callees: no
  0x009125d0 CSWGuiManager::StoreCurrentMousePosition -> Steam ?  in candidate callees: n/a
  0x009126b0 CSWGuiManager::MoveMouseToPosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x2c,0xb18} | Steam {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+19)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b2620: 16-byte aligned: YES
  008b2620  55             PUSH EBP
  008b2621  8B EC          MOV EBP,ESP
  008b2623  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008b2620
string literals (0):
direct callees (6):
  0x0041A3D0  CSWGuiButton::Destructor
  0x0041D190  (unnamed)
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008b2620 FUN_008b2620 size=560 ret=0 callers=1 callees=6

void __fastcall FUN_008b2620(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00978ea5;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 9;
  FUN_0041a3d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8._0_1_ = 8;
  FUN_0041a3d0();
  local_8._0_1_ = 7;
  FUN_0041a3d0();
  local_8._0_1_ = 6;
  FUN_0041d190();
  *(undefined ***)(param_1 + 0x6dc) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xb;
  FUN_00417290();
  local_8._0_1_ = 10;
  FUN_00414c60();
  local_8._0_1_ = 5;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x594) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xd;
  FUN_00417290();
  local_8._0_1_ = 0xc;
  FUN_00414c60();
  local_8._0_1_ = 4;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x44c) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0xf;
  FUN_00417290();
  local_8._0_1_ = 0xe;
  FUN_00414c60();
  local_8._0_1_ = 3;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x304) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x11;
  FUN_00417290();
  local_8._0_1_ = 0x10;
  FUN_00414c60();
  local_8._0_1_ = 2;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x1bc) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x13;
  FUN_00417290();
  local_8._0_1_ = 0x12;
  FUN_00414c60();
  local_8._0_1_ = 1;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x74) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x15;
  FUN_00417290();
  local_8._0_1_ = 0x14;
  FUN_00414c60();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00418640();
  local_8 = 0xffffffff;
  FUN_0040e860();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008b25f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b25f0: 16-byte aligned: YES
  008b25f0  55             PUSH EBP
  008b25f1  8B EC          MOV EBP,ESP
  008b25f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b25f0
string literals (0):
direct callees (2):
  0x008B2620  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 560 B, RET 0
- callers 1: 0x008b25f0
- callees: 0x0041a3d0 CSWGuiButton::Destructor (CALL), 0x0041d190 (CALL), 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), 0x0040e860 CSWGuiPanel::Destructor (CALL)
- twins: none

## Card B17-8-15

GOG function: Global::Transpose  gog 0x0080c8b0  callconv=(blank)  param_size_bytes=8  gog_size=354  gog_retn=0
Candidate Steam address: 0x00514a20 (size 354, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00514b90:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080c590 Global::segmentproject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080ca20 Global::operator*  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*}
imm sets (* = in both, 3 common): GOG {0xc*,0x18*,0x74*} | Steam {0xc*,0x18*,0x74*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00514a20: 16-byte aligned: YES
  00514a20  55             PUSH EBP
  00514a21  8B EC          MOV EBP,ESP
  00514a23  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00514a20
string literals (0):
direct callees (0):
struct offsets touched (top 8):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00514a20 FUN_00514a20 size=354 ret=0 callers=1 callees=0

undefined4 * FUN_00514a20(undefined4 *param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  
  uVar1 = param_2[2 = byte +0x8];
  uVar2 = param_2[5 = byte +0x14];
  uVar3 = param_2[8 = byte +0x20];
  uVar4 = param_2[1 = byte +0x4];
  uVar5 = param_2[4 = byte +0x10];
  uVar6 = param_2[7 = byte +0x1c];
  uVar7 = *param_2;
  uVar8 = param_2[3 = byte +0xc];
  uVar9 = param_2[6 = byte +0x18];
  *param_1 = 0;
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[4 = byte +0x10] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  *param_1 = uVar7;
  param_1[1 = byte +0x4] = uVar8;
  param_1[2 = byte +0x8] = uVar9;
  param_1[3 = byte +0xc] = uVar4;
  param_1[4 = byte +0x10] = uVar5;
  param_1[5 = byte +0x14] = uVar6;
  param_1[6 = byte +0x18] = uVar1;
  param_1[7 = byte +0x1c] = uVar2;
  param_1[8 = byte +0x20] = uVar3;
  return param_1;
}
```

### decoy: 0x00514b90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00514b90: 16-byte aligned: YES
  00514b90  55             PUSH EBP
  00514b91  8B EC          MOV EBP,ESP
  00514b93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00514b90
string literals (0):
direct callees (0):
struct offsets touched (top 8):
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x8 [dec 8]  x5  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via ECX
```

### Static facts (script output, report-only)
- size 354 B, RET 0
- callers 1: 0x0044d110
- twins: none

## Card B17-8-16

GOG function: CServerAIBodyBagInfo::LoadBodyBag  gog 0x006509e0  callconv=(blank)  param_size_bytes=8  gog_size=147  gog_retn=8
Candidate Steam address: 0x00520f40 (size 122, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00520fc0:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00650960 CServerAIBodyBagInfo::SaveBodyBag  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00650a80 CSWGlobalVariableTable::Constructor -> 0x00653bc0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00624d60 CResGFF::ReadFieldDWORD -> Steam 0x00718c00  in candidate callees: no
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): 'PositionZ'; 'PositionY'; 'PositionX'; 'BodyBagId'
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
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
prologue for 0x00520f40: 16-byte aligned: YES
  00520f40  55             PUSH EBP
  00520f41  8B EC          MOV EBP,ESP
  00520f43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00520f40
string literals (4):
  0x00992EF0  at 00520f47  'BodyBagId'
  0x00992EE4  at 00520f5e  'PositionX'
  0x00992ED8  at 00520f79  'PositionY'
  0x00992ECC  at 00520f94  'PositionZ'
direct callees (2):
  0x00719B90  CResGFF::WriteFieldDWORD
  0x00719CE0  CResGFF::WriteFieldFLOAT
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00520f40 FUN_00520f40 size=122 ret=8 callers=1 callees=2

undefined4 __thiscall FUN_00520f40(undefined4 *param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_00719b90(param_3,*param_1,"BodyBagId");
  FUN_00719ce0(param_3,param_1[1 = byte +0x4],"PositionX");
  FUN_00719ce0(param_3,param_1[2 = byte +0x8],"PositionY");
  FUN_00719ce0(param_3,param_1[3 = byte +0xc],"PositionZ");
  return 1;
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
- size 122 B, RET 8
- callers 1: 0x00520190
- callees: 0x00719b90 CResGFF::WriteFieldDWORD (CALL), 0x00719ce0 CResGFF::WriteFieldFLOAT (CALL)
- strings: 0x00992ef0 "BodyBagId"
- twins: none

## Card B17-8-17

GOG function: CSWGuiInGameEquip::Draw  gog 0x0059f8a0  callconv=(blank)  param_size_bytes=4  gog_size=209  gog_retn=4
Candidate Steam address: 0x008abe30 (size 57, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008abd30:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0059f6b0 CSWGuiInGameEquip::SetDescription  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0059f980 CSWGuiInGameEquip::OnPanelAdded  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x0059ec10 CSWGuiInGameEquip::SetCharacter -> Steam ?  in candidate callees: n/a
  0x005a1780 CSWGuiInGameEquip::UpdateInventory -> Steam ?  in candidate callees: n/a
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x88,0x4ec4,0x50e4,0x50e8} | Steam {0x8,0x6c}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0x41,0x4ec4} | Steam {0x4*,0xb,0x1d0}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008abe30: 16-byte aligned: YES
  008abe30  55             PUSH EBP
  008abe31  8B EC          MOV EBP,ESP
  008abe33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008abe30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008abe30 FUN_008abe30 size=57 ret=4 callers=1 callees=0

void __thiscall FUN_008abe30(int *param_1,int param_2)

{
  if ((0 < param_2) && (param_2 < 0xb)) {
    (**(code **)(*param_1 + 0x8 [dec 8] {int*: byte +0x20}))(param_1 + param_2 * 0x74 + 0x1b,0);
  }
  return;
}
```

### decoy: 0x008abd30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008abd30: 16-byte aligned: YES
  008abd30  55             PUSH EBP
  008abd31  8B EC          MOV EBP,ESP
  008abd33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008abd30
string literals (0):
direct callees (6):
  0x0073F750  CClientExoApp::GetInGameGui
  0x007D2220  CGuiInGame::SetTopMenuOverrideCharacter
  0x008B0140  (unnamed)
  0x0041D290  CSWGuiListBox::ClearItems
  0x006169E0  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 7):
  +0x22A0 [dec 8864]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x22A4 [dec 8868]  x1  via EAX
  +0x5094 [dec 20628]  x1  via EAX
  +0x50CC [dec 20684]  x1  via ECX
  +0x50D0 [dec 20688]  x1  via EDX
  +0x50E8 [dec 20712]  x1  via EAX
```

### Static facts (script output, report-only)
- size 57 B, RET 4
- callers 1: 0x008a92d0
- indirect: 0x008abe61 slot 0x8
- twins: none

## Card B17-8-18

GOG function: CSWCollisionMesh::not_GetWalkableMaterialMask  gog 0x00713530  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00650de0 (size 26, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650d40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713490 Global::QueryTimer -> 0x00650d40  gate_class=REVIEW
- GOG-order neighbour after:  0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00650de0: 16-byte aligned: YES
  00650de0  55             PUSH EBP
  00650de1  8B EC          MOV EBP,ESP
  00650de3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00650de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00650de0 FUN_00650de0 size=26 ret=0 callers=1 callees=1

LARGE_INTEGER FUN_00650de0(void)

{
  LARGE_INTEGER local_c;
  
  QueryPerformanceFrequency(&local_c);
  return (LARGE_INTEGER)local_c.QuadPart;
}
```

### decoy: 0x00650d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650d40: 16-byte aligned: YES
  00650d40  55             PUSH EBP
  00650d41  8B EC          MOV EBP,ESP
  00650d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00650d40
string literals (0):
direct callees (3):
  0x00650DE0  (unnamed)
  0x0091FC30  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 26 B, RET 0
- callers 1: 0x00650d40
- callees: 0x000000d6 (CALL)
- imports: QueryPerformanceFrequency (KERNEL32.DLL)
- twins: none

## Card B17-8-19

GOG function: CSWSPlayerLUOInventory::Destructor  gog 0x007f2b50  callconv=(blank)  param_size_bytes=0  gog_size=164  gog_retn=0
Candidate Steam address: 0x006f2fb0 (size 164, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006f2f00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007f2aa0 CSWSPlayerLUOInventory::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007f2c00 CSWSPlayerLUOInventory::ClearRepository  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00758fa0 CExoLinkedListInternal::Destructor -> Steam ?  in candidate callees: n/a
  0x007f2c00 CSWSPlayerLUOInventory::ClearRepository -> Steam ?  in candidate callees: n/a
  0x007f2c50 CSWSPlayerLUOInventory::ClearContainer -> Steam ?  in candidate callees: n/a
  0x007f2ca0 CSWSPlayerLUOInventory::ClearStore -> Steam ?  in candidate callees: n/a
  0x007f2cf0 CSWSPlayerLUOInventory::ClearSlots -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xa4*} | Steam {0xa4*}
imm sets (* = in both, 4 common): GOG {0x2*,0xa0*,0xa8*,0xac*} | Steam {0x2*,0xa0*,0xa8*,0xac*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006f2fb0: 16-byte aligned: YES
  006f2fb0  55             PUSH EBP
  006f2fb1  8B EC          MOV EBP,ESP
  006f2fb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006f2fb0
string literals (0):
direct callees (5):
  0x006F3150  (unnamed)
  0x006F3060  (unnamed)
  0x006F30B0  (unnamed)
  0x006F3100  (unnamed)
  0x0072F5F0  (unnamed)
struct offsets touched (top 1):
  +0xA4 [dec 164]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006f2fb0 FUN_006f2fb0 size=164 ret=0 callers=1 callees=5

void __fastcall FUN_006f2fb0(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0095e8aa;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 2;
  FUN_006f3150(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  FUN_006f3060();
  FUN_006f30b0();
  FUN_006f3100();
  *(undefined4 *)(param_1 + 0xa4) = DAT_00994484;
  local_8._0_1_ = 1;
  FUN_0072f5f0();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  local_8 = 0xffffffff;
  FUN_0072f5f0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x006f2f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f2f00: 16-byte aligned: YES
  006f2f00  55             PUSH EBP
  006f2f01  8B EC          MOV EBP,ESP
  006f2f03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006f2f00
string literals (0):
direct callees (5):
  0x00722870  (unnamed)
  0x006F3150  (unnamed)
  0x006F3060  (unnamed)
  0x006F30B0  (unnamed)
  0x006F3100  (unnamed)
struct offsets touched (top 1):
  +0xA4 [dec 164]  x1  via EAX
```

### Static facts (script output, report-only)
- size 164 B, RET 0
- callers 1: 0x0062f920
- callees: 0x006f3150 (CALL), 0x006f3060 (CALL), 0x006f30b0 (CALL), 0x006f3100 (CALL), 0x0072f5f0 (CALL)
- twins: none

## Card B17-8-20

GOG function: CSWRoomSurfaceMesh::TransformAABBToWorld  gog 0x00714f60  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=8
Candidate Steam address: 0x00652810 (size 286, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006529c0:STR0/CALLEE0/SIG0.361/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00714f10 CSWRoomSurfaceMesh::TransformToWorld -> 0x006527c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00715080 CSWRoomSurfaceMesh::GetEdgeVertex  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00714f60 CSWRoomSurfaceMesh::TransformAABBToWorld -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x24*,0x28*,0xac*} | Steam {0x4*,0x8*,0x24*,0x28*,0xac*}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x2c*,0x74*} | Steam {0x8*,0xc*,0x2c*,0x74*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00652810: 16-byte aligned: YES
  00652810  55             PUSH EBP
  00652811  8B EC          MOV EBP,ESP
  00652813  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00652810
string literals (0):
direct callees (2):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00652810  Global::AurGrassAddInfo
struct offsets touched (top 3):
  +0xAC [dec 172]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00652810 FUN_00652810 size=286 ret=8 callers=2 callees=2

void __thiscall FUN_00652810(int param_1,int param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined1 local_34 [12];
  undefined1 local_28 [12];
  undefined1 local_1c [12];
  undefined1 local_10 [12];
  
  if (param_2 != -1) {
    iVar4 = param_2 * 0x2c + *(int *)(param_1 + 0xac);
    uVar1 = FUN_0044c8a0(local_10,param_2 * 0x2c + *(int *)(param_1 + 0xac) + 0xc,param_1 + 0x2c,
                         param_3);
    puVar2 = (undefined4 *)FUN_0044c8a0(local_1c,uVar1);
    *(undefined4 *)(iVar4 + 0xc) = *puVar2;
    *(undefined4 *)(iVar4 + 0x10) = puVar2[1 = byte +0x4];
    *(undefined4 *)(iVar4 + 0x14) = puVar2[2 = byte +0x8];
    puVar2 = (undefined4 *)(param_2 * 0x2c + *(int *)(param_1 + 0xac));
    uVar1 = FUN_0044c8a0(local_28,param_2 * 0x2c + *(int *)(param_1 + 0xac),param_1 + 0x2c,param_3);
    puVar3 = (undefined4 *)FUN_0044c8a0(local_34,uVar1);
    *puVar2 = *puVar3;
    puVar2[1 = byte +0x4] = puVar3[1 = byte +0x4];
    puVar2[2 = byte +0x8] = puVar3[2 = byte +0x8];
    FUN_00652810(*(undefined4 *)(*(int *)(param_1 + 0xac) + 0x24 + param_2 * 0x2c),param_3);
    FUN_00652810(*(undefined4 *)(*(int *)(param_1 + 0xac) + 0x28 + param_2 * 0x2c),param_3);
  }
  return;
}
```

### decoy: 0x006529c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006529c0: 16-byte aligned: YES
  006529c0  55             PUSH EBP
  006529c1  8B EC          MOV EBP,ESP
  006529c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006529c0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x54 [dec 84]  x1  via ECX
  +0x60 [dec 96]  x1  via ECX
  +0x8C [dec 140]  x1  via ECX
  +0x90 [dec 144]  x1  via EAX
```

### Static facts (script output, report-only)
- size 286 B, RET 8
- callers 2: 0x006527c0, 0x00652810
- callees: 0x0044c8a0 (CALL), 0x00652810 (CALL)
- twins: none

## Card B17-8-21

GOG function: CSWRoomSurfaceMesh::ComputeClippedLineSegment  gog 0x00714000  callconv=(blank)  param_size_bytes=36  gog_size=1366  gog_retn=36
Candidate Steam address: 0x006518b0 (size 1366, retn 36)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x00714580:0.56>0.34
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: TestSafeLocationPoint 0x54b650); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00651e30:STR0/CALLEE0/SIG0.433/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007137e0 CSWRoomSurfaceMesh::ClippedLineSegmentWalkable -> 0x00651090  gate_class=REVIEW
- GOG-order neighbour after:  0x00714580 CSWRoomSurfaceMesh::FindTriangle  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00714900 CSWRoomSurfaceMesh::IsPointInPolygon -> Steam ?  in candidate callees: n/a
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007059f0 CSWSArea::TestSafeLocationPoint -> Steam 0x0054b650  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xbc*} | Steam {0x4*,0x8*,0xbc*}
imm sets (* = in both, 8 common): GOG {0x2*,0x5*,0x8*,0xc*,0x24*,0x15c*,0x80000001*,0xfffffffe*} | Steam {0x2*,0x5*,0x8*,0xc*,0x24*,0x15c*,0x80000001*,0xfffffffe*}

### abi
```
GOG param bytes: 36; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(36/4)*4 = 36 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=36 ; asm RET = 36
Stack Slot Match: TRUE (GOG 36 bytes -> x86 slots 36; ASM RET 36)
```

### prologue
```
prologue for 0x006518b0: 16-byte aligned: YES
  006518b0  55             PUSH EBP
  006518b1  8B EC          MOV EBP,ESP
  006518b3  81 EC 5C 01 00 00 SUB ESP,0x15c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006518b0
string literals (0):
direct callees (4):
  0x00517980  Vec3::Equal
  0x006521B0  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x24  via EAX,ECX,EDX
  +0x8 [dec 8]  x21  via EAX,ECX,EDX
  +0xBC [dec 188]  x10  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006518b0 FUN_006518b0 size=1366 ret=36 callers=2 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall
FUN_006518b0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            ,float param_6,undefined4 param_7,undefined4 *param_8,undefined4 *param_9,
            undefined4 param_10)

{
  undefined4 *puVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  float10 fVar5;
  int local_cc;
  undefined4 *local_c8;
  undefined1 local_c4 [12];
  undefined1 local_b8 [12];
  undefined1 local_ac [12];
  undefined1 local_a0 [12];
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined4 local_80;
  undefined1 local_7c [12];
  undefined1 local_70 [12];
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined1 local_58 [12];
  undefined1 local_4c [12];
  uint local_40;
  float local_3c;
  undefined4 local_34;
  float local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_10 = param_2;
  local_c = param_3;
  local_8 = 0;
  local_34 = param_5;
  local_30 = param_6;
  local_2c = 0;
  local_cc = 2;
  local_c8 = &local_28;
  while (local_cc = local_cc + -1, -1 < local_cc) {
    *local_c8 = 0;
    local_c8[1 = byte +0x4] = 0;
    local_c8[2 = byte +0x8] = 0;
    local_c8 = local_c8 + 0x3 [dec 3] {undefined4*: byte +0xc};
  }
  iVar2 = FUN_00517980(&local_10,&local_34);
  if (iVar2 == 0) {
    local_40 = FUN_006521b0(&local_10,&local_34,param_10);
    uVar4 = FUN_004452b0(local_4c,&local_34,&local_10);
    uVar4 = FUN_004452b0(local_58,&local_34,&local_10,uVar4);
    fVar5 = (float10)FUN_0044b8a0(uVar4);
    local_3c = (float)fVar5;
    if (local_40 == 0) {
      *param_8 = param_2;
      param_8[1 = byte +0x4] = param_3;
      param_8[2 = byte +0x8] = param_4;
      *param_9 = param_5;
      param_9[1 = byte +0x4] = param_6;
      param_9[2 = byte +0x8] = param_7;
      uVar4 = 0;
    }
    else {
      uVar3 = local_40 & 0x80000001;
/* ... 115 more lines truncated by --max-lines */
```

### decoy: 0x00651e30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00651e30: 16-byte aligned: YES
  00651e30  55             PUSH EBP
  00651e31  8B EC          MOV EBP,ESP
  00651e33  83 EC 68       SUB ESP,0x68
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00651e30
string literals (0):
direct callees (5):
  0x005EE520  (unnamed)
  0x0064E080  (unnamed)
  0x00650AB0  (unnamed)
  0x00650BE0  (unnamed)
  0x005143C0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1366 B, RET 36
- callers 2: 0x0054b650 CSWSArea::TestSafeLocationPoint, 0x00651090
- callees: 0x00517980 (CALL), 0x006521b0 (CALL), 0x004452b0 (CALL), 0x0044b8a0 (CALL)
- twins: none

## Card B17-8-22

GOG function: CSWRoomSurfaceMesh::TransformToWorld  gog 0x00714f10  callconv=(blank)  param_size_bytes=4  gog_size=78  gog_retn=4
Candidate Steam address: 0x006527c0 (size 78, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00652810:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00714e40 CSWRoomSurfaceMesh::AddCrossing  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00714f60 CSWRoomSurfaceMesh::TransformAABBToWorld -> 0x00652810  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00714f60 CSWRoomSurfaceMesh::TransformAABBToWorld -> Steam ?  in candidate callees: n/a
  0x007281b0 CSWCollisionMesh::TransformToWorld -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4c*,0xb0*,0xd4*} | Steam {0x4c*,0xb0*,0xd4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006527c0: 16-byte aligned: YES
  006527c0  55             PUSH EBP
  006527c1  8B EC          MOV EBP,ESP
  006527c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006527c0
string literals (0):
direct callees (2):
  0x005EE290  CSWSPlaceable::DoDamage
  0x00652810  Global::AurGrassAddInfo
struct offsets touched (top 3):
  +0x4C [dec 76]  x1  via EAX
  +0xB0 [dec 176]  x1  via EDX
  +0xD4 [dec 212]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006527c0 FUN_006527c0 size=78 ret=4 callers=0 callees=2

void __thiscall FUN_006527c0(int param_1,undefined4 param_2)

{
  if ((*(int *)(param_1 + 0x4c) == 0) && (FUN_005ee290(param_2), *(int *)(param_1 + 0xb0) != 0)) {
    FUN_00652810(*(undefined4 *)(param_1 + 0xd4),param_2);
  }
  return;
}
```

### decoy: 0x00652810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00652810: 16-byte aligned: YES
  00652810  55             PUSH EBP
  00652811  8B EC          MOV EBP,ESP
  00652813  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00652810
string literals (0):
direct callees (2):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00652810  Global::AurGrassAddInfo
struct offsets touched (top 3):
  +0xAC [dec 172]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
```

### Static facts (script output, report-only)
- size 78 B, RET 4
- callees: 0x005ee290 CSWSPlaceable::DoDamage (CALL), 0x00652810 (CALL)
- twins: none
