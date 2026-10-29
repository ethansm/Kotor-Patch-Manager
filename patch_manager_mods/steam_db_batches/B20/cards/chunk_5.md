# Review cards B20 chunk 5

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-5-01

GOG function: Global::AurMessageParseFloat  gog 0x008c5350  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=0
Candidate Steam address: 0x00484e00 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484e20:STR0/CALLEE3/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c5310 Global::AurMessageParseString  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c5370 Global::AurMessageParseIntOrBool  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009243dd Global::sscanf -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x008161b0 CAurTextureBasic::ParseField -> Steam 0x00423ab0  in candidate callers: yes
  0x008c7500 MdlNodeLight::InternalParseField -> Steam 0x00486fb0  in candidate callers: yes
  0x008c7e00 MdlNodeEmitter::InternalParseField -> Steam 0x004878b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00484e00: 16-byte aligned: YES
  00484e00  55             PUSH EBP
  00484e01  8B EC          MOV EBP,ESP
  00484e03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484e00
string literals (0):
direct callees (1):
  0x0091C7AD  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484e00 FUN_00484e00 size=32 ret=0 callers=18 callees=1
THUNK -> 0x0091C7AD (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall FUN_00484e00(undefined4 param_1,char *param_2,undefined4 param_3)

{
  FID_conflict__sscanf [GHIDRA-AUTO-LABEL: unreliable](param_2,"%f",param_3,param_1);
  return;
}
```

### decoy: 0x00484e20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484e20: 16-byte aligned: YES
  00484e20  55             PUSH EBP
  00484e21  8B EC          MOV EBP,ESP
  00484e23  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484e20
string literals (0):
direct callees (3):
  0x0091C7AD  (unnamed)
  0x009370C7  _stricmp
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 32 B, RET 0
- callers 18: 0x00423ab0 CAurTextureBasic::ParseField, 0x00424f60, 0x00486fb0 MdlNodeLight::InternalParseField, 0x004878b0 MdlNodeEmitter::InternalParseField, 0x0048b6f0, ...
- callees: 0x0091c7ad (CALL)
- twins: none

## Card B20-5-02

GOG function: CExoArrayList_Vector_::Allocate  gog 0x00457b00  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x004293b0 (size 49, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00429330:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457ac0 CExoArrayList_Vector_::CExoArrayList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457bc0 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 11): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004293b0: 16-byte aligned: YES
  004293b0  55             PUSH EBP
  004293b1  8B EC          MOV EBP,ESP
  004293b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004293b0
string literals (0):
direct callees (1):
  0x00474370  Target::AddBackRef(holder)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004293b0 FUN_004293b0 size=49 ret=4 callers=1 callees=1

int * __thiscall FUN_004293b0(int *param_1,int param_2)

{
  *param_1 = param_2;
  if (*param_1 != 0) {
    FUN_00474370(param_1);
  }
  return param_1;
}
```

### decoy: 0x00429330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00429330: 16-byte aligned: YES
  00429330  55             PUSH EBP
  00429331  8B EC          MOV EBP,ESP
  00429333  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00429330
string literals (0):
direct callees (1):
  0x004291C0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX
```

### Static facts (script output, report-only)
- size 49 B, RET 4
- callers 1: 0x00983430
- callees: 0x00474370 (CALL)
- twins: none

## Card B20-5-03

GOG function: CExoArrayList::DelIndex  gog 0x00903df0  callconv=__thiscall  param_size_bytes=4  gog_size=75  gog_retn=4
Candidate Steam address: 0x0049b840 (size 75, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 25 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 25 (callers: SetSpecialWalkAnimation_2 0x773970, UnloadArea 0x7a5fa0, Update 0x7a6760)
- best decoy (neighbour/other candidate) scores: 0x0049b820:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00903d10 TimeTrigger::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00903e40 IODispatcher::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (27 of 89): the card's A-CALLEE credit may come from these, not from callees
  0x0041f840 CSWCCreature::SetSpecialWalkAnimation_2 -> Steam 0x00773970  in candidate callers: yes
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: yes
  0x00453160 CSWCArea::Update -> Steam 0x007a6760  in candidate callers: yes
  0x00456ce0 CSWCArea::RemovePlaceableObjectLight -> Steam 0x007aa2e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0049b840: 16-byte aligned: YES
  0049b840  55             PUSH EBP
  0049b841  8B EC          MOV EBP,ESP
  0049b843  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0049b840
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0049b840 FUN_0049b840 size=75 ret=4 callers=89 callees=0

void __thiscall FUN_0049b840(int *param_1,int param_2)

{
  param_1[1 = byte +0x4] = param_1[1 = byte +0x4] + -1;
  for (; param_2 < param_1[1 = byte +0x4]; param_2 = param_2 + 1) {
    *(undefined4 *)(*param_1 + param_2 * 4) = *(undefined4 *)(*param_1 + 0x4 [dec 4] {int*: byte +0x10} + param_2 * 4);
  }
  return;
}
```

### decoy: 0x0049b820 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0049b820: 16-byte aligned: YES
  0049b820  55             PUSH EBP
  0049b821  8B EC          MOV EBP,ESP
  0049b823  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0049b820
string literals (0):
direct callees (1):
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 75 B, RET 4
- callers 89: 0x0040fdc0, 0x0040fe10, 0x00410670, 0x00410780, 0x004109c0 CSWGuiManager::PopModalPanel, ...
- twins: none

## Card B20-5-04

GOG function: CExoArrayList_Vector_::Allocate  gog 0x00457b00  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=4
Candidate Steam address: 0x004291c0 (size 257, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004292d0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00457ac0 CExoArrayList_Vector_::CExoArrayList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00457bc0 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 11): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0xc,0x28}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004291c0: 16-byte aligned: YES
  004291c0  55             PUSH EBP
  004291c1  8B EC          MOV EBP,ESP
  004291c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004291c0
string literals (0):
direct callees (2):
  0x0091D288  operator new[]
  0x0091C6AA  operator delete[]
struct offsets touched (top 2):
  +0x8 [dec 8]  x5  via ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004291c0 FUN_004291c0 size=257 ret=4 callers=3 callees=2

void __thiscall FUN_004291c0(int *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *local_2c;
  uint local_24;
  undefined4 *local_20;
  int local_c;
  
  iVar1 = *param_1;
  param_1[2 = byte +0x8] = param_2;
  local_24 = param_1[2 = byte +0x8];
  local_2c = (undefined4 *)
             FUN_0091d288(-(uint)((int)((ulonglong)local_24 * 0xc >> 0x20) != 0) |
                          (uint)((ulonglong)local_24 * 0xc));
  local_20 = local_2c;
  if (local_2c == (undefined4 *)0x0) {
    local_2c = (undefined4 *)0x0;
  }
  else {
    while (local_24 = local_24 - 1, -1 < (int)local_24) {
      *local_20 = 0;
      local_20[1 = byte +0x4] = 0;
      local_20[2 = byte +0x8] = 0;
      local_20 = local_20 + 0x3 [dec 3] {undefined4*: byte +0xc};
    }
  }
  *param_1 = (int)local_2c;
  for (local_c = 0; local_c < param_1[1 = byte +0x4]; local_c = local_c + 1) {
    puVar3 = (undefined4 *)(local_c * 0xc + iVar1);
    puVar2 = (undefined4 *)(*param_1 + local_c * 0xc);
    *puVar2 = *puVar3;
    puVar2[1 = byte +0x4] = puVar3[1 = byte +0x4];
    puVar2[2 = byte +0x8] = puVar3[2 = byte +0x8];
  }
  if (iVar1 != 0) {
    FUN_0091c6aa(iVar1);
  }
  return;
}
```

### decoy: 0x004292d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004292d0: 16-byte aligned: YES
  004292d0  55             PUSH EBP
  004292d1  8B EC          MOV EBP,ESP
  004292d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004292d0
string literals (0):
direct callees (2):
  0x0091C6AA  operator delete[]
  0x004291C0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
```

### Static facts (script output, report-only)
- size 257 B, RET 4
- callers 3: 0x00428f80, 0x004292d0, 0x00429330
- callees: 0x0091d288 operator new[] (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none

## Card B20-5-05

GOG function: Material::GetTextureTID  gog 0x008f5120  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x004daef0 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004daf90:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f50d0 Material::BindEnvMap  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f5150 Material::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008192e0 Global::AurTextureGetTID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004daef0: 16-byte aligned: YES
  004daef0  55             PUSH EBP
  004daef1  8B EC          MOV EBP,ESP
  004daef3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004daef0
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004daef0 FUN_004daef0 size=80 ret=0 callers=1 callees=2

void __fastcall FUN_004daef0(int param_1)

{
  FUN_004da750(param_1 + 4);
  (**(code **)(**(int **)(param_1 + 4) + 0xb8))();
  FUN_00426be0(*(undefined4 *)(param_1 + 4));
  return;
}
```

### decoy: 0x004daf90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004daf90: 16-byte aligned: YES
  004daf90  55             PUSH EBP
  004daf91  8B EC          MOV EBP,ESP
  004daf93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004daf90
string literals (0):
direct callees (1):
  0x00426BE0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 80 B, RET 0
- callers 1: 0x0049d340
- callees: 0x004da750 (CALL), 0x00426be0 (CALL)
- indirect: 0x004daf25 slot 0xb8
- twins: none

## Card B20-5-06

GOG function: Global::AurEnableShadows  gog 0x0086ab80  callconv=(blank)  param_size_bytes=0  gog_size=15  gog_retn=0
Candidate Steam address: 0x00984710 (size 29, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009846f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ab70 Global::AurInitExtensions  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ab90 Global::AurDisableShadows  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00468f00 CClientOptions::SetShadows -> Steam 0x007bc230  in candidate callers: no
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
prologue for 0x00984710: 16-byte aligned: YES
  00984710  55             PUSH EBP
  00984711  8B EC          MOV EBP,ESP
  00984713  D9 E8          FLD1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00984710
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00984710 FUN_00984710 size=29 ret=0 callers=0 callees=0

void FUN_00984710(void)

{
  DAT_00a7fe18 = 0x3f800000;
  DAT_00a7fe1c = 0x3f800000;
  DAT_00a7fe20 = 0x3f800000;
  return;
}
```

### decoy: 0x009846f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009846f0: 16-byte aligned: YES
  009846f0  55             PUSH EBP
  009846f1  8B EC          MOV EBP,ESP
  009846f3  D9 EE          FLDZ
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009846f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 29 B, RET 0
- twins: 0x00984640 (distinguishing: 0x00984715: FSTP float ptr [0x00a7fe18])

## Card B20-5-07

GOG function: MdlNode::InternalControl  gog 0x008f9030  callconv=(blank)  param_size_bytes=16  gog_size=139  gog_retn=None
Candidate Steam address: 0x004def00 (size 267, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: Control 0x4dea20); SIG 0.75
- best decoy (neighbour/other candidate) scores: 0x004dea20:STR0/CALLEE0/SIG0.042/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f8b50 NewController::Control -> 0x004dea20  trusted
- GOG-order neighbour after:  0x008f90bb AnimRun::m_pTraceRun  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008f8b50 NewController::Control -> Steam 0x004dea20  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x2c*,0x38*,0x3c*} | Steam {0x2c*,0x38*,0x3c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x2c*} | Steam {0x4*,0x5,0x10,0x2c*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x004def00: 16-byte aligned: YES
  004def00  55             PUSH EBP
  004def01  8B EC          MOV EBP,ESP
  004def03  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004def00
string literals (0):
direct callees (2):
  0x004DEA20  NewController::Control
  0x004DD580  (unnamed)
struct offsets touched (top 3):
  +0x38 [dec 56]  x3  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004def00 FUN_004def00 size=267 ret=16 callers=6 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_004def00(int param_1,int *param_2,float param_3,undefined4 param_4,undefined4 param_5)

{
  undefined4 local_18;
  
  (**(code **)(*param_2 + 0x2c))();
  for (local_18 = 0; local_18 < *(int *)(param_1 + 0x3c); local_18 = local_18 + 1) {
    FUN_004dea20(param_2,param_1,param_3,param_4,param_5);
    if (DAT_00a7bc18 != 0) {
      if ((float)_DAT_00991340 <= param_3) {
        FUN_004dd580(param_2,param_1,param_3,param_4);
      }
      else {
        FUN_004dd580(param_2,param_1,(float)_DAT_00991348 - param_3,param_4);
      }
    }
  }
  return;
}
```

### decoy: 0x004dea20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dea20: 16-byte aligned: YES
  004dea20  55             PUSH EBP
  004dea21  8B EC          MOV EBP,ESP
  004dea23  81 EC 2C 01 00 00 SUB ESP,0x12c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 267 B, RET 16
- callers 6: 0x00453860, 0x004df010, 0x004df050, 0x004df0a0, 0x004df0e0, ...
- callees: 0x004dea20 NewController::Control (CALL), 0x004dd580 (CALL)
- indirect: 0x004def37 slot 0x2c
- twins: none

## Card B20-5-08

GOG function: GLU32_DLL::gluBuild2DMipmaps  gog 0x009433e0  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b436 (size 6, retn None)  THUNK -> 0x00000145 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: fail:unaligned | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000145 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR n/a | A-CALLEE n/a | A-SIG n/a | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00401000:STRn/a/CALLEEn/a/SIGn/a/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00943360 DINPUT8_DLL::DirectInput8Create  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009433e6 GLU32_DLL::gluPerspective -> 0x0093b436  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
(no GOG fingerprint available for 0x009433e0)

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=n/a ; asm RET = none
Stack Slot Match: UNKNOWN (GOG 0 bytes -> x86 slots 0; ASM RET none)
```

### prologue
```
prologue: no asm for 0x00000145
```

### anchors
```
anchors: no asm for 0x00000145
```

### annotate (candidate, first 80 lines)
```
(no decompile for 0x00000145)
```

### decoy: 0x00401000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00401000: 16-byte aligned: YES
  00401000  55             PUSH EBP
  00401001  8B EC          MOV EBP,ESP
  00401003  C6 05 20 3C A1 00 00 MOV byte ptr [0x00a13c20],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00401000
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size unknown (not in steam_functions)
- asm: no decomp file in dump
- twins: none

## Card B20-5-09

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e480 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e360:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090cb30 CSWGuiPanel::Constructor -> 0x0040e5a0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e480 FUN_0040e480 size=285 ret=0 callers=1 callees=0

int * FUN_0040e480(int *param_1,int *param_2,int *param_3)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  if (*param_2 < *param_3) {
    local_20 = *param_2;
  }
  else {
    local_20 = *param_3;
  }
  if (param_2[1 = byte +0x4] < param_3[1 = byte +0x4]) {
    local_24 = param_2[1 = byte +0x4];
  }
  else {
    local_24 = param_3[1 = byte +0x4];
  }
  if (*param_3 + param_3[2 = byte +0x8] < *param_2 + param_2[2 = byte +0x8]) {
    local_28 = *param_2 + param_2[2 = byte +0x8];
  }
  else {
    local_28 = *param_3 + param_3[2 = byte +0x8];
  }
  if (param_3[1 = byte +0x4] + param_3[3 = byte +0xc] < param_2[1 = byte +0x4] + param_2[3 = byte +0xc]) {
    local_2c = param_2[1 = byte +0x4] + param_2[3 = byte +0xc];
  }
  else {
    local_2c = param_3[1 = byte +0x4] + param_3[3 = byte +0xc];
  }
  *param_1 = local_20;
  param_1[1 = byte +0x4] = local_24;
  param_1[2 = byte +0x8] = local_28 - local_20;
  param_1[3 = byte +0xc] = local_2c - local_24;
  return param_1;
}
```

### decoy: 0x0040e360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e360: 16-byte aligned: YES
  0040e360  55             PUSH EBP
  0040e361  8B EC          MOV EBP,ESP
  0040e363  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e360
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### Static facts (script output, report-only)
- size 285 B, RET 0
- callers 1: 0x008690a0
- twins: none

## Card B20-5-10

GOG function: CSWGuiContainer::Destructor  gog 0x005a6190  callconv=(blank)  param_size_bytes=4  gog_size=46  gog_retn=4
Candidate Steam address: 0x008b2f70 (size 1410, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiContainer::SetupGiveMode | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b3500:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a5a40 CSWGuiContainer::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a61c0 CSWGuiContainer::Destructor_2 -> 0x008b2620  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005a61c0 CSWGuiContainer::Destructor_2 -> Steam 0x008b2620  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10,0x70,0x88,0xa0,0x10c,0x2a4,0x2cc,0x2d0,0x824,0xafa,0xb00}
imm sets (* = in both, 1 common): GOG {0x4*,0x1c} | Steam {0x2,0x3,0x4*,0x5,0x7,0x8,0x9,0xa,0x27,0x2d,0x60,0x61,0x78,0xd8,...(+16)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008b2f70: 16-byte aligned: YES
  008b2f70  55             PUSH EBP
  008b2f71  8B EC          MOV EBP,ESP
  008b2f73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008b2f70
string literals (0):
direct callees (21):
  0x0084A640  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F7A30  CSWPartyTable::GetPartyItems
  0x00606160  CItemRepository::ItemListGetItemObjectID
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x00606100  CItemRepository::ItemListGetItem
  0x00919723  operator new
  0x008B0A60  CSWGuiInGameItemEntry::Constructor
  0x0083EA60  CExoArrayList<uint32>::Add
  0x008B1250  CSWGuiInGameItemEntry::SetItem
  0x00418AF0  CSWGuiControl::AddEvent
  0x00733570  CExoString::CExoString_char
  0x007408F0  CClientExoApp::GetGUIString
  0x00733DF0  CExoString::operator+
  0x00733780  CExoString::~CExoString
  0x00416EA0  CSWGuiTextParams::SetStrRef
  0x00416E30  CSWGuiTextParams::SetText
  0x0041FC30  CSWGuiListBox::AddControls
  0x0041E870  CSWGuiListBox::SetSelectedControl
  0x00760EA0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x70 [dec 112]  x2  via EAX,ECX
  +0x2CC [dec 716]  x2  via EAX,EDX
  +0xAFA [dec 2810]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EAX
  +0x88 [dec 136]  x1  via EDX
  +0xA0 [dec 160]  x1  via EDX
  +0x10C [dec 268]  x1  via EDX
  +0x2A4 [dec 676]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008b2f70 FUN_008b2f70 size=1410 ret=4 callers=2 callees=21

void __thiscall FUN_008b2f70(int *param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  int local_ec;
  undefined4 local_cc;
  undefined1 local_94 [8];
  undefined1 local_8c [8];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  int local_74;
  undefined4 local_70;
  int local_6c;
  int local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  uint local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  int local_48;
  int local_44;
  int local_3c;
  int local_38;
  undefined4 local_34;
  undefined1 local_30 [8];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978f71;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0084a640(0);
  local_8 = 0;
  local_3c = param_1[0x2c0 = byte +0xb00];
  local_38 = local_3c + 0xd8;
  local_14 = local_3c + 0x60;
  local_1c = local_3c + 0x148;
  local_34 = *(undefined4 *)(DAT_00a1b4a4 + 4);
  FUN_0051c8d0(uVar1);
  local_20 = FUN_005f7a30();
  local_6c = param_1[0x2b2 = byte +0xac8] + (uint)*(byte *)(param_1 + 0x2bd) * -2;
  local_68 = 0;
  local_64 = DAT_00a7fde8;
  local_60 = DAT_00a7fdec;
  local_5c = DAT_00a7fdf0;
  local_54 = DAT_00a7fdac;
  local_50 = DAT_00a7fdb0;
  local_4c = DAT_00a7fdb4;
  for (local_58 = 0; local_58 < *(uint *)(local_20 + 0x10); local_58 = local_58 + 1) {
    uVar2 = FUN_00606160(local_58);
    local_28 = FUN_0073fec0(uVar2);
    local_18 = FUN_00606100(local_58);
    if ((local_18 != 0) && (*(int *)(local_18 + 0x10c) == 0)) {
      local_74 = FUN_00919723(0x3b4);
      local_8._0_1_ = 1;
      if (local_74 == 0) {
        local_cc = 0;
      }
      else {
        local_cc = FUN_008b0a60();
      }
      local_70 = local_cc;
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_0083ea60(local_cc);
      (**(code **)(**(int **)(local_48 + local_68 * 4) + 0xa0))
                (local_38,local_14,local_1c,local_6c,1);
      FUN_008b1250(local_28,0,0);
/* ... 54 more lines truncated by --max-lines */
```

### decoy: 0x008b3500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b3500: 16-byte aligned: YES
  008b3500  55             PUSH EBP
  008b3501  8B EC          MOV EBP,ESP
  008b3503  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b3500
string literals (0):
direct callees (5):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008B1C40  CSWGuiInGameItemEntry::GetNumItems
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x00775670  CSWCCreature::PerformGiveItem
struct offsets touched (top 8):
  +0x70 [dec 112]  x4  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via ECX
  +0x68 [dec 104]  x1  via EDX
  +0x1D0 [dec 464]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1410 B, RET 4
- callers 2: 0x008b2850, 0x008b35d0 CSWGuiContainer::Update
- callees: 0x0084a640 (CALL), 0x0051c8d0 (CALL), 0x005f7a30 CSWPartyTable::GetPartyItems (CALL), 0x00606160 CItemRepository::ItemListGetItemObjectID (CALL), 0x0073fec0 CClientExoApp::ServerToClientObjectId (CALL), 0x00606100 CItemRepository::ItemListGetItem (CALL), ...
- indirect: 0x008b3177 slot 0xa0, 0x008b3398 slot 0x88, 0x008b344c slot 0x8, 0x008b346e slot 0x8
- twins: none

## Card B20-5-11

GOG function: CSWGuiPowersLevelUp::Destructor  gog 0x005f6ed0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00907360 (size 326, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiMainCharGen::Draw | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00907340:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f6370 CSWGuiPowersLevelUp::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x48,0x68,0x80,0x94,0xd8,0x104,0x2b38,0x2b3c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x6,0x30,0x3c,0xff,0x3ea}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00907360: 16-byte aligned: YES
  00907360  55             PUSH EBP
  00907361  8B EC          MOV EBP,ESP
  00907363  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00907360
string literals (0):
direct callees (3):
  0x007ED830  CSWCObject::GetAnimationBase
  0x0077D390  CSWCCreature::CGPauseCycle
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 10):
  +0x68 [dec 104]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x48 [dec 72]  x2  via ECX
  +0x2B38 [dec 11064]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
  +0x80 [dec 128]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0xD8 [dec 216]  x1  via EAX
  +0x104 [dec 260]  x1  via EDX
  +0x2B3C [dec 11068]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00907360 FUN_00907360 size=326 ret=4 callers=0 callees=3

void __thiscall FUN_00907360(int param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  float10 fVar4;
  
  if (((*(int *)(param_1 + 0x68) != 0) && (DAT_009f42b4 != 0)) &&
     ((*(uint *)(param_1 + 0x48) >> 6 & 1) != 0)) {
    piVar1 = (int *)FUN_007ed830();
    piVar1 = (int *)(**(code **)(*piVar1 + 0x8 [dec 8] {int*: byte +0x20}))(0xff);
    iVar3 = *(int *)(param_1 + 0x2b38);
    iVar2 = (**(code **)(*piVar1 + 0x104))();
    if (iVar2 != iVar3) {
      (**(code **)(**(int **)(param_1 + 0x68) + 0x94))(*(undefined4 *)(param_1 + 0x2b38));
      iVar3 = (**(code **)(**(int **)(param_1 + 0x2b3c) + 0x80))(0x3ea);
      if (iVar3 != 0) {
        *(undefined4 *)(iVar3 + 0x48) = 1;
        fVar4 = (float10)(**(code **)(**(int **)(param_1 + 0x68) + 0xd8))();
        *(undefined4 *)(iVar3 + 0x3c) = 0;
        *(undefined4 *)(iVar3 + 0x40) = 0;
        *(float *)(iVar3 + 0x44) = (float)fVar4;
      }
    }
    FUN_0077d390(param_2,0);
  }
  FUN_0040ec90(param_2);
  return;
}
```

### decoy: 0x00907340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00907340: 16-byte aligned: YES
  00907340  55             PUSH EBP
  00907341  8B EC          MOV EBP,ESP
  00907343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00907340
string literals (0):
direct callees (2):
  0x00906110  (unnamed)
  0x008FACA0  CSWGuiMainCharGen::BeginPlayGame
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```

### Static facts (script output, report-only)
- size 326 B, RET 4
- callees: 0x007ed830 (CALL), 0x0077d390 (CALL), 0x0040ec90 CSWGuiPanel::Draw (CALL)
- indirect: 0x009073b3 slot 0x8, 0x009073d2 slot 0x104, 0x00907401 slot 0x94, 0x00907428 slot 0x80, 0x00907451 slot 0xd8
- twins: none

## Card B20-5-12

GOG function: Global::AurEnableShadows  gog 0x0086ab80  callconv=(blank)  param_size_bytes=0  gog_size=15  gog_retn=0
Candidate Steam address: 0x00984760 (size 33, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::AurEnableGrass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00984730:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ab70 Global::AurInitExtensions  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ab90 Global::AurDisableShadows  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00468f00 CClientOptions::SetShadows -> Steam 0x007bc230  in candidate callers: no
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
prologue for 0x00984760: 16-byte aligned: YES
  00984760  55             PUSH EBP
  00984761  8B EC          MOV EBP,ESP
  00984763  D9 05 A4 75 98 00 FLD float ptr [0x009875a4]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00984760
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00984760 FUN_00984760 size=33 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00984760(void)

{
  DAT_00a7fe0c = _DAT_009875a4;
  DAT_00a7fe10 = 0x3f800000;
  DAT_00a7fe14 = 0;
  return;
}
```

### decoy: 0x00984730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00984730: 16-byte aligned: YES
  00984730  55             PUSH EBP
  00984731  8B EC          MOV EBP,ESP
  00984733  D9 EE          FLDZ
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00984730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 33 B, RET 0
- twins: none

## Card B20-5-13

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

## Card B20-5-14

GOG function: List_SafePointer_WindManager___::Destructor  gog 0x008fe2a0  callconv=__thiscall  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x00463b20 (size 41, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 12 (callers: EnableRenderBumpedOut 0x45bf50, DisableRenderBumpedOut 0x45c490, SetLightPriority 0x45c580)
- best decoy (neighbour/other candidate) scores: 0x00463a90:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fe280 Global::AurVertexProgramMakeCurrent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fe2d0 PointSourceWind::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (24 of 466): the card's A-CALLEE credit may come from these, not from callees
  0x0041e770 CSWCCreature::DoFootstepVisual -> Steam 0x00772900  in candidate callers: no
  0x0044f6b0 CSWCArea::Destructor_2 -> Steam 0x007a2cb0  in candidate callers: no
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00463b20: 16-byte aligned: YES
  00463b20  55             PUSH EBP
  00463b21  8B EC          MOV EBP,ESP
  00463b23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00463b20
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00463b20 FUN_00463b20 size=41 ret=0 callers=241 callees=1

void __fastcall FUN_00463b20(int *param_1)

{
  if (*param_1 != 0) {
    FUN_009196fd(*param_1);
  }
  return;
}
```

### decoy: 0x00463a90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463a90: 16-byte aligned: YES
  00463a90  55             PUSH EBP
  00463a91  8B EC          MOV EBP,ESP
  00463a93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00463a90
string literals (0):
direct callees (3):
  0x00463FF0  (unnamed)
  0x00463BA0  (unnamed)
  0x00463B20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### Static facts (script output, report-only)
- size 41 B, RET 0
- callers 241: 0x00451ff0, 0x00452ab0, 0x00452f80, 0x004545d0, 0x00454750, ...
- callees: 0x009196fd operator delete (CALL)
- twins: none

## Card B20-5-15

GOG function: GLU32_DLL::gluBuild2DMipmaps  gog 0x009433e0  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b3b0 (size 6, retn None)  THUNK -> 0x00000143 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000143 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR n/a | A-CALLEE n/a | A-SIG n/a | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00401000:STRn/a/CALLEEn/a/SIGn/a/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00943360 DINPUT8_DLL::DirectInput8Create  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009433e6 GLU32_DLL::gluPerspective -> 0x0093b436  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
(no GOG fingerprint available for 0x009433e0)

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=n/a ; asm RET = none
Stack Slot Match: UNKNOWN (GOG 0 bytes -> x86 slots 0; ASM RET none)
```

### prologue
```
prologue: no asm for 0x00000143
```

### anchors
```
anchors: no asm for 0x00000143
```

### annotate (candidate, first 80 lines)
```
(no decompile for 0x00000143)
```

### decoy: 0x00401000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00401000: 16-byte aligned: YES
  00401000  55             PUSH EBP
  00401001  8B EC          MOV EBP,ESP
  00401003  C6 05 20 3C A1 00 00 MOV byte ptr [0x00a13c20],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00401000
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size unknown (not in steam_functions)
- asm: no decomp file in dump
- twins: none

## Card B20-5-16

GOG function: CSWGuiPowersLevelUp::Destructor  gog 0x005f6ed0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x009084f0 (size 74, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00908460:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f6370 CSWGuiPowersLevelUp::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x1c,0xac}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x009084f0: 16-byte aligned: YES
  009084f0  55             PUSH EBP
  009084f1  8B EC          MOV EBP,ESP
  009084f3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009084f0
string literals (0):
direct callees (2):
  0x0073F750  CClientExoApp::GetInGameGui
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0xAC [dec 172]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 009084f0 FUN_009084f0 size=74 ret=4 callers=1 callees=2

void FUN_009084f0(void)

{
  int iVar1;
  
  iVar1 = FUN_0073f750();
  FUN_00410530(*(undefined4 *)(iVar1 + 0xac),1,1);
  return;
}
```

### decoy: 0x00908460 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00908460: 16-byte aligned: YES
  00908460  55             PUSH EBP
  00908461  8B EC          MOV EBP,ESP
  00908463  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00908460
string literals (0):
direct callees (5):
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x0073F750  CClientExoApp::GetInGameGui
  0x0075CA30  CSWGuiMessageBox::SetCallback
  0x009084F0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0xB0 [dec 176]  x1  via EAX
  +0x1BB4 [dec 7092]  x1  via EAX
```

### Static facts (script output, report-only)
- size 74 B, RET 4
- callers 1: 0x00908460 CSWGuiPowersLevelUp::OnPanelAdded
- callees: 0x0073f750 CClientExoApp::GetInGameGui (CALL), 0x00410530 CSWGuiManager::AddPanel (CALL)
- twins: none

## Card B20-5-17

GOG function: CAuroraTexture::GetTextureDataPtr  gog 0x00909e50  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f830 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f7e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00909e00 CAuroraTexture::GetTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00909e90 CAuroraTexture::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x00943a40 CResTGA::GetTGADataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0093f830: 16-byte aligned: YES
  0093f830  55             PUSH EBP
  0093f831  8B EC          MOV EBP,ESP
  0093f833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f830
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453F0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093f830 FUN_0093f830 size=51 ret=0 callers=3 callees=2

undefined4 __fastcall FUN_0093f830(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_009453f0();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093f7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f7e0: 16-byte aligned: YES
  0093f7e0  55             PUSH EBP
  0093f7e1  8B EC          MOV EBP,ESP
  0093f7e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f7e0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453A0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 3: 0x0093b910, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x0093fab0
- callees: 0x00711250 (CALL), 0x009453f0 (CALL)
- twins: 0x0093f870 (distinguishing: 0x0093f85a: CALL 0x009453f0)

## Card B20-5-18

GOG function: CAuroraInterface::GetProcessedTextureInternal_thunk  gog 0x009080d0  callconv=(blank)  param_size_bytes=4  gog_size=17  gog_retn=0
Candidate Steam address: 0x0093dab0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetProcessedTextureInternal 0x93e880)
- best decoy (neighbour/other candidate) scores: 0x0093da90:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x009080b0 CAuroraInterface::ReleaseCompressedTextureInternal_thunk  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009080f0 CAuroraInterface::ReleaseProcessedTextureInternal_thunk  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00908ea0 CAuroraInterface::GetProcessedTextureInternal -> Steam 0x0093e880  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0093dab0: 16-byte aligned: YES
  0093dab0  55             PUSH EBP
  0093dab1  8B EC          MOV EBP,ESP
  0093dab3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093dab0
string literals (0):
direct callees (1):
  0x0093E880  CAuroraInterface::GetProcessedTextureInternal
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093dab0 FUN_0093dab0 size=17 ret=0 callers=2 callees=1
THUNK -> 0x0093E880 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0093dab0(undefined4 param_1)

{
  FUN_0093e880(param_1);
  return;
}
```

### decoy: 0x0093da90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093da90: 16-byte aligned: YES
  0093da90  55             PUSH EBP
  0093da91  8B EC          MOV EBP,ESP
  0093da93  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093da90
string literals (0):
direct callees (1):
  0x0093E7D0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 2: 0x0093b9c0, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C)
- callees: 0x0093e880 CAuroraInterface::GetProcessedTextureInternal (CALL)
- twins: 0x0093d9b0, 0x0093d9d0, 0x0093d9f0, 0x0093da10, 0x0093da30, ... (distinguishing: 0x0093dab7: CALL 0x0093e880)
