# Review cards B17 chunk 14

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-14-01

GOG function: Global::Transpose  gog 0x0080c8b0  callconv=(blank)  param_size_bytes=8  gog_size=354  gog_retn=0
Candidate Steam address: 0x005147e0 (size 561, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00514a20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080c590 Global::segmentproject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080ca20 Global::operator*  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0xc,0x10,0x14,0x18,0x1c,0x20} | Steam {0x4*,0x8*}
imm sets (* = in both, 0 common): GOG {0xc,0x18,0x74} | Steam {0x5,0x8,0x41,0x44,0x70}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x005147e0: 16-byte aligned: YES
  005147e0  55             PUSH EBP
  005147e1  8B EC          MOV EBP,ESP
  005147e3  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005147e0
string literals (0):
direct callees (1):
  0x00517980  Vec3::Equal
struct offsets touched (top 2):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005147e0 FUN_005147e0 size=561 ret=0 callers=2 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * FUN_005147e0(float *param_1,float *param_2,float *param_3,float *param_4)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  float local_74;
  float local_70;
  float local_8;
  
  iVar5 = FUN_00517980(param_2,param_3);
  if (iVar5 == 0) {
    if ((DAT_00a7d79c & 1) == 0) {
      DAT_00a7d79c = DAT_00a7d79c | 1;
    }
    _DAT_00a7d790 = *param_3 - *param_2;
    _DAT_00a7d794 = param_3[1 = byte +0x4] - param_2[1 = byte +0x4];
    _DAT_00a7d798 = param_3[2 = byte +0x8] - param_2[2 = byte +0x8];
    local_8 = _DAT_00a7d790 * _DAT_00a7d790 + _DAT_00a7d794 * _DAT_00a7d794 +
              _DAT_00a7d798 * _DAT_00a7d798;
    if (local_8 != (float)_DAT_00986b38) {
      local_8 = ((*param_4 - *param_2) * _DAT_00a7d790 + (param_4[1 = byte +0x4] - param_2[1 = byte +0x4]) * _DAT_00a7d794 +
                (param_4[2 = byte +0x8] - param_2[2 = byte +0x8]) * _DAT_00a7d798) / local_8;
    }
    if (local_8 <= (float)_DAT_00986b50) {
      local_70 = local_8;
    }
    else {
      local_70 = 1.0;
    }
    if ((float)_DAT_00986b38 <= local_70) {
      local_74 = local_70;
    }
    else {
      local_74 = 0.0;
    }
    fVar3 = local_74 * _DAT_00a7d794;
    fVar4 = local_74 * _DAT_00a7d798;
    fVar1 = param_2[1 = byte +0x4];
    fVar2 = param_2[2 = byte +0x8];
    *param_1 = local_74 * _DAT_00a7d790 + *param_2;
    param_1[1 = byte +0x4] = fVar3 + fVar1;
    param_1[2 = byte +0x8] = fVar4 + fVar2;
  }
  else {
    *param_1 = *param_2;
    param_1[1 = byte +0x4] = param_2[1 = byte +0x4];
    param_1[2 = byte +0x8] = param_2[2 = byte +0x8];
  }
  return param_1;
}
```

### decoy: 0x00514a20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00514a20: 16-byte aligned: YES
  00514a20  55             PUSH EBP
  00514a21  8B EC          MOV EBP,ESP
  00514a23  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 561 B, RET 0
- callers 2: 0x004efbd0, 0x0062e320 CSWSAreaOfEffectObject::UpdateSubAreas
- callees: 0x00517980 (CALL)
- twins: none

## Card B17-14-02

GOG function: NewController::Control  gog 0x008f8b50  callconv=(blank)  param_size_bytes=20  gog_size=1242  gog_retn=20
Candidate Steam address: 0x004dea20 (size 1242, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004de880:STR0/CALLEE0/SIG0.568/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f89b0 NewController::GetQuaternionValue  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f9030 MdlNode::InternalControl  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0080d220 Global::slerp -> Steam ?  in candidate callees: n/a
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00846bb0 MdlNode::AsMdlNodeTriMesh -> Steam ?  in candidate callees: n/a
  0x008f7dd0 NewController::GetFloatValue -> Steam ?  in candidate callees: n/a
  0x008f7fd0 NewController::GetVectorValue -> Steam ?  in candidate callees: n/a
  0x008f89b0 NewController::GetQuaternionValue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x50*,0x189*} | Steam {0x4*,0x8*,0xc*,0x50*,0x189*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0xc*,0xf*,0x10*,0x14*,0x1c*,0x24*,0x12c*} | Steam {0x2*,0x3*,0x4*,0x5*,0xc*,0xf*,0x10*,0x14*,0x1c*,0x24*,0x12c*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x004dea20: 16-byte aligned: YES
  004dea20  55             PUSH EBP
  004dea21  8B EC          MOV EBP,ESP
  004dea23  81 EC 2C 01 00 00 SUB ESP,0x12c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dea20
string literals (0):
direct callees (8):
  0x004DDC40  (unnamed)
  0x004545A0  (unnamed)
  0x00452830  (unnamed)
  0x004DDE40  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x004DE880  (unnamed)
  0x00515390  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x18  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,EDX
  +0xC [dec 12]  x6  via EAX,EDX
  +0x50 [dec 80]  x1  via ECX
  +0x189 [dec 393]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004dea20 FUN_004dea20 size=1242 ret=20 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_004dea20(int *param_1,int param_2,undefined4 param_3,float param_4,undefined4 param_5,
            float param_6)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_100 [12];
  undefined1 local_f4 [36];
  undefined1 local_d0 [12];
  undefined1 local_c4 [12];
  undefined4 local_b8;
  undefined4 local_b4;
  undefined4 local_b0;
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
  undefined4 *local_80;
  undefined4 local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 *local_4c;
  float *local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  float local_38;
  float local_34;
  float local_30;
  undefined4 local_2c;
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_1c;
  int local_14;
  float *local_10;
  float *local_c;
  uint local_8;
  
  local_8 = *(byte *)(param_1 + 0x3 [dec 3] {int*: byte +0xc}) & 0xf;
  if (local_8 == 1) {
    local_c = (float *)(param_2 + *param_1);
    _DAT_00a7bc80 = *local_c;
    FUN_004ddc40(param_5,param_3,&DAT_00a7bc7c);
    if ((short)param_1[1 = byte +0x4] != -1) {
      local_10 = (float *)((int)(short)param_1[1 = byte +0x4] + *(int *)(param_2 + 4));
      _DAT_00a7bc7c = _DAT_00a7bc7c + *local_10;
    }
    if ((float)_DAT_00991340 <= param_4) {
      FUN_004ddc40(param_4,param_3,&DAT_00a7bc78);
      *local_c = (1.0 - param_6) * _DAT_00a7bc78 + param_6 * _DAT_00a7bc7c;
    }
    else {
      *local_c = (1.0 - param_6) * _DAT_00a7bc80 + param_6 * _DAT_00a7bc7c;
    }
    if ((((*param_1 == 0x24) && (local_14 = FUN_004545a0(), local_14 != 0)) &&
        (iVar1 = FUN_00452830(), iVar1 != 0)) && ((*(ushort *)(iVar1 + 0x50) & 0x10) != 0)) {
      *(undefined1 *)(local_14 + 0x189) = 1;
    }
  }
/* ... 76 more lines truncated by --max-lines */
```

### decoy: 0x004de880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004de880: 16-byte aligned: YES
  004de880  55             PUSH EBP
  004de881  8B EC          MOV EBP,ESP
  004de883  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004de880
string literals (0):
direct callees (3):
  0x004DDB60  (unnamed)
  0x004DE630  (unnamed)
  0x00515390  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x6 [dec 6]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 1242 B, RET 20
- callers 1: 0x004def00
- callees: 0x004ddc40 (CALL), 0x004545a0 (CALL), 0x00452830 (CALL), 0x004dde40 (CALL), 0x00436200 (CALL), 0x0044c8a0 (CALL), ...
- twins: none

## Card B17-14-03

GOG function: LightManager::GetShadowLights  gog 0x00901f50  callconv=(blank)  param_size_bytes=4  gog_size=1048  gog_retn=4
Candidate Steam address: 0x0050a630 (size 1048, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0050a050:STR0/CALLEE0/SIG0.096/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00901970 LightManager::ControlLightsFade  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00902370 LightManager::GetBumpMapLights  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00856330 CExoArrayList::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00873000 List_T__::operator= -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x008fdd20 List_T__::Insert -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xc*,0x50*,0x54*,0x58*,0x5c*,0x60*,0x68*,0x70*,0x80*,0x90*,0x98*,0x9c*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x50*,0x54*,0x58*,0x5c*,0x60*,0x68*,0x70*,0x80*,0x90*,0x98*,0x9c*,...(+1)}
imm sets (* = in both, 9 common): GOG {0x2*,0x4*,0x5*,0x6*,0x41*,0x44*,0x98*,0xac*,0xb8*} | Steam {0x2*,0x4*,0x5*,0x6*,0x41*,0x44*,0x98*,0xac*,0xb8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0050a630: 16-byte aligned: YES
  0050a630  55             PUSH EBP
  0050a631  8B EC          MOV EBP,ESP
  0050a633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0050a630
string literals (0):
direct callees (6):
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x00463ED0  (unnamed)
  0x00473FB0  (unnamed)
  0x00476620  (unnamed)
  0x0050C380  (unnamed)
  0x00463B20  (unnamed)
struct offsets touched (top 10):
  +0x90 [dec 144]  x9  via EAX,ECX,EDX
  +0x9C [dec 156]  x6  via EAX,ECX,EDX
  +0x58 [dec 88]  x2  via ECX
  +0x60 [dec 96]  x2  via EAX,EDX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x98 [dec 152]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x50 [dec 80]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0050a630 FUN_0050a630 size=1048 ret=4 callers=4 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_0050a630(int param_1,int param_2)

{
  int iVar1;
  float fVar2;
  int iVar3;
  int local_30;
  int local_2c;
  int local_24;
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094d3a8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x9c) = 0;
  if (param_2 == 2) {
    local_1c = 0;
    local_24 = 6;
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0xc); local_14 = local_14 + 1) {
      iVar1 = *(int *)(*(int *)(param_1 + 8) + local_14 * 4);
      if ((((*(int *)(iVar1 + 0x5c) != 0) && (*(int *)(iVar1 + 0x68) == 0)) &&
          (((float)_DAT_00986b48 <= *(float *)(iVar1 + 0x58) || (*(int *)(iVar1 + 0x70) == 0)))) &&
         (iVar3 = *(int *)(*(int *)(iVar1 + 4) + 0x90), iVar3 < local_24)) {
        local_24 = iVar3;
        local_1c = iVar1;
      }
    }
    if (local_1c != 0) {
      FUN_00473f40(local_1c);
      *(int *)(param_1 + 0xa4) = local_1c;
    }
  }
  else {
    local_14 = 0;
    while (((*(int *)(param_1 + 0x9c) < DAT_00a0f040 && (local_14 < *(int *)(param_1 + 0x54))) &&
           (((iVar1 = *(int *)(*(int *)(param_1 + 0x50) + local_14 * 4), param_2 == 1 &&
             (*(int *)(iVar1 + 0x70) == 0)) ||
            (((float)_DAT_00986b50 < *(float *)(iVar1 + 0x80) || (FUN_00473f40(iVar1), param_2 != 1)
             )))))) {
      local_14 = local_14 + 1;
    }
    FUN_00463ed0(0);
    local_8 = 0;
    if (param_2 == 1) {
      FUN_00473fb0(param_1 + 0xb8);
    }
    else {
      FUN_00473fb0(param_1 + 0xac);
    }
    for (local_14 = 0; local_14 < *(int *)(param_1 + 0x9c); local_14 = local_14 + 1) {
      iVar1 = *(int *)(*(int *)(param_1 + 0x98) + local_14 * 4);
      iVar3 = FUN_00476620(iVar1);
      if (iVar3 == 0) {
        *(undefined4 *)(iVar1 + 0x90) = 0;
      }
    }
    for (local_14 = 0; (local_14 < DAT_00a0f040 && (local_14 < local_2c)); local_14 = local_14 + 1)
    {
      iVar1 = *(int *)(local_30 + local_14 * 4);
      iVar3 = FUN_00476620(iVar1);
      if (iVar3 == 0) {
        if ((float)_DAT_00986b38 <=
            *(float *)(iVar1 + 0x90) - (_DAT_009f62b0 * (float)_DAT_00987490) / _DAT_00a0f050) {
          fVar2 = *(float *)(iVar1 + 0x90) - (_DAT_009f62b0 * (float)_DAT_00987490) / _DAT_00a0f050;
        }
        else {
          fVar2 = 0.0;
        }
        *(float *)(iVar1 + 0x90) = fVar2;
        if ((float)_DAT_00986b38 < *(float *)(iVar1 + 0x90)) {
          FUN_0050c380(iVar1,0);
        }
/* ... 27 more lines truncated by --max-lines */
```

### decoy: 0x0050a050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0050a050: 16-byte aligned: YES
  0050a050  55             PUSH EBP
  0050a051  8B EC          MOV EBP,ESP
  0050a053  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0050a050
string literals (0):
direct callees (8):
  0x00476620  (unnamed)
  0x004142E0  (unnamed)
  0x0049B840  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x0091E140  (unnamed)
  0x00436200  (unnamed)
  0x00508350  (unnamed)
  0x00473FB0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0xA8 [dec 168]  x2  via EAX
  +0x40 [dec 64]  x1  via EDX
  +0x8C [dec 140]  x1  via EDX
```

### Static facts (script output, report-only)
- size 1048 B, RET 4
- callers 4: 0x004687e0, 0x004691d0, 0x00469310, 0x00497e60
- callees: 0x00473f40 (CALL), 0x00463ed0 (CALL), 0x00473fb0 (CALL), 0x00476620 (CALL), 0x0050c380 (CALL), 0x00463b20 (CALL)
- twins: none

## Card B17-14-04

GOG function: CSWGuiObject::SetExtent  gog 0x00912f40  callconv=__thiscall  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00414b70 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00414b50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: autolabel:SetRect
- GOG-order neighbour before: 0x00912e20 CSWGuiBorder::Constructor -> 0x00414a50  trusted
- GOG-order neighbour after:  0x00912f70 CRes::GetData  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00414b70: 16-byte aligned: YES
  00414b70  55             PUSH EBP
  00414b71  8B EC          MOV EBP,ESP
  00414b73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00414b70
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00414b70 CMFCCaptionButtonEx::SetRect size=44 ret=4 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: void __thiscall CMFCCaptionButtonEx::SetRect(class CRect const &)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CMFCCaptionButtonEx::SetRect(CMFCCaptionButtonEx *this,CRect *param_1)

{
  *(undefined4 *)(this + 4) = *(undefined4 *)param_1;
  *(undefined4 *)(this + 8) = *(undefined4 *)(param_1 + 4);
  *(undefined4 *)(this + 0xc) = *(undefined4 *)(param_1 + 8);
  *(undefined4 *)(this + 0x10) = *(undefined4 *)(param_1 + 0xc);
  return;
}
```

### decoy: 0x00414b50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414b50: 16-byte aligned: YES
  00414b50  55             PUSH EBP
  00414b51  8B EC          MOV EBP,ESP
  00414b53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414b50
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- twins: none

## Card B17-14-05

GOG function: Global::ProceduralAnimateHierarchy  gog 0x008f9cd0  callconv=(blank)  param_size_bytes=4  gog_size=815  gog_retn=0
Candidate Steam address: 0x004dfba0 (size 815, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004df160:STR0/CALLEE0/SIG0.286/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008f9290 MdlNodeAnimMesh::InternalControl  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fa000 Global::AnimateHierarchy -> 0x004dfed0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x008255f0 GLRender::LockPool -> Steam ?  in candidate callees: n/a
  0x00825620 GLRender::UnlockPool -> Steam ?  in candidate callees: n/a
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00846bb0 MdlNode::AsMdlNodeTriMesh -> Steam ?  in candidate callees: n/a
  0x008f9cd0 Global::ProceduralAnimateHierarchy -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
  0x00925936 Global::rand -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0x2c*,0x30*,0x34*,0x78*,0x90*,0xac*,0x138*,0x13c*,0x140*,0x144*,0x148*,0x14c*,...(+3)} | Steam {0x4*,0x8*,0x2c*,0x30*,0x34*,0x78*,0x90*,0xac*,0x138*,0x13c*,0x140*,0x144*,0x148*,0x14c*,...(+3)}
imm sets (* = in both, 4 common): GOG {0x4*,0x5*,0x10*,0x44*} | Steam {0x4*,0x5*,0x10*,0x44*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
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

### decoy: 0x004df160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004df160: 16-byte aligned: YES
  004df160  55             PUSH EBP
  004df161  8B EC          MOV EBP,ESP
  004df163  81 EC BC 02 00 00 SUB ESP,0x2bc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004df160
string literals (0):
direct callees (6):
  0x0091C860  _ftol2_sse
  0x00432EE0  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00432F10  (unnamed)
  0x004DF120  (unnamed)
struct offsets touched (top 10):
  +0x1CC [dec 460]  x9  via EAX,EDX
  +0x1D8 [dec 472]  x7  via EAX,ECX,EDX
  +0x70 [dec 112]  x6  via EAX,ECX,EDX
  +0x1D0 [dec 464]  x6  via ECX,EDX
  +0x1D4 [dec 468]  x6  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x180 [dec 384]  x4  via EAX,EDX
  +0x1A4 [dec 420]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EDX
  +0x20 [dec 32]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 815 B, RET 0
- callers 2: 0x004dfba0, 0x004e1d40
- callees: 0x004545a0 (CALL), 0x0091c860 (CALL), 0x00432ee0 (CALL), 0x0091eb8b (CALL), 0x00452830 (CALL), 0x00432f10 (CALL), ...
- indirect: 0x004dfbc1 slot 0x2c
- twins: none

## Card B17-14-06

GOG function: Global::Transpose  gog 0x0080c8b0  callconv=(blank)  param_size_bytes=8  gog_size=354  gog_retn=0
Candidate Steam address: 0x00514b90 (size 164, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.67 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00514a20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080c590 Global::segmentproject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080ca20 Global::operator*  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*}
imm sets (* = in both, 1 common): GOG {0xc*,0x18,0x74} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00514b90: 16-byte aligned: YES
  00514b90  55             PUSH EBP
  00514b91  8B EC          MOV EBP,ESP
  00514b93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00514b90 FUN_00514b90 size=164 ret=0 callers=1 callees=0

float * FUN_00514b90(float *param_1,float *param_2,float *param_3)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  float fVar12;
  
  fVar1 = param_3[2 = byte +0x8];
  fVar2 = param_2[7 = byte +0x1c];
  fVar3 = param_3[1 = byte +0x4];
  fVar4 = param_2[4 = byte +0x10];
  fVar5 = *param_3;
  fVar6 = param_2[1 = byte +0x4];
  fVar7 = param_3[2 = byte +0x8];
  fVar8 = param_2[8 = byte +0x20];
  fVar9 = param_3[1 = byte +0x4];
  fVar10 = param_2[5 = byte +0x14];
  fVar11 = *param_3;
  fVar12 = param_2[2 = byte +0x8];
  *param_1 = *param_3 * *param_2 + param_3[1 = byte +0x4] * param_2[3 = byte +0xc] + param_3[2 = byte +0x8] * param_2[6 = byte +0x18];
  param_1[1 = byte +0x4] = fVar5 * fVar6 + fVar3 * fVar4 + fVar1 * fVar2;
  param_1[2 = byte +0x8] = fVar11 * fVar12 + fVar9 * fVar10 + fVar7 * fVar8;
  return param_1;
}
```

### decoy: 0x00514a20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00514a20: 16-byte aligned: YES
  00514a20  55             PUSH EBP
  00514a21  8B EC          MOV EBP,ESP
  00514a23  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 164 B, RET 0
- callers 1: 0x004c0dd0
- twins: none

## Card B17-14-07

GOG function: CSWGuiManager::UpdateAllFonts  gog 0x00912250  callconv=(blank)  param_size_bytes=0  gog_size=191  gog_retn=0
Candidate Steam address: 0x00413dd0 (size 191, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00413e90:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00912160 CSWGuiManager::GetUpdatedFontName  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00912310 CSWGuiManager::DisplayToolTip  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x004e2390 CGuiInGame::CopyQuickSaveGameToFutureGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x18*,0x3c*,0x6c*,0x88*,0x8c*} | Steam {0x4*,0x18*,0x3c*,0x6c*,0x88*,0x8c*}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00413dd0: 16-byte aligned: YES
  00413dd0  55             PUSH EBP
  00413dd1  8B EC          MOV EBP,ESP
  00413dd3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00413dd0
string literals (0):
direct callees (2):
  0x0073F750  CClientExoApp::GetInGameGui
  0x007D1C90  CGuiInGame::CopyQuickSaveGameToFutureGame
struct offsets touched (top 6):
  +0x3C [dec 60]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x6C [dec 108]  x2  via EAX,EDX
  +0x88 [dec 136]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EDX
  +0x8C [dec 140]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00413dd0 FUN_00413dd0 size=191 ret=0 callers=2 callees=2

void __fastcall FUN_00413dd0(int param_1)

{
  int iVar1;
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < *(int *)(param_1 + 0x8c); local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(*(int *)(param_1 + 0x88) + local_8 * 4) + 0x18) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0x88) + local_8 * 4) + 0x6c))();
    }
  }
  if (*(int *)(param_1 + 0x3c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x3c) + 0x6c))();
  }
  iVar1 = FUN_0073f750();
  if (iVar1 != 0) {
    FUN_0073f750();
    FUN_007d1c90();
  }
  return;
}
```

### decoy: 0x00413e90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00413e90: 16-byte aligned: YES
  00413e90  55             PUSH EBP
  00413e91  8B EC          MOV EBP,ESP
  00413e93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00413e90
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0075FF30  (unnamed)
  0x0075FF50  (unnamed)
struct offsets touched (top 4):
  +0x1C [dec 28]  x3  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
```

### Static facts (script output, report-only)
- size 191 B, RET 0
- callers 2: 0x00411a80, 0x007bc5a0
- callees: 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x007d1c90 (CALL)
- indirect: 0x00413e39 slot 0x6c, 0x00413e57 slot 0x6c
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B17-14-08

GOG function: CAuroraInterface::GetModelResHelper  gog 0x00908190  callconv=(blank)  param_size_bytes=16  gog_size=771  gog_retn=0
Candidate Steam address: 0x0093db70 (size 771, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0093de80:STR0/CALLEE0/SIG0.556/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00908170 CAuroraInterface::ReleaseVisibilityInternal_thunk  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009084a0 CAuroraInterface::GetModelInternal  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  0x00605d50 CExoString::operator> -> Steam ?  in candidate callees: n/a
  0x006067a0 CExoString::LowerCase -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  ... 8 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0xc*,0x10*,0x1c*,0x70*,0x7d2*} | Steam {0x2*,0x3*,0x4*,0xc*,0x10*,0x1c*,0x70*,0x7d2*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 16 bytes -> x86 slots 16; ASM RET 0)
```

### prologue
```
prologue for 0x0093db70: 16-byte aligned: YES
  0093db70  55             PUSH EBP
  0093db71  8B EC          MOV EBP,ESP
  0093db73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0093db70
string literals (0):
direct callees (16):
  0x00733540  CExoString::Constructor
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x00710930  (unnamed)
  0x007339D0  CExoString::operator==
  0x0064DDE0  (unnamed)
  0x00733C40  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x00710810  (unnamed)
  0x0093D050  (unnamed)
  0x00711250  (unnamed)
  0x0093D160  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093db70 FUN_0093db70 size=771 ret=0 callers=1 callees=16

undefined4 * FUN_0093db70(undefined4 param_1,int param_2,undefined4 *param_3,int *param_4)

{
  int iVar1;
  undefined1 auStack_98 [12];
  undefined4 uStack_8c;
  int *piStack_88;
  uint uStack_84;
  undefined4 local_80;
  undefined4 *local_7c;
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  int local_64;
  undefined4 *local_60;
  undefined4 *local_5c;
  undefined4 *local_58;
  undefined4 *local_54;
  undefined1 *local_50;
  int local_4c;
  undefined4 *local_48;
  undefined4 *local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined4 *local_30;
  int *local_2c;
  undefined1 local_28 [8];
  int local_20;
  undefined4 *local_1c;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00982e6b;
  local_10 = ExceptionList;
  uStack_84 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  piStack_88 = (int *)0x93db9d;
  FUN_00733540();
  local_8 = 0;
  piStack_88 = (int *)0x93dbac;
  FUN_00733540();
  local_8._0_1_ = 1;
  piStack_88 = (int *)local_38;
  uStack_8c = 0x93dbbc;
  piStack_88 = (int *)FUN_00734690();
  local_8._0_1_ = 2;
  uStack_8c = 0x93dbd5;
  local_70 = piStack_88;
  local_6c = piStack_88;
  FUN_007337c0();
  local_8._0_1_ = 1;
  piStack_88 = (int *)0x93dbe1;
  FUN_00733780();
  piStack_88 = (int *)0x93dbeb;
  local_20 = FUN_007a1720();
  piStack_88 = (int *)0x93dbf8;
  local_1c = (undefined4 *)FUN_00521060();
  if (param_4 != (int *)0x0) {
    *param_4 = 0;
  }
  do {
    if (local_20 == 0) {
LAB_0093dd19:
      if (local_1c == (undefined4 *)0x0) {
        piStack_88 = (int *)0x1c;
        uStack_8c = 0x93dd2a;
        local_4c = FUN_00919723();
        local_8._0_1_ = 4;
        if (local_4c == 0) {
          local_7c = (undefined4 *)0x0;
        }
        else {
          piStack_88 = (int *)param_2;
          local_50 = auStack_98;
/* ... 118 more lines truncated by --max-lines */
```

### decoy: 0x0093de80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093de80: 16-byte aligned: YES
  0093de80  55             PUSH EBP
  0093de81  8B EC          MOV EBP,ESP
  0093de83  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093de80
string literals (0):
direct callees (3):
  0x0093DB70  (unnamed)
  0x0093D110  (unnamed)
  0x00711250  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 771 B, RET 0
- callers 1: 0x0093de80
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00734690 CExoString::ToLower (CALL), 0x007337c0 CExoString::operator= (CALL), 0x00733780 CExoString::Destructor (CALL), 0x007a1720 (CALL), 0x00521060 (CALL), ...
- indirect: 0x0093de2e slot 0x0
- twins: none

## Card B17-14-09

GOG function: Global::AnimateHierarchy  gog 0x008fa000  callconv=(blank)  param_size_bytes=21  gog_size=614  gog_retn=0
Candidate Steam address: 0x004dfed0 (size 614, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: IsInCutscene 0x459e20); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004dfba0:STR0/CALLEE0/SIG0.163/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008f9cd0 Global::ProceduralAnimateHierarchy -> 0x004dfba0  gate_class=REVIEW
- GOG-order neighbour after:  0x008fa270 Part::Animate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00844e40 MaxTree::AsModel -> Steam ?  in candidate callees: n/a
  0x00845e70 MdlNode::ControlDispatch -> Steam ?  in candidate callees: n/a
  0x0084c400 Gob::IsInCutscene -> Steam 0x00459e20  in candidate callees: yes
  0x008fa000 Global::AnimateHierarchy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x2*,0x4*,0x8*,0x16*,0x28*,0x2c*,0x30*,0x34*,0x40*,0x48*,0x58*,0xa8*,0x164*} | Steam {0x2*,0x4*,0x8*,0x16*,0x28*,0x2c*,0x30*,0x34*,0x40*,0x48*,0x58*,0xa8*,0x164*}
imm sets (* = in both, 5 common): GOG {0x5*,0x18*,0x28*,0x41*,0x64*} | Steam {0x5*,0x18*,0x28*,0x41*,0x64*}

### abi
```
GOG param bytes: 21; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(21/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 21 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x004dfed0: 16-byte aligned: YES
  004dfed0  55             PUSH EBP
  004dfed1  8B EC          MOV EBP,ESP
  004dfed3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004dfed0 FUN_004dfed0 size=614 ret=0 callers=2 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004dfed0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5,
                 char param_6)

{
  char cVar1;
  int iVar2;
  float local_2c;
  int local_14;
  int local_10;
  int local_8;
  
  DAT_00a7bc20 = DAT_00a7bc20 + 1;
  if (DAT_00a7bc24 < DAT_00a7bc20) {
    DAT_00a7bc24 = DAT_00a7bc20;
  }
  if (((param_6 == '\0') && (param_1 != 0)) &&
     (iVar2 = FUN_00452830(), *(int *)(param_1 + 4) == *(int *)(iVar2 + 0xa8))) {
    param_6 = '\x01';
  }
  if ((*(int *)(param_1 + 0x48) == 0) && (param_6 != '\0')) {
    FUN_00453860(param_1,param_3,param_4,param_5);
  }
  local_10 = 100;
  iVar2 = *(int *)(param_1 + 0x40);
  if (((iVar2 != 0) && (DAT_00a04a0c != 0)) &&
     ((cVar1 = FUN_00459e20(), cVar1 == '\0' && (*(char *)(iVar2 + 0x16) != '\0')))) {
    if ((float)_DAT_00987f08 <= *(float *)(iVar2 + 0x164)) {
      local_2c = *(float *)(iVar2 + 0x164);
    }
    else {
      local_2c = _DAT_0098c014;
    }
    if (*(float *)(iVar2 + 0x58) <= _DAT_00a04a00 * (local_2c / (float)_DAT_00991350)) {
      if (_DAT_00a049fc * (local_2c / (float)_DAT_00991350) < *(float *)(iVar2 + 0x58)) {
        local_10 = DAT_00a04a08;
      }
    }
    else {
      local_10 = DAT_00a04a04;
    }
  }
  if ((DAT_00a7bc20 < DAT_00a049f8) && (DAT_00a7bc20 < local_10)) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x34); local_8 = local_8 + 1) {
      if (*(int *)(*(int *)(*(int *)(param_1 + 0x30) + local_8 * 4) + 0x28) != -1) {
        for (local_14 = 0; local_14 < *(int *)(param_2 + 0x30); local_14 = local_14 + 1) {
          iVar2 = (local_14 + local_8) % *(int *)(param_2 + 0x30);
          if (*(int *)(*(int *)(*(int *)(param_1 + 0x30) + local_8 * 4) + 0x28) ==
              (int)*(short *)(*(int *)(*(int *)(param_2 + 0x2c) + iVar2 * 4) + 2)) {
            FUN_004dfed0(*(undefined4 *)(*(int *)(param_1 + 0x30) + local_8 * 4),
                         *(undefined4 *)(*(int *)(param_2 + 0x2c) + iVar2 * 4),param_3,param_4,
                         param_5,param_6);
            break;
          }
        }
      }
    }
  }
  DAT_00a7bc20 = DAT_00a7bc20 + -1;
  return;
}
```

### decoy: 0x004dfba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dfba0: 16-byte aligned: YES
  004dfba0  55             PUSH EBP
  004dfba1  8B EC          MOV EBP,ESP
  004dfba3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 614 B, RET 0
- callers 2: 0x004dfed0, 0x004e1d40
- callees: 0x00452830 (CALL), 0x00453860 (CALL), 0x00459e20 Gob::IsInCutscene (CALL), 0x004dfed0 (CALL)
- twins: none

## Card B17-14-10

GOG function: OOAnimRun::Destructor  gog 0x008f7090  callconv=(blank)  param_size_bytes=0  gog_size=75  gog_retn=0
Candidate Steam address: 0x004dcf00 (size 75, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004dce30:STR0/CALLEE0/SIG0.325/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f6fc0 OOAnimRun::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f70e0 Global::FindAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x28*,0x2c*} | Steam {0x28*,0x2c*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004dcf00: 16-byte aligned: YES
  004dcf00  55             PUSH EBP
  004dcf01  8B EC          MOV EBP,ESP
  004dcf03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dcf00
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x28 [dec 40]  x2  via EAX
  +0x2C [dec 44]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004dcf00 FUN_004dcf00 size=75 ret=0 callers=1 callees=1

void __fastcall FUN_004dcf00(int param_1)

{
  FUN_009196fd(*(undefined4 *)(param_1 + 0x28));
  FUN_009196fd(*(undefined4 *)(param_1 + 0x2c));
  *(undefined4 *)(param_1 + 0x28) = 0;
  *(undefined4 *)(param_1 + 0x2c) = 0;
  return;
}
```

### decoy: 0x004dce30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dce30: 16-byte aligned: YES
  004dce30  55             PUSH EBP
  004dce31  8B EC          MOV EBP,ESP
  004dce33  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004dce30
string literals (0):
direct callees (2):
  0x004DCDD0  (unnamed)
  0x00919723  operator new
struct offsets touched (top 5):
  +0x28 [dec 40]  x2  via EAX,ECX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 75 B, RET 0
- callers 1: 0x004e1d40
- callees: 0x009196fd operator delete (CALL)
- twins: none

## Card B17-14-11

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040fa30 (size 79, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040f9f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x5*} | Steam {0x2*,0x5*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040fa30: 16-byte aligned: YES
  0040fa30  55             PUSH EBP
  0040fa31  8B EC          MOV EBP,ESP
  0040fa33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040fa30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040fa30 FUN_0040fa30 size=79 ret=0 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_0040fa30(float param_1,float param_2,float param_3)

{
  if ((param_1 < param_2 / (float)_DAT_00987490) || (DAT_00a1b9f0 == 2)) {
    param_1 = param_1 * param_3;
  }
  else {
    param_1 = param_2 - (param_2 - param_1) * param_3;
  }
  return (float10)param_1;
}
```

### decoy: 0x0040f9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040f9f0: 16-byte aligned: YES
  0040f9f0  55             PUSH EBP
  0040f9f1  8B EC          MOV EBP,ESP
  0040f9f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040f9f0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
```

### Static facts (script output, report-only)
- size 79 B, RET 0
- callers 2: 0x0040fa80, 0x0040fb40
- twins: none

## Card B17-14-12

GOG function: CSWCVisualEffectOnObject::ApplyBumpedOutShell  gog 0x005047f0  callconv=(blank)  param_size_bytes=2  gog_size=786  gog_retn=4
Candidate Steam address: 0x00879310 (size 213, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.22 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00879290:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005042d0 CSWCVisualEffectOnObject::AttachLight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00504b70 CSWCVisualEffectOnObject::RemoveBumpedOutShell  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam 0x0076e610  in candidate callees: no
  0x004775f0 CSWCObject::EnableBumpedOutShell -> Steam 0x007f2450  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005036a0 CSWCVisualEffectOnObject::GetTargetCSWCObject -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (26): '?fx_tex_stealth'; 'fx_tex_26'; 'fx_tex_25'; 'fx_tex_24' ...
disp sets (* = in both, 1 common): GOG {0x4*,0x2c,0xd8} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x14,0x19,0x2c,0x578} | Steam {0x2,0x4*,0x20,0x640}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00879310: 16-byte aligned: YES
  00879310  55             PUSH EBP
  00879311  8B EC          MOV EBP,ESP
  00879313  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00879310
string literals (0):
direct callees (5):
  0x00877DC0  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x007400A0  CClientExoApp::GetMiniGame
  0x00478FA0  (unnamed)
  0x00478E60  Global::AurEnableSpeedBlur
struct offsets touched (top 1):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00879310 FUN_00879310 size=213 ret=4 callers=1 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00879310(short param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  
  iVar1 = FUN_00877dc0();
  if ((iVar1 != 0) && ((uVar3 = (uint)(ushort)(param_1 - 0x640), uVar3 == 1 || (uVar3 == 2)))) {
    DAT_00a7fff0 = DAT_00a7fff0 + 1;
    uVar4 = extraout_var;
    iVar2 = FUN_0073f450(uVar3,extraout_var);
    if (((iVar2 == 0) || (*(int *)(iVar2 + 4) == *(int *)(iVar1 + 4))) &&
       ((DAT_00a7fff0 == 1 &&
        (iVar1 = FUN_007400a0(uVar3,uVar4,*(undefined4 *)(DAT_00a1b4a4 + 4)), iVar1 == 0)))) {
      FUN_00478fa0(_DAT_009875b4);
      FUN_00478e60();
    }
  }
  return;
}
```

### decoy: 0x00879290 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00879290: 16-byte aligned: YES
  00879290  55             PUSH EBP
  00879291  8B EC          MOV EBP,ESP
  00879293  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00879290
string literals (0):
direct callees (4):
  0x00877DC0  (unnamed)
  0x007F2490  CSWCObject::DisableBumpedOutShell
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
```

### Static facts (script output, report-only)
- size 213 B, RET 4
- callers 1: 0x00872910
- callees: 0x00877dc0 (CALL), 0x0073f450 CClientExoApp::GetPlayerCreature (CALL), 0x007400a0 CClientExoApp::GetMiniGame (CALL), 0x00478fa0 (CALL), 0x00478e60 (CALL)
- twins: none

## Card B17-14-13

GOG function: CExoStreamingSoundSourceInternal::Update3DStream  gog 0x0063ca40  callconv=(blank)  param_size_bytes=0  gog_size=438  gog_retn=0
Candidate Steam address: 0x0070dc60 (size 71, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.58 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070daa0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063c970 CExoStreamingSoundSourceInternal::SetPriorityGroup  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063cc00 CExoStreamingSoundSourceInternal::IsDone -> 0x0070dc60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0063ac30 CExoSound::GetSoundMode -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x0093e9ce Global::strncmp -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 8): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x24*} | Steam {0xc*,0x24*}
imm sets (* = in both, 1 common): GOG {0x3,0x4,0xc*,0xd,0x40} | Steam {0x2,0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070dc60: 16-byte aligned: YES
  0070dc60  55             PUSH EBP
  0070dc61  8B EC          MOV EBP,ESP
  0070dc63  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070dc60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070dc60 FUN_0070dc60 size=71 ret=0 callers=1 callees=1

bool __fastcall FUN_0070dc60(int *param_1)

{
  int iVar1;
  bool bVar2;
  
  if ((*(int *)(param_1[9 = byte +0x24] + 0xc) == 0) || (*param_1 == 0)) {
    bVar2 = false;
  }
  else {
    iVar1 = _AIL_stream_status_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
    bVar2 = iVar1 == 2;
  }
  return bVar2;
}
```

### decoy: 0x0070daa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070daa0: 16-byte aligned: YES
  0070daa0  55             PUSH EBP
  0070daa1  8B EC          MOV EBP,ESP
  0070daa3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070daa0
string literals (0):
direct callees (4):
  0x00710980  (unnamed)
  0x0093720B  (unnamed)
  0x0070BC90  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 71 B, RET 0
- callers 1: 0x0070b110
- callees: 0x0000007f (CALL)
- imports: _AIL_stream_status@4 (MSS32.DLL)
- twins: none

## Card B17-14-14

GOG function: CExoStreamingSoundSourceInternal::Update3DStream  gog 0x0063ca40  callconv=(blank)  param_size_bytes=0  gog_size=438  gog_retn=0
Candidate Steam address: 0x0070caa0 (size 3882, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=StreamingSource::OpenStream | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070c0c0:STR0/CALLEE0/SIG0.234/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063c970 CExoStreamingSoundSourceInternal::SetPriorityGroup  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063cc00 CExoStreamingSoundSourceInternal::IsDone -> 0x0070dc60  gate_class=REVIEW
- server Steam name at this address: ::CExoStreamingSoundSourceInternal__InitializeSource

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0061a2d0 CResRef::CopyToString_2 -> Steam ?  in candidate callees: n/a
  0x0063ac30 CExoSound::GetSoundMode -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x0093e9ce Global::strncmp -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 8): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x24*} | Steam {0x4,0xc*,0x24*,0x30,0x13c,0x148}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0xc*,0xd*,0x40} | Steam {0x2,0x3*,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0xd*,0xe,0xf,...(+8)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070caa0: 16-byte aligned: YES
  0070caa0  55             PUSH EBP
  0070caa1  8B EC          MOV EBP,ESP
  0070caa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0070caa0
string literals (3):
  0x0099C324  at 0070d18a  '*%s*%d*%d'
  0x0099C324  at 0070d568  '*%s*%d*%d'
  0x0099C324  at 0070d839  '*%s*%d*%d'
direct callees (22):
  0x00733540  CExoString::Constructor
  0x0091C5B3  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00919F10  _strlen
  0x00734500  CExoString::Left
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x0091D67E  (unnamed)
  0x00733DB0  CExoString::operator[]
  0x007337C0  CExoString::operator=
  0x00734010  CExoString::CStr
  0x0091CAEB  (unnamed)
  0x0091F3EF  (unnamed)
  0x0091CF23  (unnamed)
  0x0091A825  (unnamed)
  0x009205FC  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00920D3A  (unnamed)
  0x00920C3B  (unnamed)
  0x00920963  (unnamed)
struct offsets touched (top 5):
  +0x13C [dec 316]  x51  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via ECX,EDX
  +0x148 [dec 328]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0070caa0 FUN_0070caa0 size=3882 ret=0 callers=2 callees=24

void __fastcall FUN_0070caa0(int *param_1)

{
  undefined1 uVar1;
  char cVar2;
  char *pcVar3;
  FILE *pFVar4;
  int iVar5;
  undefined4 uVar6;
  long lVar7;
  char *pcVar8;
  undefined4 uVar9;
  size_t local_214;
  size_t local_20c;
  undefined1 local_1fc [8];
  undefined1 local_1f4 [8];
  undefined1 local_1ec [8];
  undefined1 local_1e4 [8];
  undefined1 local_1dc [8];
  undefined1 local_1d4 [8];
  undefined1 local_1cc [8];
  undefined1 local_1c4 [8];
  undefined1 local_1bc [8];
  undefined1 local_1b4 [8];
  undefined1 local_1ac [8];
  int local_1a4;
  intptr_t local_1a0;
  undefined1 local_19c [8];
  long local_194;
  int local_190;
  undefined4 local_18c;
  undefined1 local_188 [8];
  char local_180 [8];
  int local_178;
  intptr_t local_174;
  int local_170;
  int local_16c;
  int local_168;
  int local_15c;
  char local_158 [8];
  uint local_150;
  int local_14c;
  int local_148;
  intptr_t local_144;
  undefined1 local_140 [8];
  undefined1 local_138 [8];
  undefined1 local_130 [260];
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c[3 = byte +0xc];
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f793;
  local_10 = ExceptionList;
  local_150 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (((*param_1 != 0) || (param_1[1 = byte +0x4] != 0)) || (0 < DAT_00a7e144)) {
    FUN_00733540(local_150);
    local_8 = 0;
    if (DAT_00a10768 == 0) {
      if (*(int *)(param_1[9 = byte +0x24] + 0xc) != 0) {
        param_1[0x4f = byte +0x13c] = 0;
        if ((short)param_1[0x52 = byte +0x148] == 8) {
          for (local_1a4 = 0; local_1a4 < 0x20; local_1a4 = local_1a4 + 1) {
            pcVar8 = "rb";
            pcVar3 = (char *)FUN_00734010();
            pFVar4 = _fopen [GHIDRA-AUTO-LABEL: unreliable](pcVar3,pcVar8);
            param_1[0x4f = byte +0x13c] = (int)pFVar4;
            if (param_1[0x4f = byte +0x13c] != 0) {
              FUN_007337c0(param_1 + local_1a4 * 2 + 0xc);
              break;
            }
          }
          if (param_1[0x4f = byte +0x13c] == 0) {
/* ... 298 more lines truncated by --max-lines */
```

### decoy: 0x0070c0c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070c0c0: 16-byte aligned: YES
  0070c0c0  55             PUSH EBP
  0070c0c1  8B EC          MOV EBP,ESP
  0070c0c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0070c0c0
string literals (7):
  0x0099692C  at 0070c1cd  'HD0:STREAMMUSIC\\\\%s'
  0x0099C300  at 0070c225  '%s\\\\streammusic\\\\%s'
  0x0099A580  at 0070c3e8  'HD0:STREAMVOICE\\\\%s\\\\%s\\\\%s'
  0x0099A59C  at 0070c45a  '%s\\\\streamvoice\\\\%s\\\\%s\\\\%s'
  0x0099A56C  at 0070c4a0  'HD0:STREAMVOICE\\\\%s'
  0x0099A558  at 0070c4b9  'HD0:STREAMVOICE\\\\'
  0x0099C2EC  at 0070c524  '%s\\\\streamsounds\\\\%s'
direct callees (29):
  0x00710A70  ResRef16 != (memcmp of 16 bytes)
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00710CA0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734270  CExoString::Format
  0x00407800  (unnamed)
  0x00919671  (unnamed)
  0x00919F10  _strlen
  0x00733DB0  CExoString::operator[]
  0x0091CF18  (unnamed)
  0x009370C7  _stricmp
  0x007397B0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733B60  (unnamed)
  0x0070DD10  StreamingSource::Stop(closeFlag) (ret 4)
  0x0070CAA0  StreamingSource::OpenStream
  0x0070DFE0  (unnamed)
struct offsets touched (top 10):
  +0x24 [dec 36]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x5 [dec 5]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x120 [dec 288]  x1  via ECX
  +0x144 [dec 324]  x1  via EAX
  +0x148 [dec 328]  x1  via EDX
  +0x14A [dec 330]  x1  via ECX
```

### Static facts (script output, report-only)
- size 3882 B, RET 0
- callers 2: 0x007051c0, 0x0070c0c0
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x0091c5b3 (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00919f10 _strlen (CALL), 0x00734500 (CALL), 0x00733df0 CExoString::operator+ (CALL), ...
- strings: 0x0099c324 "*%s*%d*%d"
- imports: _AIL_open_stream@12 (MSS32.DLL), _AIL_quick_handles@12 (MSS32MIDI.DLL)
- twins: none

## Card B17-14-15

GOG function: CExoInputInternal::BufferEvent  gog 0x00614de0  callconv=(blank)  param_size_bytes=4  gog_size=1743  gog_retn=4
Candidate Steam address: 0x0072e9c0 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072e8d0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00614db0 CExoInputInternal::UnpauseRumble  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006154e0 CExoInputInternal::BufferEvent_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006154e0 CExoInputInternal::BufferEvent_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x3e4,0x3e8,0x3ec} | Steam {0x3e0}
imm sets (* = in both, 1 common): GOG {0x4*,0xa,0x10,0x20,0x200,0xffff} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072e9c0: 16-byte aligned: YES
  0072e9c0  55             PUSH EBP
  0072e9c1  8B EC          MOV EBP,ESP
  0072e9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072e9c0
string literals (0):
direct callees (1):
  0x0072EA20  (unnamed)
struct offsets touched (top 1):
  +0x3E0 [dec 992]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0072e9c0 FUN_0072e9c0 size=84 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_0072e9c0(int param_1,char param_2)

{
  if (param_2 != *(char *)(param_1 + 0x3e0)) {
    *(char *)(param_1 + 0x3e0) = param_2;
    if (param_2 == '\x01') {
      DAT_00a10234 = 0;
    }
    FUN_0072ea20(0);
  }
  return 1;
}
```

### decoy: 0x0072e8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072e8d0: 16-byte aligned: YES
  0072e8d0  55             PUSH EBP
  0072e8d1  8B EC          MOV EBP,ESP
  0072e8d3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072e8d0
string literals (0):
direct callees (1):
  0x0073EEE0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 84 B, RET 4
- callers 1: 0x0072fbf0
- callees: 0x0072ea20 (CALL)
- twins: none

## Card B17-14-16

GOG function: CExoInputInternal::BufferEvent  gog 0x00614de0  callconv=(blank)  param_size_bytes=4  gog_size=1743  gog_retn=4
Candidate Steam address: 0x0072df60 (size 55, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072dfa0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00614db0 CExoInputInternal::UnpauseRumble  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006154e0 CExoInputInternal::BufferEvent_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006154e0 CExoInputInternal::BufferEvent_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x3e4,0x3e8,0x3ec} | Steam {0x1a0,0x1b4}
imm sets (* = in both, 1 common): GOG {0x4*,0xa,0x10,0x20,0x200,0xffff} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0072df60: 16-byte aligned: YES
  0072df60  55             PUSH EBP
  0072df61  8B EC          MOV EBP,ESP
  0072df63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072df60
string literals (0):
direct callees (1):
  0x00733090  (unnamed)
struct offsets touched (top 2):
  +0x1A0 [dec 416]  x2  via ECX,EDX
  +0x1B4 [dec 436]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0072df60 FUN_0072df60 size=55 ret=4 callers=1 callees=1

void __thiscall FUN_0072df60(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x1b4) = param_2;
  if (*(int *)(param_1 + 0x1a0) != 0) {
    FUN_00733090(param_2);
  }
  return;
}
```

### decoy: 0x0072dfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072dfa0: 16-byte aligned: YES
  0072dfa0  55             PUSH EBP
  0072dfa1  8B EC          MOV EBP,ESP
  0072dfa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072dfa0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 55 B, RET 4
- callers 1: 0x0072fb20
- callees: 0x00733090 (CALL)
- twins: none

## Card B17-14-17

GOG function: CAuroraModel::Unload  gog 0x0090bb40  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x00941520 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: Release 0x7112b0; callers: ReleaseProcessedTextureInternal 0x93ea10)
- best decoy (neighbour/other candidate) scores: 0x00941560:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090baf0 CAurora16BitTexture::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090bb80 CAurora16BitTexture::ReadTextureHeader  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x0061ac10 CRes::Release -> Steam 0x007112b0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x00909030 CAuroraInterface::ReleaseProcessedTextureInternal -> Steam 0x0093ea10  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x00941520: 16-byte aligned: YES
  00941520  55             PUSH EBP
  00941521  8B EC          MOV EBP,ESP
  00941523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00941520
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x007112B0  CRes::Release
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00941520 FUN_00941520 size=49 ret=0 callers=9 callees=2

void __fastcall FUN_00941520(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    FUN_007112b0();
  }
  return;
}
```

### decoy: 0x00941560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00941560: 16-byte aligned: YES
  00941560  55             PUSH EBP
  00941561  8B EC          MOV EBP,ESP
  00941563  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00941560
string literals (0):
direct callees (1):
  0x00711200  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 49 B, RET 0
- callers 9: 0x0093b700, 0x0093e350, 0x0093e590, 0x0093e7d0, 0x0093ea10 CAuroraInterface::ReleaseProcessedTextureInternal, ...
- callees: 0x00711250 (CALL), 0x007112b0 CRes::Release (CALL)
- twins: none

## Card B17-14-18

GOG function: CSWGuiText::GetIdealHeight  gog 0x00915c10  callconv=(blank)  param_size_bytes=0  gog_size=95  gog_retn=0
Candidate Steam address: 0x00417840 (size 95, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004178b0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915bd0 CSWGuiText::wrapText  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00915c80 CSWGuiText::GetFontHeight -> 0x004178b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x14*,0x48*,0x50*} | Steam {0x14*,0x48*,0x50*}
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
prologue for 0x00417840: 16-byte aligned: YES
  00417840  55             PUSH EBP
  00417841  8B EC          MOV EBP,ESP
  00417843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00417840
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x14 [dec 20]  x7  via EAX,ECX
  +0x50 [dec 80]  x2  via EDX
  +0x48 [dec 72]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00417840 FUN_00417840 size=95 ret=0 callers=0 callees=0

int __fastcall FUN_00417840(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x48))();
    if (iVar1 < 0x10) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x50))(0xffffffff);
      iVar1 = iVar1 + 1;
    }
    else {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x14) + 0x50))(0xffffffff);
    }
  }
  return iVar1;
}
```

### decoy: 0x004178b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004178b0: 16-byte aligned: YES
  004178b0  55             PUSH EBP
  004178b1  8B EC          MOV EBP,ESP
  004178b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004178b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x14 [dec 20]  x7  via EAX,ECX
  +0x48 [dec 72]  x3  via EDX
```

### Static facts (script output, report-only)
- size 95 B, RET 0
- indirect: 0x00417861 slot 0x48, 0x0041787b slot 0x50, 0x00417897 slot 0x50
- twins: none

## Card B17-14-19

GOG function: CSWGuiContainer::Destructor_2  gog 0x005a61c0  callconv=(blank)  param_size_bytes=0  gog_size=560  gog_retn=0
Candidate Steam address: 0x008b1cc0 (size 60, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiInGameItemEntry::UpdateItem | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b1d00:STR0/CALLEE0/SIG0.038/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a6190 CSWGuiContainer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a63f0 CSWGuiContainer::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: no
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: no
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: no
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0091b1a0 CSWGuiListBox::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1d0,0x3ac}
imm sets (* = in both, 1 common): GOG {0x2*,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+19)} | Steam {0x2*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b1cc0: 16-byte aligned: YES
  008b1cc0  55             PUSH EBP
  008b1cc1  8B EC          MOV EBP,ESP
  008b1cc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008b1cc0
string literals (0):
direct callees (1):
  0x008B1250  CSWGuiInGameItemEntry::SetItem
struct offsets touched (top 2):
  +0x3AC [dec 940]  x2  via EAX,EDX
  +0x1D0 [dec 464]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008b1cc0 FUN_008b1cc0 size=60 ret=0 callers=1 callees=1

void __fastcall FUN_008b1cc0(int param_1)

{
  FUN_008b1250(*(undefined4 *)(param_1 + 0x1d0),*(uint *)(param_1 + 0x3ac) >> 1 & 1,
               *(uint *)(param_1 + 0x3ac) >> 2 & 1);
  return;
}
```

### decoy: 0x008b1d00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b1d00: 16-byte aligned: YES
  008b1d00  55             PUSH EBP
  008b1d01  8B EC          MOV EBP,ESP
  008b1d03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b1d00
string literals (0):
direct callees (1):
  0x004149F0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x3AC [dec 940]  x2  via EAX,EDX
  +0x3B0 [dec 944]  x1  via ECX
```

### Static facts (script output, report-only)
- size 60 B, RET 0
- callers 1: 0x008a83d0 CSWGuiInGameInventory::OnControlSelected
- callees: 0x008b1250 CSWGuiInGameItemEntry::SetItem (CALL)
- twins: none

## Card B17-14-20

GOG function: CAuroraModel::Load  gog 0x00907730  callconv=(blank)  param_size_bytes=0  gog_size=74  gog_retn=0
Candidate Steam address: 0x0093d110 (size 74, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: Demand 0x7111e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0093d160:STR0/CALLEE0/SIG0.583/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00907710 CAuroraModel::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00907780 CAuroraModel::GetPtrToResHelperInfo_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061ab40 CRes::Demand -> Steam 0x007111e0  in candidate callees: yes
  0x007f7ad0 CResVIS::GetVISDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x2*,0x8*} | Steam {0x2*,0x8*}
imm sets (* = in both, 3 common): GOG {0x6*,0x8*,0x7d2*} | Steam {0x6*,0x8*,0x7d2*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093d110: 16-byte aligned: YES
  0093d110  55             PUSH EBP
  0093d111  8B EC          MOV EBP,ESP
  0093d113  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093d110
string literals (0):
direct callees (2):
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0093d110 FUN_0093d110 size=74 ret=0 callers=1 callees=2

bool __fastcall FUN_0093d110(undefined4 param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_007111e0();
  if (iVar1 != 0) {
    iVar2 = FUN_006f3d90();
    *(undefined2 *)(iVar2 + -6) = 0x7d2;
    *(undefined4 *)(iVar2 + -4) = param_1;
  }
  return iVar1 != 0;
}
```

### decoy: 0x0093d160 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093d160: 16-byte aligned: YES
  0093d160  55             PUSH EBP
  0093d161  8B EC          MOV EBP,ESP
  0093d163  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093d160
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00941490  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 74 B, RET 0
- callers 1: 0x0093de80
- callees: 0x007111e0 CRes::Demand (CALL), 0x006f3d90 (CALL)
- twins: none

## Card B17-14-21

GOG function: CSWGuiContainer::Destructor_2  gog 0x005a61c0  callconv=(blank)  param_size_bytes=0  gog_size=560  gog_retn=0
Candidate Steam address: 0x008b3630 (size 243, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b35d0:STR0/CALLEE0/SIG0.029/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a6190 CSWGuiContainer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a63f0 CSWGuiContainer::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: no
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: no
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: no
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x0091b1a0 CSWGuiListBox::Destructor_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+19)} | Steam {0x4*,0x2c,0xb18}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b3630: 16-byte aligned: YES
  008b3630  55             PUSH EBP
  008b3631  8B EC          MOV EBP,ESP
  008b3633  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 008b3630 FUN_008b3630 size=243 ret=0 callers=0 callees=4

void __fastcall FUN_008b3630(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  char cVar7;
  
  cVar7 = FUN_0073eee0();
  if (cVar7 == '\0') {
    iVar1 = *(int *)(param_1 + 4);
    iVar2 = *(int *)(param_1 + 8);
    iVar3 = *(int *)(param_1 + 0xb18);
    iVar4 = *(int *)(param_1 + 0xb1c);
    iVar5 = *(int *)(param_1 + 0xb20);
    iVar6 = *(int *)(param_1 + 0xb24);
    FUN_00414150();
    FUN_00414230(iVar5 / 2 + iVar1 + iVar3,iVar6 / 2 + iVar2 + iVar4);
    FUN_0040f3c0();
  }
  return;
}
```

### decoy: 0x008b35d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b35d0: 16-byte aligned: YES
  008b35d0  55             PUSH EBP
  008b35d1  8B EC          MOV EBP,ESP
  008b35d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b35d0
string literals (0):
direct callees (1):
  0x008B2F70  CSWGuiContainer::SetupGiveMode
struct offsets touched (top 1):
  +0x70 [dec 112]  x7  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 243 B, RET 0
- callees: 0x0073eee0 (CALL), 0x00414150 (CALL), 0x00414230 (CALL), 0x0040f3c0 CSWGuiPanel::OnPanelAdded (CALL)
- globals: 0x00a1b49c READ
- twins: none

## Card B17-14-22

GOG function: CSWCVisualEffectOnObject::ApplyBumpedOutShell  gog 0x005047f0  callconv=(blank)  param_size_bytes=2  gog_size=786  gog_retn=4
Candidate Steam address: 0x00878010 (size 61, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00878050:STR0/CALLEE0/SIG0.174/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005042d0 CSWCVisualEffectOnObject::AttachLight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00504b70 CSWCVisualEffectOnObject::RemoveBumpedOutShell  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam 0x0076e610  in candidate callees: no
  0x004775f0 CSWCObject::EnableBumpedOutShell -> Steam 0x007f2450  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005036a0 CSWCVisualEffectOnObject::GetTargetCSWCObject -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x0061a160 CResRef::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (26): '?fx_tex_stealth'; 'fx_tex_26'; 'fx_tex_25'; 'fx_tex_24' ...
disp sets (* = in both, 0 common): GOG {0x4,0x2c,0xd8} | Steam {0x8c,0xd4}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x14,0x19,0x2c,0x578} | Steam {0x4*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00878010: 16-byte aligned: YES
  00878010  55             PUSH EBP
  00878011  8B EC          MOV EBP,ESP
  00878013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00878010
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xD4 [dec 212]  x2  via ECX,EDX
  +0x8C [dec 140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00878010 FUN_00878010 size=61 ret=4 callers=1 callees=0

void __thiscall FUN_00878010(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x8c) != 0) {
    if (param_2 == 0) {
      *(undefined4 *)(param_1 + 0xd4) = 0;
    }
    else {
      *(undefined4 *)(param_1 + 0xd4) = 1;
    }
  }
  return;
}
```

### decoy: 0x00878050 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00878050: 16-byte aligned: YES
  00878050  55             PUSH EBP
  00878051  8B EC          MOV EBP,ESP
  00878053  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00878050
string literals (0):
direct callees (4):
  0x00877EB0  (unnamed)
  0x00877DC0  (unnamed)
  0x0073F450  CClientExoApp::GetPlayerCreature
  0x007AA7A0  CSWCArea::UpdateShadowingLights
struct offsets touched (top 7):
  +0x8C [dec 140]  x5  via EAX,ECX,EDX
  +0x4C [dec 76]  x2  via EDX
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x20 [dec 32]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0xE0 [dec 224]  x1  via EAX
```

### Static facts (script output, report-only)
- size 61 B, RET 4
- callers 1: 0x00873f70
- twins: none
