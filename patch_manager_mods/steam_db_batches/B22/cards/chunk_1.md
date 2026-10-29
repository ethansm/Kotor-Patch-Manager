# Review cards B22 chunk 1

Answer each card with one TSV row in `steam_db_batches/B22/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B22-1-01

GOG function: Material::GetTextureTID  gog 0x008f5120  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x004daf40 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x14,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x10,0x14}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004daf40: 16-byte aligned: YES
  004daf40  55             PUSH EBP
  004daf41  8B EC          MOV EBP,ESP
  004daf43  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004daf40
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x14 [dec 20]  x2  via ECX,EDX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004daf40 FUN_004daf40 size=80 ret=0 callers=1 callees=2

void __fastcall FUN_004daf40(int param_1)

{
  FUN_004da750(param_1 + 0x14);
  (**(code **)(**(int **)(param_1 + 0x14) + 0xb8))();
  FUN_00426be0(*(undefined4 *)(param_1 + 0x14));
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
- indirect: 0x004daf75 slot 0xb8
- twins: none

## Card B22-1-02

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852a0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00985240:STR0/CALLEE0/SIG0/SRV0/VT0
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
prologue for 0x009852a0: 16-byte aligned: YES
  009852a0  55             PUSH EBP
  009852a1  8B EC          MOV EBP,ESP
  009852a3  B9 08 BC A7 00 MOV ECX,0xa7bc08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852a0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852a0 FUN_009852a0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852a0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x00985240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00985240: 16-byte aligned: YES
  00985240  55             PUSH EBP
  00985241  8B EC          MOV EBP,ESP
  00985243  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00985240
string literals (0):
direct callees (2):
  0x004D5A60  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852a3: MOV ECX,0xa7bc08)

## Card B22-1-03

GOG function: Material::GetTextureTID  gog 0x008f5120  callconv=__thiscall  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x004dae60 (size 132, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GUI | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004daef0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f50d0 Material::BindEnvMap  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f5150 Material::ReplaceTexture  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x008192e0 Global::AurTextureGetTID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x18,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x4*,0x1c}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004dae60: 16-byte aligned: YES
  004dae60  55             PUSH EBP
  004dae61  8B EC          MOV EBP,ESP
  004dae63  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dae60
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x18 [dec 24]  x2  via ECX
  +0xB8 [dec 184]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004dae60 FUN_004dae60 size=132 ret=0 callers=16 callees=2

void __fastcall FUN_004dae60(undefined4 *param_1)

{
  FUN_004da750(param_1);
  if ((DAT_009f62ec == 0) || (param_1[6 = byte +0x18] == 0)) {
    (**(code **)(*(int *)*param_1 + 0xb8))();
  }
  else {
    (**(code **)(*(int *)param_1[6 = byte +0x18] + 0xb8))();
  }
  FUN_00426be0(*param_1);
  return;
}
```

### decoy: 0x004daef0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004daef0: 16-byte aligned: YES
  004daef0  55             PUSH EBP
  004daef1  8B EC          MOV EBP,ESP
  004daef3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004daef0
string literals (0):
direct callees (2):
  0x004DA750  GUI image NULL-texture fallback
  0x00426BE0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0xB8 [dec 184]  x1  via EDX
```

### Static facts (script output, report-only)
- size 132 B, RET 0
- callers 16: 0x00465f90, 0x004662e0, 0x00466700, 0x0047d2f0, 0x0047ea60, ...
- callees: 0x004da750 (CALL), 0x00426be0 (CALL)
- indirect: 0x004daeaa slot 0xb8, 0x004daeca slot 0xb8
- twins: none

## Card B22-1-04

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
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

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

## Card B22-1-05

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
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

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

## Card B22-1-06

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
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

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

## Card B22-1-07

GOG function: Camera::MapToScreen  gog 0x008c1b40  callconv=(blank)  param_size_bytes=17  gog_size=1042  gog_retn=20
Candidate Steam address: 0x004810d0 (size 80, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.40 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00481080:STR0/CALLEE0/SIG0.245/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c1870 Camera::MouseDir  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c1f60 Camera::HitCheck_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0080c200 Global::planelineintersection -> Steam ?  in candidate callees: n/a
  0x00927760 Global::tan -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*,0x204,0x218} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 2 common): GOG {0x4,0x8,0xc,0x14*,0x41,0x44,0xa8,0xb4*,0xcc,0xba2} | Steam {0x14*,0xb4*}

### abi
```
GOG param bytes: 17; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(17/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 17 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x004810d0: 16-byte aligned: YES
  004810d0  55             PUSH EBP
  004810d1  8B EC          MOV EBP,ESP
  004810d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004810d0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004810d0 FUN_004810d0 size=80 ret=20 callers=1 callees=0

undefined4 * __thiscall
FUN_004810d0(int param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  *(undefined4 *)(param_1 + 0xb4) = param_3;
  *(undefined4 *)(param_1 + 0xb8) = param_4;
  *(undefined4 *)(param_1 + 0xbc) = param_5;
  *(undefined4 *)(param_1 + 0xc0) = param_6;
  *param_2 = *(undefined4 *)(param_1 + 0xb4);
  param_2[1 = byte +0x4] = *(undefined4 *)(param_1 + 0xb8);
  param_2[2 = byte +0x8] = *(undefined4 *)(param_1 + 0xbc);
  param_2[3 = byte +0xc] = *(undefined4 *)(param_1 + 0xc0);
  return param_2;
}
```

### decoy: 0x00481080 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00481080: 16-byte aligned: YES
  00481080  55             PUSH EBP
  00481081  8B EC          MOV EBP,ESP
  00481083  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00481080
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
```

### Static facts (script output, report-only)
- size 80 B, RET 20
- callers 1: 0x00481d30
- twins: none

## Card B22-1-08

GOG function: CExoTimersInternal::GetLowResolutionTimer  gog 0x0060de70  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0073b7f0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetLowResolutionTimer 0x736200)
- best decoy (neighbour/other candidate) scores: 0x0073b720:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060dda0 CExoTimersInternal::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060de90 CExoTimersInternal::GetHighResolutionTimer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00608330 CExoTimers::GetLowResolutionTimer -> Steam 0x00736200  in candidate callers: yes
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
prologue for 0x0073b7f0: 16-byte aligned: YES
  0073b7f0  55             PUSH EBP
  0073b7f1  8B EC          MOV EBP,ESP
  0073b7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073b7f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073b7f0 FUN_0073b7f0 size=17 ret=0 callers=1 callees=1
THUNK -> 0x000000B2 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073b7f0(void)

{
  GetTickCount();
  return;
}
```

### decoy: 0x0073b720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073b720: 16-byte aligned: YES
  0073b720  55             PUSH EBP
  0073b721  8B EC          MOV EBP,ESP
  0073b723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073b720
string literals (0):
direct callees (2):
  0x0091D220  Global::__aulldiv
  0x00920670  Global::__alldiv
struct offsets touched (top 6):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 1: 0x00736200 CExoTimers::GetLowResolutionTimer
- callees: 0x000000b2 (CALL)
- imports: GetTickCount (KERNEL32.DLL)
- twins: none

## Card B22-1-09

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

## Card B22-1-10

GOG function: CExoTimersInternal::GetLowResolutionTimer  gog 0x0060de70  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0073b720 (size 199, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoTimersInternal::Constructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073b7f0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060dda0 CExoTimersInternal::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060de90 CExoTimersInternal::GetHighResolutionTimer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00608330 CExoTimers::GetLowResolutionTimer -> Steam 0x00736200  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x10,0x14,0x18}
imm sets (* = in both, 0 common): GOG {} | Steam {0x14,0xc8,0xf4240}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073b720: 16-byte aligned: YES
  0073b720  55             PUSH EBP
  0073b721  8B EC          MOV EBP,ESP
  0073b723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073b720
string literals (0):
direct callees (2):
  0x0091D220  Global::__aulldiv
  0x00920670  Global::__alldiv
struct offsets touched (top 6):
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073b720 FUN_0073b720 size=199 ret=0 callers=1 callees=3

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * __fastcall FUN_0073b720(int *param_1)

{
  BOOL BVar1;
  undefined8 uVar2;
  LARGE_INTEGER local_c;
  
  BVar1 = QueryPerformanceFrequency(&local_c);
  *param_1 = BVar1;
  if (*param_1 == 1) {
    param_1[2 = byte +0x8] = local_c.s.LowPart;
    param_1[3 = byte +0xc] = local_c.s.HighPart;
    param_1[1 = byte +0x4] = 0;
    uVar2 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](param_1[2 = byte +0x8],param_1[3 = byte +0xc],1000000,0);
    if (((int)((ulonglong)uVar2 >> 0x20) == 0) && ((uint)uVar2 < 200)) {
      param_1[1 = byte +0x4] = 1;
      *(double *)(param_1 + 0x6 [dec 6] {int*: byte +0x18}) = 1.0 / ((double)*(longlong *)(param_1 + 0x2 [dec 2] {int*: byte +0x8}) / _DAT_00996518);
    }
    else {
      *(double *)(param_1 + 0x6 [dec 6] {int*: byte +0x18}) = 1.0 / ((double)*(longlong *)(param_1 + 0x2 [dec 2] {int*: byte +0x8}) / _DAT_00996518);
      uVar2 = __alldiv [GHIDRA-AUTO-LABEL: unreliable](local_c.s.LowPart,local_c.s.HighPart,1000000,0);
      *(undefined8 *)(param_1 + 0x4 [dec 4] {int*: byte +0x10}) = uVar2;
    }
  }
  return param_1;
}
```

### decoy: 0x0073b7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073b7f0: 16-byte aligned: YES
  0073b7f0  55             PUSH EBP
  0073b7f1  8B EC          MOV EBP,ESP
  0073b7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073b7f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 199 B, RET 0
- callers 1: 0x00736130
- callees: 0x000000d6 (CALL), 0x0091d220 (CALL), 0x00920670 (CALL)
- imports: QueryPerformanceFrequency (KERNEL32.DLL)
- twins: none

## Card B22-1-11

GOG function: Global::AurEnableShadows  gog 0x0086ab80  callconv=(blank)  param_size_bytes=0  gog_size=15  gog_retn=0
Candidate Steam address: 0x00984730 (size 37, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478ca0:STR0/CALLEE1/SIG0/SRV0/VT0
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
prologue for 0x00984730: 16-byte aligned: YES
  00984730  55             PUSH EBP
  00984731  8B EC          MOV EBP,ESP
  00984733  D9 EE          FLDZ
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00984730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00984730 FUN_00984730 size=37 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00984730(void)

{
  DAT_00a7fd88 = 0;
  DAT_00a7fd8c = _DAT_0099e240;
  DAT_00a7fd90 = _DAT_009875a4;
  return;
}
```

### decoy: 0x00478ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478ca0: 16-byte aligned: YES
  00478ca0  55             PUSH EBP
  00478ca1  8B EC          MOV EBP,ESP
  00478ca3  C7 05 30 5D 9F 00 01 00 00 00 MOV dword ptr [0x009f5d30],0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478ca0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 37 B, RET 0
- twins: 0x00984690, 0x00984a30, 0x00984a60 (distinguishing: 0x00984735: FSTP float ptr [0x00a7fd88])

## Card B22-1-12

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

## Card B22-1-13

GOG function: CSWCModule::Destructor  gog 0x0046a080  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x007f2e60 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: Destructor_2 0x7f2eb0)
- best decoy (neighbour/other candidate) scores: 0x007f3040:STR0/CALLEE0/SIG0.038/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00469cd0 CSWCModule::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046a0b0 Global::CSWCModule_local_camera_target_Part_cleanup_helper  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0046a0d0 CSWCModule::Destructor_2 -> Steam 0x007f2eb0  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007f2e60: 16-byte aligned: YES
  007f2e60  55             PUSH EBP
  007f2e61  8B EC          MOV EBP,ESP
  007f2e63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2e60
string literals (0):
direct callees (2):
  0x007F2EB0  CSWCModule::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007f2e60 FUN_007f2e60 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_007f2e60(undefined4 param_1,uint param_2)

{
  FUN_007f2eb0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007f3040 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f3040: 16-byte aligned: YES
  007f3040  55             PUSH EBP
  007f3041  8B EC          MOV EBP,ESP
  007f3043  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007f3040
string literals (0):
direct callees (30):
  0x0073F810  CClientExoApp::GetSWCMessage
  0x00919723  operator new
  0x007A2590  CSWCArea::Constructor
  0x00481AC0  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5E50  CClientOptions::SetCameraMode
  0x005E3C90  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007107F0  (unnamed)
  0x008937B0  (unnamed)
  0x005E3720  (unnamed)
  0x005E3BF0  (unnamed)
  0x00710CA0  (unnamed)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051A800  CWorldTimer::SetMinutesPerHour
  0x00710AD0  (unnamed)
  0x00733540  CExoString::Constructor
  0x007338D0  CExoString::operator=_2
  0x00458940  identity (basic vtable slot 0x4)
struct offsets touched (top 4):
  +0x40 [dec 64]  x10  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0xA0 [dec 160]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x007f2eb0 CSWCModule::Destructor_2 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B22-1-14

GOG function: CExoTimersInternal::GetLowResolutionTimer  gog 0x0060de70  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0073b810 (size 85, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Timer::Now | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073b7f0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060dda0 CExoTimersInternal::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060de90 CExoTimersInternal::GetHighResolutionTimer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00608330 CExoTimers::GetLowResolutionTimer -> Steam 0x00736200  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x18}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x3e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0073b810: 16-byte aligned: YES
  0073b810  55             PUSH EBP
  0073b811  8B EC          MOV EBP,ESP
  0073b813  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073b810
string literals (0):
direct callees (1):
  0x0091C896  (unnamed)
struct offsets touched (top 2):
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073b810 FUN_0073b810 size=85 ret=0 callers=1 callees=3

int __fastcall FUN_0073b810(int *param_1)

{
  int iVar1;
  DWORD DVar2;
  LARGE_INTEGER local_c;
  
  if (*param_1 == 1) {
    QueryPerformanceCounter(&local_c);
    if (param_1[1 = byte +0x4] == 1) {
      iVar1 = FUN_0091c896();
    }
    else {
      iVar1 = FUN_0091c896();
    }
  }
  else {
    DVar2 = GetTickCount();
    iVar1 = DVar2 * 1000;
  }
  return iVar1;
}
```

### decoy: 0x0073b7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073b7f0: 16-byte aligned: YES
  0073b7f0  55             PUSH EBP
  0073b7f1  8B EC          MOV EBP,ESP
  0073b7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073b7f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 85 B, RET 0
- callers 1: 0x00736220
- callees: 0x000000d5 (CALL), 0x0091c896 (CALL), 0x000000b2 (CALL)
- imports: GetTickCount (KERNEL32.DLL), QueryPerformanceCounter (KERNEL32.DLL)
- twins: none

## Card B22-1-15

GOG function: Camera::MapToScreen  gog 0x008c1b40  callconv=(blank)  param_size_bytes=17  gog_size=1042  gog_retn=20
Candidate Steam address: 0x004809d0 (size 1042, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.56) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00480700:STR0/CALLEE0/SIG0.773/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c1870 Camera::MouseDir  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c1f60 Camera::HitCheck_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0080c200 Global::planelineintersection -> Steam ?  in candidate callees: n/a
  0x00927760 Global::tan -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x204*,0x218*} | Steam {0x4*,0x8*,0xc*,0x204*,0x218*}
imm sets (* = in both, 10 common): GOG {0x4*,0x8*,0xc*,0x14*,0x41*,0x44*,0xa8*,0xb4*,0xcc*,0xba2*} | Steam {0x4*,0x8*,0xc*,0x14*,0x41*,0x44*,0xa8*,0xb4*,0xcc*,0xba2*}

### abi
```
GOG param bytes: 17; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(17/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 17 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x004809d0: 16-byte aligned: YES
  004809d0  55             PUSH EBP
  004809d1  8B EC          MOV EBP,ESP
  004809d3  81 EC CC 00 00 00 SUB ESP,0xcc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004809d0
string literals (0):
direct callees (3):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00514450  (unnamed)
  0x0091F820  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x204 [dec 516]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004809d0 FUN_004809d0 size=1042 ret=20 callers=0 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

float * __thiscall
FUN_004809d0(int param_1,float *param_2,float param_3,float param_4,float param_5,char param_6)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float *pfVar4;
  float10 fVar5;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  float local_58;
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  float local_44;
  float local_40;
  undefined1 local_3c [16];
  float local_2c;
  float local_28;
  float local_20;
  float local_1c;
  int local_18[4 = byte +0x10];
  int local_8;
  
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    local_18[local_8] = *(int *)(param_1 + 0x218 + local_8 * 4);
  }
  if ((local_18[2 = byte +0x8] == 0) || (local_18[3 = byte +0xc] == 0)) {
    glGetIntegerv(0xba2,local_18);
  }
  if (*(float *)(param_1 + 0x204) <= (float)_DAT_00986b38) {
    *param_2 = 0.0;
    param_2[1 = byte +0x4] = 0.0;
    param_2[2 = byte +0x8] = 0.0;
  }
  else {
    pfVar4 = (float *)FUN_004452b0(local_3c,&param_3,param_1 + 0xa8);
    local_58 = *pfVar4;
    local_54 = pfVar4[1 = byte +0x4];
    local_50 = pfVar4[2 = byte +0x8];
    local_4c = *(float *)(param_1 + 0xb4) * (float)_DAT_00986d98;
    local_48 = *(float *)(param_1 + 0xb8);
    local_44 = *(float *)(param_1 + 0xbc);
    local_40 = *(float *)(param_1 + 0xc0);
    if ((DAT_00a0f190 == 0) ||
       ((local_4c != (float)_DAT_00986b50 && (local_4c != (float)_DAT_00986d98)))) {
      fVar1 = (local_44 * local_4c + local_48 * local_40) * (float)_DAT_00987490 * local_50;
      fVar2 = (local_44 * local_40 - local_48 * local_4c) * (float)_DAT_00987490 * local_50;
      fVar3 = (local_40 * local_4c + local_48 * local_44) * (float)_DAT_00987490 * local_58;
      local_50 = (local_48 * local_40 - local_44 * local_4c) * (float)_DAT_00987490 * local_58 +
                 (local_48 * local_4c + local_44 * local_40) * (float)_DAT_00987490 * local_54 +
                 (1.0 - (local_44 * local_44 + local_48 * local_48) * (float)_DAT_00987490) *
                 local_50;
      local_58 = (1.0 - (local_40 * local_40 + local_44 * local_44) * (float)_DAT_00987490) *
                 local_58 +
                 (local_48 * local_44 - local_40 * local_4c) * (float)_DAT_00987490 * local_54 +
                 fVar1;
      local_54 = fVar3 + (1.0 - (local_40 * local_40 + local_48 * local_48) * (float)_DAT_00987490)
                         * local_54 + fVar2;
    }
    param_3 = local_58;
    param_4 = local_54;
    param_5 = local_50;
    local_64 = 0;
    local_60 = 0;
    local_5c = 0;
    local_70 = 0;
    local_6c = 0;
    local_68 = 0x3f800000;
    FUN_00514450(&local_2c,&local_70,0x3f800000,&local_64,&param_3);
/* ... 18 more lines truncated by --max-lines */
```

### decoy: 0x00480700 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00480700: 16-byte aligned: YES
  00480700  55             PUSH EBP
  00480701  8B EC          MOV EBP,ESP
  00480703  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00480700
string literals (0):
direct callees (2):
  0x0091F820  (unnamed)
  0x00514340  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x204 [dec 516]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1042 B, RET 20
- callees: 0x00000027 (CALL), 0x004452b0 (CALL), 0x00514450 (CALL), 0x0091f820 (CALL)
- imports: glGetIntegerv (OPENGL32.DLL)
- twins: none

## Card B22-1-16

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852c0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009852b0:STR0/CALLEE0/SIG0/SRV0/VT0
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
prologue for 0x009852c0: 16-byte aligned: YES
  009852c0  55             PUSH EBP
  009852c1  8B EC          MOV EBP,ESP
  009852c3  B9 08 BE A7 00 MOV ECX,0xa7be08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852c0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852c0 FUN_009852c0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852c0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x009852b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009852b0: 16-byte aligned: YES
  009852b0  55             PUSH EBP
  009852b1  8B EC          MOV EBP,ESP
  009852b3  B9 D0 BC A7 00 MOV ECX,0xa7bcd0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009852b0
string literals (0):
direct callees (1):
  0x00452F80  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852c3: MOV ECX,0xa7be08)
