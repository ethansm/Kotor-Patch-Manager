# Review cards B17 chunk 12

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-12-01

GOG function: CTlkTable::ClearCustomTokens  gog 0x00628ac0  callconv=(blank)  param_size_bytes=0  gog_size=76  gog_retn=0
Candidate Steam address: 0x0071f3d0 (size 41, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071ebd0:STR0/CALLEE0/SIG0.168/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628850 CTlkTable::SetCustomToken  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00628b10 CTlkTable::ParseStr -> 0x0071ebd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00627790 CTlkTableCustomToken::`vector -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x48,0x4c} | Steam {0x4,0xc}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0071f3d0: 16-byte aligned: YES
  0071f3d0  55             PUSH EBP
  0071f3d1  8B EC          MOV EBP,ESP
  0071f3d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071f3d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0071f3d0 FUN_0071f3d0 size=41 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0071f3d0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = *(undefined4 *)(*(int *)(param_1 + 4) + 0xc);
  }
  return uVar1;
}
```

### decoy: 0x0071ebd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071ebd0: 16-byte aligned: YES
  0071ebd0  55             PUSH EBP
  0071ebd1  8B EC          MOV EBP,ESP
  0071ebd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071ebd0
string literals (2):
  0x0099C504  at 0071edcf  '<UNRECOGNIZED TOKEN>'
  0x0099C4FC  at 0071ee61  'CUSTOM'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x0071D930  (unnamed)
  0x00733680  CExoString::Constructor_4
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x0091AC7C  (unnamed)
  0x00722E50  (unnamed)
  0x00919F10  _strlen
  0x0091CF23  (unnamed)
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 6):
  +0x50 [dec 80]  x6  via EAX,ECX
  +0x40 [dec 64]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX,EDX
  +0x1 [dec 1]  x2  via ECX
  +0x44 [dec 68]  x1  via EDX
  +0x4C [dec 76]  x1  via EAX
```

### Static facts (script output, report-only)
- size 41 B, RET 0
- callers 1: 0x00788d90 CClientExoAppInternal::StartServices
- twins: none

## Card B17-12-02

GOG function: Global::AurGUIStopLayer  gog 0x008cfd00  callconv=(blank)  param_size_bytes=0  gog_size=101  gog_retn=0
Candidate Steam address: 0x004793f0 (size 101, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004792c0:STR0/CALLEE3/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfbd0 Global::AurGUIStartLayer -> 0x004792c0  gate_class=REVIEW
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
imm sets (* = in both, 3 common): GOG {0x2*,0x5*,0xb*} | Steam {0x2*,0x5*,0xb*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004793f0: 16-byte aligned: YES
  004793f0  55             PUSH EBP
  004793f1  8B EC          MOV EBP,ESP
  004793f3  83 C8 FF       OR EAX,0xffffffff
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004793f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x988550 [dec 9995600]  x2  via EAX,ECX
  +0x9F4928 [dec 10438952]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004793f0 FUN_004793f0 size=101 ret=0 callers=8 callees=3

void FUN_004793f0(void)

{
  DAT_00a309c0 = 0xffff;
  glMatrixMode(DAT_00988550);
  glPopMatrix();
  glMatrixMode(DAT_00988558);
  glEnable(DAT_009f493c);
  glEnable(DAT_009f4954);
  return;
}
```

### decoy: 0x004792c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004792c0: 16-byte aligned: YES
  004792c0  55             PUSH EBP
  004792c1  8B EC          MOV EBP,ESP
  004792c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004792c0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x9F4928 [dec 10438952]  x3  via EAX,ECX
  +0x988530 [dec 9995568]  x2  via ECX,EDX
  +0x988550 [dec 9995600]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 101 B, RET 0
- callers 8: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x00743280, 0x007457e0, 0x0074b3f0, ...
- callees: 0x0000001e (CALL), 0x0000002e (CALL), 0x0000000e (CALL)
- globals: 0x00a309c0 WRITE
- imports: glEnable (OPENGL32.DLL), glMatrixMode (OPENGL32.DLL), glPopMatrix (OPENGL32.DLL)
- twins: none

## Card B17-12-03

GOG function: Global::RenderSkinnedVPBlur  gog 0x0087f060  callconv=(blank)  param_size_bytes=4  gog_size=9743  gog_retn=0
Candidate Steam address: 0x004cbc50 (size 5866, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.54 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004c9640:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00878e50 Global::RenderSkinnedVP -> 0x004c3430  gate_class=REVIEW
- GOG-order neighbour after:  0x00881670 PartSkin::DrawBumpedOutShellVP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x0080c7d0 unnamed -> Steam ?  in candidate callees: n/a
  ... 10 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '!!VP1.0 # This is the standard vertex pr'
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x18*,0x2c*,0x3c*,0x40*,0x44*,0x50*,0x182*,0x1ac,0x1b8*,0x1bc*,0x1c0*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x18*,0x2c*,0x3c*,0x40*,0x44*,0x50*,0x60,0x64,0x68,0x70,0xb8,...(+6)}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x5,0x8*,0x9*,0xc*,0xd,0xe,0x10*,0x11*,0x12*,0x14*,0x22,...(+11)} | Steam {0x2*,0x3*,0x4*,0x6,0x8*,0x9*,0xc*,0x10*,0x11*,0x12*,0x14*,0x20,0x28*,0x32,...(+6)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004cbc50: 16-byte aligned: YES
  004cbc50  55             PUSH EBP
  004cbc51  8B EC          MOV EBP,ESP
  004cbc53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004cbc50
string literals (0):
direct callees (18):
  0x00484710  (unnamed)
  0x004DB1F0  (unnamed)
  0x00451800  (unnamed)
  0x0091D3BA  (unnamed)
  0x00428F80  (unnamed)
  0x0044E300  (unnamed)
  0x004BE4F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00463120  (unnamed)
  0x0046CE40  (unnamed)
  0x0046CDA0  (unnamed)
  0x0046CCF0  (unnamed)
  0x004DB420  (unnamed)
  0x00454960  (unnamed)
  0x004844C0  (unnamed)
  0x00506640  (unnamed)
  0x0049CC50  (unnamed)
  0x0049CF40  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x44  via EAX,ECX,EDX
  +0x4 [dec 4]  x43  via EAX,ECX,EDX
  +0xC [dec 12]  x20  via EAX,ECX,EDX
  +0x44 [dec 68]  x4  via EAX,EDX
  +0x988530 [dec 9995568]  x4  via EAX,ECX,EDX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via ECX
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x988658 [dec 9995864]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004cbc50 FUN_004cbc50 size=5866 ret=0 callers=1 callees=23

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_004cbc50(int param_1)

{
  char cVar1;
  int iVar2;
  undefined4 *puVar3;
  float *pfVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined1 local_334 [12];
  undefined1 local_328 [12];
  undefined1 local_31c [12];
  int local_310;
  float local_30c;
  float local_308;
  float local_304;
  float local_300;
  float *local_2fc;
  float *local_2f8;
  float local_2f4;
  float local_2f0;
  float local_2ec;
  float *local_2e8;
  float *local_2e4;
  float local_2e0;
  float local_2dc;
  float local_2d8;
  float local_2d4;
  float local_2d0;
  float local_2cc;
  float local_2c8;
  float local_2c4;
  float local_2c0;
  float local_2bc;
  float local_2b8;
  float local_2b4;
  float *local_2b0;
  float *local_2ac;
  float local_2a8;
  float local_2a4;
  float local_2a0;
  float local_29c;
  float *local_298;
  float local_294;
  float local_290;
  float local_28c;
  float local_288;
  float local_284;
  float local_280;
  float local_27c;
  float *local_278;
  float local_274;
  float local_270;
  float local_26c;
  float local_268;
  float local_264;
  float local_260;
  float local_25c;
  float local_258;
  float local_254;
  float local_250;
  float local_24c;
  float local_248;
  float *local_244;
  float local_240;
  float local_23c;
  float local_238;
  float local_234;
  float local_230;
  float local_22c;
  float local_228;
/* ... 466 more lines truncated by --max-lines */
```

### decoy: 0x004c9640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004c9640: 16-byte aligned: YES
  004c9640  55             PUSH EBP
  004c9641  8B EC          MOV EBP,ESP
  004c9643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004c9640
string literals (1):
  0x00A0D558  at 004cab21  '!!VP1.0 # This is the standard vertex program.\\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];DP3   R5.x, v[NRML], c[A0.x + 18];DP3   R5.y, v[NRML], c[A0.x + 19];DP3   R5.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.y;DP4   R2.x, v[OPOS], c[A0.x + 18];DP4   R2.y, v[OPOS], c[A0.x + 19];DP4   R2.z, v[OPOS], c[A0.x + 20];DP3   R6.x, v[NRML], c[A0.x + 18];DP3   R6.y, v[NRML], c[A0.x + 19];DP3   R6.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.z;DP4   R3.x, v[OPOS], c[A0.x + 18];DP4   R3.y, v[OPOS], c[A0.x + 19];DP4   R3.z, v[OPOS], c[A0.x + 20];DP3   R7.x, v[NRML], c[A0.x + 18];DP3   R7.y, v[NRML], c[A0.x + 19];DP3   R7.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.w;DP4   R4.x, v[OPOS], c[A0.x + 18];DP4   R4.y, v[OPOS], c[A0.x + 19];DP4   R4.z, v[OPOS], c[A0.x + 20];DP3   R8.x, v[NRML], c[A0.x + 18];DP3   R8.y, v[NRML], c[A0.x + 19];DP3   R8.z, v[NRML], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;MAD   R9, R2, v[WGHT].y, R9;MAD   R9, R3, v[WGHT].z, R9;MAD   R9, R4, v[WGHT].w, R9;MOV   R9.w, c[16].w;MUL   R10, v[WGHT].x, R5;MAD   R10, R6, v[WGHT].y, R10;MAD   R10, R7, v[WGHT].z, R10;MAD   R10, R8, v[WGHT].w, R10;DP3   R10.w, R10, R10;RSQ   R10.w, R10.w;MUL   R10,   R10, R10.w;MOV   R10.w, c[16].x;MOV  R4, c[12];MOV   R7.x, c[14].x;MIN   R2.y, R7.x, c[14].y;MUL   R5.xyz, R4, R2.y;MOV   R5.w, c[16].x;DP3   R3.w, R4, R10;SLT   R2.w, R3.w, c[16].x;MOV   R1, c[17].xyzw;MOV   R1.w, c[16].x;MAD   R6, R2.w, -R5, R9.xyzw;MUL   R7.y, R2.y, c[13].x;RCP   R8.y, c[14].y;MUL   R8.w, R2.y, R8.y;MUL   R7.y, R7.y, R8.w;MUL   R7.y, R7.y, c[17].w;MOV   R3.w,   c[16].x;MOV   R3.xyz, c[69];DP3   R3.w,   R3, R3;RSQ   R5.w,   R3.w;MUL   R3,     R3, R5.w;ADD   R5.xyz, R6, -R1;DP3   R5.w, R5, R3;SLT   R5.x, R5.w, c[16].x;MOV   R7.x, c[16].w;MAD   R5.y, R5.x, -c[16].y, R7.x;MUL   R5.z, R5.y, R7.y;MUL   R5.z, R5.z, R2.w;MAD   R6, R5.z, R3, R6;ADD   R2.x, -R2.w, c[16].w;MUL   R1, R10, -c[14].z;MOV   R1.w, c[16].x;MAD   R5, R2.x, R1, R6;DP4   R1.x, R5, c[0];DP4   R1.y, R5, c[1];DP4   R1.z, R5, c[2];DP4   R1.w, R5, c[3];MOV   o[HPOS].xyzw, R1.xyzw;ADD   R2.w, c[16].w, -R2.w;MOV   R9.xyz, c[16].xxxx;MUL   R9.w, c[69].w, R2.w;MOV   o[COL0], R9;DP3   R11.x, c[8],  R10;DP3   R11.y, c[9],  R10;DP3   R11.z, c[10], R10;MOV   R11.w, c[16].w;DP4   R10.x, c[8],  R5;DP4   R10.y, c[9],  R5;DP4   R10.z, c[10], R5;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV   o[FOGC].x, -R1.w;MOV   o[TEX0], v[TEX0];'
direct callees (18):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x0044E300  (unnamed)
  0x00451800  (unnamed)
  0x0091D3BA  (unnamed)
  0x00428F80  (unnamed)
  0x0040F9F0  (unnamed)
  0x004BE4F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00463120  (unnamed)
  0x0046CE40  (unnamed)
  0x0046CDA0  (unnamed)
  0x0046CCF0  (unnamed)
  0x00919723  operator new
  0x00506560  (unnamed)
  0x005143C0  (unnamed)
  0x00506640  (unnamed)
  0x0049CF40  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x51  via EAX,ECX,EDX
  +0x8 [dec 8]  x51  via EAX,ECX,EDX
  +0xC [dec 12]  x20  via EAX,ECX,EDX
  +0x40 [dec 64]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x988580 [dec 9995648]  x2  via ECX
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
  +0x2C [dec 44]  x1  via EAX
  +0x44 [dec 68]  x1  via EAX
```

### Static facts (script output, report-only)
- size 5866 B, RET 0
- callers 1: 0x004cd340
- callees: 0x00484710 (CALL), 0x004db1f0 (CALL), 0x00451800 (CALL), 0x0091d3ba (CALL), 0x00428f80 (CALL), 0x0044e300 (CALL), ...
- indirect: 0x004cbc9e unresolved, 0x004cbcd6 slot 0xb8, 0x004cbd10 slot 0x8, 0x004cbd26 slot 0x50, 0x004cbd52 slot 0x2c, ...
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B17-12-04

GOG function: CSWGui3DSceneView::Draw  gog 0x0053c050  callconv=(blank)  param_size_bytes=4  gog_size=37  gog_retn=4
Candidate Steam address: 0x0084de70 (size 120, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0084de40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053bff0 CSWGuiMainMenu::OnMoreGamesPressed  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053c080 CSWGui3DSceneView::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xc,0x60} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x60*} | Steam {0x4*,0x60*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0084de70: 16-byte aligned: YES
  0084de70  55             PUSH EBP
  0084de71  8B EC          MOV EBP,ESP
  0084de73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0084de70
string literals (0):
direct callees (3):
  0x00418130  (unnamed)
  0x00418640  CSWGuiControl::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0084de70 FUN_0084de70 size=120 ret=4 callers=0 callees=3

undefined4 * __thiscall FUN_0084de70(undefined4 *param_1,uint param_2)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009718b8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGui3DSceneView::vftable;
  local_8 = 0;
  FUN_00418130(uVar1);
  local_8 = 0xffffffff;
  FUN_00418640();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0084de40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084de40: 16-byte aligned: YES
  0084de40  55             PUSH EBP
  0084de41  8B EC          MOV EBP,ESP
  0084de43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084de40
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x60 [dec 96]  x1  via EAX
```

### Static facts (script output, report-only)
- size 120 B, RET 4
- callees: 0x00418130 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B17-12-05

GOG function: Global::AurGUIStartLayer  gog 0x008cfbd0  callconv=(blank)  param_size_bytes=0  gog_size=301  gog_retn=0
Candidate Steam address: 0x004792c0 (size 301, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004793f0:STR0/CALLEE3/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cf7e0 Global::AnimationParseEvent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008cfd00 Global::AurGUIStopLayer -> 0x004793f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (3 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: yes
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x2*,0x5*,0x8*,0xb*,0x10*,0xba2*} | Steam {0x2*,0x5*,0x8*,0xb*,0x10*,0xba2*}

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

## Card B17-12-06

GOG function: Global::BinaryInsertModel  gog 0x008ce5e0  callconv=(blank)  param_size_bytes=12  gog_size=148  gog_retn=0
Candidate Steam address: 0x0048e090 (size 148, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048e060:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008ce5b0 Global::BinaryFindModel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008ce680 Global::InsertModel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x008ce5e0 Global::BinaryInsertModel -> Steam ?  in candidate callees: n/a
  0x008fdd20 List_T__::Insert -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0048e090: 16-byte aligned: YES
  0048e090  55             PUSH EBP
  0048e091  8B EC          MOV EBP,ESP
  0048e093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048e090
string literals (0):
direct callees (3):
  0x0050C380  (unnamed)
  0x009370C7  _stricmp
  0x0048E090  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048e090 FUN_0048e090 size=148 ret=0 callers=2 callees=3

void FUN_0048e090(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  if (param_3 < param_2) {
    FUN_0050c380(param_1,param_2);
  }
  else {
    iVar1 = (param_2 + param_3) / 2;
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(param_1 + 8),(char *)(*(int *)(DAT_00a1ffa8 + iVar1 * 4) + 8));
    if (iVar2 < 0) {
      FUN_0048e090(param_1,param_2,iVar1 + -1);
    }
    else {
      FUN_0048e090(param_1,iVar1 + 1,param_3);
    }
  }
  return;
}
```

### decoy: 0x0048e060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0048e060: 16-byte aligned: YES
  0048e060  55             PUSH EBP
  0048e061  8B EC          MOV EBP,ESP
  0048e063  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0048e060
string literals (0):
direct callees (1):
  0x0048DFC0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 148 B, RET 0
- callers 2: 0x0048e090, 0x0048e130
- callees: 0x0050c380 (CALL), 0x009370c7 _stricmp (CALL), 0x0048e090 (CALL)
- twins: none

## Card B17-12-07

GOG function: CExoRawInputInternal::GetMouseState  gog 0x00619a00  callconv=(blank)  param_size_bytes=4  gog_size=253  gog_retn=4
Candidate Steam address: 0x00732c40 (size 433, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.63 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00732e00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00619840 CExoRawInputInternal::GetKeyboardBuffer  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00619b00 CExoRawInputInternal::KeyboardModeOn  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00617c70 CExoRawInputInternal::InitializeDirectInputMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x24*,0x28*} | Steam {0x4*,0x8,0x1c*,0x24*,0x28*}
imm sets (* = in both, 6 common): GOG {0x4*,0xc*,0x10,0x8000000a,0x8007000c*,0x80070015*,0x8007001e*,0x80070057*} | Steam {0x4*,0xc*,0x14,0x18,0x100,0x1400,0x80040207,0x8007000c*,0x80070015*,0x8007001e*,0x80070057*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00732c40: 16-byte aligned: YES
  00732c40  55             PUSH EBP
  00732c41  8B EC          MOV EBP,ESP
  00732c43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00732c40
string literals (0):
direct callees (3):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091C630  (unnamed)
struct offsets touched (top 5):
  +0x24 [dec 36]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x1C [dec 28]  x2  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x28 [dec 40]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00732c40 FUN_00732c40 size=433 ret=4 callers=2 callees=3

undefined4 __thiscall FUN_00732c40(int param_1,int *param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_c;
  int local_8;
  
  if (*(int *)(param_1 + 0x24) == 0) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 4) == 0) {
    uVar1 = 0;
  }
  else {
    if (*param_2 != 0) {
      FUN_009196fd(*param_2);
      *param_2 = 0;
    }
    param_2[1 = byte +0x4] = 0;
    if (*(int *)(param_1 + 8) == 0) {
      iVar2 = FUN_00919723(0x1400);
      *param_2 = iVar2;
      if (*param_2 == 0) {
        uVar1 = 0;
      }
      else {
        _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)*param_2,0,0x100);
        local_8 = 0x100;
        local_c = -0x7ff8ffe2;
        while (local_c == -0x7ff8ffe2) {
          iVar2 = (**(code **)(**(int **)(param_1 + 0x24) + 0x28))
                            (*(undefined4 *)(param_1 + 0x24),0x14,*param_2,&local_8,0);
          if (iVar2 == 0) {
            param_2[1 = byte +0x4] = local_8;
            return 1;
          }
          if (-0x7ff8ffe2 < iVar2) {
            if (iVar2 == -0x7ff8ffa9) {
              return 0;
            }
            if (iVar2 != 1) {
              return 0;
            }
            return 1;
          }
          if (iVar2 != -0x7ff8ffe2) {
            if (iVar2 == -0x7ffbfdf9) {
              return 0;
            }
            if (iVar2 != -0x7ff8fff4) {
              if (iVar2 != -0x7ff8ffeb) {
                return 0;
              }
              return 0;
            }
            (**(code **)(**(int **)(param_1 + 0x24) + 0x1c))(*(undefined4 *)(param_1 + 0x24));
            return 1;
          }
          iVar2 = (**(code **)(**(int **)(param_1 + 0x24) + 0x1c))(*(undefined4 *)(param_1 + 0x24));
          if (iVar2 != 0) {
            return 0;
          }
          local_c = 0;
        }
        uVar1 = 1;
      }
    }
    else {
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00732e00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00732e00: 16-byte aligned: YES
  00732e00  55             PUSH EBP
  00732e01  8B EC          MOV EBP,ESP
  00732e03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00732e00
string literals (0):
direct callees (1):
  0x00731070  (unnamed)
struct offsets touched (top 4):
  +0x28 [dec 40]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

### Static facts (script output, report-only)
- size 433 B, RET 4
- callers 2: 0x0072c7e0, 0x0072f300
- callees: 0x009196fd operator delete (CALL), 0x00919723 operator new (CALL), 0x0091c630 (CALL)
- indirect: 0x00732d22 slot 0x28, 0x00732d90 slot 0x1c, 0x00732db7 slot 0x1c
- twins: none

## Card B17-12-08

GOG function: Global::GetResolutionTextScalar  gog 0x008d03c0  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x00479ab0 (size 92, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 6 (callers: ? 0x752f50, Constructor 0x895620, Draw 0x895cc0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004799c0:STR0/CALLEE1/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d02d0 Global::AurGUICloseViewport -> 0x004799c0  gate_class=REVIEW
- GOG-order neighbour after:  0x008d0420 CAurGUIStringInternal::RefreshResolutionTextScalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 43): the card's A-CALLEE credit may come from these, not from callees
  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> Steam 0x00752f50  in candidate callers: yes
  0x00564590 CSWGuiMapHider::Constructor -> Steam 0x00895620  in candidate callers: yes
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x005af4f0 CSWGuiDialogLetterbox::SetBottom -> Steam 0x008bada0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0xa*,0xc*,0x3fe*,0x3ff*} | Steam {0xa*,0xc*,0x3fe*,0x3ff*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00479ab0: 16-byte aligned: YES
  00479ab0  55             PUSH EBP
  00479ab1  8B EC          MOV EBP,ESP
  00479ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00479ab0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00479ab0 FUN_00479ab0 size=92 ret=0 callers=43 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00479ab0(void)

{
  undefined4 local_10;
  
  if (DAT_009f42a4 + -0x3fe < 0) {
    local_10 = 0;
  }
  else {
    local_10 = DAT_009f42a4 + -0x3fe;
  }
  _DAT_009f5f64 = (float)((int)(local_10 + (local_10 >> 0x1f & 0x3ffU)) >> 10) * _DAT_0098c014 + 1.0
  ;
  return (float10)_DAT_009f5f64;
}
```

### decoy: 0x004799c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004799c0: 16-byte aligned: YES
  004799c0  55             PUSH EBP
  004799c1  8B EC          MOV EBP,ESP
  004799c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004799c0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0xA309CE [dec 10684878]  x2  via ECX,EDX
  +0xA309C8 [dec 10684872]  x1  via EDX
  +0xA309CA [dec 10684874]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EDX
```

### Static facts (script output, report-only)
- size 92 B, RET 0
- callers 43: 0x00417910, 0x00479ca0, 0x0047a510, 0x0047b500, 0x007457e0, ...
- twins: none

## Card B17-12-09

GOG function: CTlkTable::ClearCustomTokens  gog 0x00628ac0  callconv=(blank)  param_size_bytes=0  gog_size=76  gog_retn=0
Candidate Steam address: 0x0071e450 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071e010:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00628850 CTlkTable::SetCustomToken  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00628b10 CTlkTable::ParseStr -> 0x0071ebd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00627790 CTlkTableCustomToken::`vector -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x48,0x4c} | Steam {}
imm sets (* = in both, 0 common): GOG {0x3,0x10} | Steam {0x4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0071e450: 16-byte aligned: YES
  0071e450  55             PUSH EBP
  0071e451  8B EC          MOV EBP,ESP
  0071e453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0071e450
string literals (0):
direct callees (1):
  0x00733540  CExoString::Constructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0071e450 FUN_0071e450 size=25 ret=0 callers=0 callees=1

undefined4 __fastcall FUN_0071e450(undefined4 param_1)

{
  FUN_00733540();
  return param_1;
}
```

### decoy: 0x0071e010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071e010: 16-byte aligned: YES
  0071e010  55             PUSH EBP
  0071e011  8B EC          MOV EBP,ESP
  0071e013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0071e010
string literals (1):
  0x0099C4EC  at 0071e0cc  'StringTokens'
direct callees (17):
  0x0071E4F0  (unnamed)
  0x0071D8A0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00710840  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x0071AE80  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x0091D67E  (unnamed)
  0x0071AFA0  C2DA::GetCExoStringEntry
  0x0071B8B0  (unnamed)
  0x00919F10  _strlen
  0x00734010  CExoString::CStr
  0x00722E50  (unnamed)
  0x0071E470  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 2):
  +0x40 [dec 64]  x16  via EAX,ECX,EDX,ESI
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 25 B, RET 0
- callees: 0x00733540 CExoString::DefaultConstructor (CALL)
- twins: none

## Card B17-12-10

GOG function: CSWGui3DSceneView::Draw  gog 0x0053c050  callconv=(blank)  param_size_bytes=4  gog_size=37  gog_retn=4
Candidate Steam address: 0x0084dd00 (size 181, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0084dcb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053bff0 CSWGuiMainMenu::OnMoreGamesPressed  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053c080 CSWGui3DSceneView::`scalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xc,0x60} | Steam {0x10,0x3c,0x1d0,0x1d8,0x1dc}
imm sets (* = in both, 1 common): GOG {0x4*,0x60} | Steam {0x4*,0x5,0x8,0x16256,0x16257}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0084dd00: 16-byte aligned: YES
  0084dd00  55             PUSH EBP
  0084dd01  8B EC          MOV EBP,ESP
  0084dd03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084dd00
string literals (0):
direct callees (1):
  0x0041A0A0  CSWGuiButton::Draw
struct offsets touched (top 5):
  +0x1D8 [dec 472]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EAX
  +0x1D0 [dec 464]  x1  via ECX
  +0x1DC [dec 476]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0084dd00 FUN_0084dd00 size=181 ret=4 callers=0 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0084dd00(int *param_1,float param_2)

{
  FUN_0041a0a0(param_2);
  if (*(int **)(DAT_00a1b49c + 0x10) == param_1) {
    param_1[0x76 = byte +0x1d8] = (int)((float)param_1[0x76 = byte +0x1d8] - param_2);
    while ((float)param_1[0x76 = byte +0x1d8] < (float)_DAT_00986b38) {
      param_1[0x76 = byte +0x1d8] = (int)((float)param_1[0x76 = byte +0x1d8] + (float)param_1[0x74 = byte +0x1d0]);
      if ((param_1[0x77 = byte +0x1dc] & 1U) == 0) {
        (**(code **)(*param_1 + 0x3c))(0x16257,1);
      }
      else {
        (**(code **)(*param_1 + 0x3c))(0x16256,1);
      }
    }
  }
  return;
}
```

### decoy: 0x0084dcb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084dcb0: 16-byte aligned: YES
  0084dcb0  55             PUSH EBP
  0084dcb1  8B EC          MOV EBP,ESP
  0084dcb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084dcb0
string literals (0):
direct callees (1):
  0x00419240  (unnamed)
struct offsets touched (top 4):
  +0x1DC [dec 476]  x2  via EAX,EDX
  +0x3C [dec 60]  x1  via EDX
  +0x1D4 [dec 468]  x1  via EDX
  +0x1D8 [dec 472]  x1  via ECX
```

### Static facts (script output, report-only)
- size 181 B, RET 4
- callees: 0x0041a0a0 CSWGuiButton::Draw (CALL)
- indirect: 0x0084dd95 slot 0x3c, 0x0084ddab slot 0x3c
- globals: 0x00a1b49c READ
- twins: none

## Card B17-12-11

GOG function: MdlNodeEmitter::InternalParseField  gog 0x008c7e00  callconv=(blank)  param_size_bytes=4  gog_size=15890  gog_retn=4
Candidate Steam address: 0x004878b0 (size 15890, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 167 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'random'@00988318; 'tangentlengthbezierkey'@0098c8a8; 'tangentlengthkey'@0098c8c0; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00486fb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
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
imm sets (* = in both, 58 common): GOG {0x2*,0x4*,0x8*,0x18*,0x24*,0x38*,0x44*,0x50*,0x54*,0x58*,0x5c*,0x60*,0x64*,0x68*,...(+44)} | Steam {0x2*,0x4*,0x8*,0x18*,0x24*,0x38*,0x44*,0x50*,0x54*,0x58*,0x5c*,0x60*,0x64*,0x68*,...(+44)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004878b0: 16-byte aligned: YES
  004878b0  55             PUSH EBP
  004878b1  8B EC          MOV EBP,ESP
  004878b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004878b0 FUN_004878b0 size=15890 ret=4 callers=1 callees=25

void __thiscall FUN_004878b0(CFrameWnd *param_1,int param_2)

{
  char *pcVar1;
  int iVar2;
  size_t sVar3;
  CFrameWnd *pCVar4;
  CFrameWnd *pCVar5;
  int *piVar6;
  uint *puVar7;
  undefined4 uVar8;
  char local_24;
  char local_23;
  char local_22;
  char local_21;
  int local_20;
  char local_19;
  uint local_18;
  char local_12;
  char local_11;
  CFrameWnd *local_10;
  char local_b;
  char local_a;
  char local_9;
  int local_8;
  
  local_8 = 0;
  if (DAT_00a309bd == '\0') {
    local_11 = '\0';
    local_20 = 0;
    local_19 = '\0';
    local_b = '\0';
    local_21 = '\0';
    local_18 = 0;
    local_24 = '\0';
    local_12 = '\0';
    local_a = '\0';
    local_22 = '\0';
    local_23 = '\0';
    local_9 = '\0';
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("p2p",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_11;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("p2p");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("p2p_sel",pcVar1);
    if (iVar2 == 0) {
      piVar6 = &local_20;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("p2p_sel");
      FUN_00484e20(sVar3 + param_2,piVar6);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("affectedByWind",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_19;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("affectedByWind");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("m_isTinted",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_b;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("m_isTinted");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("bounce",pcVar1);
    if (iVar2 == 0) {
      pcVar1 = &local_21;
      sVar3 = _strlen [GHIDRA-AUTO-LABEL: unreliable]("bounce");
      FUN_00484f10(sVar3 + param_2,pcVar1);
    }
    pcVar1 = (char *)FUN_00484d10(param_2);
    iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]("random",pcVar1);
    if (iVar2 == 0) {
      puVar7 = &local_18;
/* ... 1991 more lines truncated by --max-lines */
```

### decoy: 0x00486fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00486fb0: 16-byte aligned: YES
  00486fb0  55             PUSH EBP
  00486fb1  8B EC          MOV EBP,ESP
  00486fb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 15890 B, RET 4
- callers 1: 0x00453b50
- callees: 0x00484d10 (CALL), 0x009370c7 _stricmp (CALL), 0x00919f10 _strlen (CALL), 0x00484f10 (CALL), 0x00484e20 (CALL), 0x004e52d0 (CALL), ...
- strings: 0x0098c8a8 "tangentlengthbezierkey", 0x0098c8c0 "tangentlengthkey", 0x0098c8d4 "tangentlength", ...
- globals: 0x00a309bd READ
- twins: none

## Card B17-12-12

GOG function: FountainEmitter::Update  gog 0x008b6860  callconv=(blank)  param_size_bytes=4  gog_size=1598  gog_retn=4
Candidate Steam address: 0x004fac30 (size 1598, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004fac00:STR0/CALLEE0/SIG0.038/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b6830 PartEmitter::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008b6ed0 ExplosionEmitter::Update -> 0x004fb2d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x008a1270 Particle::randomPosition -> Steam ?  in candidate callees: n/a
  0x008a43c0 ChunkyParticle::Constructor -> Steam ?  in candidate callees: n/a
  0x008a7210 PartEmitter::MoveFunc -> Steam ?  in candidate callees: n/a
  0x008aed40 PartEmitter::chkParticleLife -> Steam ?  in candidate callees: n/a
  0x008aee60 PartEmitter::chkRandom -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x008ff4d0 WindManager::AddPointSourceWind -> Steam ?  in candidate callees: n/a
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 19 common): GOG {0x4*,0x8*,0xc*,0x14*,0x40*,0x54*,0x58*,0x88*,0x90*,0xb4*,0xf0*,0xf4*,0x114*,0x1c8*,...(+5)} | Steam {0x4*,0x8*,0xc*,0x14*,0x40*,0x54*,0x58*,0x88*,0x90*,0xb4*,0xf0*,0xf4*,0x114*,0x1c8*,...(+5)}
imm sets (* = in both, 13 common): GOG {0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0x60*,0x74*,0x78*,0xc4*,0x1c8*,0x1d4*} | Steam {0x4*,0x5*,0x8*,0xc*,0x14*,0x41*,0x44*,0x60*,0x74*,0x78*,0xc4*,0x1c8*,0x1d4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004fac30: 16-byte aligned: YES
  004fac30  55             PUSH EBP
  004fac31  8B EC          MOV EBP,ESP
  004fac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004fac30
string literals (0):
direct callees (10):
  0x0091C860  _ftol2_sse
  0x004F31A0  PartEmitter::chkParticleLife (`ret 4`, ECX=emitter)
  0x004EB670  (unnamed)
  0x00507BB0  (unnamed)
  0x0049B840  (unnamed)
  0x00919723  operator new
  0x004E8820  (unnamed)
  0x004E56D0  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x004F32C0  (unnamed)
struct offsets touched (top 10):
  +0x1C8 [dec 456]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0xB4 [dec 180]  x3  via EAX,ECX,EDX
  +0x1F6 [dec 502]  x3  via ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x54 [dec 84]  x2  via ECX,EDX
  +0x58 [dec 88]  x2  via EAX,ECX
  +0xF4 [dec 244]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004fac30 FUN_004fac30 size=1598 ret=4 callers=0 callees=10

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004fac30(int *param_1,float param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  float fVar8;
  float *pfVar9;
  int iVar10;
  int local_d4;
  int *local_d0;
  int *local_cc;
  float local_bc;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  undefined4 local_24;
  float local_20;
  int local_1c;
  int *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ce36;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_28 = FUN_0091c860(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_1c = param_1[1 = byte +0x4];
  local_14 = 0;
  local_24 = 0x3f800000;
  if (param_1[0x10 = byte +0x40] != 0) {
    local_24 = *(undefined4 *)(param_1[0x10 = byte +0x40] + 0x1e0);
  }
  FUN_004f31a0(param_2);
  local_14 = param_1[0x73 = byte +0x1cc];
  if (param_1[0x45 = byte +0x114] == DAT_00a04aa8) {
    if ((float)param_1[0x2d = byte +0xb4] <= param_2) {
      local_bc = (float)param_1[0x2d = byte +0xb4];
    }
    else {
      local_bc = param_2;
    }
    local_20 = local_bc / (float)param_1[0x2d = byte +0xb4];
  }
  while (iVar10 = local_14 + -1, 0 < local_14) {
    if (param_1[0x45 = byte +0x114] == DAT_00a04aa8) {
      iVar5 = *(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4);
      fVar8 = 1.0 - local_20;
      fVar1 = *(float *)(iVar5 + 0x64 [dec 100]);
      fVar2 = *(float *)(iVar5 + 0x68);
      iVar6 = *(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4);
      fVar3 = *(float *)(iVar6 + 0xc);
      fVar4 = *(float *)(iVar6 + 0x10);
      iVar7 = *(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4);
      *(float *)(iVar7 + 0x60) = fVar8 * *(float *)(iVar5 + 0x60) + local_20 * *(float *)(iVar6 + 8)
      ;
      *(float *)(iVar7 + 0x64 [dec 100]) = fVar8 * fVar1 + local_20 * fVar3;
      *(float *)(iVar7 + 0x68) = fVar8 * fVar2 + local_20 * fVar4;
    }
    pfVar9 = (float *)(*(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4) + 0x14);
    *pfVar9 = (float)param_1[0x22 = byte +0x88] * param_2 + *pfVar9;
    if (*(float *)(*(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4) + 0x14) <= (float)_DAT_0098b970) {
      local_14 = iVar10;
      if (*(float *)(*(int *)(param_1[0x72 = byte +0x1c8] + iVar10 * 4) + 0x14) < (float)_DAT_009916c0) {
/* ... 74 more lines truncated by --max-lines */
```

### decoy: 0x004fac00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004fac00: 16-byte aligned: YES
  004fac00  55             PUSH EBP
  004fac01  8B EC          MOV EBP,ESP
  004fac03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004fac00
string literals (0):
direct callees (1):
  0x004E8A30  PartEmitter::ctor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1598 B, RET 4
- callees: 0x0091c860 (CALL), 0x004f31a0 (CALL), 0x004eb670 (CALL), 0x00507bb0 (CALL), 0x0049b840 (CALL), 0x00919723 operator new (CALL), ...
- indirect: 0x004fb03b slot 0xc, 0x004fb0f7 slot 0x8, 0x004fb1d7 slot 0x8
- twins: none

## Card B17-12-13

GOG function: Global::CullUnnecessaryKeys  gog 0x008c68e0  callconv=(blank)  param_size_bytes=4  gog_size=874  gog_retn=0
Candidate Steam address: 0x00486390 (size 874, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004861d0:STR0/CALLEE0/SIG0.425/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008c6720 Global::AddGenericControllerSingle  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c6c50 Global::AddGenericController  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cebd0 unnamed -> Steam ?  in candidate callees: n/a
  0x00925c49 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x10*} | Steam {0x4*,0x8*,0xc*,0x10*}
imm sets (* = in both, 5 common): GOG {0x2*,0x8*,0x2c*,0x41*,0xa4*} | Steam {0x2*,0x8*,0x2c*,0x41*,0xa4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00486390: 16-byte aligned: YES
  00486390  55             PUSH EBP
  00486391  8B EC          MOV EBP,ESP
  00486393  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00486390
string literals (0):
direct callees (2):
  0x0091EBAD  (unnamed)
  0x0048E680  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x10 [dec 16]  x4  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00486390 FUN_00486390 size=874 ret=0 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00486390(int *param_1)

{
  int iVar1;
  double dVar2;
  int local_8;
  
  iVar1 = param_1[1 = byte +0x4] + -1;
  while (local_8 = iVar1, iVar1 = local_8 + -1, 1 < iVar1) {
    dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)(iVar1 * 0x2c + *param_1 + 0x4 [dec 4] {int*: byte +0x10}) -
                          *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x4 [dec 4] {int*: byte +0x10})));
    if ((((((float)dVar2 < _DAT_009f614c) &&
          (dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)(iVar1 * 0x2c + *param_1 + 0x8 [dec 8] {int*: byte +0x20}) -
                                 *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x8 [dec 8] {int*: byte +0x20}))),
          (float)dVar2 < _DAT_009f614c)) &&
         (dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)(iVar1 * 0x2c + *param_1 + 0xc) -
                                *(float *)((local_8 + -2) * 0x2c + *param_1 + 0xc))),
         (float)dVar2 < _DAT_009f614c)) &&
        ((dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)(iVar1 * 0x2c + *param_1 + 0x10) -
                                *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x10))),
         (float)dVar2 < _DAT_009f614c &&
         (dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)((local_8 + -3) * 0x2c + *param_1 + 0x4 [dec 4] {int*: byte +0x10}) -
                                *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x4 [dec 4] {int*: byte +0x10}))),
         (float)dVar2 < _DAT_009f614c)))) &&
       ((dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)((local_8 + -3) * 0x2c + *param_1 + 0x8 [dec 8] {int*: byte +0x20}) -
                               *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x8 [dec 8] {int*: byte +0x20}))),
        (float)dVar2 < _DAT_009f614c &&
        ((dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)((local_8 + -3) * 0x2c + *param_1 + 0xc) -
                                *(float *)((local_8 + -2) * 0x2c + *param_1 + 0xc))),
         (float)dVar2 < _DAT_009f614c &&
         (dVar2 = _fabs [GHIDRA-AUTO-LABEL: unreliable]((double)(*(float *)((local_8 + -3) * 0x2c + *param_1 + 0x10) -
                                *(float *)((local_8 + -2) * 0x2c + *param_1 + 0x10))),
         (float)dVar2 < _DAT_009f614c)))))) {
      FUN_0048e680();
    }
  }
  return;
}
```

### decoy: 0x004861d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004861d0: 16-byte aligned: YES
  004861d0  55             PUSH EBP
  004861d1  8B EC          MOV EBP,ESP
  004861d3  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004861d0
string literals (0):
direct callees (4):
  0x004850B0  (unnamed)
  0x005195A0  (unnamed)
  0x00451660  (unnamed)
  0x0048E460  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via EDX
```

### Static facts (script output, report-only)
- size 874 B, RET 0
- callers 1: 0x00486700
- callees: 0x0091ebad (CALL), 0x0048e680 (CALL)
- twins: none

## Card B17-12-14

GOG function: CExoRawInputInternal::GetMouseState  gog 0x00619a00  callconv=(blank)  param_size_bytes=4  gog_size=253  gog_retn=4
Candidate Steam address: 0x00732f80 (size 260, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00733090:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00619840 CExoRawInputInternal::GetKeyboardBuffer  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00619b00 CExoRawInputInternal::KeyboardModeOn  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00617c70 CExoRawInputInternal::InitializeDirectInputMouse -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x1c,0x24,0x28} | Steam {0x8,0xc,0x20,0x2c}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x10,0x8000000a,0x8007000c,0x80070015,0x8007001e,0x80070057} | Steam {0x4*,0x2c,0x3c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00732f80: 16-byte aligned: YES
  00732f80  55             PUSH EBP
  00732f81  8B EC          MOV EBP,ESP
  00732f83  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00732f80
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x2C [dec 44]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x20 [dec 32]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00732f80 FUN_00732f80 size=260 ret=4 callers=1 callees=0

void __thiscall FUN_00732f80(int param_1,int param_2)

{
  int iVar1;
  int *local_34;
  undefined4 local_30;
  uint local_2c;
  
  iVar1 = (**(code **)(**(int **)(param_1 + 0x20) + 0xc))
                    (*(undefined4 *)(param_1 + 0x20),param_2 + 4,&local_34,0);
  if (iVar1 == 0) {
    iVar1 = (**(code **)*local_34)
                      (local_34,&DAT_0099cad8,*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4);
    (**(code **)(*local_34 + 0x8 [dec 8] {int*: byte +0x20}))(local_34);
    if (iVar1 == 0) {
      local_30 = 0x2c;
      (**(code **)(**(int **)(*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4) + 0xc))
                (*(undefined4 *)(*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4),&local_30);
      if ((local_2c & 1) == 0) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0x2c) + DAT_00a7ea6c * 4) = 0;
      }
      else {
        DAT_00a7ea6c = DAT_00a7ea6c + 1;
      }
    }
  }
  return;
}
```

### decoy: 0x00733090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00733090: 16-byte aligned: YES
  00733090  55             PUSH EBP
  00733091  8B EC          MOV EBP,ESP
  00733093  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00733090
string literals (0):
direct callees (3):
  0x007311C0  (unnamed)
  0x007312A0  (unnamed)
  0x0040E770  (unnamed)
struct offsets touched (top 8):
  +0x2C [dec 44]  x6  via EAX,ECX,EDX
  +0x24 [dec 36]  x5  via EAX,ECX,EDX
  +0x28 [dec 40]  x5  via ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x1C [dec 28]  x3  via EAX,EDX
  +0x20 [dec 32]  x3  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EAX
```

### Static facts (script output, report-only)
- size 260 B, RET 4
- callers 1: 0x007306c0
- indirect: 0x00732fa8 slot 0xc, 0x00732fde slot 0x0, 0x00732fef slot 0x8, 0x0073302a slot 0xc, 0x0073305a slot 0x8
- twins: none

## Card B17-12-15

GOG function: Global::RotationArc  gog 0x0080df50  callconv=(blank)  param_size_bytes=28  gog_size=362  gog_retn=0
Candidate Steam address: 0x00516230 (size 1033, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.40 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005160c0:STR0/CALLEE6/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080dcf0 Global::pointto  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x00924754 Global::sqrt -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 16): the card's A-CALLEE credit may come from these, not from callees
  0x00420130 CSWCCreature::ApplyShadowBlob -> Steam 0x00774260  in candidate callers: no
  0x004204f0 CSWCCreature::UpdateShadowBlob -> Steam 0x00774620  in candidate callers: no
  0x00477070 CSWCObject::SetGroundNormal -> Steam 0x007f1f10  in candidate callers: no
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0xc} | Steam {0x4*,0x8*}
imm sets (* = in both, 1 common): GOG {0x8,0x28,0x41*} | Steam {0x2,0x5,0xc,0x41*,0x44,0xc4}

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

### decoy: 0x005160c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005160c0: 16-byte aligned: YES
  005160c0  55             PUSH EBP
  005160c1  8B EC          MOV EBP,ESP
  005160c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 1033 B, RET 0
- callers 2: 0x0045d620, 0x0045dfa0
- twins: none

## Card B17-12-16

GOG function: Global::RotationArc  gog 0x0080df50  callconv=(blank)  param_size_bytes=28  gog_size=362  gog_retn=0
Candidate Steam address: 0x00515e60 (size 598, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.43 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005160c0:STR0/CALLEE6/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0080dcf0 Global::pointto  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0080e0c0 Global::polyhit -> 0x00516230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0080c0f0 Vector::Normalize -> Steam ?  in candidate callees: n/a
  0x00924754 Global::sqrt -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (6 of 16): the card's A-CALLEE credit may come from these, not from callees
  0x00420130 CSWCCreature::ApplyShadowBlob -> Steam 0x00774260  in candidate callers: no
  0x004204f0 CSWCCreature::UpdateShadowBlob -> Steam 0x00774620  in candidate callers: no
  0x00477070 CSWCObject::SetGroundNormal -> Steam 0x007f1f10  in candidate callers: no
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0xc} | Steam {0x4*,0x8*}
imm sets (* = in both, 2 common): GOG {0x8*,0x28,0x41*} | Steam {0x2,0x4,0x5,0x8*,0xc,0x41*,0x44,0x5c,0xfffffffc}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 28 bytes -> x86 slots 28; ASM RET 0)
```

### prologue
```
prologue for 0x00515e60: 16-byte aligned: YES
  00515e60  55             PUSH EBP
  00515e61  8B EC          MOV EBP,ESP
  00515e63  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00515e60 FUN_00515e60 size=598 ret=0 callers=4 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00515e60(undefined4 param_1,undefined4 param_2)

{
  float *pfVar1;
  undefined4 *puVar2;
  float10 fVar3;
  float fVar4;
  undefined4 local_58;
  undefined1 local_48 [12];
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  float local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18 [12];
  float local_c;
  float local_8;
  
  if ((DAT_00a7d7f4 & 1) == 0) {
    DAT_00a7d7f4 = DAT_00a7d7f4 | 1;
    _DAT_00a7d7e8 = 0.0;
    _DAT_00a7d7ec = 0.0;
    _DAT_00a7d7f0 = 0.0;
  }
  if ((DAT_00a7d7f4 & 2) == 0) {
    DAT_00a7d7f4 = DAT_00a7d7f4 | 2;
    _DAT_00a7d7dc = 0.0;
    _DAT_00a7d7e0 = 0.0;
    _DAT_00a7d7e4 = 0.0;
  }
  pfVar1 = (float *)FUN_005143c0(local_18,param_2);
  _DAT_00a7d7e8 = *pfVar1;
  _DAT_00a7d7ec = pfVar1[1 = byte +0x4];
  _DAT_00a7d7f0 = pfVar1[2 = byte +0x8];
  local_24 = 0;
  local_20 = 0;
  local_1c = 0x3f800000;
  _DAT_00a7d7dc = _DAT_00a7d7f0 * 0.0 - _DAT_00a7d7ec * 1.0;
  _DAT_00a7d7e0 = _DAT_00a7d7e8 * 1.0 - _DAT_00a7d7f0 * 0.0;
  _DAT_00a7d7e4 = _DAT_00a7d7ec * 0.0 - _DAT_00a7d7e8 * 0.0;
  local_30 = _DAT_00a7d7dc;
  local_2c = _DAT_00a7d7e0;
  local_28 = _DAT_00a7d7e4;
  fVar3 = (float10)FUN_005142f0();
  local_c = (float)fVar3;
  if ((float)_DAT_00986b50 < local_c) {
    local_c = 1.0;
  }
  if (local_c < (float)_DAT_00986d98) {
    local_c = _DAT_00987474;
  }
  if (local_c == (float)_DAT_00986b38) {
    if (_DAT_00a7d7f0 <= (float)_DAT_00986b38) {
      local_58 = _DAT_00991490;
    }
    else {
      local_58 = 0;
    }
    FUN_005195a0(DAT_00a7d870,DAT_00a7d874,DAT_00a7d878,local_58);
  }
  else {
    fVar3 = (float10)FUN_0091fa70((double)local_c);
    local_8 = (float)fVar3;
    local_3c = 0;
    local_38 = 0;
    local_34 = 0x3f800000;
    if (_DAT_00a7d7e8 * 0.0 + _DAT_00a7d7ec * 0.0 + _DAT_00a7d7f0 * 1.0 < (float)_DAT_00986b38) {
      local_8 = (float)_DAT_0098bd60 - local_8;
    }
    fVar4 = local_8;
    puVar2 = (undefined4 *)FUN_005143c0(local_48,&DAT_00a7d7dc,local_8);
    FUN_005195a0(*puVar2,puVar2[1 = byte +0x4],puVar2[2 = byte +0x8],fVar4);
  }
/* ... 2 more lines truncated by --max-lines */
```

### decoy: 0x005160c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005160c0: 16-byte aligned: YES
  005160c0  55             PUSH EBP
  005160c1  8B EC          MOV EBP,ESP
  005160c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 598 B, RET 0
- callers 4: 0x007f3ea0, 0x00839ad0, 0x0083eb30, 0x00865830
- callees: 0x005143c0 (CALL), 0x005142f0 (CALL), 0x005195a0 (CALL), 0x0091fa70 (CALL)
- twins: none

## Card B17-12-17

GOG function: MdlNodeLight::InternalParseField  gog 0x008c7500  callconv=(blank)  param_size_bytes=4  gog_size=2293  gog_retn=4
Candidate Steam address: 0x00486fb0 (size 2293, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 26 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'fadingLight'@0098c6f0; 'flareradius'@0098c6fc; 'flarecolorshifts'@0098c708; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x004878b0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c6ff0 MdlNode::InternalParseField  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c7e00 MdlNodeEmitter::InternalParseField -> 0x004878b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0081b7a0 Global::Parse -> Steam ?  in candidate callees: n/a
  0x008c5260 Global::AurMessageGetToken -> Steam ?  in candidate callees: n/a
  0x008c5350 Global::AurMessageParseFloat -> Steam ?  in candidate callees: n/a
  0x008c5370 Global::AurMessageParseIntOrBool -> Steam ?  in candidate callees: n/a
  0x008c5f90 Global::Parse_11 -> Steam ?  in candidate callees: n/a
  0x008c6720 Global::AddGenericControllerSingle -> Steam ?  in candidate callees: n/a
  0x008c6c50 Global::AddGenericController -> Steam ?  in candidate callees: n/a
  0x008c6e50 Global::AddBezierController -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (28): 'fadingLight'; 'flareradius'; 'flarecolorshifts'; 'flarepositions' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 22 common): GOG {0x4*,0x8*,0xc*,0x18*,0x38*,0x44*,0x4c*,0x50*,0x58*,0x60*,0x64*,0x6c*,0x78*,0x84*,...(+8)} | Steam {0x4*,0x8*,0xc*,0x18*,0x38*,0x44*,0x4c*,0x50*,0x58*,0x60*,0x64*,0x6c*,0x78*,0x84*,...(+8)}

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

## Card B17-12-18

GOG function: CAurBehaviorCameraFollow::Control  gog 0x008c4b30  callconv=(blank)  param_size_bytes=4  gog_size=1703  gog_retn=4
Candidate Steam address: 0x00505da0 (size 1703, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.96) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00506450:STR0/CALLEE0/SIG0.159/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c4700 CAurBehaviorCameraFollow::Message  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c51e0 CAurBehaviorCameraFollow::GetTargetWorldPosition  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x0080d120 Global::normalize_2 -> Steam ?  in candidate callees: n/a
  0x0080d220 Global::slerp -> Steam ?  in candidate callees: n/a
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
  0x008c31a0 CAurBehavior::Control -> Steam ?  in candidate callees: n/a
  0x00927220 Global::pow -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 22 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,...(+8)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x30*,0x34*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,...(+8)}
imm sets (* = in both, 10 common): GOG {0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x24*,0x41*,0x44*,0x198*} | Steam {0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x24*,0x41*,0x44*,0x198*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00505da0: 16-byte aligned: YES
  00505da0  55             PUSH EBP
  00505da1  8B EC          MOV EBP,ESP
  00505da3  81 EC 98 01 00 00 SUB ESP,0x198
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00505da0
string literals (0):
direct callees (8):
  0x005043D0  (unnamed)
  0x0091E390  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00436200  (unnamed)
  0x00515620  (unnamed)
  0x00515390  (unnamed)
  0x00515290  (unnamed)
  0x00463120  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x12  via EAX,ECX,EDX
  +0x10 [dec 16]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x8  via EAX,ECX,EDX
  +0x3C [dec 60]  x8  via EAX,ECX,EDX
  +0x44 [dec 68]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x7  via EAX,ECX,EDX
  +0x60 [dec 96]  x5  via ECX,EDX
  +0x54 [dec 84]  x4  via EAX,ECX
  +0x68 [dec 104]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00505da0 FUN_00505da0 size=1703 ret=4 callers=0 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00505da0(int param_1,float param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  float10 fVar4;
  undefined1 *puVar5;
  undefined4 local_190;
  undefined4 local_18c;
  float local_188;
  undefined4 local_180;
  undefined4 local_17c;
  float local_178;
  undefined4 local_174;
  undefined4 local_170;
  float local_16c;
  undefined1 local_fc [12];
  undefined1 local_f0 [12];
  undefined1 local_e4 [12];
  undefined1 local_d8 [12];
  undefined1 local_cc [16];
  undefined1 local_bc [16];
  undefined1 local_ac [16];
  undefined1 local_9c [16];
  undefined1 local_8c [16];
  undefined1 local_7c [16];
  undefined1 local_6c [12];
  undefined1 local_60 [12];
  undefined1 local_54 [12];
  undefined1 local_48 [12];
  undefined1 local_3c [12];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  float local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (DAT_00a20020 != (int *)0x0) {
    (**(code **)(*DAT_00a20020 + 0xa8))();
  }
  local_30 = 0;
  local_2c = 0;
  local_28 = 0;
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  local_18 = 0;
  local_1c = 0;
  local_20 = 0;
  local_24 = 0x3f800000;
  iVar1 = FUN_005043d0();
  if (iVar1 != 0) {
    if (*(int *)(param_1 + 0x14) == 0) {
      (**(code **)(**(int **)(param_1 + 0x10) + 0x58))();
    }
    else {
      *(float *)(param_1 + 0x3c) = *(float *)(param_1 + 0x40) * param_2 + *(float *)(param_1 + 0x3c)
      ;
      *(undefined4 *)(param_1 + 0x40) = 0;
      if (*(float *)(param_1 + 0x4c) != (float)_DAT_00986b38) {
        if (*(float *)(param_1 + 0x54) <= *(float *)(param_1 + 0x3c)) {
          local_16c = *(float *)(param_1 + 0x54);
        }
        else {
          local_16c = *(float *)(param_1 + 0x3c);
        }
        if (*(float *)(param_1 + 0x50) <= local_16c) {
          if (*(float *)(param_1 + 0x54) <= *(float *)(param_1 + 0x3c)) {
            local_174 = *(undefined4 *)(param_1 + 0x54);
          }
/* ... 101 more lines truncated by --max-lines */
```

### decoy: 0x00506450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00506450: 16-byte aligned: YES
  00506450  55             PUSH EBP
  00506451  8B EC          MOV EBP,ESP
  00506453  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00506450
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x14 [dec 20]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1703 B, RET 4
- callees: 0x005043d0 (CALL), 0x0091e390 (CALL), 0x0044c8a0 (CALL), 0x00436200 (CALL), 0x00515620 (CALL), 0x00515390 (CALL), ...
- indirect: 0x00505dcb slot 0xa8, 0x00505e50 slot 0x58, 0x0050621a slot 0x64, 0x00506309 slot 0x68, 0x00506392 slot 0x60, ...
- twins: none

## Card B17-12-19

GOG function: Camera::Animate  gog 0x008c1fb0  callconv=__thiscall  param_size_bytes=4  gog_size=574  gog_retn=4
Candidate Steam address: 0x00480e40 (size 574, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00480df0:STR0/CALLEE0/SIG0.229/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008c1f60 Camera::HitCheck_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008c21f0 Camera::SetPosition  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x008fbe70 Gob::Animate -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x009259f0 Global::sin -> Steam ?  in candidate callees: n/a
  0x00925b20 Global::cos -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x10*,0x204*,0x208*} | Steam {0x4*,0x8*,0xc*,0x10*,0x204*,0x208*}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x24*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x24*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00480e40: 16-byte aligned: YES
  00480e40  55             PUSH EBP
  00480e41  8B EC          MOV EBP,ESP
  00480e43  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00480e40
string literals (0):
direct callees (4):
  0x009196FD  operator delete
  0x0091EA50  (unnamed)
  0x0091E770  (unnamed)
  0x004E1D40  (unnamed)
struct offsets touched (top 6):
  +0x208 [dec 520]  x21  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0x204 [dec 516]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00480e40 FUN_00480e40 size=574 ret=4 callers=1 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00480e40(int param_1,float param_2)

{
  float fVar1;
  float10 fVar2;
  
  if (*(int *)(param_1 + 0x208) != 0) {
    *(float *)(*(int *)(param_1 + 0x208) + 4) = *(float *)(*(int *)(param_1 + 0x208) + 4) + param_2;
    if (*(float *)(*(int *)(param_1 + 0x208) + 4) <= **(float **)(param_1 + 0x208)) {
      fVar1 = *(float *)(*(int *)(param_1 + 0x208) + 4) / **(float **)(param_1 + 0x208);
      if (*(int *)(*(int *)(param_1 + 0x208) + 0x10) == 0) {
        *(float *)(param_1 + 0x204) =
             (1.0 - fVar1) * *(float *)(*(int *)(param_1 + 0x208) + 8) +
             fVar1 * *(float *)(*(int *)(param_1 + 0x208) + 0xc);
      }
      else if (*(int *)(*(int *)(param_1 + 0x208) + 0x10) == 1) {
        fVar2 = (float10)FUN_0091ea50((double)(fVar1 * (float)_DAT_0098bd60));
        fVar1 = 1.0 - ((float)fVar2 + (float)_DAT_00986b50) / (float)_DAT_00987490;
        *(float *)(param_1 + 0x204) =
             (1.0 - fVar1) * *(float *)(*(int *)(param_1 + 0x208) + 8) +
             fVar1 * *(float *)(*(int *)(param_1 + 0x208) + 0xc);
      }
      else if (*(int *)(*(int *)(param_1 + 0x208) + 0x10) == 2) {
        fVar2 = (float10)FUN_0091ea50((double)((fVar1 * (float)_DAT_0098bd60) / (float)_DAT_00987490
                                              ));
        *(float *)(param_1 + 0x204) =
             (1.0 - (1.0 - (float)fVar2)) * *(float *)(*(int *)(param_1 + 0x208) + 8) +
             (1.0 - (float)fVar2) * *(float *)(*(int *)(param_1 + 0x208) + 0xc);
      }
      else if (*(int *)(*(int *)(param_1 + 0x208) + 0x10) == 3) {
        fVar2 = (float10)FUN_0091e770((double)((fVar1 * (float)_DAT_0098bd60) / (float)_DAT_00987490
                                              ));
        *(float *)(param_1 + 0x204) =
             (1.0 - (float)fVar2) * *(float *)(*(int *)(param_1 + 0x208) + 8) +
             (float)fVar2 * *(float *)(*(int *)(param_1 + 0x208) + 0xc);
      }
    }
    else {
      FUN_009196fd();
      *(undefined4 *)(param_1 + 0x208) = 0;
    }
  }
  FUN_004e1d40();
  return;
}
```

### decoy: 0x00480df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00480df0: 16-byte aligned: YES
  00480df0  55             PUSH EBP
  00480df1  8B EC          MOV EBP,ESP
  00480df3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00480df0
string literals (0):
direct callees (1):
  0x0045B1C0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 574 B, RET 4
- callers 1: 0x00481d50
- callees: 0x009196fd operator delete (CALL), 0x0091ea50 (CALL), 0x0091e770 (CALL), 0x004e1d40 (CALL)
- twins: none

## Card B17-12-20

GOG function: ExplosionEmitter::Update  gog 0x008b6ed0  callconv=(blank)  param_size_bytes=4  gog_size=1384  gog_retn=4
Candidate Steam address: 0x004fb2d0 (size 1384, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.87) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004fb840:STR0/CALLEE0/SIG0.134/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b6860 FountainEmitter::Update -> 0x004fac30  gate_class=REVIEW
- GOG-order neighbour after:  0x008b7440 SingleEmitter::Update  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x008a1270 Particle::randomPosition -> Steam ?  in candidate callees: n/a
  0x008a43c0 ChunkyParticle::Constructor -> Steam ?  in candidate callees: n/a
  0x008a7210 PartEmitter::MoveFunc -> Steam ?  in candidate callees: n/a
  0x008aee60 PartEmitter::chkRandom -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x14*,0x58*,0x70*,0x78*,0x88*,0xb4*,0xf0*,0x104*,0x114*,0x1c8*,0x1cc*} | Steam {0x4*,0x8*,0x14*,0x58*,0x70*,0x78*,0x88*,0xb4*,0xf0*,0x104*,0x114*,0x1c8*,0x1cc*}
imm sets (* = in both, 12 common): GOG {0x2*,0x4*,0x5*,0x8*,0x14*,0x41*,0x60*,0x70*,0x74*,0x78*,0xa4*,0x1c8*} | Steam {0x2*,0x4*,0x5*,0x8*,0x14*,0x41*,0x60*,0x70*,0x74*,0x78*,0xa4*,0x1c8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004fb2d0: 16-byte aligned: YES
  004fb2d0  55             PUSH EBP
  004fb2d1  8B EC          MOV EBP,ESP
  004fb2d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004fb2d0
string literals (0):
direct callees (8):
  0x0091C860  _ftol2_sse
  0x00919723  operator new
  0x004E8820  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x004E56D0  (unnamed)
  0x004F32C0  (unnamed)
  0x004EB670  (unnamed)
  0x0049B840  (unnamed)
struct offsets touched (top 10):
  +0x1C8 [dec 456]  x16  via EAX,ECX,EDX
  +0x70 [dec 112]  x5  via EAX,EDX
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x78 [dec 120]  x3  via EAX,ECX
  +0xB4 [dec 180]  x3  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x1CC [dec 460]  x2  via EAX,ECX
  +0x58 [dec 88]  x1  via EDX
  +0x88 [dec 136]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004fb2d0 FUN_004fb2d0 size=1384 ret=4 callers=0 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004fb2d0(int param_1,float param_2)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  int iVar5;
  undefined4 *puVar6;
  float fVar7;
  int iVar8;
  int iVar9;
  float *pfVar10;
  float local_b0;
  undefined4 local_9c;
  undefined4 local_98;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094ce76;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar9 = *(int *)(param_1 + 4);
  iVar8 = FUN_0091c860(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar8 == 0) {
    if (*(int *)(param_1 + 0x1cc) != 0) {
      puVar6 = (undefined4 *)**(int **)(param_1 + 0x1c8);
      FUN_0049b840(0);
      if (puVar6 != (undefined4 *)0x0) {
        (**(code **)*puVar6)(1);
      }
    }
  }
  else if (*(int *)(param_1 + 0x1cc) == 0) {
    if (*(char *)(iVar9 + 0xf0) == '\0') {
      iVar9 = FUN_00919723(0x74);
      local_8 = 1;
      if (iVar9 == 0) {
        local_9c = 0;
      }
      else {
        local_9c = FUN_004e56d0(param_1,param_2);
      }
      local_8 = 0xffffffff;
      FUN_00473f40(local_9c);
    }
    else {
      iVar9 = FUN_00919723(0x78);
      local_8 = 0;
      if (iVar9 == 0) {
        local_98 = 0;
      }
      else {
        local_98 = FUN_004e8820(param_1,param_2);
      }
      local_8 = 0xffffffff;
      FUN_00473f40(local_98);
    }
    FUN_004f32c0(**(undefined4 **)(param_1 + 0x1c8));
  }
  else {
    pfVar10 = (float *)(**(int **)(param_1 + 0x1c8) + 0x70);
    *pfVar10 = *pfVar10 + param_2;
    if (*(float *)(param_1 + 0x78) <= (float)_DAT_00986b38) {
      if (*(float *)(**(int **)(param_1 + 0x1c8) + 0x70) < (float)_DAT_00986b38) {
        if (*(float *)(param_1 + 0x78) <= (float)_DAT_00986b38) {
          *(float *)(**(int **)(param_1 + 0x1c8) + 0x70) = param_2;
        }
        else {
          *(undefined4 *)(**(int **)(param_1 + 0x1c8) + 0x70) = 0;
        }
      }
    }
    else if ((*(float *)(param_1 + 0x78) < *(float *)(**(int **)(param_1 + 0x1c8) + 0x70)) &&
            (*(int *)(iVar9 + 0x104) != 0)) {
/* ... 40 more lines truncated by --max-lines */
```

### decoy: 0x004fb840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004fb840: 16-byte aligned: YES
  004fb840  55             PUSH EBP
  004fb841  8B EC          MOV EBP,ESP
  004fb843  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004fb840
string literals (0):
direct callees (4):
  0x004E8A30  PartEmitter::ctor
  0x00428F80  (unnamed)
  0x00463ED0  (unnamed)
  0x00449230  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0xC0 [dec 192]  x1  via EDX
  +0x28C [dec 652]  x1  via EAX
  +0x290 [dec 656]  x1  via ECX
```

### Static facts (script output, report-only)
- size 1384 B, RET 4
- callees: 0x0091c860 (CALL), 0x00919723 operator new (CALL), 0x004e8820 (CALL), 0x00473f40 (CALL), 0x004e56d0 (CALL), 0x004f32c0 (CALL), ...
- indirect: 0x004fb813 slot 0x0
- twins: none

## Card B17-12-21

GOG function: Global::AurGUICloseViewport  gog 0x008d02d0  callconv=(blank)  param_size_bytes=0  gog_size=235  gog_retn=0
Candidate Steam address: 0x004799c0 (size 235, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00479460:STR0/CALLEE4/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d03c0 Global::GetResolutionTextScalar -> 0x00479ab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x0051cb70 CSWGuiMainInterface::Draw -> Steam 0x0074b3f0  in candidate callers: yes
  0x00521f10 CSWGuiMainInterface::DrawMap -> Steam 0x00750790  in candidate callers: yes
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0xa*,0xb*,0x10*} | Steam {0x2*,0xa*,0xb*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004799c0: 16-byte aligned: YES
  004799c0  55             PUSH EBP
  004799c1  8B EC          MOV EBP,ESP
  004799c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004799c0
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0xA309CE [dec 10684878]  x2  via ECX,EDX
  +0xA309C8 [dec 10684872]  x1  via EDX
  +0xA309CA [dec 10684874]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via EAX
  +0xA309D0 [dec 10684880]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004799c0 FUN_004799c0 size=235 ret=0 callers=12 callees=3

void FUN_004799c0(void)

{
  if (DAT_00a32a30 == 0) {
    DAT_00a309c0 = DAT_00a309c0 + -1;
    glViewport((int)*(short *)(&DAT_00a309c8 + DAT_00a309c0 * 10),
               ((int)DAT_00a309ce - (int)*(short *)(&DAT_00a309ca + DAT_00a309c0 * 10)) -
               (int)(&DAT_00a309ce)[DAT_00a309c0 * 5],
               (int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10),
               (int)(&DAT_00a309ce)[DAT_00a309c0 * 5]);
    if (*(short *)(&DAT_00a309d0 + DAT_00a309c0 * 10) == 0) {
      glDisable(DAT_009f4954);
    }
    else {
      glEnable(DAT_009f4954);
    }
  }
  return;
}
```

### decoy: 0x00479460 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00479460: 16-byte aligned: YES
  00479460  55             PUSH EBP
  00479461  8B EC          MOV EBP,ESP
  00479463  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00479460
string literals (0):
direct callees (2):
  0x00517980  Vec3::Equal
  0x00408740  (unnamed)
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

### Static facts (script output, report-only)
- size 235 B, RET 0
- callers 12: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x004184f0, 0x0041bf40, 0x0041f980, ...
- callees: 0x0000004f (CALL), 0x0000000e (CALL), 0x0000001c (CALL)
- globals: 0x00a309c0 READ, 0x00a309c0 READ, 0x00a309c0 READ, ...
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), glViewport (OPENGL32.DLL)
- twins: none

## Card B17-12-22

GOG function: Global::RenderSkinnedVPBlur  gog 0x0087f060  callconv=(blank)  param_size_bytes=4  gog_size=9743  gog_retn=0
Candidate Steam address: 0x004c3430 (size 25096, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.34 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004c9640:STR1/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00878e50 Global::RenderSkinnedVP -> 0x004c3430  gate_class=REVIEW
- GOG-order neighbour after:  0x00881670 PartSkin::DrawBumpedOutShellVP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x0080c7d0 unnamed -> Steam ?  in candidate callees: n/a
  ... 10 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '!!VP1.0 # This is the standard vertex pr'
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x18*,0x2c*,0x3c*,0x40*,0x44*,0x50*,0x182*,0x1ac,0x1b8*,0x1bc*,0x1c0*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x10,0x13,0x14,0x18*,0x2c*,0x2d,0x30,0x3c*,0x40*,0x44*,0x50*,...(+20)}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x9*,0xc*,0xd*,0xe*,0x10*,0x11*,0x12*,0x14*,0x22,...(+11)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6,0x7,0x8*,0x9*,0xa,0xb,0xc*,0xd*,0xe*,0xf,...(+42)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004c3430: 16-byte aligned: YES
  004c3430  55             PUSH EBP
  004c3431  8B EC          MOV EBP,ESP
  004c3433  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004c3430
string literals (17):
  0x00A036A8  at 004c4fa1  '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vTex0 = vertex.texcoord[0];\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg1.x, -fogParams.y, vReg0.x;\\nMUL\\tresult.color.secondary.x, vReg1.x, state.fog.params.w;\\nMOV\\tresult.fogcoord.x, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vTex0;\\n'
  0x009FDD78  at 004c54db  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nPARAM boneArray[51] = {program.env[18..68] };\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nATTRIB vWght = vertex.attrib[1];\\nMUL   vReg0, vertex.attrib[4], program.env[16].z;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\\nMUL   vReg4, vWght.x, vReg1;\\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\\nMUL   vReg5, vWght.x, vReg1;\\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\\nMUL   vReg6, vWght.x, vReg1;\\nARL   A0.x, vReg0.y;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1, vWght.y, vReg9;\\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\\nMAD   vReg4, vWght.y, vReg1, vReg4;\\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\\nMAD   vReg5, vWght.y, vReg1, vReg5;\\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\\nMAD   vReg6, vWght.y, vReg1, vReg6;\\nARL   A0.x, vReg0.z;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1, vWght.z, vReg9;\\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\\nMAD   vReg4, vWght.z, vReg1, vReg4;\\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\\nMAD   vReg5, vWght.z, vReg1, vReg5;\\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\\nMAD   vReg6, vWght.z, vReg1, vReg6;\\nARL   A0.x, vReg0.w;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1,vWght.w, vReg9;\\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\\nMAD   vReg4, vWght.w, vReg1, vReg4\\n;DP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\\nMAD   vReg5, vWght.w, vReg1, vReg5\\n;DP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\\nMAD   vReg10,vWght.w, vReg1, vReg6\\n;MOV   vReg9.w, program.env[16].w;\\nMOV   vReg10.w, program.env[16].w;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nMOV   vNormalWorld.w, program.env[16].w;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nADD  vReg0, program.env[12], -vReg9;\\nDP3   vReg0.w, vReg0, vReg0;\\nRSQ   vReg1.w, vReg0.w;\\nDST vReg2, vReg0.w, vReg1.w;\\nDP3 vReg2.w, vReg2, program.env[14];\\nRCP vReg2.w, vReg2.w;\\nMOV   result.color.secondary, vReg2.wwww;\\nMUL   vReg0, vReg0, vReg1.w;\\nMOV   vReg0.w, program.env[16].w;\\nADD vReg2, program.env[13],-vReg9;\\nDP3   vReg2.w, vReg2, vReg2;\\nRSQ   vReg2.w, vReg2.w;\\nMUL   vReg2, vReg2, vReg2.w;\\nMOV   vReg2.w, program.env[16].w;\\nADD vReg2, vReg2,vReg0;\\nMOV   vReg2.w, program.env[16].w;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[3], vertex.texcoord[0];\\nDP3   vReg1.x, vReg4, vReg0;\\nDP3   vReg1.y, vReg5, vReg0;\\nDP3   vReg1.z, vReg6, vReg0;\\nMOV   vReg1.w, program.env[16].w;\\nMOV   result.texcoord[1], vReg1;\\nDP3   vReg1.x, vReg4, vReg2;\\nDP3   vReg1.y, vReg5, vReg2;\\nDP3   vReg1.z, vReg6, vReg2;\\nMOV   vReg1.w, program.env[16].w;\\nMOV   result.texcoord[2], vReg1;\\n'
  0x00A0AE28  at 004c54e7  '!!VP1.0 # This is the vertex program for skinned animations.\\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;DP3   R1.x, v[TEX4], c[A0.x + 18];DP3   R1.y, v[TEX4], c[A0.x + 19];DP3   R1.z, v[TEX4], c[A0.x + 20];MUL   R4, v[WGHT].x, R1;DP3   R1.x, v[TEX5], c[A0.x + 18];DP3   R1.y, v[TEX5], c[A0.x + 19];DP3   R1.z, v[TEX5], c[A0.x + 20];MUL   R5, v[WGHT].x, R1;DP3   R1.x, v[TEX6], c[A0.x + 18];DP3   R1.y, v[TEX6], c[A0.x + 19];DP3   R1.z, v[TEX6], c[A0.x + 20];MUL   R6, v[WGHT].x, R1;ARL   A0.x, R0.y;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];MAD   R9, R1, v[WGHT].y, R9;DP3   R1.x, v[TEX4], c[A0.x + 18];DP3   R1.y, v[TEX4], c[A0.x + 19];DP3   R1.z, v[TEX4], c[A0.x + 20];MAD   R4, R1, v[WGHT].y, R4;DP3   R1.x, v[TEX5], c[A0.x + 18];DP3   R1.y, v[TEX5], c[A0.x + 19];DP3   R1.z, v[TEX5], c[A0.x + 20];MAD   R5, R1, v[WGHT].y, R5;DP3   R1.x, v[TEX6], c[A0.x + 18];DP3   R1.y, v[TEX6], c[A0.x + 19];DP3   R1.z, v[TEX6], c[A0.x + 20];MAD   R6, R1, v[WGHT].y, R6;ARL   A0.x, R0.z;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];MAD   R9, R1, v[WGHT].z, R9;DP3   R1.x, v[TEX4], c[A0.x + 18];DP3   R1.y, v[TEX4], c[A0.x + 19];DP3   R1.z, v[TEX4], c[A0.x + 20];MAD   R4, R1, v[WGHT].z, R4;DP3   R1.x, v[TEX5], c[A0.x + 18];DP3   R1.y, v[TEX5], c[A0.x + 19];DP3   R1.z, v[TEX5], c[A0.x + 20];MAD   R5, R1, v[WGHT].z, R5;DP3   R1.x, v[TEX6], c[A0.x + 18];DP3   R1.y, v[TEX6], c[A0.x + 19];DP3   R1.z, v[TEX6], c[A0.x + 20];MAD   R6, R1, v[WGHT].z, R6;ARL   A0.x, R0.w;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];MAD   R9, R1, v[WGHT].w, R9;DP3   R1.x, v[TEX4], c[A0.x + 18];DP3   R1.y, v[TEX4], c[A0.x + 19];DP3   R1.z, v[TEX4], c[A0.x + 20];MAD   R4, R1, v[WGHT].w, R4;DP3   R1.x, v[TEX5], c[A0.x + 18];DP3   R1.y, v[TEX5], c[A0.x + 19];DP3   R1.z, v[TEX5], c[A0.x + 20];MAD   R5, R1, v[WGHT].w, R5;DP3   R1.x, v[TEX6], c[A0.x + 18];DP3   R1.y, v[TEX6], c[A0.x + 19];DP3   R1.z, v[TEX6], c[A0.x + 20];MAD   R6, R1, v[WGHT].w, R6;MOV   R9.w, c[16].w;DP4   o[HPOS].x, c[0], R9;DP4   o[HPOS].y, c[1], R9;DP4   o[HPOS].z, c[2], R9;DP4   o[HPOS].w, c[3], R9;DP4   R10.x, c[8], R9;DP4   R10.y, c[9], R9;DP4   R10.z, c[10], R9;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV   o[FOGC], R1.w;ADD  R0, c[12], -R9;DP3   R0.w, R0, R0;RSQ   R1.w, R0.w;DST R2, R0.w, R1.w;DP3 R2.w, R2, c[14];RCP R2.w, R2.w;MOV   o[COL1].xyzw, R2.wwww;MUL   R0, R0, R1.w;MOV   R0.w, c[16].w;ADD R2, c[13],-R9;DP3   R2.w, R2, R2;RSQ   R2.w, R2.w;MUL   R2, R2, R2.w;MOV   R2.w, c[16].w;ADD R2, R2,R0;MOV   R2.w, c[16].w;MOV   R9, c[16].xxxw;MOV   o[TEX0], v[TEX0];MOV   o[TEX3], v[TEX0];DP3   R1.x, R4, R0;DP3   R1.y, R5, R0;DP3   R1.z, R6, R0;MOV   R1.w, c[16].w;MOV   o[TEX1], R1;DP3   R1.x, R4, R2;DP3   R1.y, R5, R2;DP3   R1.z, R6, R2;MOV   R1.w, c[16].w;MOV   o[TEX2], R1;'
  0x00A036A8  at 004c5596  '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vTex0 = vertex.texcoord[0];\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg1.x, -fogParams.y, vReg0.x;\\nMUL\\tresult.color.secondary.x, vReg1.x, state.fog.params.w;\\nMOV\\tresult.fogcoord.x, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vTex0;\\n'
  0x009FA368  at 004c65b9  '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vTex0 = vertex.texcoord[0];\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg1.x, -fogParams.y, vReg0.x;\\nMUL\\tresult.color.secondary.x, vReg1.x, state.fog.params.w;\\nMOV\\tresult.fogcoord.x, vReg0.x;\\nMOV   result.texcoord[0], vTex0;\\n'
  0x00A08CB0  at 004c65c5  '!!VP1.0 # This is the vertex program for skinned animations.\\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];DP3   R5.x, v[NRML], c[A0.x + 18];DP3   R5.y, v[NRML], c[A0.x + 19];DP3   R5.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.y;DP4   R2.x, v[OPOS], c[A0.x + 18];DP4   R2.y, v[OPOS], c[A0.x + 19];DP4   R2.z, v[OPOS], c[A0.x + 20];DP3   R6.x, v[NRML], c[A0.x + 18];DP3   R6.y, v[NRML], c[A0.x + 19];DP3   R6.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.z;DP4   R3.x, v[OPOS], c[A0.x + 18];DP4   R3.y, v[OPOS], c[A0.x + 19];DP4   R3.z, v[OPOS], c[A0.x + 20];DP3   R7.x, v[NRML], c[A0.x + 18];DP3   R7.y, v[NRML], c[A0.x + 19];DP3   R7.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.w;DP4   R4.x, v[OPOS], c[A0.x + 18];DP4   R4.y, v[OPOS], c[A0.x + 19];DP4   R4.z, v[OPOS], c[A0.x + 20];DP3   R8.x, v[NRML], c[A0.x + 18];DP3   R8.y, v[NRML], c[A0.x + 19];DP3   R8.z, v[NRML], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;MAD   R9, R2, v[WGHT].y, R9;MAD   R9, R3, v[WGHT].z, R9;MAD   R9, R4, v[WGHT].w, R9;MOV   R9.w, c[16].w;MUL   R10, v[WGHT].x, R5;MAD   R10, R6, v[WGHT].y, R10;MAD   R10, R7, v[WGHT].z, R10;MAD   R10, R8, v[WGHT].w, R10;DP4   o[HPOS].x, c[0], R9;DP4   o[HPOS].y, c[1], R9;DP4   o[HPOS].z, c[2], R9;DP4   o[HPOS].w, c[3], R9;DP3   R11.x, c[8], R10;DP3   R11.y, c[9], R10;DP3   R11.z, c[10], R10;MOV   R11.w, c[16].w;DP3   R11.w, R11, R11;RSQ   R11.w, R11.w;MUL   R11, R11, R11.w;DP4   R10.x, c[8], R9;DP4   R10.y, c[9], R9;DP4   R10.z, c[10], R9;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV o[FOGC], R1.w;MOV   R9, c[16].xxxw;MOV   o[TEX0], v[TEX0];'
  0x009FAED8  at 004c679d  '!!ARBvp1.0\\nPARAM const0 = 0.0;\\nPARAM const1 = 1.0;\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg0.x, -fogParams.y, vReg0.x;\\nMOV result.fogcoord.x, const1;\\nMOV    result.color.xyz, state.fog.color;\\nMUL\\tresult.color.w, vReg0.x, state.fog.params.w;\\n'
  0x009FF608  at 004c68ef  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nPARAM boneArray[51] = {program.env[18..68] };\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vLightVectorWS = vReg4;\\nALIAS vFinalColour = vReg9;\\nALIAS vBoneIndices = vReg8;\\nATTRIB vWght = vertex.attrib[1];\\nMUL   vBoneIndices, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vBoneIndices.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nARL   A0.x, vBoneIndices.y;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1, vWght.y, vReg9;\\nARL   A0.x, vBoneIndices.z;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1, vWght.z, vReg9;\\nARL   A0.x, vBoneIndices.w;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nMAD   vReg9, vReg1,vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP4   vReg10.x, program.env[8], vReg9;\\nDP4   vReg10.y, program.env[9], vReg9;\\nDP4   vReg10.z, program.env[10], vReg9;\\nMOV   vReg10.w, program.env[16].w;\\nDP3\\tvReg1.x, vReg10, vReg10;\\nRSQ\\tvReg1.x, vReg1.x;\\nRCP\\tvReg1.x, vReg1.x;\\nMOV result.fogcoord, vReg1.x;\\nADD  vReg0, program.env[12], -vReg9;\\nDP3   vReg0.w, vReg0, vReg0;\\nRSQ   vReg1.w, vReg0.w;\\nDST vReg2, vReg0.w, vReg1.w;\\nDP3 vReg2.w, vReg2, program.env[14];\\nRCP vReg2.w, vReg2.w;\\nMOV   result.color.secondary, vReg2.wwww;\\nMUL   vReg0, vReg0, vReg1.w;\\nMOV   vReg0.w, program.env[16].w;\\nADD vReg2, program.env[13],-vReg9;\\nDP3   vReg2.w, vReg2, vReg2;\\nRSQ   vReg2.w, vReg2.w;\\nMUL   vReg2, vReg2, vReg2.w;\\nMOV   vReg2.w, program.env[16].w;\\nADD vReg2, vReg2,vReg0;\\nMOV   vReg2.w, program.env[16].w;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[3], vertex.texcoord[0];\\nMUL\\tvReg9, boneArray[A0.x + 0], vReg0.x;\\nMAD\\tvReg9, boneArray[A0.x + 1], vReg0.y, vReg9;\\nMAD\\tvReg9, boneArray[A0.x + 2], vReg0.z, vReg9;\\nMOV\\tvReg5.x, boneArray[A0.x+0].w;\\nMOV\\tvReg5.y, boneArray[A0.x+1].w;\\nMOV\\tvReg5.z, boneArray[A0.x+2].w;\\nSUB\\tvReg9, vReg9, vReg5;\\nMUL\\tvReg4, vReg9, vWght.w;\\nMUL\\tvReg9, vReg2.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg2.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg2.z, boneArray[A0.x + 2], vReg9;\\nSUB\\tvReg9, vReg9, vReg5;\\nMUL\\tvReg6, vReg9, vWght.w;\\nARL   A0.x, vBoneIndices.z;\\nMUL\\tvReg9, vReg0.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg0.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg0.z, boneArray[A0.x + 2], vReg9;\\nMOV\\tvReg5.x, boneArray[A0.x+0].w;\\nMOV\\tvReg5.y, boneArray[A0.x+1].w;\\nMOV\\tvReg5.z, boneArray[A0.x+2].w;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg4, vReg9, vWght.z, vReg4;\\nMUL\\tvReg9, vReg2.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg2.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg2.z, boneArray[A0.x + 2], vReg9;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg6, vReg9, vWght.z, vReg6;\\nARL   A0.x, vBoneIndices.y;\\nMUL\\tvReg9, vReg0.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg0.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg0.z, boneArray[A0.x + 2], vReg9;\\nMOV\\tvReg5.x, boneArray[A0.x+0].w;\\nMOV\\tvReg5.y, boneArray[A0.x+1].w;\\nMOV\\tvReg5.z, boneArray[A0.x+2].w;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg4, vReg9, vWght.y, vReg4;\\nMUL\\tvReg9, vReg2.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg2.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg2.z, boneArray[A0.x + 2], vReg9;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg6, vReg9, vWght.y, vReg6;\\nARL   A0.x, vBoneIndices.x;\\nMUL\\tvReg9, vReg0.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg0.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg0.z, boneArray[A0.x + 2], vReg9;\\nMOV\\tvReg5.x, boneArray[A0.x+0].w;\\nMOV\\tvReg5.y, boneArray[A0.x+1].w;\\nMOV\\tvReg5.z, boneArray[A0.x+2].w;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg4, vReg9, vWght.x, vReg4;\\nMOV   vReg4.w, program.env[16].w;\\nMOV   result.texcoord[2], vReg4;\\nMUL\\tvReg9, vReg2.x, boneArray[A0.x + 0];\\nMAD\\tvReg9, vReg2.y, boneArray[A0.x + 1], vReg9;\\nMAD\\tvReg9, vReg2.z, boneArray[A0.x + 2], vReg9;\\nSUB\\tvReg9, vReg9, vReg5;\\nMAD\\tvReg6, vReg9, vWght.z, vReg6;\\nMOV   vReg6.w, program.env[16].w;\\nMOV   result.texcoord[3], vReg6;\\n'
  0x00A036A8  at 004c6990  '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vTex0 = vertex.texcoord[0];\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg1.x, -fogParams.y, vReg0.x;\\nMUL\\tresult.color.secondary.x, vReg1.x, state.fog.params.w;\\nMOV\\tresult.fogcoord.x, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vTex0;\\n'
  0x009FA368  at 004c7a2c  '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nATTRIB vTex0 = vertex.texcoord[0];\\nATTRIB vWght = vertex.attrib[1];\\nPARAM boneArray[51] = {program.env[18..68] };\\nPARAM fogParams = state.fog.params;\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nADD\\tvReg1.x, -fogParams.y, vReg0.x;\\nMUL\\tresult.color.secondary.x, vReg1.x, state.fog.params.w;\\nMOV\\tresult.fogcoord.x, vReg0.x;\\nMOV   result.texcoord[0], vTex0;\\n'
  0x00A08CB0  at 004c7a38  '!!VP1.0 # This is the vertex program for skinned animations.\\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];DP3   R5.x, v[NRML], c[A0.x + 18];DP3   R5.y, v[NRML], c[A0.x + 19];DP3   R5.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.y;DP4   R2.x, v[OPOS], c[A0.x + 18];DP4   R2.y, v[OPOS], c[A0.x + 19];DP4   R2.z, v[OPOS], c[A0.x + 20];DP3   R6.x, v[NRML], c[A0.x + 18];DP3   R6.y, v[NRML], c[A0.x + 19];DP3   R6.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.z;DP4   R3.x, v[OPOS], c[A0.x + 18];DP4   R3.y, v[OPOS], c[A0.x + 19];DP4   R3.z, v[OPOS], c[A0.x + 20];DP3   R7.x, v[NRML], c[A0.x + 18];DP3   R7.y, v[NRML], c[A0.x + 19];DP3   R7.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.w;DP4   R4.x, v[OPOS], c[A0.x + 18];DP4   R4.y, v[OPOS], c[A0.x + 19];DP4   R4.z, v[OPOS], c[A0.x + 20];DP3   R8.x, v[NRML], c[A0.x + 18];DP3   R8.y, v[NRML], c[A0.x + 19];DP3   R8.z, v[NRML], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;MAD   R9, R2, v[WGHT].y, R9;MAD   R9, R3, v[WGHT].z, R9;MAD   R9, R4, v[WGHT].w, R9;MOV   R9.w, c[16].w;MUL   R10, v[WGHT].x, R5;MAD   R10, R6, v[WGHT].y, R10;MAD   R10, R7, v[WGHT].z, R10;MAD   R10, R8, v[WGHT].w, R10;DP4   o[HPOS].x, c[0], R9;DP4   o[HPOS].y, c[1], R9;DP4   o[HPOS].z, c[2], R9;DP4   o[HPOS].w, c[3], R9;DP3   R11.x, c[8], R10;DP3   R11.y, c[9], R10;DP3   R11.z, c[10], R10;MOV   R11.w, c[16].w;DP3   R11.w, R11, R11;RSQ   R11.w, R11.w;MUL   R11, R11, R11.w;DP4   R10.x, c[8], R9;DP4   R10.y, c[9], R9;DP4   R10.z, c[10], R9;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV o[FOGC], R1.w;MOV   R9, c[16].xxxw;MOV   o[TEX0], v[TEX0];'
  0x009FCEC8  at 004c7c00  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nPARAM boneArray[51] = {program.env[18..68] };\\nATTRIB vWght = vertex.attrib[1];\\nATTRIB vTex4 = vertex.attrib[12];\\nATTRIB vTex5 = vertex.attrib[13];\\nATTRIB vTex6 = vertex.attrib[14];\\nADDRESS A0;\\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vReg9;\\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\\nARL   A0.x, vReg0.x;\\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.y;\\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.z;\\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\\nARL   A0.x, vReg0.w;\\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\\nMUL   vReg9, vWght.x, vReg1;\\nMAD   vReg9, vReg2, vWght.y, vReg9;\\nMAD   vReg9, vReg3, vWght.z, vReg9;\\nMAD   vReg9, vReg4, vWght.w, vReg9;\\nMOV   vReg9.w, program.env[16].w;\\nDP4   result.position.x, mvpMatrix[0], vReg9;\\nDP4   result.position.y, mvpMatrix[1], vReg9;\\nDP4   result.position.z, mvpMatrix[2], vReg9;\\nDP4   result.position.w, mvpMatrix[3], vReg9;\\nMUL   vReg10, vWght.x, vReg5;\\nMAD   vReg10, vReg6, vWght.y, vReg10;\\nMAD   vReg10, vReg7, vWght.z, vReg10;\\nMAD   vReg10, vReg8, vWght.w, vReg10;\\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\\nMOV   vNormalWorld.w, program.env[16].w;\\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\\nRSQ   vNormalWorld.w, vNormalWorld.w;\\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\\nDP4   vReg10.x, mvMatrix[0], vReg9;\\nDP4   vReg10.y, mvMatrix[1], vReg9;\\nDP4   vReg10.z, mvMatrix[2], vReg9;\\nMOV   vReg10.w, mvMatrix[3].w;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord.x, vReg0.x;\\nADD  vReg0, program.env[12], -vReg9;\\nDP3   vReg0.w, vReg0, vReg0;\\nRSQ   vReg1.w, vReg0.w;\\nDST vReg2, vReg0.w, vReg1.w;\\nDP3 vReg2.w, vReg2, program.env[14];\\nRCP vReg2.w, vReg2.w;\\nMOV   result.color.secondary, vReg2.w;\\nMUL   vReg0, vReg0, vReg1.w;\\nMOV   vReg0.w, program.env[16].w;\\nADD vReg2, program.env[13],-vReg9;\\nDP3   vReg2.w, vReg2, vReg2;\\nRSQ   vReg2.w, vReg2.w;\\nMUL   vReg2, vReg2, vReg2.w;\\nMOV   vReg2.w, program.env[16].w;\\nADD vReg2, vReg2,vReg0;\\nMOV   vReg2.w, program.env[16].w;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[3], vertex.texcoord[0];\\nDP3   vReg1.x, vTex4, vReg0;\\nDP3   vReg1.y, vTex5, vReg0;\\nDP3   vReg1.z, vTex6, vReg0;\\nMOV   vReg1.w, program.env[16].w;\\nMOV   result.texcoord[1], vReg1;\\nDP3   vReg1.x, vTex4, vReg2;\\nDP3   vReg1.y, vTex5, vReg2;\\nDP3   vReg1.z, vTex6, vReg2;\\nMOV   vReg1.w, program.env[16].w;\\nMOV   result.texcoord[2], vReg1;\\n'
direct callees (35):
  0x0044E300  (unnamed)
  0x00451800  (unnamed)
  0x0091D3BA  (unnamed)
  0x00428F80  (unnamed)
  0x004BE4F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00463120  (unnamed)
  0x0046CE40  (unnamed)
  0x0046CDA0  (unnamed)
  0x0046CCF0  (unnamed)
  0x004DB420  (unnamed)
  0x004DB580  (unnamed)
  0x00454960  (unnamed)
  0x004845E0  (unnamed)
  0x00919723  operator new
  0x00506560  (unnamed)
  0x00506640  (unnamed)
  0x0049CF40  (unnamed)
  0x00484710  (unnamed)
struct offsets touched (top 10):
  +0x988530 [dec 9995568]  x70  via EAX,ECX,EDX
  +0x8 [dec 8]  x68  via EAX,ECX,EDX
  +0x4 [dec 4]  x66  via EAX,ECX,EDX
  +0x44 [dec 68]  x30  via EAX,ECX,EDX
  +0xC [dec 12]  x23  via EAX,ECX,EDX
  +0x8C [dec 140]  x18  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x18  via EAX,ECX,EDX
  +0x988658 [dec 9995864]  x15  via EAX,ECX,EDX
  +0x40 [dec 64]  x14  via EAX,ECX,EDX
  +0x10 [dec 16]  x10  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004c3430 FUN_004c3430 size=25096 ret=0 callers=1 callees=45

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void FUN_004c3430(int *param_1)

{
  float fVar1;
  int iVar2;
  char cVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  int iVar6;
  undefined4 *puVar7;
  float10 fVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 local_c98;
  undefined4 local_c94;
  undefined4 local_c90;
  undefined4 local_c7c;
  undefined4 local_c78;
  char *local_c74;
  undefined4 local_c70;
  char *local_c6c;
  undefined4 local_c58;
  undefined4 local_c54;
  undefined4 local_c50;
  undefined4 local_c4c;
  char *local_c48;
  undefined4 local_c34;
  undefined4 local_c30;
  char *local_c2c;
  undefined4 local_c28;
  undefined1 local_7f8 [12];
  undefined1 local_7ec [12];
  undefined1 local_7e0 [12];
  float local_7d4;
  float local_7d0;
  float local_7cc;
  float local_7c8;
  float local_7c4;
  float local_7c0;
  float local_7bc;
  float local_7b8;
  float *local_7b4;
  float *local_7b0;
  float local_7a0;
  float local_79c;
  float local_798;
  float *local_794;
  float local_790;
  float local_78c;
  float local_788;
  float local_784;
  float local_780;
  float local_77c;
  float local_778;
  float local_774;
  float local_770;
  float local_76c;
  float local_768;
  float local_764;
  float *local_760;
  float *local_75c;
  float local_758;
  float local_754;
  float local_750;
  float local_74c;
  float local_748;
  float local_744;
  float local_740;
  float local_73c;
  float local_738;
  float local_734;
  float local_730;
  float local_72c;
/* ... 1978 more lines truncated by --max-lines */
```

### decoy: 0x004c9640 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004c9640: 16-byte aligned: YES
  004c9640  55             PUSH EBP
  004c9641  8B EC          MOV EBP,ESP
  004c9643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004c9640
string literals (1):
  0x00A0D558  at 004cab21  '!!VP1.0 # This is the standard vertex program.\\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];DP3   R5.x, v[NRML], c[A0.x + 18];DP3   R5.y, v[NRML], c[A0.x + 19];DP3   R5.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.y;DP4   R2.x, v[OPOS], c[A0.x + 18];DP4   R2.y, v[OPOS], c[A0.x + 19];DP4   R2.z, v[OPOS], c[A0.x + 20];DP3   R6.x, v[NRML], c[A0.x + 18];DP3   R6.y, v[NRML], c[A0.x + 19];DP3   R6.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.z;DP4   R3.x, v[OPOS], c[A0.x + 18];DP4   R3.y, v[OPOS], c[A0.x + 19];DP4   R3.z, v[OPOS], c[A0.x + 20];DP3   R7.x, v[NRML], c[A0.x + 18];DP3   R7.y, v[NRML], c[A0.x + 19];DP3   R7.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.w;DP4   R4.x, v[OPOS], c[A0.x + 18];DP4   R4.y, v[OPOS], c[A0.x + 19];DP4   R4.z, v[OPOS], c[A0.x + 20];DP3   R8.x, v[NRML], c[A0.x + 18];DP3   R8.y, v[NRML], c[A0.x + 19];DP3   R8.z, v[NRML], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;MAD   R9, R2, v[WGHT].y, R9;MAD   R9, R3, v[WGHT].z, R9;MAD   R9, R4, v[WGHT].w, R9;MOV   R9.w, c[16].w;MUL   R10, v[WGHT].x, R5;MAD   R10, R6, v[WGHT].y, R10;MAD   R10, R7, v[WGHT].z, R10;MAD   R10, R8, v[WGHT].w, R10;DP3   R10.w, R10, R10;RSQ   R10.w, R10.w;MUL   R10,   R10, R10.w;MOV   R10.w, c[16].x;MOV  R4, c[12];MOV   R7.x, c[14].x;MIN   R2.y, R7.x, c[14].y;MUL   R5.xyz, R4, R2.y;MOV   R5.w, c[16].x;DP3   R3.w, R4, R10;SLT   R2.w, R3.w, c[16].x;MOV   R1, c[17].xyzw;MOV   R1.w, c[16].x;MAD   R6, R2.w, -R5, R9.xyzw;MUL   R7.y, R2.y, c[13].x;RCP   R8.y, c[14].y;MUL   R8.w, R2.y, R8.y;MUL   R7.y, R7.y, R8.w;MUL   R7.y, R7.y, c[17].w;MOV   R3.w,   c[16].x;MOV   R3.xyz, c[69];DP3   R3.w,   R3, R3;RSQ   R5.w,   R3.w;MUL   R3,     R3, R5.w;ADD   R5.xyz, R6, -R1;DP3   R5.w, R5, R3;SLT   R5.x, R5.w, c[16].x;MOV   R7.x, c[16].w;MAD   R5.y, R5.x, -c[16].y, R7.x;MUL   R5.z, R5.y, R7.y;MUL   R5.z, R5.z, R2.w;MAD   R6, R5.z, R3, R6;ADD   R2.x, -R2.w, c[16].w;MUL   R1, R10, -c[14].z;MOV   R1.w, c[16].x;MAD   R5, R2.x, R1, R6;DP4   R1.x, R5, c[0];DP4   R1.y, R5, c[1];DP4   R1.z, R5, c[2];DP4   R1.w, R5, c[3];MOV   o[HPOS].xyzw, R1.xyzw;ADD   R2.w, c[16].w, -R2.w;MOV   R9.xyz, c[16].xxxx;MUL   R9.w, c[69].w, R2.w;MOV   o[COL0], R9;DP3   R11.x, c[8],  R10;DP3   R11.y, c[9],  R10;DP3   R11.z, c[10], R10;MOV   R11.w, c[16].w;DP4   R10.x, c[8],  R5;DP4   R10.y, c[9],  R5;DP4   R10.z, c[10], R5;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV   o[FOGC].x, -R1.w;MOV   o[TEX0], v[TEX0];'
direct callees (18):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x0044E300  (unnamed)
  0x00451800  (unnamed)
  0x0091D3BA  (unnamed)
  0x00428F80  (unnamed)
  0x0040F9F0  (unnamed)
  0x004BE4F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00463120  (unnamed)
  0x0046CE40  (unnamed)
  0x0046CDA0  (unnamed)
  0x0046CCF0  (unnamed)
  0x00919723  operator new
  0x00506560  (unnamed)
  0x005143C0  (unnamed)
  0x00506640  (unnamed)
  0x0049CF40  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x51  via EAX,ECX,EDX
  +0x8 [dec 8]  x51  via EAX,ECX,EDX
  +0xC [dec 12]  x20  via EAX,ECX,EDX
  +0x40 [dec 64]  x5  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x988580 [dec 9995648]  x2  via ECX
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
  +0x2C [dec 44]  x1  via EAX
  +0x44 [dec 68]  x1  via EAX
```

### Static facts (script output, report-only)
- size 25096 B, RET 0
- callers 1: 0x004cd380
- callees: 0x0044e300 (CALL), 0x00451800 (CALL), 0x0091d3ba (CALL), 0x00428f80 (CALL), 0x0000000e (CALL), 0x004be4f0 (CALL), ...
- indirect: 0x004c346d slot 0x50, 0x004c34a0 slot 0x2c, 0x004c34f3 slot 0x18, 0x004c35d9 unresolved, 0x004c3652 unresolved, ...
- strings: 0x009faed8 "!!ARBvp1.0\nPARAM const0 = 0.0;\nPARAM const1 = 1.0;\nPARAM mvMatrix[4] = { state.matrix.modelview[0] };\nPARAM mvpMatrix[4] = { state.matrix.mvp };\nATTRIB vWght = vertex.attrib[1];\nPARAM boneArray[51] = {program.env[18..68] };\nPARAM fogParams = state.fog.params;\nADDRESS A0;\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\nTEMP vReg0, vReg1, vReg2, vReg3;\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\nARL   A0.x, vReg0.x;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nARL   A0.x, vReg0.y;\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\nARL   A0.x, vReg0.z;\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\nARL   A0.x, vReg0.w;\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\nMUL   vReg9, vWght.x, vReg1;\nMAD   vReg9, vReg2, vWght.y, vReg9;\nMAD   vReg9, vReg3, vWght.z, vReg9;\nMAD   vReg9, vReg4, vWght.w, vReg9;\nMOV   vReg9.w, program.env[16].w;\nDP4   result.position.x, mvpMatrix[0], vReg9;\nDP4   result.position.y, mvpMatrix[1], vReg9;\nDP4   result.position.z, mvpMatrix[2], vReg9;\nDP4   result.position.w, mvpMatrix[3], vReg9;\nDP4   vReg10.x, mvMatrix[0], vReg9;\nDP4   vReg10.y, mvMatrix[1], vReg9;\nDP4   vReg10.z, mvMatrix[2], vReg9;\nMOV   vReg10.w, mvMatrix[3].w;\nDP3\tvReg0.x, vReg10, vReg10;\nRSQ\tvReg0.x, vReg0.x;\nRCP\tvReg0.x, vReg0.x;\nADD\tvReg0.x, -fogParams.y, vReg0.x;\nMOV result.fogcoord.x, const1;\nMOV    result.color.xyz, state.fog.color;\nMUL\tresult.color.w, vReg0.x, state.fog.params.w;\n", 0x009fcec8 "!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\nPARAM mvpMatrix[4] = { state.matrix.mvp };\nPARAM boneArray[51] = {program.env[18..68] };\nATTRIB vWght = vertex.attrib[1];\nATTRIB vTex4 = vertex.attrib[12];\nATTRIB vTex5 = vertex.attrib[13];\nATTRIB vTex6 = vertex.attrib[14];\nADDRESS A0;\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\nTEMP vReg0, vReg1, vReg2, vReg3;\nTEMP vNormalWorld;\nALIAS vFinalColour = vReg9;\nMUL   vReg0, vertex.attrib[4], program.env[16].zzzz;\nARL   A0.x, vReg0.x;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nDP3   vReg5.x, vertex.normal, boneArray[A0.x + 0];\nDP3   vReg5.y, vertex.normal, boneArray[A0.x + 1];\nDP3   vReg5.z, vertex.normal, boneArray[A0.x + 2];\nARL   A0.x, vReg0.y;\nDP4   vReg2.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg2.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg2.z, vertex.position, boneArray[A0.x + 2];\nDP3   vReg6.x, vertex.normal, boneArray[A0.x + 0];\nDP3   vReg6.y, vertex.normal, boneArray[A0.x + 1];\nDP3   vReg6.z, vertex.normal, boneArray[A0.x + 2];\nARL   A0.x, vReg0.z;\nDP4   vReg3.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg3.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg3.z, vertex.position, boneArray[A0.x + 2];\nDP3   vReg7.x, vertex.normal, boneArray[A0.x + 0];\nDP3   vReg7.y, vertex.normal, boneArray[A0.x + 1];\nDP3   vReg7.z, vertex.normal, boneArray[A0.x + 2];\nARL   A0.x, vReg0.w;\nDP4   vReg4.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg4.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg4.z, vertex.position, boneArray[A0.x + 2];\nDP3   vReg8.x, vertex.normal, boneArray[A0.x + 0];\nDP3   vReg8.y, vertex.normal, boneArray[A0.x + 1];\nDP3   vReg8.z, vertex.normal, boneArray[A0.x + 2];\nMUL   vReg9, vWght.x, vReg1;\nMAD   vReg9, vReg2, vWght.y, vReg9;\nMAD   vReg9, vReg3, vWght.z, vReg9;\nMAD   vReg9, vReg4, vWght.w, vReg9;\nMOV   vReg9.w, program.env[16].w;\nDP4   result.position.x, mvpMatrix[0], vReg9;\nDP4   result.position.y, mvpMatrix[1], vReg9;\nDP4   result.position.z, mvpMatrix[2], vReg9;\nDP4   result.position.w, mvpMatrix[3], vReg9;\nMUL   vReg10, vWght.x, vReg5;\nMAD   vReg10, vReg6, vWght.y, vReg10;\nMAD   vReg10, vReg7, vWght.z, vReg10;\nMAD   vReg10, vReg8, vWght.w, vReg10;\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\nMOV   vNormalWorld.w, program.env[16].w;\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\nRSQ   vNormalWorld.w, vNormalWorld.w;\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\nDP4   vReg10.x, mvMatrix[0], vReg9;\nDP4   vReg10.y, mvMatrix[1], vReg9;\nDP4   vReg10.z, mvMatrix[2], vReg9;\nMOV   vReg10.w, mvMatrix[3].w;\nDP3\tvReg0.x, vReg10, vReg10;\nRSQ\tvReg0.x, vReg0.x;\nRCP\tvReg0.x, vReg0.x;\nMOV result.fogcoord.x, vReg0.x;\nADD  vReg0, program.env[12], -vReg9;\nDP3   vReg0.w, vReg0, vReg0;\nRSQ   vReg1.w, vReg0.w;\nDST vReg2, vReg0.w, vReg1.w;\nDP3 vReg2.w, vReg2, program.env[14];\nRCP vReg2.w, vReg2.w;\nMOV   result.color.secondary, vReg2.w;\nMUL   vReg0, vReg0, vReg1.w;\nMOV   vReg0.w, program.env[16].w;\nADD vReg2, program.env[13],-vReg9;\nDP3   vReg2.w, vReg2, vReg2;\nRSQ   vReg2.w, vReg2.w;\nMUL   vReg2, vReg2, vReg2.w;\nMOV   vReg2.w, program.env[16].w;\nADD vReg2, vReg2,vReg0;\nMOV   vReg2.w, program.env[16].w;\nMOV   result.texcoord[0], vertex.texcoord[0];\nMOV   result.texcoord[3], vertex.texcoord[0];\nDP3   vReg1.x, vTex4, vReg0;\nDP3   vReg1.y, vTex5, vReg0;\nDP3   vReg1.z, vTex6, vReg0;\nMOV   vReg1.w, program.env[16].w;\nMOV   result.texcoord[1], vReg1;\nDP3   vReg1.x, vTex4, vReg2;\nDP3   vReg1.y, vTex5, vReg2;\nDP3   vReg1.z, vTex6, vReg2;\nMOV   vReg1.w, program.env[16].w;\nMOV   result.texcoord[2], vReg1;\n", 0x009fdd78 "!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\nPARAM mvpMatrix[4] = { state.matrix.mvp };\nPARAM boneArray[51] = {program.env[18..68] };\nADDRESS A0;\nTEMP vReg4, vReg5, vReg6, vReg7, vReg8, vReg9, vReg10;\nTEMP vReg0, vReg1, vReg2, vReg3;\nTEMP vNormalWorld;\nALIAS vFinalColour = vReg9;\nATTRIB vWght = vertex.attrib[1];\nMUL   vReg0, vertex.attrib[4], program.env[16].z;\nARL   A0.x, vReg0.x;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nMUL   vReg9, vWght.x, vReg1;\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\nMUL   vReg4, vWght.x, vReg1;\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\nMUL   vReg5, vWght.x, vReg1;\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\nMUL   vReg6, vWght.x, vReg1;\nARL   A0.x, vReg0.y;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nMAD   vReg9, vReg1, vWght.y, vReg9;\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\nMAD   vReg4, vWght.y, vReg1, vReg4;\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\nMAD   vReg5, vWght.y, vReg1, vReg5;\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\nMAD   vReg6, vWght.y, vReg1, vReg6;\nARL   A0.x, vReg0.z;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nMAD   vReg9, vReg1, vWght.z, vReg9;\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\nMAD   vReg4, vWght.z, vReg1, vReg4;\nDP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\nMAD   vReg5, vWght.z, vReg1, vReg5;\nDP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\nMAD   vReg6, vWght.z, vReg1, vReg6;\nARL   A0.x, vReg0.w;\nDP4   vReg1.x, vertex.position, boneArray[A0.x + 0];\nDP4   vReg1.y, vertex.position, boneArray[A0.x + 1];\nDP4   vReg1.z, vertex.position, boneArray[A0.x + 2];\nMAD   vReg9, vReg1,vWght.w, vReg9;\nDP3   vReg1.x, vertex.attrib[12], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[12], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[12], boneArray[A0.x + 2];\nMAD   vReg4, vWght.w, vReg1, vReg4\n;DP3   vReg1.x, vertex.attrib[13], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[13], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[13], boneArray[A0.x + 2];\nMAD   vReg5, vWght.w, vReg1, vReg5\n;DP3   vReg1.x, vertex.attrib[14], boneArray[A0.x + 0];\nDP3   vReg1.y, vertex.attrib[14], boneArray[A0.x + 1];\nDP3   vReg1.z, vertex.attrib[14], boneArray[A0.x + 2];\nMAD   vReg10,vWght.w, vReg1, vReg6\n;MOV   vReg9.w, program.env[16].w;\nMOV   vReg10.w, program.env[16].w;\nDP4   result.position.x, mvpMatrix[0], vReg9;\nDP4   result.position.y, mvpMatrix[1], vReg9;\nDP4   result.position.z, mvpMatrix[2], vReg9;\nDP4   result.position.w, mvpMatrix[3], vReg9;\nDP3   vNormalWorld.x, mvMatrix[0], vReg10;\nDP3   vNormalWorld.y, mvMatrix[1], vReg10;\nDP3   vNormalWorld.z, mvMatrix[2], vReg10;\nMOV   vNormalWorld.w, program.env[16].w;\nDP3   vNormalWorld.w, vNormalWorld, vNormalWorld;\nRSQ   vNormalWorld.w, vNormalWorld.w;\nMUL   vNormalWorld, vNormalWorld, vNormalWorld.w;\nDP4   vReg10.x, mvMatrix[0], vReg9;\nDP4   vReg10.y, mvMatrix[1], vReg9;\nDP4   vReg10.z, mvMatrix[2], vReg9;\nMOV   vReg10.w, mvMatrix[3].w;\nADD  vReg0, program.env[12], -vReg9;\nDP3   vReg0.w, vReg0, vReg0;\nRSQ   vReg1.w, vReg0.w;\nDST vReg2, vReg0.w, vReg1.w;\nDP3 vReg2.w, vReg2, program.env[14];\nRCP vReg2.w, vReg2.w;\nMOV   result.color.secondary, vReg2.wwww;\nMUL   vReg0, vReg0, vReg1.w;\nMOV   vReg0.w, program.env[16].w;\nADD vReg2, program.env[13],-vReg9;\nDP3   vReg2.w, vReg2, vReg2;\nRSQ   vReg2.w, vReg2.w;\nMUL   vReg2, vReg2, vReg2.w;\nMOV   vReg2.w, program.env[16].w;\nADD vReg2, vReg2,vReg0;\nMOV   vReg2.w, program.env[16].w;\nMOV   result.texcoord[0], vertex.texcoord[0];\nMOV   result.texcoord[3], vertex.texcoord[0];\nDP3   vReg1.x, vReg4, vReg0;\nDP3   vReg1.y, vReg5, vReg0;\nDP3   vReg1.z, vReg6, vReg0;\nMOV   vReg1.w, program.env[16].w;\nMOV   result.texcoord[1], vReg1;\nDP3   vReg1.x, vReg4, vReg2;\nDP3   vReg1.y, vReg5, vReg2;\nDP3   vReg1.z, vReg6, vReg2;\nMOV   vReg1.w, program.env[16].w;\nMOV   result.texcoord[2], vReg1;\n", ...
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none
