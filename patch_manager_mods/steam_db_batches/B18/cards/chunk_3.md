# Review cards B18 chunk 3

Answer each card with one TSV row in `steam_db_batches/B18/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B18-3-01

GOG function: Global::AurSetVSyncState  gog 0x0086ac50  callconv=(blank)  param_size_bytes=1  gog_size=43  gog_retn=0
Candidate Steam address: 0x00478d70 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: SetVSync 0x7bc520)
- best decoy (neighbour/other candidate) scores: 0x00478d50:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ac30 Global::AurSetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ac80 Global::AurSetPauseOverlayColor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b270 Global::AurBumpyShinyAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004691f0 CClientOptions::SetVSync -> Steam 0x007bc520  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478d70: 16-byte aligned: YES
  00478d70  55             PUSH EBP
  00478d71  8B EC          MOV EBP,ESP
  00478d73  E8 68 B7 00 00 CALL 0x004844e0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478d70
string literals (0):
direct callees (1):
  0x004844E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478d70 FUN_00478d70 size=43 ret=0 callers=1 callees=1

void FUN_00478d70(char param_1)

{
  char cVar1;
  
  cVar1 = FUN_004844e0();
  if (cVar1 != '\0') {
    (*DAT_00a32aa8)(param_1 != '\0');
    DAT_009f48f8 = param_1;
  }
  return;
}
```

### decoy: 0x00478d50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478d50: 16-byte aligned: YES
  00478d50  55             PUSH EBP
  00478d51  8B EC          MOV EBP,ESP
  00478d53  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478d50
string literals (0):
direct callees (1):
  0x00478FB0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 43 B, RET 0
- callers 1: 0x007bc520 CClientOptions::SetVSync
- callees: 0x004844e0 (CALL)
- indirect: 0x00478d8a unresolved
- twins: none

## Card B18-3-02

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e360 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e480:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x0040e360: 16-byte aligned: YES
  0040e360  55             PUSH EBP
  0040e361  8B EC          MOV EBP,ESP
  0040e363  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e360
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e360 FUN_0040e360 size=285 ret=0 callers=10 callees=0

int * FUN_0040e360(int *param_1,int *param_2,int *param_3)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  if (*param_3 < *param_2) {
    local_20 = *param_2;
  }
  else {
    local_20 = *param_3;
  }
  if (param_3[1 = byte +0x4] < param_2[1 = byte +0x4]) {
    local_24 = param_2[1 = byte +0x4];
  }
  else {
    local_24 = param_3[1 = byte +0x4];
  }
  if (*param_2 + param_2[2 = byte +0x8] < *param_3 + param_3[2 = byte +0x8]) {
    local_28 = *param_2 + param_2[2 = byte +0x8];
  }
  else {
    local_28 = *param_3 + param_3[2 = byte +0x8];
  }
  if (param_2[1 = byte +0x4] + param_2[3 = byte +0xc] < param_3[1 = byte +0x4] + param_3[3 = byte +0xc]) {
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

### decoy: 0x0040e480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### Static facts (script output, report-only)
- size 285 B, RET 0
- callers 10: 0x00419a80, 0x00419c80, 0x00419dc0, 0x00419ed0 CSWGuiButton::SetExtent, 0x0041a670, ...
- twins: none

## Card B18-3-03

GOG function: CResRef::operator==  gog 0x0061a490  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710b70 (size 164, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710b40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a420 CResRef::operator!=_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009228ac unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 24): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x10*} | Steam {0x4*,0xc*,0x10*,0x18}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710b70: 16-byte aligned: YES
  00710b70  55             PUSH EBP
  00710b71  8B EC          MOV EBP,ESP
  00710b73  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710b70
string literals (0):
direct callees (3):
  0x00734010  CExoString::CStr
  0x00919F10  _strlen
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710b70 FUN_00710b70 size=164 ret=4 callers=1 callees=3

bool __thiscall FUN_00710b70(void *param_1,int *param_2)

{
  void *_Buf1 [GHIDRA-AUTO-LABEL: unreliable];
  int iVar1;
  size_t local_1c;
  size_t local_10;
  
  _Buf1 [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00734010();
  if (*param_2 == 0) {
    local_10 = 0;
  }
  else {
    local_10 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
  }
  if (local_10 + 1 < 0x11) {
    local_1c = local_10 + 1;
  }
  else {
    local_1c = 0x10;
  }
  iVar1 = _memcmp [GHIDRA-AUTO-LABEL: unreliable](_Buf1 [GHIDRA-AUTO-LABEL: unreliable],param_1,local_1c);
  return iVar1 == 0;
}
```

### decoy: 0x00710b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710b40: 16-byte aligned: YES
  00710b40  55             PUSH EBP
  00710b41  8B EC          MOV EBP,ESP
  00710b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710b40
string literals (0):
direct callees (1):
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 164 B, RET 4
- callers 1: 0x00710aa0
- callees: 0x00734010 (CALL), 0x00919f10 _strlen (CALL), 0x0091ac7c (CALL)
- twins: none

## Card B18-3-04

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x00435e80 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00435f00:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications -> 0x00435720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 0 common): GOG {0xc,0x18} | Steam {0x2,0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00435e80: 16-byte aligned: YES
  00435e80  55             PUSH EBP
  00435e81  8B EC          MOV EBP,ESP
  00435e83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435e80
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435e80 FUN_00435e80 size=113 ret=0 callers=1 callees=0

undefined4 FUN_00435e80(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  
  uVar1 = (**(code **)(*DAT_00a1dd7c + 4))(0,8,2);
  puVar2 = (undefined4 *)(**(code **)(*DAT_00a1dd7c + 8))(uVar1,0,0,1);
  *puVar2 = 0;
  puVar2[1 = byte +0x4] = 0;
  (**(code **)(*DAT_00a1dd7c + 0xc))(uVar1);
  return uVar1;
}
```

### decoy: 0x00435f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00435f00: 16-byte aligned: YES
  00435f00  55             PUSH EBP
  00435f01  8B EC          MOV EBP,ESP
  00435f03  81 EC E0 00 00 00 SUB ESP,0xe0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00435f00
string literals (0):
direct callees (3):
  0x00518390  (unnamed)
  0x004360F0  (unnamed)
  0x0091E620  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x988550 [dec 9995600]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
```

### Static facts (script output, report-only)
- size 113 B, RET 0
- callers 1: 0x004cdf40
- indirect: 0x00435e9c slot 0x4, 0x00435ebc slot 0x8, 0x00435ee8 slot 0xc
- twins: none

## Card B18-3-05

GOG function: Global::AurResGetNextLine  gog 0x00867ee0  callconv=(blank)  param_size_bytes=0  gog_size=69  gog_retn=0
Candidate Steam address: 0x00475f70 (size 69, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callees: AurResFree_2 0x476030; callers: Init 0x424b10, EnableRenderBumpedOut 0x45bf50, LoadVisibility 0x46fbd0)
- best decoy (neighbour/other candidate) scores: 0x00475ea0:STR0/CALLEE3/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00867e10 Global::AurResFree  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00867fa0 Global::AurResFree_2 -> 0x00476030  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00867f30 unnamed -> Steam ?  in candidate callees: n/a
  0x00867fa0 Global::AurResFree_2 -> Steam 0x00476030  in candidate callees: yes
GOG callers mapped through trusted rows (3 of 20): the card's A-CALLEE credit may come from these, not from callees
  0x00817210 CAurTextureBasic::Init -> Steam 0x00424b10  in candidate callers: yes
  0x0084e530 Gob::EnableRenderBumpedOut -> Steam 0x0045bf50  in candidate callers: yes
  0x00861e10 Scene::LoadVisibility -> Steam 0x0046fbd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00475f70: 16-byte aligned: YES
  00475f70  55             PUSH EBP
  00475f71  8B EC          MOV EBP,ESP
  00475f73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00475f70
string literals (0):
direct callees (2):
  0x00475FC0  (unnamed)
  0x00476030  Global::AurResFree_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00475f70 FUN_00475f70 size=69 ret=0 callers=20 callees=2

undefined4 FUN_00475f70(void)

{
  undefined4 uVar1;
  
  if (DAT_00a30844 == 0) {
    uVar1 = 0;
  }
  else if (**(int **)(DAT_00a30840 + -4 + DAT_00a30844 * 4) == 0) {
    uVar1 = FUN_00476030(0);
  }
  else {
    uVar1 = FUN_00475fc0(0);
  }
  return uVar1;
}
```

### decoy: 0x00475ea0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00475ea0: 16-byte aligned: YES
  00475ea0  55             PUSH EBP
  00475ea1  8B EC          MOV EBP,ESP
  00475ea3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00475ea0
string literals (0):
direct callees (5):
  0x0091C5B3  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
  0x00475A70  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x18 [dec 24]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 69 B, RET 0
- callers 20: 0x00424b10 CAurTextureBasic::Init, 0x00429010, 0x0045bf50 Gob::EnableRenderBumpedOut, 0x0046fbd0 Scene::LoadVisibility, 0x00470000, ...
- callees: 0x00475fc0 (CALL), 0x00476030 Global::AurResFree_2 (CALL)
- twins: none

## Card B18-3-06

GOG function: Global::InsertModel  gog 0x008ce680  callconv=(blank)  param_size_bytes=4  gog_size=48  gog_retn=0
Candidate Steam address: 0x0048e130 (size 48, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048e090:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008ce5e0 Global::BinaryInsertModel -> 0x0048e090  trusted
- GOG-order neighbour after:  0x008ce6b0 Global::FindModel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008ce5e0 Global::BinaryInsertModel -> Steam 0x0048e090  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0048e130: 16-byte aligned: YES
  0048e130  55             PUSH EBP
  0048e131  8B EC          MOV EBP,ESP
  0048e133  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048e130
string literals (0):
direct callees (1):
  0x0048E090  Global::BinaryInsertModel
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048e130 FUN_0048e130 size=48 ret=0 callers=3 callees=1
THUNK -> 0x0048E090 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0048e130(undefined4 param_1)

{
  FUN_0048e090(param_1,0,DAT_00a1ffac + -1);
  return;
}
```

### decoy: 0x0048e090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0048e090: 16-byte aligned: YES
  0048e090  55             PUSH EBP
  0048e091  8B EC          MOV EBP,ESP
  0048e093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0048e090
string literals (0):
direct callees (3):
  0x0050C380  (unnamed)
  0x009370C7  _stricmp
  0x0048E090  Global::BinaryInsertModel
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 48 B, RET 0
- callers 3: 0x004528c0, 0x005124d0, 0x00512750
- callees: 0x0048e090 Global::BinaryInsertModel (CALL)
- twins: 0x0048e060 (distinguishing: 0x0048e154: CALL 0x0048e090)

## Card B18-3-07

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x004356e0 (size 54, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004356c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications -> 0x00435720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x18*} | Steam {0xc*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004356e0 FUN_004356e0 size=54 ret=0 callers=1 callees=3

void FUN_004356e0(void)

{
  DAT_00a1dd24 = wglGetCurrentContext();
  DAT_00a1bcb8 = wglGetCurrentDC();
  _memset [GHIDRA-AUTO-LABEL: unreliable](&DAT_00a1dd0c,0,0x18);
  DAT_00a1dd70 = 0;
  return;
}
```

### decoy: 0x004356c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356c0: 16-byte aligned: YES
  004356c0  55             PUSH EBP
  004356c1  8B EC          MOV EBP,ESP
  004356c3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356c0
string literals (0):
direct callees (1):
  0x00435470  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 54 B, RET 0
- callers 1: 0x00476be0
- callees: 0x00000043 (CALL), 0x0000003d (CALL), 0x0091c630 (CALL)
- imports: wglGetCurrentContext (OPENGL32.DLL), wglGetCurrentDC (OPENGL32.DLL)
- twins: none

## Card B18-3-08

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e750 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e5a0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090cb30 CSWGuiPanel::Constructor -> 0x0040e5a0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
imm sets (* = in both, 0 common): GOG {0x28} | Steam {}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040e750: 16-byte aligned: YES
  0040e750  55             PUSH EBP
  0040e751  8B EC          MOV EBP,ESP
  0040e753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e750
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040e750 FUN_0040e750 size=20 ret=0 callers=4 callees=0

void __fastcall FUN_0040e750(undefined4 *param_1)

{
  *param_1 = CSWGuiObject::vftable;
  return;
}
```

### decoy: 0x0040e5a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e5a0: 16-byte aligned: YES
  0040e5a0  55             PUSH EBP
  0040e5a1  8B EC          MOV EBP,ESP
  0040e5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040e5a0
string literals (0):
direct callees (3):
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x00414A50  CSWGuiBorder::Constructor
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x4C [dec 76]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 20 B, RET 0
- callers 4: 0x00948290, 0x009482d0, 0x00948910, 0x00948950
- twins: none

## Card B18-3-09

GOG function: CAurGUIStringInternal::SetSizeBoundingRect_2  gog 0x008d10b0  callconv=(blank)  param_size_bytes=8  gog_size=89  gog_retn=8
Candidate Steam address: 0x0047a740 (size 83, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a7a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d1050 CAurGUIStringInternal::GetSizeBoundingRect_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d1110 CAurGUIStringInternal::DrawBoundingRect  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x10*} | Steam {0xc*,0x10*}
imm sets (* = in both, 3 common): GOG {0x8*,0xa*,0xc*} | Steam {0x8*,0xa*,0xc*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0047a740: 16-byte aligned: YES
  0047a740  55             PUSH EBP
  0047a741  8B EC          MOV EBP,ESP
  0047a743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0047a740
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0047a740 FUN_0047a740 size=83 ret=8 callers=0 callees=0

void __thiscall FUN_0047a740(int param_1,float *param_2,float *param_3)

{
  *param_2 = (float)*(int *)(param_1 + 0xc) /
             (float)(int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10);
  *param_3 = (float)*(int *)(param_1 + 0x10) / (float)(int)(short)(&DAT_00a309ce)[DAT_00a309c0 * 5];
  return;
}
```

### decoy: 0x0047a7a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a7a0: 16-byte aligned: YES
  0047a7a0  55             PUSH EBP
  0047a7a1  8B EC          MOV EBP,ESP
  0047a7a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a7a0
string literals (0):
direct callees (1):
  0x0091C860  _ftol2_sse
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via EDX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309CE [dec 10684878]  x1  via EAX
```

### Static facts (script output, report-only)
- size 83 B, RET 8
- globals: 0x00a309c0 READ, 0x00a309c0 READ
- twins: none

## Card B18-3-10

GOG function: Global::AurGUIStopLayer  gog 0x008cfd00  callconv=(blank)  param_size_bytes=0  gog_size=101  gog_retn=0
Candidate Steam address: 0x00479460 (size 845, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=GUI | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004793f0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfbd0 Global::AurGUIStartLayer -> 0x004792c0  trusted
- GOG-order neighbour after:  0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: yes
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8}
imm sets (* = in both, 3 common): GOG {0x2*,0x5*,0xb*} | Steam {0x2*,0x3,0x4,0x5*,0x6,0x8,0xa,0xb*,0x10,0x18,0x70,0x96000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00479460: 16-byte aligned: YES
  00479460  55             PUSH EBP
  00479461  8B EC          MOV EBP,ESP
  00479463  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00479460
string literals (0):
direct callees (2):
  0x00517980  Vec3::Equal
  0x00408740  Global::glClear_glClear_int_engineMask
struct offsets touched (top 9):
  +0x9F4928 [dec 10438952]  x5  via EAX,ECX
  +0xA309C8 [dec 10684872]  x2  via EAX,ECX
  +0xA309CA [dec 10684874]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x9885F4 [dec 9995764]  x1  via EDX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00479460 FUN_00479460 size=845 ret=0 callers=12 callees=8

/* WARNING: Switch with 1 destination removed at 0x00479701 */
/* WARNING: Switch with 1 destination removed at 0x0047978a */
/* WARNING: Switch with 1 destination removed at 0x00479813 */
/* WARNING: Switch with 1 destination removed at 0x0047989c */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4
FUN_00479460(int param_1,int param_2,int param_3,int param_4,float *param_5,byte param_6,
            float param_7)

{
  undefined4 uVar1;
  int iVar2;
  uint local_60;
  float local_5c;
  int *local_58;
  float local_54;
  undefined1 *local_50;
  float local_4c;
  undefined1 *local_48;
  float local_44;
  undefined1 *local_40;
  int local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20 [4];
  uint local_1c;
  undefined1 local_18 [4];
  short local_14;
  short local_10;
  undefined1 local_c [4];
  int local_8;
  
  if (DAT_00a32a30 == 0) {
    if (((((param_1 + param_3 < 0) || (param_2 + param_4 < 0)) || (param_3 < 1)) ||
        ((param_4 < 1 || (DAT_009f42a4 <= param_1)))) || (DAT_009f42a8 <= param_2)) {
      uVar1 = 0;
    }
    else {
      local_14 = *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) + (short)param_1;
      local_10 = *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) + (short)param_2;
      DAT_00a309c0 = DAT_00a309c0 + 1;
      *(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10) = local_14;
      *(short *)(&DAT_00a309ca + DAT_00a309c0 * 10) = local_10;
      *(undefined2 *)(&DAT_00a309cc + DAT_00a309c0 * 10) = (undefined2)param_3;
      (&DAT_00a309ce)[DAT_00a309c0 * 5] = (undefined2)param_4;
      *(ushort *)(&DAT_00a309d0 + DAT_00a309c0 * 10) = (ushort)param_6;
      *(float *)(&DAT_00a30980 + DAT_00a309c0 * 4) = param_7;
      local_3c = ((int)DAT_00a309ce - (int)local_10) - param_4;
      glViewport((int)local_14,local_3c,param_3,param_4);
      local_44 = *param_5 * (float)_DAT_009884a0;
      local_40 = local_c;
      local_4c = param_5[1 = byte +0x4] * (float)_DAT_009884a0;
      local_48 = local_18;
      local_54 = param_5[2 = byte +0x8] * (float)_DAT_009884a0;
      local_50 = local_20;
      local_5c = param_7 * (float)_DAT_009884a0;
      local_58 = &local_8;
      local_8 = (int)ROUND(ROUND(local_5c));
      local_1c = (int)ROUND(ROUND(local_4c)) << 8 | (int)ROUND(ROUND(local_44)) |
                 (int)ROUND(ROUND(local_54)) << 0x10 | local_8 << 0x18;
      local_2c = _DAT_00987474;
      local_28 = _DAT_00987474;
      local_24 = _DAT_00987474;
      iVar2 = FUN_00517980(param_5,&local_2c);
      if ((iVar2 == 0) && (0 < local_8)) {
        local_38 = _DAT_0098bd70;
        local_34 = _DAT_0098bd70;
        local_30 = _DAT_0098bd70;
        iVar2 = FUN_00517980(param_5,&local_38);
        if (iVar2 != 0) {
          local_1c = 0x96000000;
        }
        glDisable(DAT_009f4938);
        glDisable(DAT_009f493c);
/* ... 23 more lines truncated by --max-lines */
```

### decoy: 0x004793f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004793f0: 16-byte aligned: YES
  004793f0  55             PUSH EBP
  004793f1  8B EC          MOV EBP,ESP
  004793f3  83 C8 FF       OR EAX,0xffffffff
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004793f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x988550 [dec 9995600]  x2  via EAX,ECX
  +0x9F4928 [dec 10438952]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 845 B, RET 0
- callers 12: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x004184f0 CSWGuiScene::Draw, 0x0041bf40 CSWGuiProgressBarReveal::Draw, 0x0041f980, ...
- callees: 0x0000004f (CALL), 0x00517980 (CALL), 0x0000001c (CALL), 0x0000002d (CALL), 0x0000002c (CALL), 0x00000029 (CALL), ...
- globals: 0x00a309c0 READ, 0x00a309c0 READ, 0x00a309c0 READ, ...
- imports: glBegin (OPENGL32.DLL), glColor4ubv (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B18-3-11

GOG function: Global::AurMultiTextureAvailable  gog 0x0086b4a0  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x00484710 (size 53, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callers: AsNormalizeVectorCubeMap 0x427110, RenderSkinnedVP 0x4c3430, EnableFragmentShadersARB 0x4b50c0)
- best decoy (neighbour/other candidate) scores: 0x00484690:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b420 Global::AurOnePassEnvMapAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b4e0 Global::AurVertexProgramsNVAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: yes
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: yes
  0x008de4b0 GLRender::EnableFragmentShadersARB -> Steam 0x004b50c0  in candidate callers: yes
  0x008e85f0 VertexPrimitiveFlat::ReconnectArraysVAR -> Steam 0x004ce460  in candidate callers: yes
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
prologue for 0x00484710: 16-byte aligned: YES
  00484710  55             PUSH EBP
  00484711  8B EC          MOV EBP,ESP
  00484713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484710
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484710 FUN_00484710 size=53 ret=0 callers=46 callees=0

uint FUN_00484710(void)

{
  if (DAT_009f610c == 0xffffffff) {
    DAT_009f610c = (uint)((DAT_009f6070 & DAT_00a32df8) == DAT_009f6070);
  }
  return DAT_009f610c;
}
```

### decoy: 0x00484690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484690: 16-byte aligned: YES
  00484690  55             PUSH EBP
  00484691  8B EC          MOV EBP,ESP
  00484693  83 3D 08 61 9F 00 FF CMP dword ptr [0x009f6108],-0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484690
string literals (0):
direct callees (2):
  0x00484480  (unnamed)
  0x004843C0  GL_HasFragmentShaderBit
struct offsets touched (top 1):
  +0x988528 [dec 9995560]  x1  via ECX
```

### Static facts (script output, report-only)
- size 53 B, RET 0
- callers 46: 0x00427110 CAurTextureBasic::AsNormalizeVectorCubeMap, 0x0042abc0, 0x00430c50, 0x00434970, 0x00434b60, ...
- twins: 0x00484750, 0x00484a20 (distinguishing: 0x00484714: CMP dword ptr [0x009f610c],-0x1)

## Card B18-3-12

GOG function: Global::AurEnableForceFury  gog 0x0086ae30  callconv=(blank)  param_size_bytes=2  gog_size=18  gog_retn=0
Candidate Steam address: 0x00478f50 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: EnableVideoEffect 0x79e550, DisableVideoEffect 0x79e960)
- best decoy (neighbour/other candidate) scores: 0x00478f40:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ae20 Global::AurDisableForceSight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ae50 Global::AurSetSaturationParams  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00834760 GLRender::EnableFury -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam 0x0079e550  in candidate callers: yes
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam 0x0079e960  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 2 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478f50: 16-byte aligned: YES
  00478f50  55             PUSH EBP
  00478f51  8B EC          MOV EBP,ESP
  00478f53  0F B7 45 08    MOVZX EAX,word ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478f50
string literals (0):
direct callees (1):
  0x00442090  SetFuryLevel ([0xA1DD54], loads fx_Fury01..03)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478f50 FUN_00478f50 size=18 ret=0 callers=2 callees=1
THUNK -> 0x00442090 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00478f50(undefined2 param_1)

{
  FUN_00442090(param_1);
  return;
}
```

### decoy: 0x00478f40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478f40: 16-byte aligned: YES
  00478f40  55             PUSH EBP
  00478f41  8B EC          MOV EBP,ESP
  00478f43  C6 05 C4 BA A7 00 00 MOV byte ptr [0x00a7bac4],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478f40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 18 B, RET 0
- callers 2: 0x0079e550 CClientExoAppInternal::EnableVideoEffect, 0x0079e960 CClientExoAppInternal::DisableVideoEffect
- callees: 0x00442090 SetFuryLevel ([0xA1DD54], loads fx_Fury01..03) (CALL)
- twins: none

## Card B18-3-13

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852b0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009852a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b6a0 Global::AurATIFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b770 Global::AurAnisotropicFilterAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b2e0 Global::AurBumpMapCardValidate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x009852b0: 16-byte aligned: YES
  009852b0  55             PUSH EBP
  009852b1  8B EC          MOV EBP,ESP
  009852b3  B9 D0 BC A7 00 MOV ECX,0xa7bcd0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852b0
string literals (0):
direct callees (1):
  0x00452F80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852b0 FUN_009852b0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00452F80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852b0(void)

{
  FUN_00452f80();
  return;
}
```

### decoy: 0x009852a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009852a0: 16-byte aligned: YES
  009852a0  55             PUSH EBP
  009852a1  8B EC          MOV EBP,ESP
  009852a3  B9 08 BC A7 00 MOV ECX,0xa7bc08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009852a0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00452f80 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852b3: MOV ECX,0xa7bcd0)

## Card B18-3-14

GOG function: Global::AurGUIStopLayer  gog 0x008cfd00  callconv=(blank)  param_size_bytes=0  gog_size=101  gog_retn=0
Candidate Steam address: 0x004792c0 (size 301, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::AurGUIStartLayer | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004793f0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfbd0 Global::AurGUIStartLayer -> 0x004792c0  trusted
- GOG-order neighbour after:  0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: yes
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2*,0x5*,0xb*} | Steam {0x2*,0x5*,0x8,0xb*,0x10,0xba2}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004792c0: 16-byte aligned: YES
  004792c0  55             PUSH EBP
  004792c1  8B EC          MOV EBP,ESP
  004792c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004792c0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x9F4928 [dec 10438952]  x3  via EAX,ECX
  +0x988530 [dec 9995568]  x2  via ECX,EDX
  +0x988550 [dec 9995600]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004792c0 FUN_004792c0 size=301 ret=0 callers=8 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004792c0(void)

{
  undefined2 local_14 [2];
  undefined2 local_10;
  undefined2 local_c;
  undefined2 local_8;
  
  glGetIntegerv(0xba2,local_14);
  _DAT_00a309c8 = local_14[0];
  _DAT_00a309ca = local_10;
  _DAT_00a309cc = local_c;
  DAT_00a309ce = local_8;
  _DAT_00a309d0 = 0;
  _DAT_00a30980 = 0x3f800000;
  DAT_00a309c0 = 0;
  glDisable(DAT_009f493c);
  glDisable(DAT_009f4954);
  glMatrixMode(DAT_00988550);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0,0x3ff0000000000000,0,0x3ff0000000000000,_DAT_00986d98,0x3ff0000000000000);
  glMatrixMode(DAT_00988558);
  glLoadIdentity();
  glEnable(DAT_009f4928);
  glBlendFunc(DAT_00988530,DAT_00988534);
  return;
}
```

### decoy: 0x004793f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004793f0: 16-byte aligned: YES
  004793f0  55             PUSH EBP
  004793f1  8B EC          MOV EBP,ESP
  004793f3  83 C8 FF       OR EAX,0xffffffff
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004793f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x988550 [dec 9995600]  x2  via EAX,ECX
  +0x9F4928 [dec 10438952]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 301 B, RET 0
- callers 8: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x00743280, 0x007457e0, 0x0074b3f0, ...
- callees: 0x00000027 (CALL), 0x0000001c (CALL), 0x0000001e (CALL), 0x00000032 (CALL), 0x0000001d (CALL), 0x0000001b (CALL), ...
- globals: 0x00a309c0 WRITE
- imports: glBlendFunc (OPENGL32.DLL), glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), ...
- twins: none

## Card B18-3-15

GOG function: CResRef::operator==  gog 0x0061a490  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710ad0 (size 110, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710b40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a420 CResRef::operator!=_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009228ac unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 24): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0xc,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710ad0: 16-byte aligned: YES
  00710ad0  55             PUSH EBP
  00710ad1  8B EC          MOV EBP,ESP
  00710ad3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710ad0
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710ad0 FUN_00710ad0 size=110 ret=4 callers=45 callees=1

undefined4 __thiscall FUN_00710ad0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint local_8;
  
  local_8 = 0;
  while( true ) {
    if (0xf < local_8) {
      return 0;
    }
    iVar1 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((int)*(char *)(param_2 + local_8));
    iVar2 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((uint)*(byte *)(param_1 + local_8));
    if (iVar1 != iVar2) break;
    if (iVar1 == 0) {
      return 0;
    }
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### decoy: 0x00710b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710b40: 16-byte aligned: YES
  00710b40  55             PUSH EBP
  00710b41  8B EC          MOV EBP,ESP
  00710b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710b40
string literals (0):
direct callees (1):
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 110 B, RET 4
- callers 45: 0x0052e590, 0x0052e640, 0x00561190, 0x00561240, 0x00631560, ...
- callees: 0x0091e01a (CALL)
- twins: none

## Card B18-3-16

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040fb70 (size 209, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040fc50:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> Steam 0x0040fa80  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x24,0x28,0x4c}
imm sets (* = in both, 0 common): GOG {0x2,0x5} | Steam {0x4,0x1c}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040fb70: 16-byte aligned: YES
  0040fb70  55             PUSH EBP
  0040fb71  8B EC          MOV EBP,ESP
  0040fb73  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040fb70
string literals (0):
direct callees (2):
  0x00418720  (unnamed)
  0x004186F0  (unnamed)
struct offsets touched (top 3):
  +0x24 [dec 36]  x3  via EAX,ECX
  +0x28 [dec 40]  x1  via EAX
  +0x4C [dec 76]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040fb70 FUN_0040fb70 size=209 ret=0 callers=1 callees=2

void __fastcall FUN_0040fb70(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_18;
  undefined4 local_c;
  
  iVar1 = *(int *)(param_1 + 0x28);
  for (local_c = 0; local_c < iVar1; local_c = local_c + 1) {
    if (*(int *)(*(int *)(param_1 + 0x24) + local_c * 4) != 0) {
      iVar2 = (**(code **)(**(int **)(*(int *)(param_1 + 0x24) + local_c * 4) + 0x4c))();
      if (iVar2 != 0) {
        for (local_18 = 0; local_18 < 4; local_18 = local_18 + 1) {
          iVar2 = FUN_00418720(local_18);
          if ((iVar2 < 0) || (iVar1 <= iVar2)) {
            FUN_004186f0(local_18,0);
          }
          else {
            FUN_004186f0(local_18,*(undefined4 *)(*(int *)(param_1 + 0x24) + iVar2 * 4));
          }
        }
      }
    }
  }
  return;
}
```

### decoy: 0x0040fc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040fc50: 16-byte aligned: YES
  0040fc50  55             PUSH EBP
  0040fc51  8B EC          MOV EBP,ESP
  0040fc53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040fc50
string literals (5):
  0x009874D8  at 0040fc7f  '800x600'
  0x009874CC  at 0040fc9f  '1600x1200'
  0x009874C0  at 0040fcba  '1280x1024'
  0x009874B4  at 0040fcd5  '1280x960'
  0x009874A8  at 0040fcf0  '1024x768'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x007338D0  CExoString::operator=_2
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x4C [dec 76]  x1  via EAX
```

### Static facts (script output, report-only)
- size 209 B, RET 0
- callers 1: 0x0040f5a0 CSWGuiPanel::StopLoadFromLayout
- callees: 0x00418720 (CALL), 0x004186f0 (CALL)
- indirect: 0x0040fbcd slot 0x4c
- twins: none
