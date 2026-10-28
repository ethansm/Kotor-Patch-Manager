# Review cards B17 chunk 9

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-9-01

GOG function: CAuroraModel::Load  gog 0x00907730  callconv=(blank)  param_size_bytes=0  gog_size=74  gog_retn=0
Candidate Steam address: 0x0093d0f0 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093d110:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00907710 CAuroraModel::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00907780 CAuroraModel::GetPtrToResHelperInfo_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061ab40 CRes::Demand -> Steam 0x007111e0  in candidate callees: no
  0x007f7ad0 CResVIS::GetVISDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2,0x8} | Steam {}
imm sets (* = in both, 0 common): GOG {0x6,0x8,0x7d2} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093d0f0: 16-byte aligned: YES
  0093d0f0  55             PUSH EBP
  0093d0f1  8B EC          MOV EBP,ESP
  0093d0f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093d0f0
string literals (0):
direct callees (1):
  0x0093D200  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093d0f0 FUN_0093d0f0 size=28 ret=0 callers=1 callees=1

void __fastcall FUN_0093d0f0(undefined4 *param_1)

{
  *param_1 = CAuroraModel::vftable;
  FUN_0093d200();
  return;
}
```

### decoy: 0x0093d110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093d110: 16-byte aligned: YES
  0093d110  55             PUSH EBP
  0093d111  8B EC          MOV EBP,ESP
  0093d113  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093d110
string literals (0):
direct callees (2):
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
```

### Static facts (script output, report-only)
- size 28 B, RET 0
- callers 1: 0x0093d0c0
- callees: 0x0093d200 (CALL)
- twins: none

## Card B17-9-02

GOG function: Global::BoxIntersect  gog 0x0080ec30  callconv=(blank)  param_size_bytes=20  gog_size=1323  gog_retn=0
Candidate Steam address: 0x00516da0 (size 1323, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005172d0:STR0/CALLEE0/SIG0.85/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080e4d0 Global::polyhit_2 -> 0x00516640  gate_class=REVIEW
- GOG-order neighbour after:  0x0080f240 Global::BoxIntersect_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080f160 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0x10*,0x20*,0x41*,0x4c*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0x10*,0x20*,0x41*,0x4c*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 20 bytes -> x86 slots 20; ASM RET 0)
```

### prologue
```
prologue for 0x00516da0: 16-byte aligned: YES
  00516da0  55             PUSH EBP
  00516da1  8B EC          MOV EBP,ESP
  00516da3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00516da0
string literals (0):
direct callees (1):
  0x005172D0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x32  via EAX,ECX,EDX
  +0x8 [dec 8]  x32  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00516da0 FUN_00516da0 size=1323 ret=0 callers=1 callees=1

undefined4 FUN_00516da0(float *param_1,float *param_2,float *param_3,float *param_4,float *param_5)

{
  float fVar1;
  float fVar2;
  undefined4 uVar3;
  byte local_6;
  byte local_5;
  
  FUN_005172d0(param_1,param_3,param_4,&local_6);
  if (local_6 == 0) {
    uVar3 = 0xffffffff;
  }
  else {
    FUN_005172d0(param_2,param_3,param_4,&local_5);
    if ((local_6 & local_5) == 0) {
      if ((local_6 & 0x20) == 0) {
        if ((local_6 & 0x10) != 0) {
          fVar1 = (*param_4 - *param_1) / (*param_2 - *param_1);
          fVar2 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
          fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
          if ((((param_3[1 = byte +0x4] < fVar2 != (param_3[1 = byte +0x4] == fVar2)) && (fVar2 <= param_4[1 = byte +0x4])) &&
              (param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1))) && (fVar1 <= param_4[2 = byte +0x8])) {
            *param_5 = *param_4;
            param_5[1 = byte +0x4] = fVar2;
            param_5[2 = byte +0x8] = fVar1;
            return 2;
          }
        }
      }
      else {
        fVar1 = (*param_3 - *param_1) / (*param_2 - *param_1);
        fVar2 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
        fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
        if (((param_3[1 = byte +0x4] < fVar2 != (param_3[1 = byte +0x4] == fVar2)) && (fVar2 <= param_4[1 = byte +0x4])) &&
           ((param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1) && (fVar1 <= param_4[2 = byte +0x8])))) {
          *param_5 = *param_3;
          param_5[1 = byte +0x4] = fVar2;
          param_5[2 = byte +0x8] = fVar1;
          return 1;
        }
      }
      if ((local_6 & 8) == 0) {
        if ((local_6 & 4) != 0) {
          fVar1 = (param_4[1 = byte +0x4] - param_1[1 = byte +0x4]) / (param_2[1 = byte +0x4] - param_1[1 = byte +0x4]);
          fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
          fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
          if (((*param_3 < fVar2 != (*param_3 == fVar2)) && (fVar2 <= *param_4)) &&
             ((param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1) && (fVar1 <= param_4[2 = byte +0x8])))) {
            *param_5 = fVar2;
            param_5[1 = byte +0x4] = param_4[1 = byte +0x4];
            param_5[2 = byte +0x8] = fVar1;
            return 4;
          }
        }
      }
      else {
        fVar1 = (param_3[1 = byte +0x4] - param_1[1 = byte +0x4]) / (param_2[1 = byte +0x4] - param_1[1 = byte +0x4]);
        fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
        fVar1 = fVar1 * param_2[2 = byte +0x8] + (1.0 - fVar1) * param_1[2 = byte +0x8];
        if ((((*param_3 < fVar2 != (*param_3 == fVar2)) && (fVar2 <= *param_4)) &&
            (param_3[2 = byte +0x8] < fVar1 != (param_3[2 = byte +0x8] == fVar1))) && (fVar1 <= param_4[2 = byte +0x8])) {
          *param_5 = fVar2;
          param_5[1 = byte +0x4] = param_3[1 = byte +0x4];
          param_5[2 = byte +0x8] = fVar1;
          return 3;
        }
      }
      if ((local_6 & 2) == 0) {
        if ((local_6 & 1) != 0) {
          fVar1 = (param_4[2 = byte +0x8] - param_1[2 = byte +0x8]) / (param_2[2 = byte +0x8] - param_1[2 = byte +0x8]);
          fVar2 = fVar1 * *param_2 + (1.0 - fVar1) * *param_1;
          fVar1 = fVar1 * param_2[1 = byte +0x4] + (1.0 - fVar1) * param_1[1 = byte +0x4];
          if (((param_3[1 = byte +0x4] < fVar1 != (param_3[1 = byte +0x4] == fVar1)) && (fVar1 <= param_4[1 = byte +0x4])) &&
             ((*param_3 < fVar2 != (*param_3 == fVar2) && (fVar2 <= *param_4)))) {
            *param_5 = fVar2;
            param_5[1 = byte +0x4] = fVar1;
            param_5[2 = byte +0x8] = param_4[2 = byte +0x8];
            return 6;
/* ... 23 more lines truncated by --max-lines */
```

### decoy: 0x005172d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005172d0: 16-byte aligned: YES
  005172d0  55             PUSH EBP
  005172d1  8B EC          MOV EBP,ESP
  005172d3  8B 45 14       MOV EAX,dword ptr [EBP + 0x14]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005172d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1323 B, RET 0
- callers 1: 0x0045d620
- callees: 0x005172d0 (CALL)
- twins: none

## Card B17-9-03

GOG function: CClientOptions::SetVSync  gog 0x004691f0  callconv=(blank)  param_size_bytes=4  gog_size=53  gog_retn=4
Candidate Steam address: 0x007bc4f0 (size 34, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc520:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004691c0 CClientOptions::SetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00469230 CClientOptions::SetGammaSetting  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086ac50 Global::AurSetVSyncState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x48} | Steam {0x3c}
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
prologue for 0x007bc4f0: 16-byte aligned: YES
  007bc4f0  55             PUSH EBP
  007bc4f1  8B EC          MOV EBP,ESP
  007bc4f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc4f0
string literals (0):
direct callees (1):
  0x00478D50  CSWCArea::SetGlobalLightAmbient
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc4f0 FUN_007bc4f0 size=34 ret=4 callers=2 callees=1

void __thiscall FUN_007bc4f0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x3c) = param_2;
  FUN_00478d50(param_2);
  return;
}
```

### decoy: 0x007bc520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc520: 16-byte aligned: YES
  007bc520  55             PUSH EBP
  007bc521  8B EC          MOV EBP,ESP
  007bc523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc520
string literals (0):
direct callees (1):
  0x00478D70  (unnamed)
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 34 B, RET 4
- callers 2: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d75f0
- callees: 0x00478d50 (CALL)
- twins: none

## Card B17-9-04

GOG function: Global::polyhit_2  gog 0x0080e4d0  callconv=(blank)  param_size_bytes=24  gog_size=1874  gog_retn=0
Candidate Steam address: 0x00516640 (size 1874, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00516230:STR0/CALLEE0/SIG0.812/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW
- GOG-order neighbour after:  0x0080ec30 Global::BoxIntersect -> 0x00516da0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x5*,0xc*,0x41*,0x44*,0x18c*} | Steam {0x2*,0x4*,0x5*,0xc*,0x41*,0x44*,0x18c*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 24 bytes -> x86 slots 24; ASM RET 0)
```

### prologue
```
prologue for 0x00516640: 16-byte aligned: YES
  00516640  55             PUSH EBP
  00516641  8B EC          MOV EBP,ESP
  00516643  81 EC 8C 01 00 00 SUB ESP,0x18c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00516640
string literals (0):
direct callees (1):
  0x005142F0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00516640 FUN_00516640 size=1874 ret=0 callers=5 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_00516640(float *param_1,int param_2,float *param_3,float *param_4,float *param_5,
                 float *param_6)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  bool bVar8;
  float *pfVar9;
  float *pfVar10;
  int iVar11;
  float *pfVar12;
  float *pfVar13;
  float10 fVar14;
  int local_38;
  float local_18;
  float local_14;
  float local_10;
  int local_c;
  
  local_18 = 0.0;
  local_14 = 0.0;
  local_10 = 0.0;
  for (local_c = 0; local_c < param_2 + -2; local_c = local_c + 1) {
    iVar11 = (local_c + 1) % param_2;
    pfVar12 = param_1 + iVar11 * 3;
    pfVar9 = param_1 + ((local_c + 2) % param_2) * 3;
    pfVar10 = param_1 + local_c * 3;
    pfVar13 = param_1 + iVar11 * 3;
    local_18 = ((pfVar9[2 = byte +0x8] - pfVar12[2 = byte +0x8]) * (pfVar13[1 = byte +0x4] - pfVar10[1 = byte +0x4]) -
               (pfVar9[1 = byte +0x4] - pfVar12[1 = byte +0x4]) * (pfVar13[2 = byte +0x8] - pfVar10[2 = byte +0x8])) + local_18;
    local_14 = ((*pfVar9 - *pfVar12) * (pfVar13[2 = byte +0x8] - pfVar10[2 = byte +0x8]) -
               (pfVar9[2 = byte +0x8] - pfVar12[2 = byte +0x8]) * (*pfVar13 - *pfVar10)) + local_14;
    local_10 = ((pfVar9[1 = byte +0x4] - pfVar12[1 = byte +0x4]) * (*pfVar13 - *pfVar10) -
               (*pfVar9 - *pfVar12) * (pfVar13[1 = byte +0x4] - pfVar10[1 = byte +0x4])) + local_10;
  }
  fVar14 = (float10)FUN_005142f0(&local_18);
  if ((float)_DAT_00992308 <= (float)fVar14) {
    fVar1 = 1.0 / (float)fVar14;
    local_18 = fVar1 * local_18;
    local_14 = fVar1 * local_14;
    fVar1 = fVar1 * local_10;
    fVar2 = -(*param_1 * local_18 + param_1[1 = byte +0x4] * local_14 + param_1[2 = byte +0x8] * fVar1);
    fVar3 = local_18 * *param_3 + local_14 * param_3[1 = byte +0x4] + fVar1 * param_3[2 = byte +0x8] + fVar2;
    if ((fVar3 < (float)_DAT_00986b38) ||
       (fVar2 = local_18 * *param_4 + local_14 * param_4[1 = byte +0x4] + fVar1 * param_4[2 = byte +0x8] + fVar2,
       (float)_DAT_00986b38 < fVar2)) {
      bVar8 = false;
    }
    else if (fVar2 == fVar3) {
      bVar8 = false;
    }
    else {
      if ((DAT_00a7d834 & 1) == 0) {
        DAT_00a7d834 = DAT_00a7d834 | 1;
      }
      fVar3 = fVar3 / (fVar3 - fVar2);
      fVar2 = 1.0 - fVar3;
      DAT_00a7d828 = fVar3 * *param_4 + fVar2 * *param_3;
      DAT_00a7d82c = fVar3 * param_4[1 = byte +0x4] + fVar2 * param_3[1 = byte +0x4];
      DAT_00a7d830 = fVar3 * param_4[2 = byte +0x8] + fVar2 * param_3[2 = byte +0x8];
      bVar8 = true;
      for (local_38 = 0; (bVar8 != false && (local_38 < param_2)); local_38 = local_38 + 1) {
        pfVar9 = param_1 + local_38 * 3;
        pfVar10 = param_1 + ((local_38 + 1) % param_2) * 3;
        fVar5 = DAT_00a7d828 - *pfVar9;
        fVar6 = DAT_00a7d82c - pfVar9[1 = byte +0x4];
        fVar7 = DAT_00a7d830 - pfVar9[2 = byte +0x8];
        fVar2 = *pfVar10 - *pfVar9;
        fVar3 = pfVar10[1 = byte +0x4] - pfVar9[1 = byte +0x4];
        fVar4 = pfVar10[2 = byte +0x8] - pfVar9[2 = byte +0x8];
        bVar8 = (float)_DAT_00986b38 <=
                (fVar7 * fVar3 - fVar6 * fVar4) * local_18 +
/* ... 22 more lines truncated by --max-lines */
```

### decoy: 0x00516230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00516230: 16-byte aligned: YES
  00516230  55             PUSH EBP
  00516231  8B EC          MOV EBP,ESP
  00516233  81 EC C4 00 00 00 SUB ESP,0xc4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00516230
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1874 B, RET 0
- callers 5: 0x0045d0e0, 0x004e4850, 0x004e4ad0, 0x00650e00, 0x00651090
- callees: 0x005142f0 (CALL)
- twins: none

## Card B17-9-05

GOG function: RequestPoolManagerVBO::DeletePool  gog 0x0081cb00  callconv=(blank)  param_size_bytes=4  gog_size=159  gog_retn=4
Candidate Steam address: 0x0042a3d0 (size 159, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0042a150:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0081c950 RequestPoolManagerVBO::ResizePool -> 0x0042a220  gate_class=REVIEW
- GOG-order neighbour after:  0x0081cba0 RequestPoolManagerSys::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x009242da unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x10*,0x2c*,0x5c*,0x68*} | Steam {0x10*,0x2c*,0x5c*,0x68*}
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
prologue for 0x0042a3d0: 16-byte aligned: YES
  0042a3d0  55             PUSH EBP
  0042a3d1  8B EC          MOV EBP,ESP
  0042a3d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a3d0
string literals (0):
direct callees (2):
  0x004491C0  CExoPtrList::Add (doubling)
  0x0091C6AA  operator delete[]
struct offsets touched (top 4):
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
  +0x5C [dec 92]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a3d0 FUN_0042a3d0 size=159 ret=4 callers=0 callees=2

void __thiscall FUN_0042a3d0(int param_1,int param_2)

{
  undefined4 local_8;
  
  if (param_2 != -1) {
    FUN_004491c0(param_2);
    *(undefined4 *)(*(int *)(param_1 + 0x10) + param_2 * 4) = 0;
    local_8 = *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d54)(1,&local_8);
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4) = 0;
    FUN_0091c6aa(*(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4) = 0;
  }
  return;
}
```

### decoy: 0x0042a150 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a150: 16-byte aligned: YES
  0042a150  55             PUSH EBP
  0042a151  8B EC          MOV EBP,ESP
  0042a153  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0042a150
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x8C [dec 140]  x2  via EAX,ECX
  +0x2C [dec 44]  x1  via EAX
  +0x50 [dec 80]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
  +0x68 [dec 104]  x1  via EDX
```

### Static facts (script output, report-only)
- size 159 B, RET 4
- callees: 0x004491c0 (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a418 unresolved
- twins: none

## Card B17-9-06

GOG function: MdlNodeEmitter::InternalParseField  gog 0x008c7e00  callconv=(blank)  param_size_bytes=4  gog_size=15890  gog_retn=4
Candidate Steam address: 0x0048b6d0 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004878b0:STR167/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c7500 MdlNodeLight::InternalParseField -> 0x00486fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x008cbc20 MdlNodeCamera::InternalParseField  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (25), mapped to Steam through trusted rows only:
  0x008a0e70 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0e90 CFrameWnd::DelayUpdateFrameTitle -> Steam ?  in candidate callees: n/a
  0x008a0eb0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0ed0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0ef0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f10 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f30 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f60 MdlNodeEmitter::IsAffectedByWind -> Steam ?  in candidate callees: n/a
  ... 17 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (168): 'bounce'; 'random'; 'tangentlengthbezierkey'; 'tangentlengthkey' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x8,0x18,0x24,0x38,0x44,0x50,0x54,0x58,0x5c,0x60,0x64,0x68,...(+44)} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0048b6d0: 16-byte aligned: YES
  0048b6d0  55             PUSH EBP
  0048b6d1  8B EC          MOV EBP,ESP
  0048b6d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048b6d0
string literals (0):
direct callees (1):
  0x00486AA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048b6d0 FUN_0048b6d0 size=25 ret=4 callers=1 callees=1
THUNK -> 0x00486AA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0048b6d0(undefined4 param_1)

{
  FUN_00486aa0(param_1);
  return;
}
```

### decoy: 0x004878b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004878b0: 16-byte aligned: YES
  004878b0  55             PUSH EBP
  004878b1  8B EC          MOV EBP,ESP
  004878b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004878b0
string literals (336):
  0x0098D304  at 0048794d  'p2p_sel'
  0x0098D304  at 00487962  'p2p_sel'
  0x0098D2F4  at 00487988  'affectedByWind'
  0x0098D2F4  at 0048799d  'affectedByWind'
  0x0098D2E8  at 004879c3  'm_isTinted'
  0x0098D2E8  at 004879d8  'm_isTinted'
  0x0098D2E0  at 004879fe  'bounce'
  0x0098D2E0  at 00487a13  'bounce'
  0x00988318  at 00487a39  'random'
  0x00988318  at 00487a4e  'random'
  0x0098D2D8  at 00487a74  'inherit'
  0x0098D2D8  at 00487a89  'inherit'
direct callees (25):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484F10  (unnamed)
  0x00484E20  (unnamed)
  0x004E52D0  (unnamed)
  0x004E52F0  (unnamed)
  0x004E5310  (unnamed)
  0x004E5330  (unnamed)
  0x004E5350  (unnamed)
  0x004E5370  (unnamed)
  0x004E5390  (unnamed)
  0x004E53C0  (unnamed)
  0x004E53E0  (unnamed)
  0x004E5410  (unnamed)
  0x004E5440  (unnamed)
  0x004E5470  (unnamed)
  0x004E54A0  (unnamed)
  0x00484EE0  (unnamed)
  0x00484DC0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 25 B, RET 4
- callers 1: 0x00453b50
- callees: 0x00486aa0 (CALL)
- twins: none

## Card B17-9-07

GOG function: Global::polyhit  gog 0x0080e0c0  callconv=(blank)  param_size_bytes=28  gog_size=1033  gog_retn=0
Candidate Steam address: 0x00516230 (size 1033, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00516640:STR0/CALLEE0/SIG0.812/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080df50 Global::RotationArc -> 0x005160c0  gate_class=REVIEW
- GOG-order neighbour after:  0x0080e4d0 Global::polyhit_2 -> 0x00516640  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 6 common): GOG {0x2*,0x5*,0xc*,0x41*,0x44*,0xc4*} | Steam {0x2*,0x5*,0xc*,0x41*,0x44*,0xc4*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 28 bytes -> x86 slots 28; ASM RET 0)
```

### prologue
```
prologue for 0x00516230: 16-byte aligned: YES
  00516230  55             PUSH EBP
  00516231  8B EC          MOV EBP,ESP
  00516233  81 EC C4 00 00 00 SUB ESP,0xc4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00516230
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00516230 FUN_00516230 size=1033 ret=0 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_00516230(int param_1,int param_2,float *param_3,float *param_4,float *param_5,
                 float *param_6,float *param_7)

{
  float fVar1;
  float fVar2;
  bool bVar3;
  float *pfVar4;
  float *pfVar5;
  float *pfVar6;
  int local_18;
  
  fVar1 = *param_3 * *param_5 + param_3[1 = byte +0x4] * param_5[1 = byte +0x4] + param_3[2 = byte +0x8] * param_5[2 = byte +0x8] + *param_4;
  fVar2 = *param_3 * *param_6 + param_3[1 = byte +0x4] * param_6[1 = byte +0x4] + param_3[2 = byte +0x8] * param_6[2 = byte +0x8] + *param_4;
  if ((fVar1 < (float)_DAT_00986b38) || ((float)_DAT_00986b38 < fVar2)) {
    bVar3 = false;
  }
  else if (fVar2 == fVar1) {
    bVar3 = false;
  }
  else {
    if ((DAT_00a7d824 & 1) == 0) {
      DAT_00a7d824 = DAT_00a7d824 | 1;
    }
    if ((DAT_00a7d824 & 2) == 0) {
      DAT_00a7d824 = DAT_00a7d824 | 2;
      _DAT_00a7d80c = 0.0;
      _DAT_00a7d810 = 0.0;
      _DAT_00a7d814 = 0.0;
    }
    fVar1 = fVar1 / (fVar1 - fVar2);
    fVar2 = 1.0 - fVar1;
    DAT_00a7d818 = fVar1 * *param_6 + fVar2 * *param_5;
    DAT_00a7d81c = fVar1 * param_6[1 = byte +0x4] + fVar2 * param_5[1 = byte +0x4];
    DAT_00a7d820 = fVar1 * param_6[2 = byte +0x8] + fVar2 * param_5[2 = byte +0x8];
    bVar3 = true;
    for (local_18 = 0; (bVar3 != false && (local_18 < param_2)); local_18 = local_18 + 1) {
      pfVar5 = (float *)(local_18 * 0xc + param_1);
      pfVar4 = (float *)(local_18 * 0xc + param_1);
      pfVar6 = (float *)(((local_18 + 1) % param_2) * 0xc + param_1);
      _DAT_00a7d80c =
           (DAT_00a7d820 - pfVar5[2 = byte +0x8]) * (pfVar6[1 = byte +0x4] - pfVar4[1 = byte +0x4]) -
           (DAT_00a7d81c - pfVar5[1 = byte +0x4]) * (pfVar6[2 = byte +0x8] - pfVar4[2 = byte +0x8]);
      _DAT_00a7d810 =
           (DAT_00a7d818 - *pfVar5) * (pfVar6[2 = byte +0x8] - pfVar4[2 = byte +0x8]) -
           (DAT_00a7d820 - pfVar5[2 = byte +0x8]) * (*pfVar6 - *pfVar4);
      _DAT_00a7d814 =
           (DAT_00a7d81c - pfVar5[1 = byte +0x4]) * (*pfVar6 - *pfVar4) -
           (DAT_00a7d818 - *pfVar5) * (pfVar6[1 = byte +0x4] - pfVar4[1 = byte +0x4]);
      bVar3 = (float)_DAT_00992300 <=
              _DAT_00a7d80c * *param_3 + _DAT_00a7d810 * param_3[1 = byte +0x4] + _DAT_00a7d814 * param_3[2 = byte +0x8];
    }
    if ((bVar3 != false) && (param_7 != (float *)0x0)) {
      *param_7 = DAT_00a7d818;
      param_7[1 = byte +0x4] = DAT_00a7d81c;
      param_7[2 = byte +0x8] = DAT_00a7d820;
    }
  }
  return bVar3;
}
```

### decoy: 0x00516640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00516640: 16-byte aligned: YES
  00516640  55             PUSH EBP
  00516641  8B EC          MOV EBP,ESP
  00516643  81 EC 8C 01 00 00 SUB ESP,0x18c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00516640
string literals (0):
direct callees (1):
  0x005142F0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x13  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 1033 B, RET 0
- callers 2: 0x0045d620, 0x0045dfa0
- twins: none

## Card B17-9-08

GOG function: CClientOptions::SetSoftShadows  gog 0x00468f30  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc230 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc260:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x30} | Steam {0x2c}
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

## Card B17-9-09

GOG function: GLRender::FrameBufferModifications  gog 0x00827e30  callconv=(blank)  param_size_bytes=4  gog_size=1885  gog_retn=0
Candidate Steam address: 0x00435720 (size 1885, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004356e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00827df0 GLRender::RenderDrawableSquares -> 0x00435720  gate_class=REVIEW
- GOG-order neighbour after:  0x00828590 GLRender::DrawLightmappedGrass  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086b640 Global::AurFrameBufferModificationsAvailable -> Steam ?  in candidate callees: n/a
  0x0086b700 Global::AurCubeMapAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}
imm sets (* = in both, 2 common): GOG {0x6*,0x14*} | Steam {0x6*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435720 FUN_00435720 size=1885 ret=0 callers=1 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00435720(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
  }
  if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
      wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1ddc8);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      wglMakeCurrent(DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1dddc);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
    else {
      (*DAT_00a32db4)(DAT_00a1ddc4,DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      (*DAT_00a32db4)(DAT_00a1ddd8,DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
  }
  if (DAT_00a1dde4 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde4);
    DAT_00a1dde4 = 0;
  }
  if (DAT_00a1dde8 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde8);
    DAT_00a1dde8 = 0;
  }
  if (DAT_00a1ddd0 != 0) {
    glDeleteTextures(1,&DAT_00a1ddd0);
    DAT_00a1ddd0 = 0;
  }
  if (DAT_00a1dd44 != 0) {
    glDeleteTextures(1,&DAT_00a1dd44);
    DAT_00a1dd44 = 0;
  }
  if (DAT_00a1dd38 != 0) {
    glDeleteTextures(1,&DAT_00a1dd38);
/* ... 111 more lines truncated by --max-lines */
```

### decoy: 0x004356e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B17-9-10

GOG function: CAuroraModel::Load  gog 0x00907730  callconv=(blank)  param_size_bytes=0  gog_size=74  gog_retn=0
Candidate Steam address: 0x0093d160 (size 65, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.58 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093d110:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00907710 CAuroraModel::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00907780 CAuroraModel::GetPtrToResHelperInfo_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061ab40 CRes::Demand -> Steam 0x007111e0  in candidate callees: no
  0x007f7ad0 CResVIS::GetVISDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2,0x8*} | Steam {0x8*}
imm sets (* = in both, 2 common): GOG {0x6*,0x8*,0x7d2} | Steam {0x6*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0093d160: 16-byte aligned: YES
  0093d160  55             PUSH EBP
  0093d161  8B EC          MOV EBP,ESP
  0093d163  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093d160
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00941490  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0093d160 FUN_0093d160 size=65 ret=0 callers=8 callees=2

int __fastcall FUN_0093d160(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    iVar1 = FUN_00941490();
    return iVar1 + -6;
  }
  return 0;
}
```

### decoy: 0x0093d110 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093d110: 16-byte aligned: YES
  0093d110  55             PUSH EBP
  0093d111  8B EC          MOV EBP,ESP
  0093d113  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093d110
string literals (0):
direct callees (2):
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x2 [dec 2]  x1  via ECX
```

### Static facts (script output, report-only)
- size 65 B, RET 0
- callers 8: 0x0093db70, 0x0093eac0, 0x0093ec50, 0x0093ed00, 0x0093ee90, ...
- callees: 0x00711250 (CALL), 0x00941490 (CALL)
- twins: none

## Card B17-9-11

GOG function: GLRender::FrameBufferModificationsATI  gog 0x00829060  callconv=(blank)  param_size_bytes=8  gog_size=496  gog_retn=0
Candidate Steam address: 0x004369a0 (size 496, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: rotation:0x0082a290:0.58>0.21
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00436780:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00828e40 GLRender::RenderOverlayOverbrighten  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00829250 GLRender::RenderOverlayConvolution -> 0x00436b90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086b4a0 Global::AurMultiTextureAvailable -> Steam ?  in candidate callees: n/a
  0x0086b520 Global::AurVertexBufferObjectARBAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x14*} | Steam {0xc*,0x14*}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x7*,0xc*,0x14*,0x1406*,0x84c0*,0x8892*} | Steam {0x2*,0x3*,0x7*,0xc*,0x14*,0x1406*,0x84c0*,0x8892*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x004369a0: 16-byte aligned: YES
  004369a0  55             PUSH EBP
  004369a1  8B EC          MOV EBP,ESP
  004369a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004369a0
string literals (0):
direct callees (2):
  0x00484790  (unnamed)
  0x00484710  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004369a0 FUN_004369a0 size=496 ret=0 callers=1 callees=6

void FUN_004369a0(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,int param_6)

{
  int iVar1;
  
  iVar1 = FUN_00484790();
  if (iVar1 == 0) {
    glVertexPointer(3,0x1406,param_2,param_1);
    glNormalPointer(0x1406,param_4,param_6 * 0x30 + param_1);
    glTexCoordPointer(2,0x1406,param_4,param_1 + 0xc + param_6 * 0x30);
    iVar1 = FUN_00484710();
    if (iVar1 != 0) {
      (*DAT_00a32c38)(0x84c1);
    }
    DAT_00a1dd70 = 1;
    glTexCoordPointer(2,0x1406,param_4,param_1 + 0x14 + param_6 * 0x30);
    iVar1 = FUN_00484710();
    if (iVar1 != 0) {
      (*DAT_00a32c38)(0x84c0);
    }
    DAT_00a1dd70 = 0;
    glDrawArrays(7,param_5,param_6 << 2);
  }
  else {
    (*DAT_00a32d4c)(0x8892,0);
    glVertexPointer(3,0x1406,param_2,param_1);
    glNormalPointer(0x1406,param_4,param_6 * 0x30 + param_1);
    glTexCoordPointer(2,0x1406,param_4,param_1 + 0xc + param_6 * 0x30);
    iVar1 = FUN_00484710();
    if (iVar1 != 0) {
      (*DAT_00a32c38)(0x84c1);
    }
    DAT_00a1dd70 = 1;
    glTexCoordPointer(2,0x1406,param_4,param_1 + 0x14 + param_6 * 0x30);
    iVar1 = FUN_00484710();
    if (iVar1 != 0) {
      (*DAT_00a32c38)(0x84c0);
    }
    DAT_00a1dd70 = 0;
    glDrawArrays(7,param_5,param_6 << 2);
    (*DAT_00a32d4c)(0x8892,0);
  }
  return;
}
```

### decoy: 0x00436780 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00436780: 16-byte aligned: YES
  00436780  55             PUSH EBP
  00436781  8B EC          MOV EBP,ESP
  00436783  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00436780
string literals (0):
direct callees (6):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x004280B0  (unnamed)
  0x00427280  (unnamed)
  0x004282D0  (unnamed)
  0x0091C6AA  operator delete[]
struct offsets touched (top 2):
  +0x9884FC [dec 9995516]  x5  via ECX,EDX
  +0x98862C [dec 9995820]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 496 B, RET 0
- callers 1: 0x0050dac0
- callees: 0x00484790 (CALL), 0x00000038 (CALL), 0x0000003b (CALL), 0x00000036 (CALL), 0x00484710 (CALL), 0x00000035 (CALL)
- indirect: 0x004369ba unresolved, 0x00436a2e unresolved, 0x00436a78 unresolved, 0x00436aa2 unresolved, 0x00436b20 unresolved, ...
- imports: glDrawArrays (OPENGL32.DLL), glNormalPointer (OPENGL32.DLL), glTexCoordPointer (OPENGL32.DLL), ...
- twins: none

## Card B17-9-12

GOG function: MdlNodeEmitter::InternalParseField  gog 0x008c7e00  callconv=(blank)  param_size_bytes=4  gog_size=15890  gog_retn=4
Candidate Steam address: 0x00486fb0 (size 2293, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004878b0:STR167/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c7500 MdlNodeLight::InternalParseField -> 0x00486fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x008cbc20 MdlNodeCamera::InternalParseField  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (25), mapped to Steam through trusted rows only:
  0x008a0e70 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0e90 CFrameWnd::DelayUpdateFrameTitle -> Steam ?  in candidate callees: n/a
  0x008a0eb0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0ed0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0ef0 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f10 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f30 unnamed -> Steam ?  in candidate callees: n/a
  0x008a0f60 MdlNodeEmitter::IsAffectedByWind -> Steam ?  in candidate callees: n/a
  ... 17 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (168): 'bounce'; 'random'; 'tangentlengthbezierkey'; 'tangentlengthkey' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 20 common): GOG {0x2,0x4*,0x8*,0x18*,0x24,0x38*,0x44*,0x50*,0x54,0x58*,0x5c,0x60*,0x64*,0x68,...(+44)} | Steam {0x4*,0x8*,0xc,0x18*,0x38*,0x44*,0x4c,0x50*,0x58*,0x60*,0x64*,0x6c*,0x78*,0x84*,...(+8)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00486fb0: 16-byte aligned: YES
  00486fb0  55             PUSH EBP
  00486fb1  8B EC          MOV EBP,ESP
  00486fb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00486fb0
string literals (56):
  0x0098C8A0  at 00486fdc  'color'
  0x0098C8A0  at 00487004  'color'
  0x0098C894  at 0048703c  'colorkey'
  0x0098C894  at 00487064  'colorkey'
  0x0098C884  at 00487099  'colorbezierkey'
  0x0098C884  at 004870c1  'colorbezierkey'
  0x0098C87C  at 004870f4  'radius'
  0x0098C87C  at 0048711c  'radius'
  0x0098C870  at 00487154  'radiuskey'
  0x0098C870  at 0048717c  'radiuskey'
  0x0098C860  at 004871b1  'radiusbezierkey'
  0x0098C860  at 004871d9  'radiusbezierkey'
direct callees (12):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x004861D0  (unnamed)
  0x00486700  (unnamed)
  0x00486900  (unnamed)
  0x00484E20  (unnamed)
  0x00485A40  (unnamed)
  0x0048E8E0  (unnamed)
  0x00429010  (unnamed)
  0x00484E00  (unnamed)
  0x00486AA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00486fb0 FUN_00486fb0 size=2293 ret=4 callers=1 callees=12

void __thiscall FUN_00486fb0(int param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  
  if (DAT_00a309bd == '\0') {
    pcVar1 = (char *)FUN_00484d10(param_2,param_1);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("color",pcVar1);
    if (iVar2 == 0) {
      uVar6 = 0xffffffff;
      uVar5 = 0x4c;
      iVar2 = param_1 + 0x44;
      iVar4 = param_1 + 0x38;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("color");
      uVar5 = FUN_00484d10(param_2,sVar3 + param_2,iVar4,iVar2,uVar5,uVar6);
      FUN_004861d0(uVar5);
    }
    else {
      pcVar1 = (char *)FUN_00484d10(param_2);
      iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("colorkey",pcVar1);
      if (iVar2 == 0) {
        uVar6 = 0xffffffff;
        uVar5 = 0x4c;
        iVar4 = param_1 + 0x44;
        iVar2 = param_1 + 0x38;
        sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("colorkey");
        uVar5 = FUN_00484d10(param_2,sVar3 + param_2,iVar2,iVar4,uVar5,uVar6);
        FUN_00486700(uVar5);
      }
      else {
        pcVar1 = (char *)FUN_00484d10(param_2);
        iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("colorbezierkey",pcVar1);
        if (iVar2 == 0) {
          uVar6 = 0xffffffff;
          uVar5 = 0x4c;
          iVar2 = param_1 + 0x44;
          iVar4 = param_1 + 0x38;
          sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("colorbezierkey");
          uVar5 = FUN_00484d10(param_2,sVar3 + param_2,iVar4,iVar2,uVar5,uVar6);
          FUN_00486900(uVar5);
        }
      }
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("radius",pcVar1);
    if (iVar2 == 0) {
      uVar6 = 0xffffffff;
      uVar5 = 0x58;
      iVar2 = param_1 + 0x44;
      iVar4 = param_1 + 0x38;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("radius");
      uVar5 = FUN_00484d10(param_2,sVar3 + param_2,iVar4,iVar2,uVar5,uVar6);
      FUN_004861d0(uVar5);
    }
    else {
      pcVar1 = (char *)FUN_00484d10(param_2);
      iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("radiuskey",pcVar1);
      if (iVar2 == 0) {
        uVar6 = 0xffffffff;
        uVar5 = 0x58;
        iVar4 = param_1 + 0x44;
        iVar2 = param_1 + 0x38;
        sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("radiuskey");
        uVar5 = FUN_00484d10(param_2,sVar3 + param_2,iVar2,iVar4,uVar5,uVar6);
        FUN_00486700(uVar5);
      }
      else {
        pcVar1 = (char *)FUN_00484d10(param_2);
        iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("radiusbezierkey",pcVar1);
        if (iVar2 == 0) {
          uVar6 = 0xffffffff;
          uVar5 = 0x58;
          iVar2 = param_1 + 0x44;
          iVar4 = param_1 + 0x38;
/* ... 212 more lines truncated by --max-lines */
```

### decoy: 0x004878b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004878b0: 16-byte aligned: YES
  004878b0  55             PUSH EBP
  004878b1  8B EC          MOV EBP,ESP
  004878b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004878b0
string literals (336):
  0x0098D304  at 0048794d  'p2p_sel'
  0x0098D304  at 00487962  'p2p_sel'
  0x0098D2F4  at 00487988  'affectedByWind'
  0x0098D2F4  at 0048799d  'affectedByWind'
  0x0098D2E8  at 004879c3  'm_isTinted'
  0x0098D2E8  at 004879d8  'm_isTinted'
  0x0098D2E0  at 004879fe  'bounce'
  0x0098D2E0  at 00487a13  'bounce'
  0x00988318  at 00487a39  'random'
  0x00988318  at 00487a4e  'random'
  0x0098D2D8  at 00487a74  'inherit'
  0x0098D2D8  at 00487a89  'inherit'
direct callees (25):
  0x00484D10  Global::AurMessageGetToken
  0x009370C7  _stricmp
  0x00919F10  _strlen
  0x00484F10  (unnamed)
  0x00484E20  (unnamed)
  0x004E52D0  (unnamed)
  0x004E52F0  (unnamed)
  0x004E5310  (unnamed)
  0x004E5330  (unnamed)
  0x004E5350  (unnamed)
  0x004E5370  (unnamed)
  0x004E5390  (unnamed)
  0x004E53C0  (unnamed)
  0x004E53E0  (unnamed)
  0x004E5410  (unnamed)
  0x004E5440  (unnamed)
  0x004E5470  (unnamed)
  0x004E54A0  (unnamed)
  0x00484EE0  (unnamed)
  0x00484DC0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 2293 B, RET 4
- callers 1: 0x00453b50
- callees: 0x00484d10 (CALL), 0x009370c7 _stricmp (CALL), 0x00919f10 _strlen (CALL), 0x004861d0 (CALL), 0x00486700 (CALL), 0x00486900 (CALL), ...
- strings: 0x0098c6f0 "fadingLight", 0x0098c6fc "flareradius", 0x0098c708 "flarecolorshifts", ...
- globals: 0x00a309bd READ
- twins: none

## Card B17-9-13

GOG function: MdlNodeTriMesh::ProcessVerts  gog 0x0083e200  callconv=(blank)  param_size_bytes=0  gog_size=2857  gog_retn=0
Candidate Steam address: 0x0044bd70 (size 2857, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.92) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0044b8e0:STR0/CALLEE0/SIG0.366/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0083dd70 MdlNodeTriMesh::InternalGenVertices  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0083ed70 MdlNodeTriMesh::BuildVertexIndices  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x0081b9c0 unnamed -> Steam ?  in candidate callees: n/a
  0x0083ed30 unnamed -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 23 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x48*,0x4c*,0x58*,...(+9)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x48*,0x4c*,0x58*,...(+9)}
imm sets (* = in both, 11 common): GOG {0x3*,0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0x54*,0x80*,0x194*} | Steam {0x3*,0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0x54*,0x80*,0x194*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0044bd70: 16-byte aligned: YES
  0044bd70  55             PUSH EBP
  0044bd71  8B EC          MOV EBP,ESP
  0044bd73  81 EC 94 01 00 00 SUB ESP,0x194
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0044bd70
string literals (0):
direct callees (8):
  0x00429330  (unnamed)
  0x0091C860  _ftol2_sse
  0x0044C900  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x005142F0  (unnamed)
  0x00514340  (unnamed)
  0x00436200  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x38  via EAX,ECX,EDX
  +0x4 [dec 4]  x28  via EAX,ECX,EDX
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x8 [dec 8]  x22  via EAX,ECX,EDX
  +0x4C [dec 76]  x3  via EAX,ECX
  +0x64 [dec 100]  x3  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x6C [dec 108]  x3  via EAX,ECX,EDX
  +0x70 [dec 112]  x3  via EAX,ECX,EDX
  +0x74 [dec 116]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0044bd70 FUN_0044bd70 size=2857 ret=0 callers=4 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_0044bd70(int param_1)

{
  float fVar1;
  undefined4 uVar2;
  float *pfVar3;
  float *pfVar4;
  undefined4 *puVar5;
  float10 fVar6;
  float local_194;
  float local_190;
  float local_18c;
  float local_188;
  float local_184;
  float local_180;
  undefined1 local_bc [12];
  float local_b0;
  float local_ac;
  float local_a8;
  undefined1 local_a4 [12];
  float local_98;
  float local_94;
  float local_90;
  undefined1 local_8c [12];
  undefined1 local_80 [12];
  float local_74;
  float local_70;
  float local_6c;
  float local_68;
  int local_64;
  float *local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  int local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  uint local_24;
  int local_20;
  int local_1c;
  int local_18;
  float local_14;
  float local_10;
  float local_c;
  int local_8;
  
  if ((DAT_00a1fff4 == 0) && (*(char *)(param_1 + 0x184) != '\0')) {
    for (local_20 = 0; local_20 < *(int *)(*(int *)(param_1 + 0x134) + 0x4c);
        local_20 = local_20 + 1) {
      local_74 = (float)**(int **)(local_20 * 0x14 + *(int *)(*(int *)(param_1 + 0x134) + 0x48) + 4)
      ;
      local_70 = (float)*(int *)(*(int *)(local_20 * 0x14 +
                                          *(int *)(*(int *)(param_1 + 0x134) + 0x48) + 4) + 4);
      local_6c = (float)*(int *)(*(int *)(local_20 * 0x14 +
                                          *(int *)(*(int *)(param_1 + 0x134) + 0x48) + 4) + 8);
      FUN_00429330(local_74,local_70,local_6c);
    }
  }
  local_8 = 0;
  do {
    if (*(int *)(*(int *)(param_1 + 0x134) + 0x4c) <= local_8) {
      local_14 = 0.0;
      local_10 = 0.0;
      local_c = 0.0;
      for (local_8 = 0; local_8 < *(int *)(*(int *)(param_1 + 0x134) + 4); local_8 = local_8 + 1) {
        local_60 = (float *)(local_8 * 0xc + **(int **)(param_1 + 0x134));
        if (*local_60 <= *(float *)(param_1 + 0x64 [dec 100])) {
          local_180 = *local_60;
/* ... 176 more lines truncated by --max-lines */
```

### decoy: 0x0044b8e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0044b8e0: 16-byte aligned: YES
  0044b8e0  55             PUSH EBP
  0044b8e1  8B EC          MOV EBP,ESP
  0044b8e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0044b8e0
string literals (0):
direct callees (7):
  0x00919723  operator new
  0x004CD700  (unnamed)
  0x004CDA60  (unnamed)
  0x009196FD  operator delete
  0x004491C0  CExoPtrList::Add (doubling)
  0x004543C0  (unnamed)
  0x004517A0  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x24  via EAX,ECX,EDX
  +0x58 [dec 88]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x1A [dec 26]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via EDX
  +0x64 [dec 100]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
  +0x18 [dec 24]  x1  via ECX
```

### Static facts (script output, report-only)
- size 2857 B, RET 0
- callers 4: 0x0044e3e0, 0x0044eed0, 0x0044f650, 0x004506c0
- callees: 0x00429330 (CALL), 0x0091c860 (CALL), 0x0044c900 (CALL), 0x004452b0 (CALL), 0x0044c8a0 (CALL), 0x005142f0 (CALL), ...
- twins: none

## Card B17-9-14

GOG function: Global::RotationArc  gog 0x0080df50  callconv=(blank)  param_size_bytes=28  gog_size=362  gog_retn=0
Candidate Steam address: 0x005160c0 (size 362, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 6 (callers: GetHeadTurnVertical 0x774260, UpdateShadowBlob 0x774620, SetGroundNormal 0x7f1f10); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00515e60:STR0/CALLEE0/SIG0.433/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080dcf0 Global::pointto  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x00924754 Global::sqrt -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 16): the card's A-CALLEE credit may come from these, not from callees
  0x00420130 CSWCCreature::ApplyShadowBlob -> Steam 0x00774260  in candidate callers: yes
  0x004204f0 CSWCCreature::UpdateShadowBlob -> Steam 0x00774620  in candidate callers: yes
  0x00477070 CSWCObject::SetGroundNormal -> Steam 0x007f1f10  in candidate callers: yes
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 3 common): GOG {0x8*,0x28*,0x41*} | Steam {0x8*,0x28*,0x41*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 28 bytes -> x86 slots 28; ASM RET 0)
```

### prologue
```
prologue for 0x005160c0: 16-byte aligned: YES
  005160c0  55             PUSH EBP
  005160c1  8B EC          MOV EBP,ESP
  005160c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005160c0
string literals (0):
direct callees (2):
  0x00514340  (unnamed)
  0x0091CB24  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005160c0 FUN_005160c0 size=362 ret=0 callers=16 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float * FUN_005160c0(float *param_1,float param_2,float param_3,float param_4,float param_5,
                    float param_6,float param_7)

{
  float fVar1;
  float10 fVar2;
  
  if ((DAT_00a7d808 & 1) == 0) {
    DAT_00a7d808 = DAT_00a7d808 | 1;
    DAT_00a7d804 = 0.0;
    DAT_00a7d800 = 0.0;
    DAT_00a7d7fc = 0.0;
    DAT_00a7d7f8 = 1.0;
  }
  FUN_00514340();
  FUN_00514340();
  fVar1 = param_5 * param_2 + param_6 * param_3 + param_7 * param_4;
  if (fVar1 < (float)_DAT_00986d98 == (fVar1 == (float)_DAT_00986d98)) {
    fVar2 = (float10)FUN_0091cb24((double)(fVar1 + 1.0 + fVar1 + 1.0));
    fVar1 = (float)fVar2;
    DAT_00a7d7fc = (param_7 * param_3 - param_6 * param_4) / fVar1;
    DAT_00a7d800 = (param_5 * param_4 - param_7 * param_2) / fVar1;
    DAT_00a7d804 = (param_6 * param_2 - param_5 * param_3) / fVar1;
    DAT_00a7d7f8 = fVar1 / (float)_DAT_00987490;
    *param_1 = DAT_00a7d7f8;
    param_1[1 = byte +0x4] = DAT_00a7d7fc;
    param_1[2 = byte +0x8] = DAT_00a7d800;
    param_1[3 = byte +0xc] = DAT_00a7d804;
  }
  else {
    *param_1 = 0.0;
    param_1[1 = byte +0x4] = 1.0;
    param_1[2 = byte +0x8] = 0.0;
    param_1[3 = byte +0xc] = 0.0;
  }
  return param_1;
}
```

### decoy: 0x00515e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00515e60: 16-byte aligned: YES
  00515e60  55             PUSH EBP
  00515e61  8B EC          MOV EBP,ESP
  00515e63  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00515e60
string literals (0):
direct callees (4):
  0x005143C0  (unnamed)
  0x005142F0  (unnamed)
  0x005195A0  (unnamed)
  0x0091FA70  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
```

### Static facts (script output, report-only)
- size 362 B, RET 0
- callers 16: 0x00462d50, 0x004a9ea0, 0x004dbac0, 0x004ece70, 0x004ee640, ...
- callees: 0x00514340 (CALL), 0x0091cb24 (CALL)
- twins: none

## Card B17-9-15

GOG function: RequestPoolManagerVBO::ResizePool  gog 0x0081c950  callconv=(blank)  param_size_bytes=8  gog_size=428  gog_retn=8
Candidate Steam address: 0x0042a220 (size 428, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.57) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0042a150:STR0/CALLEE0/SIG0.588/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0081c920 RequestPoolManagerVBO::GetPoolSize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0081cb00 RequestPoolManagerVBO::DeletePool -> 0x0042a3d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x009242da unnamed -> Steam ?  in candidate callees: n/a
  0x00924eb8 Global::new -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x2c*,0x50*,0x5c*,0x68*} | Steam {0x2c*,0x50*,0x5c*,0x68*}
imm sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x8764*,0x88b8*,0x88e8*} | Steam {0x4*,0x8*,0xc*,0x2c*,0x8764*,0x88b8*,0x88e8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0042a220: 16-byte aligned: YES
  0042a220  55             PUSH EBP
  0042a221  8B EC          MOV EBP,ESP
  0042a223  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a220
string literals (0):
direct callees (3):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x0091C6AA  operator delete[]
struct offsets touched (top 4):
  +0x50 [dec 80]  x7  via EAX,EDX
  +0x2C [dec 44]  x5  via EAX,EDX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a220 FUN_0042a220 size=428 ret=8 callers=0 callees=3

void __thiscall FUN_0042a220(int param_1,int param_2,undefined4 param_3)

{
  undefined4 local_14;
  void *local_10;
  void *local_c;
  size_t local_8;
  
  if (param_2 != -1) {
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32b18)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0x8764,&local_8);
    local_10 = (void *)FUN_0091d288(local_8);
    local_c = (void *)(*DAT_00a32bac)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),35000)
    ;
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](local_10,local_c,local_8);
    (*DAT_00a32c68)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4));
    local_14 = *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d54)(1,&local_14);
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4) = 0;
    (*DAT_00a32b28)(1,*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32ab4)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),param_3,0,0x88e8);
    (*DAT_00a32ce8)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0,local_8,local_10);
    FUN_0091c6aa(*(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4) = param_3;
    FUN_0091c6aa(local_10);
  }
  return;
}
```

### decoy: 0x0042a150 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a150: 16-byte aligned: YES
  0042a150  55             PUSH EBP
  0042a151  8B EC          MOV EBP,ESP
  0042a153  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0042a150
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x8C [dec 140]  x2  via EAX,ECX
  +0x2C [dec 44]  x1  via EAX
  +0x50 [dec 80]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
  +0x68 [dec 104]  x1  via EDX
```

### Static facts (script output, report-only)
- size 428 B, RET 8
- callees: 0x0091d288 operator new[] (CALL), 0x0091dba0 _memcpy (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a256 unresolved, 0x0042a272 unresolved, 0x0042a2a7 unresolved, 0x0042a2d1 unresolved, 0x0042a2ec unresolved, ...
- twins: none

## Card B17-9-16

GOG function: PartEmitter::RenderLinked  gog 0x008af6e0  callconv=(blank)  param_size_bytes=0  gog_size=8781  gog_retn=0
Candidate Steam address: 0x004f5d90 (size 91, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f3b40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008af370 PartEmitter::RecursiveFractal  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008b1b50 PartEmitter::RenderMotionBlurred  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x00811430 Quaternion::Constructor -> Steam ?  in candidate callees: n/a
  0x00823860 GLRender::RenderObjectsVTCQuadUP -> Steam ?  in candidate callees: n/a
  0x008a0e30 Global::Detonate -> Steam ?  in candidate callees: n/a
  0x008a4d80 PartEmitter::GetTileAlpha -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x3c,0x40,0x44,0x4c,0x50,0x54,0x64,0x68,0x6c,...(+19)} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x4*,0x5,0x8,0xc*,0x10,0x13,0x18*,0x2c,0x41,0x44,0x48,0xb4,...(+6)} | Steam {0x4*,0xc*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5d90: 16-byte aligned: YES
  004f5d90  55             PUSH EBP
  004f5d91  8B EC          MOV EBP,ESP
  004f5d93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004f5d90
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0091DBA0  _memcpy
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004f5d90 FUN_004f5d90 size=91 ret=0 callers=3 callees=3

void * FUN_004f5d90(void *param_1,int param_2,uint param_3)

{
  void *_Dst [GHIDRA-AUTO-LABEL: unreliable];
  
  _Dst [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(-(uint)((int)((ulonglong)param_3 * 0x18 >> 0x20) != 0) |
                              (uint)((ulonglong)param_3 * 0x18));
  _memcpy [GHIDRA-AUTO-LABEL: unreliable](_Dst [GHIDRA-AUTO-LABEL: unreliable],param_1,param_2 * 0x18);
  FUN_009196fd(param_1);
  return _Dst [GHIDRA-AUTO-LABEL: unreliable];
}
```

### decoy: 0x004f3b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f3b40: 16-byte aligned: YES
  004f3b40  55             PUSH EBP
  004f3b41  8B EC          MOV EBP,ESP
  004f3b43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004f3b40
string literals (0):
direct callees (19):
  0x00919723  operator new
  0x004E5290  (unnamed)
  0x004F39B0  (unnamed)
  0x004F5D90  (unnamed)
  0x004DAE60  GUI image texture bind (MakeCurrent via vtbl +0xB8)
  0x004DB1F0  (unnamed)
  0x00463120  (unnamed)
  0x005195A0  (unnamed)
  0x0046CE40  (unnamed)
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x004F36B0  (unnamed)
  0x0091C860  _ftol2_sse
  0x004F3740  (unnamed)
  0x0046CDA0  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00431140  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x36  via EAX,ECX,EDX
  +0x8 [dec 8]  x33  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x15  via EAX,ECX,EDX
  +0xC [dec 12]  x10  via EAX,ECX,EDX
  +0x98 [dec 152]  x10  via EAX,ECX,EDX
  +0x9C [dec 156]  x9  via EAX,ECX,EDX
  +0x1CC [dec 460]  x8  via EAX,ECX,EDX
  +0x988530 [dec 9995568]  x8  via EAX,ECX,EDX
  +0x40 [dec 64]  x6  via EAX,ECX
  +0x70 [dec 112]  x5  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 91 B, RET 0
- callers 3: 0x004f3b40, 0x004f5f50, 0x004f8000
- callees: 0x00919723 operator new (CALL), 0x0091dba0 _memcpy (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-9-17

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x00435720 (size 1885, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004356e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications -> 0x00435720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x28}
imm sets (* = in both, 0 common): GOG {0xc,0x18} | Steam {0x6,0x14}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435720 FUN_00435720 size=1885 ret=0 callers=1 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00435720(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
  }
  if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
      wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1ddc8);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      wglMakeCurrent(DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1dddc);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
    else {
      (*DAT_00a32db4)(DAT_00a1ddc4,DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      (*DAT_00a32db4)(DAT_00a1ddd8,DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
  }
  if (DAT_00a1dde4 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde4);
    DAT_00a1dde4 = 0;
  }
  if (DAT_00a1dde8 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde8);
    DAT_00a1dde8 = 0;
  }
  if (DAT_00a1ddd0 != 0) {
    glDeleteTextures(1,&DAT_00a1ddd0);
    DAT_00a1ddd0 = 0;
  }
  if (DAT_00a1dd44 != 0) {
    glDeleteTextures(1,&DAT_00a1dd44);
    DAT_00a1dd44 = 0;
  }
  if (DAT_00a1dd38 != 0) {
    glDeleteTextures(1,&DAT_00a1dd38);
/* ... 111 more lines truncated by --max-lines */
```

### decoy: 0x004356e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B17-9-18

GOG function: CExoSoundSource::SetResRef  gog 0x00639ec0  callconv=(blank)  param_size_bytes=8  gog_size=90  gog_retn=8
Candidate Steam address: 0x0070aee0 (size 51, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.35 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070af20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00639e80 CExoSoundSource::SetDistance  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00639f20 CExoSoundSource::Stop  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00634cf0 CExoSoundInternal::Render -> Steam ?  in candidate callees: n/a
  0x00639850 unnamed -> Steam ?  in candidate callees: n/a
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x4c,0x60} | Steam {0x4*,0x74,0x78}
imm sets (* = in both, 1 common): GOG {0x8*,0xc} | Steam {0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0070aee0: 16-byte aligned: YES
  0070aee0  55             PUSH EBP
  0070aee1  8B EC          MOV EBP,ESP
  0070aee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070aee0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x74 [dec 116]  x1  via ECX
  +0x78 [dec 120]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070aee0 FUN_0070aee0 size=51 ret=8 callers=1 callees=0

void __thiscall FUN_0070aee0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  
  if (*(int *)(param_1 + 4) != 0) {
    iVar1 = *(int *)(param_1 + 4);
    *(undefined4 *)(iVar1 + 0x78) = param_2;
    *(undefined4 *)(iVar1 + 0x74) = param_3;
  }
  return;
}
```

### decoy: 0x0070af20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070af20: 16-byte aligned: YES
  0070af20  55             PUSH EBP
  0070af21  8B EC          MOV EBP,ESP
  0070af23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070af20
string literals (0):
direct callees (3):
  0x0070AF80  (unnamed)
  0x00705D70  CExoSoundInternal::Render (per-frame)
  0x0070A8D0  Source::LoadWaveResource (ret 8)
struct offsets touched (top 3):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x4C [dec 76]  x1  via EDX
  +0x60 [dec 96]  x1  via EAX
```

### Static facts (script output, report-only)
- size 51 B, RET 8
- callers 1: 0x00800620
- twins: none

## Card B17-9-19

GOG function: CClientOptions::SetSoftShadows  gog 0x00468f30  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc290 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc260:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x30} | Steam {0x34}
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
prologue for 0x007bc290: 16-byte aligned: YES
  007bc290  55             PUSH EBP
  007bc291  8B EC          MOV EBP,ESP
  007bc293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc290
string literals (0):
direct callees (2):
  0x00478CD0  Global::CAurBehavior_camera_local_GetType
  0x00478CE0  Global::CAurBehavior_camera_local_destructor
struct offsets touched (top 1):
  +0x34 [dec 52]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc290 FUN_007bc290 size=43 ret=4 callers=2 callees=2

void __thiscall FUN_007bc290(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x34) = param_2;
  if (*(int *)(param_1 + 0x34) == 0) {
    FUN_00478ce0();
  }
  else {
    FUN_00478cd0();
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
- callers 2: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d5c50
- callees: 0x00478cd0 (CALL), 0x00478ce0 (CALL)
- twins: none

## Card B17-9-20

GOG function: CClientOptions::SetVSync  gog 0x004691f0  callconv=(blank)  param_size_bytes=4  gog_size=53  gog_retn=4
Candidate Steam address: 0x007bc560 (size 51, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc520:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004691c0 CClientOptions::SetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00469230 CClientOptions::SetGammaSetting  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086ac50 Global::AurSetVSyncState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0xc,0x48*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007bc560: 16-byte aligned: YES
  007bc560  55             PUSH EBP
  007bc561  8B EC          MOV EBP,ESP
  007bc563  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc560
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EAX
  +0x48 [dec 72]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc560 FUN_007bc560 size=51 ret=4 callers=1 callees=0

void __thiscall FUN_007bc560(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xc) = param_2;
  if (DAT_00a1b49c != 0) {
    *(undefined4 *)(DAT_00a1b49c + 0x48) = param_2;
  }
  return;
}
```

### decoy: 0x007bc520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc520: 16-byte aligned: YES
  007bc520  55             PUSH EBP
  007bc521  8B EC          MOV EBP,ESP
  007bc523  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc520
string literals (0):
direct callees (1):
  0x00478D70  (unnamed)
struct offsets touched (top 1):
  +0x48 [dec 72]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 4
- callers 1: 0x007b5d30
- globals: 0x00a1b49c READ, 0x00a1b49c READ
- twins: none

## Card B17-9-21

GOG function: GLRender::RenderObjectsVTCQuadUP  gog 0x00823860  callconv=(blank)  param_size_bytes=8  gog_size=737  gog_retn=0
Candidate Steam address: 0x00431140 (size 737, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.30) | G8 mac_rotation: rotation:0x00823cb0:0.76>0.61
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00431430:STR0/CALLEE0/SIG0.844/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008235c0 GLRender::DrawSphereBound  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00823cb0 GLRender::RenderObjectsVTQuadUP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x008255c0 GLRender::RequestPool -> Steam ?  in candidate callees: n/a
  0x008255f0 GLRender::LockPool -> Steam ?  in candidate callees: n/a
  0x00825620 GLRender::UnlockPool -> Steam ?  in candidate callees: n/a
  0x00827890 GLRender::SetClientArray12 -> Steam ?  in candidate callees: n/a
  0x00827920 GLRender::SetClientArray4ub -> Steam ?  in candidate callees: n/a
  0x00827a40 GLRender::SetClientArray8 -> Steam ?  in candidate callees: n/a
  0x0086b520 Global::AurVertexBufferObjectARBAvailable -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10*,0x14*} | Steam {0x10*,0x14*}
imm sets (* = in both, 16 common): GOG {0x2*,0x3*,0x4*,0x7*,0xb*,0xc*,0x10*,0x14*,0x18*,0x24*,0x42*,0x800*,0x1401*,0x1406*,...(+2)} | Steam {0x2*,0x3*,0x4*,0x7*,0xb*,0xc*,0x10*,0x14*,0x18*,0x24*,0x42*,0x800*,0x1401*,0x1406*,...(+2)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00431140: 16-byte aligned: YES
  00431140  55             PUSH EBP
  00431141  8B EC          MOV EBP,ESP
  00431143  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00431140
string literals (0):
direct callees (8):
  0x00484790  (unnamed)
  0x00432EB0  (unnamed)
  0x00432EE0  (unnamed)
  0x0091DBA0  _memcpy
  0x00432F10  (unnamed)
  0x00435180  (unnamed)
  0x00435210  (unnamed)
  0x00435330  (unnamed)
struct offsets touched (top 3):
  +0x988658 [dec 9995864]  x10  via EAX,ECX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00431140 FUN_00431140 size=737 ret=0 callers=4 callees=20

void FUN_00431140(void *param_1,int param_2)

{
  int iVar1;
  void *pvVar2;
  size_t _Size [GHIDRA-AUTO-LABEL: unreliable];
  int local_1c;
  int local_8;
  
  glGetIntegerv(0x8869,&DAT_00a1de94);
  for (local_8 = 0; local_8 < DAT_00a1de94; local_8 = local_8 + 1) {
    (*DAT_00a32cfc)(local_8);
  }
  iVar1 = FUN_00484790();
  if (iVar1 == 0) {
    glBegin(7);
    for (local_1c = 0; local_1c < param_2; local_1c = local_1c + 1) {
      pvVar2 = (void *)(local_1c * 0x18 + (int)param_1);
      glColor4ubv((int)pvVar2 + 0xc);
      glTexCoord2f(*(undefined4 *)((int)pvVar2 + 0x10),*(undefined4 *)((int)pvVar2 + 0x14));
      glVertex3fv(pvVar2);
    }
    glEnd();
  }
  else {
    if (param_2 < 0x800) {
      if (DAT_00a1de90 == 0) {
        FUN_00432eb0(0,DAT_009f50a4 << 0xb,0x42,0,&DAT_00a1de90);
      }
      _Size [GHIDRA-AUTO-LABEL: unreliable] = DAT_009f50a4 * param_2;
      pvVar2 = (void *)FUN_00432ee0(DAT_00a1de90,0,_Size [GHIDRA-AUTO-LABEL: unreliable],1);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](pvVar2,param_1,_Size [GHIDRA-AUTO-LABEL: unreliable]);
      FUN_00432f10(DAT_00a1de90);
      FUN_00435180(DAT_009f50a4,0,DAT_00a1de90,0);
      FUN_00435210(DAT_009f50a4,0xc,DAT_00a1de90,0);
      FUN_00435330(DAT_009f50a4,0x10,DAT_00a1de90,0);
      glEnableClientState(DAT_00988664);
      glEnableClientState(DAT_00988660);
      glEnableClientState(DAT_00988658);
      glDisableClientState(DAT_0098865c);
      glDrawArrays(7,0,param_2);
    }
    else {
      (*DAT_00a32d4c)(0x8892,0);
      glEnableClientState(DAT_00988664);
      glEnableClientState(DAT_00988660);
      glEnableClientState(DAT_00988658);
      glDisableClientState(DAT_0098865c);
      glColorPointer(4,0x1401,0x18,(int)param_1 + 0xc);
      glTexCoordPointer(2,0x1406,0x18,(int)param_1 + 0x10);
      glVertexPointer(3,0x1406,0x18,param_1);
      glDrawArrays(7,0,param_2);
      (*DAT_00a32d4c)(0x8892,0);
    }
    glDisableClientState(DAT_00988660);
    glDisableClientState(DAT_00988658);
  }
  return;
}
```

### decoy: 0x00431430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00431430: 16-byte aligned: YES
  00431430  55             PUSH EBP
  00431431  8B EC          MOV EBP,ESP
  00431433  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00431430
string literals (0):
direct callees (2):
  0x00484790  (unnamed)
  0x00432F30  (unnamed)
struct offsets touched (top 3):
  +0x988658 [dec 9995864]  x4  via EAX,ECX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
```

### Static facts (script output, report-only)
- size 737 B, RET 0
- callers 4: 0x0047ea60, 0x004f3b40, 0x004f5f50, 0x004f8000
- callees: 0x00000027 (CALL), 0x00484790 (CALL), 0x00432eb0 (CALL), 0x00432ee0 (CALL), 0x0091dba0 _memcpy (CALL), 0x00432f10 (CALL), ...
- indirect: 0x00431177 unresolved, 0x004312cf unresolved, 0x00431387 unresolved
- imports: glBegin (OPENGL32.DLL), glColor4ubv (OPENGL32.DLL), glColorPointer (OPENGL32.DLL), ...
- twins: none

## Card B17-9-22

GOG function: Global::AurTextureBindTargetIndex  gog 0x008144e0  callconv=(blank)  param_size_bytes=8  gog_size=69  gog_retn=0
Candidate Steam address: 0x00421de0 (size 69, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: AsNormalizeVectorCubeMap 0x427110)
- best decoy (neighbour/other candidate) scores: 0x00421cf0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008143f0 Global::AurTextureBindTID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00814530 CAurTexture::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x84fe*} | Steam {0x84fe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00421de0: 16-byte aligned: YES
  00421de0  55             PUSH EBP
  00421de1  8B EC          MOV EBP,ESP
  00421de3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00421de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00421de0 FUN_00421de0 size=69 ret=0 callers=5 callees=2

void FUN_00421de0(int param_1,undefined4 param_2)

{
  glBindTexture((&DAT_009f4928)[param_1],param_2);
  glTexParameteri((&DAT_009f4928)[param_1],0x84fe,DAT_009f48f4);
  *(undefined4 *)(&DAT_00a1dd0c + DAT_00a1dd70 * 4) = param_2;
  return;
}
```

### decoy: 0x00421cf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00421cf0: 16-byte aligned: YES
  00421cf0  55             PUSH EBP
  00421cf1  8B EC          MOV EBP,ESP
  00421cf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00421cf0
string literals (0):
direct callees (2):
  0x004849E0  (unnamed)
  0x00421DE0  (unnamed)
struct offsets touched (top 1):
  +0x9F4928 [dec 10438952]  x4  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 69 B, RET 0
- callers 5: 0x00421cf0, 0x00427110 CAurTextureBasic::AsNormalizeVectorCubeMap, 0x00436fb0, 0x004a1070, 0x004c0dd0
- callees: 0x00000022 (CALL), 0x00000021 (CALL)
- imports: glBindTexture (OPENGL32.DLL), glTexParameteri (OPENGL32.DLL)
- twins: none
