# Review cards B17 chunk 11

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-11-01

GOG function: Global::RenderSkinned  gog 0x00877a20  callconv=(blank)  param_size_bytes=5  gog_size=4510  gog_retn=0
Candidate Steam address: 0x004c2000 (size 4510, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004c0dd0:STR0/CALLEE0/SIG0.282/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008767f0 Global::ExecuteSkinTransforms  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00878d00 Global::GetSkinVP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x00843be0 unnamed -> Steam ?  in candidate callees: n/a
  0x0085f070 unnamed -> Steam ?  in candidate callees: n/a
  0x00873f10 Global::GatherBones -> Steam ?  in candidate callees: n/a
  0x008767f0 Global::ExecuteSkinTransforms -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x3c*,0x1c0*,0x1cc*} | Steam {0x4*,0x8*,0xc*,0x3c*,0x1c0*,0x1cc*}
imm sets (* = in both, 11 common): GOG {0x2*,0x4*,0x8*,0x9*,0xc*,0x10*,0x14*,0x24*,0x28*,0x44*,0x2a4*} | Steam {0x2*,0x4*,0x8*,0x9*,0xc*,0x10*,0x14*,0x24*,0x28*,0x44*,0x2a4*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x004c2000: 16-byte aligned: YES
  004c2000  55             PUSH EBP
  004c2001  8B EC          MOV EBP,ESP
  004c2003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004c2000
string literals (0):
direct callees (12):
  0x00451800  (unnamed)
  0x0091D3BA  (unnamed)
  0x004CD4D0  (unnamed)
  0x00428F80  (unnamed)
  0x0040F9F0  (unnamed)
  0x004BE4F0  (unnamed)
  0x00463120  (unnamed)
  0x0046CE40  (unnamed)
  0x0046CDA0  (unnamed)
  0x0046CCF0  (unnamed)
  0x004CD510  (unnamed)
  0x004C0DD0  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x41  via EAX,ECX,EDX
  +0x8 [dec 8]  x41  via EAX,ECX,EDX
  +0xC [dec 12]  x19  via EAX,ECX,EDX
  +0x3C [dec 60]  x2  via ECX,EDX
  +0x1C0 [dec 448]  x1  via EDX
  +0x1CC [dec 460]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004c2000 FUN_004c2000 size=4510 ret=0 callers=2 callees=12

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void FUN_004c2000(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  float *pfVar3;
  undefined1 **ppuVar4;
  undefined1 *local_2e4 [3];
  float local_2d8;
  float local_2d4;
  undefined1 *local_2d0;
  undefined *local_2cc;
  float local_2c8;
  undefined1 local_294 [12];
  undefined1 local_288 [24];
  int local_270;
  float local_26c;
  float local_268;
  float local_264;
  float local_260;
  float *local_25c;
  float *local_258;
  float local_254;
  float local_250;
  float local_24c;
  float *local_248;
  float *local_244;
  float local_240;
  float local_23c;
  float local_238;
  float local_234;
  float local_230;
  float local_22c;
  float local_228;
  float local_224;
  float local_220;
  float local_21c;
  float local_218;
  float local_214;
  float *local_210;
  float *local_20c;
  float local_208;
  float local_204;
  float local_200;
  float local_1fc;
  float *local_1f8;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  float local_1e8;
  float local_1e4;
  float local_1e0;
  float local_1dc;
  float *local_1d8;
  float local_1d4;
  float local_1d0;
  float local_1cc;
  float local_1c8;
  float local_1c4;
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1b0;
  float local_1ac;
  float local_1a8;
  float *local_1a4;
  float local_1a0;
  float local_19c;
  float local_198;
  float local_194;
  float local_190;
  float local_18c;
  float local_188;
  float local_184;
  float local_180;
/* ... 398 more lines truncated by --max-lines */
```

### decoy: 0x004c0dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004c0dd0: 16-byte aligned: YES
  004c0dd0  55             PUSH EBP
  004c0dd1  8B EC          MOV EBP,ESP
  004c0dd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004c0dd0
string literals (0):
direct callees (21):
  0x00432EB0  (unnamed)
  0x00432F80  (unnamed)
  0x00432EE0  (unnamed)
  0x00452830  (unnamed)
  0x00514B90  (unnamed)
  0x00432F10  (unnamed)
  0x00484710  (unnamed)
  0x00435330  (unnamed)
  0x004352A0  (unnamed)
  0x00435180  (unnamed)
  0x004DB1F0  (unnamed)
  0x0049CF40  (unnamed)
  0x004DAE60  GUI image texture bind (MakeCurrent via vtbl +0xB8)
  0x004DB5D0  (unnamed)
  0x004A7590  (unnamed)
  0x004DB420  (unnamed)
  0x00454960  (unnamed)
  0x00484480  (unnamed)
  0x004844C0  (unnamed)
  0x00484750  (unnamed)
struct offsets touched (top 10):
  +0x8 [dec 8]  x19  via EAX,ECX,EDX
  +0x4 [dec 4]  x17  via EAX,ECX,EDX
  +0x14C [dec 332]  x12  via EAX,ECX,EDX
  +0x19C [dec 412]  x12  via EAX,ECX,EDX
  +0x180 [dec 384]  x8  via EAX,EDX
  +0x44 [dec 68]  x6  via EAX,ECX,EDX
  +0xAC [dec 172]  x6  via EAX
  +0x988530 [dec 9995568]  x6  via EAX,ECX,EDX
  +0x40 [dec 64]  x3  via ECX,EDX
  +0x9F4928 [dec 10438952]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 4510 B, RET 0
- callers 2: 0x004cd340, 0x004cd380
- callees: 0x00451800 (CALL), 0x0091d3ba (CALL), 0x004cd4d0 (CALL), 0x00428f80 (CALL), 0x0040f9f0 (CALL), 0x004be4f0 (CALL), ...
- twins: none

## Card B17-11-02

GOG function: Global::RenderSkinnedVPBlur  gog 0x0087f060  callconv=(blank)  param_size_bytes=4  gog_size=9743  gog_retn=0
Candidate Steam address: 0x004c9640 (size 9743, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '!!VP1.0 # This is the standard vertex pr'@00a0d558; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004cbc50:STR0/CALLEE0/SIG0.536/SRV0/VT0
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
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xc*,0x18*,0x2c*,0x3c*,0x40*,0x44*,0x50*,0x182*,0x1ac*,0x1b8*,0x1bc*,0x1c0*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x18*,0x2c*,0x3c*,0x40*,0x44*,0x50*,0x182*,0x1ac*,0x1b8*,0x1bc*,0x1c0*,...(+1)}
imm sets (* = in both, 25 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x9*,0xc*,0xd*,0xe*,0x10*,0x11*,0x12*,0x14*,0x22*,...(+11)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x9*,0xc*,0xd*,0xe*,0x10*,0x11*,0x12*,0x14*,0x22*,...(+11)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x004c9640: 16-byte aligned: YES
  004c9640  55             PUSH EBP
  004c9641  8B EC          MOV EBP,ESP
  004c9643  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004c9640 FUN_004c9640 size=9743 ret=0 callers=1 callees=22

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void FUN_004c9640(int *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 *puVar4;
  int iVar5;
  float *pfVar6;
  undefined4 *puVar7;
  float10 fVar8;
  float10 fVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 local_614;
  undefined1 local_420 [12];
  undefined1 local_414 [12];
  undefined1 local_408 [12];
  int local_3fc;
  float local_3f8;
  float local_3f4;
  float local_3f0;
  float local_3ec;
  float *local_3e8;
  float *local_3e4;
  float local_3e0;
  float local_3dc;
  float local_3d8;
  float *local_3d4;
  float *local_3d0;
  float local_3cc;
  float local_3c8;
  float local_3c4;
  float local_3c0;
  float local_3bc;
  float local_3b8;
  float local_3b4;
  float local_3b0;
  float local_3ac;
  float local_3a8;
  float local_3a4;
  float local_3a0;
  float *local_39c;
  float *local_398;
  float local_394;
  float local_390;
  float local_38c;
  float local_388;
  float *local_384;
  float local_380;
  float local_37c;
  float local_378;
  float local_374;
  float local_370;
  float local_36c;
  float local_368;
  float *local_364;
  float local_360;
  float local_35c;
  float local_358;
  float local_354;
  float local_350;
  float local_34c;
  float local_348;
  float local_344;
  float local_340;
  float local_33c;
  float local_338;
  float local_334;
  float *local_330;
  float local_32c;
  float local_328;
/* ... 650 more lines truncated by --max-lines */
```

### decoy: 0x004cbc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004cbc50: 16-byte aligned: YES
  004cbc50  55             PUSH EBP
  004cbc51  8B EC          MOV EBP,ESP
  004cbc53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 9743 B, RET 0
- callers 1: 0x004cd380
- callees: 0x004452b0 (CALL), 0x005142f0 (CALL), 0x0044e300 (CALL), 0x00451800 (CALL), 0x0091d3ba (CALL), 0x00428f80 (CALL), ...
- indirect: 0x004c967d slot 0x50, 0x004c96a9 slot 0x2c, 0x004c9717 slot 0x18, 0x004ca99a unresolved, 0x004caa2b unresolved, ...
- strings: 0x00a0d558 "!!VP1.0 # This is the standard vertex program.\nMUL   R0, v[COL1], c[16].zzzz;ARL   A0.x, R0.x;DP4   R1.x, v[OPOS], c[A0.x + 18];DP4   R1.y, v[OPOS], c[A0.x + 19];DP4   R1.z, v[OPOS], c[A0.x + 20];DP3   R5.x, v[NRML], c[A0.x + 18];DP3   R5.y, v[NRML], c[A0.x + 19];DP3   R5.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.y;DP4   R2.x, v[OPOS], c[A0.x + 18];DP4   R2.y, v[OPOS], c[A0.x + 19];DP4   R2.z, v[OPOS], c[A0.x + 20];DP3   R6.x, v[NRML], c[A0.x + 18];DP3   R6.y, v[NRML], c[A0.x + 19];DP3   R6.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.z;DP4   R3.x, v[OPOS], c[A0.x + 18];DP4   R3.y, v[OPOS], c[A0.x + 19];DP4   R3.z, v[OPOS], c[A0.x + 20];DP3   R7.x, v[NRML], c[A0.x + 18];DP3   R7.y, v[NRML], c[A0.x + 19];DP3   R7.z, v[NRML], c[A0.x + 20];ARL   A0.x, R0.w;DP4   R4.x, v[OPOS], c[A0.x + 18];DP4   R4.y, v[OPOS], c[A0.x + 19];DP4   R4.z, v[OPOS], c[A0.x + 20];DP3   R8.x, v[NRML], c[A0.x + 18];DP3   R8.y, v[NRML], c[A0.x + 19];DP3   R8.z, v[NRML], c[A0.x + 20];MUL   R9, v[WGHT].x, R1;MAD   R9, R2, v[WGHT].y, R9;MAD   R9, R3, v[WGHT].z, R9;MAD   R9, R4, v[WGHT].w, R9;MOV   R9.w, c[16].w;MUL   R10, v[WGHT].x, R5;MAD   R10, R6, v[WGHT].y, R10;MAD   R10, R7, v[WGHT].z, R10;MAD   R10, R8, v[WGHT].w, R10;DP3   R10.w, R10, R10;RSQ   R10.w, R10.w;MUL   R10,   R10, R10.w;MOV   R10.w, c[16].x;MOV  R4, c[12];MOV   R7.x, c[14].x;MIN   R2.y, R7.x, c[14].y;MUL   R5.xyz, R4, R2.y;MOV   R5.w, c[16].x;DP3   R3.w, R4, R10;SLT   R2.w, R3.w, c[16].x;MOV   R1, c[17].xyzw;MOV   R1.w, c[16].x;MAD   R6, R2.w, -R5, R9.xyzw;MUL   R7.y, R2.y, c[13].x;RCP   R8.y, c[14].y;MUL   R8.w, R2.y, R8.y;MUL   R7.y, R7.y, R8.w;MUL   R7.y, R7.y, c[17].w;MOV   R3.w,   c[16].x;MOV   R3.xyz, c[69];DP3   R3.w,   R3, R3;RSQ   R5.w,   R3.w;MUL   R3,     R3, R5.w;ADD   R5.xyz, R6, -R1;DP3   R5.w, R5, R3;SLT   R5.x, R5.w, c[16].x;MOV   R7.x, c[16].w;MAD   R5.y, R5.x, -c[16].y, R7.x;MUL   R5.z, R5.y, R7.y;MUL   R5.z, R5.z, R2.w;MAD   R6, R5.z, R3, R6;ADD   R2.x, -R2.w, c[16].w;MUL   R1, R10, -c[14].z;MOV   R1.w, c[16].x;MAD   R5, R2.x, R1, R6;DP4   R1.x, R5, c[0];DP4   R1.y, R5, c[1];DP4   R1.z, R5, c[2];DP4   R1.w, R5, c[3];MOV   o[HPOS].xyzw, R1.xyzw;ADD   R2.w, c[16].w, -R2.w;MOV   R9.xyz, c[16].xxxx;MUL   R9.w, c[69].w, R2.w;MOV   o[COL0], R9;DP3   R11.x, c[8],  R10;DP3   R11.y, c[9],  R10;DP3   R11.z, c[10], R10;MOV   R11.w, c[16].w;DP4   R10.x, c[8],  R5;DP4   R10.y, c[9],  R5;DP4   R10.z, c[10], R5;MOV   R10.w, c[16].w;DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV   o[FOGC].x, -R1.w;MOV   o[TEX0], v[TEX0];"
- imports: glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), ...
- twins: none

## Card B17-11-03

GOG function: Global::RenderSkinnedVP  gog 0x00878e50  callconv=(blank)  param_size_bytes=4  gog_size=25096  gog_retn=0
Candidate Steam address: 0x004c3430 (size 25096, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 7 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '!!ARBvp1.0\nPARAM const0 = 0.0;\nPARAM c'@009faed8; '!!ARBvp1.0PARAM mvMatrix[4] = { state.ma'@009fcec8; '!!ARBvp1.0PARAM mvMatrix[4] = { state.ma'@009fdd78; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004c9640:STR0/CALLEE0/SIG0.343/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00878da0 Global::GetSkinCubeMapVP  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0087f060 Global::RenderSkinnedVPBlur -> 0x004c9640  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (35), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x005b2560 CSWGuiBarkBubble::ShouldFreeMouseOnAdd -> Steam ?  in candidate callees: n/a
  0x0080c7d0 unnamed -> Steam ?  in candidate callees: n/a
  0x00843be0 unnamed -> Steam ?  in candidate callees: n/a
  ... 27 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (10): '!!ARBvp1.0\\nPARAM mvMatrix[4] = { state.'; '!!ARBvp1.0\\nPARAM const0 = 0.0;\\nPARAM c'; '!!ARBvp1.0PARAM mvMatrix[4] = { state.ma'; '!!ARBvp1.0PARAM mvMatrix[4] = { state.ma' ...
disp sets (* = in both, 34 common): GOG {0x4*,0x8*,0xc*,0x10*,0x13*,0x14*,0x18*,0x2c*,0x2d*,0x30*,0x3c*,0x40*,0x44*,0x50*,...(+20)} | Steam {0x4*,0x8*,0xc*,0x10*,0x13*,0x14*,0x18*,0x2c*,0x2d*,0x30*,0x3c*,0x40*,0x44*,0x50*,...(+20)}
imm sets (* = in both, 56 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+42)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+42)}

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

## Card B17-11-04

GOG function: Global::SceneBSPAddGob  gog 0x0086d830  callconv=(blank)  param_size_bytes=8  gog_size=135  gog_retn=0
Candidate Steam address: 0x00495f20 (size 135, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ChildAdd 0x465bb0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00495fb0:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086d690 Global::SceneBSPRemoveTiledata  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086d8c0 Global::SceneBSPRemoveGob -> 0x00495fb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
  0x00945180 Global::BSPFitSphere -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00857f90 Scene::ChildAdd -> Steam 0x00465bb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x58*,0x80*,0x84*,0xac*} | Steam {0x58*,0x80*,0x84*,0xac*}
imm sets (* = in both, 2 common): GOG {0xc*,0xa4*} | Steam {0xc*,0xa4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00495f20: 16-byte aligned: YES
  00495f20  55             PUSH EBP
  00495f21  8B EC          MOV EBP,ESP
  00495f23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00495f20
string literals (0):
direct callees (3):
  0x0051A030  (unnamed)
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 4):
  +0x80 [dec 128]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00495f20 FUN_00495f20 size=135 ret=0 callers=2 callees=3

void FUN_00495f20(int param_1,int param_2)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0xac) != 0) {
    FUN_0051a030(*(undefined4 *)(param_1 + 0xac),param_2 + 0xa4,
                 *(undefined4 *)(*(int *)(param_2 + 0x84) + 0x80),
                 *(undefined4 *)(*(int *)(param_2 + 0x84) + 0x80));
    iVar1 = FUN_00476620(param_2);
    if (iVar1 == 0) {
      FUN_00473f40(param_2);
    }
  }
  return;
}
```

### decoy: 0x00495fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00495fb0: 16-byte aligned: YES
  00495fb0  55             PUSH EBP
  00495fb1  8B EC          MOV EBP,ESP
  00495fb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00495fb0
string literals (0):
direct callees (3):
  0x0051A2B0  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### Static facts (script output, report-only)
- size 135 B, RET 0
- callers 2: 0x00465bb0 Scene::ChildAdd, 0x00499610
- callees: 0x0051a030 (CALL), 0x00476620 (CALL), 0x00473f40 (CALL)
- twins: none

## Card B17-11-05

GOG function: CSWGuiFeatsCharGen::RemoveChosenFeat  gog 0x005fae20  callconv=(blank)  param_size_bytes=2  gog_size=395  gog_retn=4
Candidate Steam address: 0x0090c120 (size 271, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.43) | G8 mac_rotation: rotation:0x005fafb0:0.57>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.60 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0090bf90:STR0/CALLEE0/SIG1/SRV0/VT0
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
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1bcc*,0x1bd0*,0x1bde*} | Steam {0x68,0x310,0x1bcc*,0x1bd0*,0x1bde*}
imm sets (* = in both, 3 common): GOG {0x4*,0x30,0x1bc0*,0x1bcc*} | Steam {0x4*,0x18,0x1bc0*,0x1bcc*}

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

## Card B17-11-06

GOG function: PartEmitter::RenderBoundingSphere  gog 0x008b3c00  callconv=(blank)  param_size_bytes=0  gog_size=7939  gog_retn=0
Candidate Steam address: 0x004f8000 (size 7939, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004f5f50:STR0/CALLEE0/SIG0.862/SRV0/VT0
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
disp sets (* = in both, 29 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+15)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+15)}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0x60*,0xb4*,0x11c*,0x17c*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0x60*,0xb4*,0x11c*,0x17c*,...(+4)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f8000: 16-byte aligned: YES
  004f8000  55             PUSH EBP
  004f8001  8B EC          MOV EBP,ESP
  004f8003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004f8000 FUN_004f8000 size=7939 ret=0 callers=1 callees=24

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8000(int param_1)

{
  float fVar1;
  float fVar2;
  uint uVar3;
  undefined4 *puVar4;
  float *pfVar5;
  int iVar6;
  float10 fVar7;
  int local_51c;
  int local_50c;
  int local_4fc;
  uint local_4ec;
  int local_4dc;
  int local_4cc;
  int local_4bc;
  uint local_4ac;
  float local_484;
  float local_480;
  float local_47c;
  float local_478;
  float local_474;
  float local_470;
  float local_46c;
  float local_468;
  float local_464;
  float local_460;
  float local_45c;
  float local_458;
  float local_454;
  float local_450;
  float local_44c;
  float local_448;
  int local_42c;
  int local_428;
  double local_424;
  undefined1 local_154 [12];
  undefined1 local_148 [12];
  undefined1 local_13c [12];
  undefined1 local_130 [12];
  undefined1 local_124 [12];
  int local_118;
  int local_114;
  int local_110;
  float local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  uint local_e4;
  float local_e0;
  float local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
/* ... 491 more lines truncated by --max-lines */
```

### decoy: 0x004f5f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f5f50: 16-byte aligned: YES
  004f5f50  55             PUSH EBP
  004f5f51  8B EC          MOV EBP,ESP
  004f5f53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 7939 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x00919723 operator new (CALL), 0x0046cda0 (CALL), 0x0046ce40 (CALL), 0x004f39b0 (CALL), 0x004f5d90 (CALL), 0x004dae60 (CALL), ...
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B17-11-07

GOG function: CPathfindInformation::Initialize  gog 0x006470b0  callconv=(blank)  param_size_bytes=0  gog_size=1447  gog_retn=0
Candidate Steam address: 0x006f99f0 (size 664, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CPathfindInformation::Shutdown | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.13 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006f9440:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00647090 CPathfindInformation::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00647660 CPathfindInformation::Shutdown  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4,0x8,0xc,0x10,0x14,0x1c,0x20,0x24,0x28,0x2c,0x30,0x38,0x40,0x44,...(+83)} | Steam {0x90*,0x98*,0xac*,0xb0*,0xb8*,0xc4*,0xc8*,0xd0*,0xd4*,0xe8*,0xec*,0xf0*,0x16c*,0x1ac*,...(+1)}
imm sets (* = in both, 1 common): GOG {0x4*,0x44,0x108,0x114,0x170,0x17c,0x258,0x2bc,0x7f000000} | Steam {0x4*,0x50}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006f99f0: 16-byte aligned: YES
  006f99f0  55             PUSH EBP
  006f99f1  8B EC          MOV EBP,ESP
  006f99f3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f99f0
string literals (0):
direct callees (5):
  0x009196FD  operator delete
  0x006FA2D0  (unnamed)
  0x0054F990  (unnamed)
  0x0054FA10  (unnamed)
  0x0054FA90  (unnamed)
struct offsets touched (top 10):
  +0x1AC [dec 428]  x4  via EAX,ECX,EDX
  +0xE8 [dec 232]  x3  via ECX,EDX
  +0xEC [dec 236]  x3  via EAX,ECX
  +0x16C [dec 364]  x3  via EAX,EDX
  +0x238 [dec 568]  x3  via EAX,ECX
  +0x90 [dec 144]  x2  via EAX,ECX
  +0x98 [dec 152]  x2  via ECX,EDX
  +0xAC [dec 172]  x2  via EAX,EDX
  +0xB0 [dec 176]  x2  via EAX,ECX
  +0xB8 [dec 184]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006f99f0 FUN_006f99f0 size=664 ret=0 callers=1 callees=5

void __fastcall FUN_006f99f0(int param_1)

{
  if (*(int *)(param_1 + 0x90) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x90));
  }
  if (*(int *)(param_1 + 0x98) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x98));
  }
  if (*(int *)(param_1 + 0xac) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xac));
  }
  if (*(int *)(param_1 + 0xb0) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xb0));
  }
  if (*(int *)(param_1 + 0xb8) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xb8));
  }
  if (*(int *)(param_1 + 0xc4) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xc4));
  }
  if (*(int *)(param_1 + 0xc8 [dec 200]) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xc8 [dec 200]));
  }
  if (*(int *)(param_1 + 0xd0) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xd0));
  }
  if (*(int *)(param_1 + 0xd4) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xd4));
  }
  if (*(int *)(param_1 + 0xf0) != 0) {
    FUN_006fa2d0();
  }
  if (*(int *)(param_1 + 0xe8) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xe8));
    *(undefined4 *)(param_1 + 0xe8) = 0;
  }
  if (*(int *)(param_1 + 0xec) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0xec));
    *(undefined4 *)(param_1 + 0xec) = 0;
  }
  if (*(int *)(param_1 + 0x16c) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x16c));
    *(undefined4 *)(param_1 + 0x16c) = 0;
  }
  if (*(int *)(param_1 + 0x238) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x238));
    *(undefined4 *)(param_1 + 0x238) = 0;
  }
  if (*(int *)(param_1 + 0x1ac) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x1ac));
    *(undefined4 *)(param_1 + 0x1ac) = 0;
  }
  *(undefined4 *)(param_1 + 0x1ac) = 0;
  FUN_0054f990(0);
  FUN_0054fa10(0);
  FUN_0054fa90(0);
  return;
}
```

### decoy: 0x006f9440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f9440: 16-byte aligned: YES
  006f9440  55             PUSH EBP
  006f9441  8B EC          MOV EBP,ESP
  006f9443  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f9440
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x194 [dec 404]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
```

### Static facts (script output, report-only)
- size 664 B, RET 0
- callers 1: 0x006f9420
- callees: 0x009196fd operator delete (CALL), 0x006fa2d0 (CALL), 0x0054f990 (CALL), 0x0054fa10 (CALL), 0x0054fa90 (CALL)
- twins: none

## Card B17-11-08

GOG function: OOAnimRun::Destructor  gog 0x008f7090  callconv=(blank)  param_size_bytes=0  gog_size=75  gog_retn=0
Candidate Steam address: 0x004dcd10 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004dcd30:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f6fc0 OOAnimRun::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f70e0 Global::FindAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x28,0x2c} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0xc} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004dcd10: 16-byte aligned: YES
  004dcd10  55             PUSH EBP
  004dcd11  8B EC          MOV EBP,ESP
  004dcd13  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dcd10
string literals (0):
direct callees (1):
  0x0048DD90  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004dcd10 FUN_004dcd10 size=17 ret=0 callers=0 callees=1
THUNK -> 0x0048DD90 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_004dcd10(undefined4 param_1,undefined4 param_2)

{
  FUN_0048dd90(param_2);
  return;
}
```

### decoy: 0x004dcd30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dcd30: 16-byte aligned: YES
  004dcd30  55             PUSH EBP
  004dcd31  8B EC          MOV EBP,ESP
  004dcd33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004dcd30
string literals (0):
direct callees (2):
  0x00451F20  (unnamed)
  0x004E3B90  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via ECX
  +0x54 [dec 84]  x1  via EDX
  +0x58 [dec 88]  x1  via EAX
  +0x84 [dec 132]  x1  via ECX
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callees: 0x0048dd90 (CALL)
- twins: none

## Card B17-11-09

GOG function: Global::BinaryInsertModel  gog 0x008ce5e0  callconv=(blank)  param_size_bytes=12  gog_size=148  gog_retn=0
Candidate Steam address: 0x0048e060 (size 48, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048dfc0:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
prologue for 0x0048e060: 16-byte aligned: YES
  0048e060  55             PUSH EBP
  0048e061  8B EC          MOV EBP,ESP
  0048e063  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0048e060
string literals (0):
direct callees (1):
  0x0048DFC0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0048e060 FUN_0048e060 size=48 ret=0 callers=2 callees=1
THUNK -> 0x0048DFC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0048e060(undefined4 param_1)

{
  FUN_0048dfc0(param_1,0,DAT_00a1ffac + -1);
  return;
}
```

### decoy: 0x0048dfc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0048dfc0: 16-byte aligned: YES
  0048dfc0  55             PUSH EBP
  0048dfc1  8B EC          MOV EBP,ESP
  0048dfc3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0048dfc0
string literals (0):
direct callees (2):
  0x009370C7  _stricmp
  0x0048DFC0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 48 B, RET 0
- callers 2: 0x0048e160, 0x0093d000
- callees: 0x0048dfc0 (CALL)
- twins: 0x0048e130 (distinguishing: 0x0048e084: CALL 0x0048dfc0)

## Card B17-11-10

GOG function: PartSkin::Draw  gog 0x00882da0  callconv=__thiscall  param_size_bytes=1  gog_size=229  gog_retn=4
Candidate Steam address: 0x004cd380 (size 229, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004cd340:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00882d60 PartSkin::DrawBumpedOutShell  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00882ef0 List_SkinVertexWeight_::Add  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0086b250 Global::AurNV2xLevelShadersAvailable -> Steam ?  in candidate callees: n/a
  0x00877a20 Global::RenderSkinned -> Steam ?  in candidate callees: n/a
  0x00878e50 Global::RenderSkinnedVP -> Steam ?  in candidate callees: n/a
  0x0087f060 Global::RenderSkinnedVPBlur -> Steam ?  in candidate callees: n/a
  0x00882d60 PartSkin::DrawBumpedOutShell -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0x40*,0x70*,0x148*} | Steam {0x8*,0x40*,0x70*,0x148*}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x8*,0x44*} | Steam {0x2*,0x4*,0x8*,0x44*}

### abi
```
GOG param bytes: 1; callconv: thiscall
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004cd380: 16-byte aligned: YES
  004cd380  55             PUSH EBP
  004cd381  8B EC          MOV EBP,ESP
  004cd383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004cd380
string literals (0):
direct callees (5):
  0x004844C0  (unnamed)
  0x004C3430  (unnamed)
  0x004C9640  (unnamed)
  0x004C2000  (unnamed)
  0x004CD340  (unnamed)
struct offsets touched (top 5):
  +0x70 [dec 112]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EDX
  +0x40 [dec 64]  x2  via ECX,EDX
  +0x148 [dec 328]  x1  via EAX
  +0x988658 [dec 9995864]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004cd380 FUN_004cd380 size=229 ret=4 callers=0 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004cd380(int param_1,char param_2)

{
  char cVar1;
  undefined4 uVar2;
  
  if (DAT_00a04924 != 0) {
    if (((DAT_00a04928 == 0) || (cVar1 = FUN_004844c0(), cVar1 == '\0')) || (DAT_00a20020 == 0)) {
      glDisableClientState(DAT_00988660);
      uVar2 = (**(code **)(**(int **)(param_1 + 0x70) + 8))(0);
      FUN_004c2000(uVar2);
    }
    else if (param_2 == '\0') {
      uVar2 = (**(code **)(**(int **)(param_1 + 0x70) + 8))();
      FUN_004c3430(uVar2);
    }
    else {
      uVar2 = (**(code **)(**(int **)(param_1 + 0x70) + 8))();
      FUN_004c9640(uVar2);
    }
    if (((*(int *)(param_1 + 0x40) != 0) &&
        (*(float *)(*(int *)(param_1 + 0x40) + 0x148) != (float)_DAT_00986b38)) &&
       (DAT_00a20020 != 0)) {
      FUN_004cd340();
    }
  }
  return;
}
```

### decoy: 0x004cd340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004cd340: 16-byte aligned: YES
  004cd340  55             PUSH EBP
  004cd341  8B EC          MOV EBP,ESP
  004cd343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004cd340
string literals (0):
direct callees (3):
  0x004844C0  (unnamed)
  0x004CBC50  (unnamed)
  0x004C2000  (unnamed)
struct offsets touched (top 2):
  +0x70 [dec 112]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
```

### Static facts (script output, report-only)
- size 229 B, RET 4
- callees: 0x004844c0 (CALL), 0x004c3430 (CALL), 0x004c9640 (CALL), 0x00000034 (CALL), 0x004c2000 (CALL), 0x004cd340 (CALL)
- indirect: 0x004cd3cc slot 0x8, 0x004cd3ea slot 0x8, 0x004cd421 slot 0x8
- imports: glDisableClientState (OPENGL32.DLL)
- twins: none

## Card B17-11-11

GOG function: FountainEmitter::Update  gog 0x008b6860  callconv=(blank)  param_size_bytes=4  gog_size=1598  gog_retn=4
Candidate Steam address: 0x004fac00 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004fac30:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x14,0x40,0x54,0x58,0x88,0x90,0xb4,0xf0,0xf4,0x114,0x1c8,...(+5)} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x5,0x8,0xc,0x14,0x41,0x44,0x60,0x74,0x78,0xc4,0x1c8,0x1d4} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004fac00: 16-byte aligned: YES
  004fac00  55             PUSH EBP
  004fac01  8B EC          MOV EBP,ESP
  004fac03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004fac00
string literals (0):
direct callees (1):
  0x004E8A30  PartEmitter::ctor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004fac00 FUN_004fac00 size=37 ret=4 callers=1 callees=1

undefined4 * __thiscall FUN_004fac00(undefined4 *param_1,undefined4 param_2)

{
  FUN_004e8a30(param_2);
  *param_1 = ExplosionEmitter::vftable;
  return param_1;
}
```

### decoy: 0x004fac30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004fac30: 16-byte aligned: YES
  004fac30  55             PUSH EBP
  004fac31  8B EC          MOV EBP,ESP
  004fac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 37 B, RET 4
- callers 1: 0x004e54d0
- callees: 0x004e8a30 (CALL)
- twins: none

## Card B17-11-12

GOG function: Global::PartOutside_2  gog 0x0086bb10  callconv=(blank)  param_size_bytes=8  gog_size=162  gog_retn=0
Candidate Steam address: 0x00494200 (size 162, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00494170:STR0/CALLEE0/SIG0.381/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ba80 Global::PartOutside  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086bbc0 Global::PartOutside_3  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080be00 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0xc*,0xd0*} | Steam {0x4*,0xc*,0xd0*}
imm sets (* = in both, 4 common): GOG {0x4*,0xc*,0x24*,0x41*} | Steam {0x4*,0xc*,0x24*,0x41*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00494200: 16-byte aligned: YES
  00494200  55             PUSH EBP
  00494201  8B EC          MOV EBP,ESP
  00494203  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00494200
string literals (0):
direct callees (1):
  0x00514050  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via EAX
  +0xD0 [dec 208]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00494200 FUN_00494200 size=162 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00494200(int *param_1,int *param_2)

{
  float10 fVar1;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_20 = 0;
  local_1c = 0;
  local_18 = 0;
  local_8 = 0;
  local_c = 0;
  local_10 = 0;
  local_14 = 0x3f800000;
  (**(code **)(*param_1 + 0xc))(&local_20,&local_14);
  local_24 = 0;
  while( true ) {
    if (param_2[1 = byte +0x4] <= local_24) {
      return 0;
    }
    fVar1 = (float10)FUN_00514050(&local_20,param_1[0x34 = byte +0xd0],local_24 * 0x10 + *param_2);
    if ((float10)_DAT_00986b38 < fVar1) break;
    local_24 = local_24 + 1;
  }
  return 1;
}
```

### decoy: 0x00494170 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00494170: 16-byte aligned: YES
  00494170  55             PUSH EBP
  00494171  8B EC          MOV EBP,ESP
  00494173  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00494170
string literals (0):
direct callees (1):
  0x00513FA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via ECX
```

### Static facts (script output, report-only)
- size 162 B, RET 0
- callers 1: 0x00499eb0
- callees: 0x00514050 (CALL)
- indirect: 0x0049423c slot 0xc
- twins: none

## Card B17-11-13

GOG function: Global::CollectBins  gog 0x0086ffc0  callconv=(blank)  param_size_bytes=12  gog_size=291  gog_retn=0
Candidate Steam address: 0x004986b0 (size 291, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0075a630:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086fef0 Global::UpdateStaticLighting  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00870230 Global::AurProcessActiveBins  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
  0x00865e90 List_T__::Add -> Steam ?  in candidate callees: n/a
  0x0089d600 CAurTriangleBin::SetMinDistanceSquared -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x58*,0x60*,0x64*} | Steam {0x4*,0x8*,0x58*,0x60*,0x64*}
imm sets (* = in both, 6 common): GOG {0x4*,0x5*,0xc*,0x2c*,0x41*,0x4c*} | Steam {0x4*,0x5*,0xc*,0x2c*,0x41*,0x4c*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x004986b0: 16-byte aligned: YES
  004986b0  55             PUSH EBP
  004986b1  8B EC          MOV EBP,ESP
  004986b3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004986b0
string literals (0):
direct callees (4):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x0050E7B0  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x58 [dec 88]  x1  via EAX
  +0x60 [dec 96]  x1  via EAX
  +0x64 [dec 100]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004986b0 FUN_004986b0 size=291 ret=0 callers=0 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004986b0(int param_1,undefined4 param_2,float param_3)

{
  undefined4 *puVar1;
  float10 fVar2;
  undefined1 local_44 [12];
  float local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_15;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = *(int *)(param_1 + 0x58);
  local_14 = *(int *)(local_c + 0x64 [dec 100]);
  for (local_10 = 0; local_10 < local_14; local_10 = local_10 + 1) {
    local_8 = *(int *)(*(int *)(local_c + 0x60) + local_10 * 4);
    local_34 = 0;
    local_30 = 0;
    local_2c = 0;
    local_24 = 0;
    local_20 = 0;
    local_1c = 0;
    local_15 = 0;
    local_38 = param_3;
    local_28 = 0.0;
    puVar1 = (undefined4 *)FUN_004452b0(local_44,param_2,local_8 + 0x2c);
    local_24 = *puVar1;
    local_20 = puVar1[1 = byte +0x4];
    local_1c = puVar1[2 = byte +0x8];
    fVar2 = (float10)FUN_005142f0(&local_24);
    local_28 = (float)(fVar2 - (float10)*(float *)(local_8 + 8));
    if (local_28 < (float)_DAT_00986b38) {
      local_28 = 0.0;
    }
    local_28 = local_28 * local_28;
    if (local_28 < local_38) {
      DAT_00a73254 = DAT_00a73254 + 1;
      local_38 = local_28;
      FUN_004491c0(local_8);
      FUN_0050e7b0(local_38);
    }
  }
  return;
}
```

### decoy: 0x0075a630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0075a630: 16-byte aligned: YES
  0075a630  55             PUSH EBP
  0075a631  8B EC          MOV EBP,ESP
  0075a633  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0075a630
string literals (0):
direct callees (2):
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x0073F750  CClientExoApp::GetInGameGui
struct offsets touched (top 4):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xFC8 [dec 4040]  x1  via EAX
```

### Static facts (script output, report-only)
- size 291 B, RET 0
- callees: 0x004452b0 (CALL), 0x005142f0 (CALL), 0x004491c0 (CALL), 0x0050e7b0 (CALL)
- twins: none

## Card B17-11-14

GOG function: OOAnimRun::Destructor  gog 0x008f7090  callconv=(blank)  param_size_bytes=0  gog_size=75  gog_retn=0
Candidate Steam address: 0x004dcf50 (size 298, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004dcf00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008f6fc0 OOAnimRun::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008f70e0 Global::FindAnimation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x28,0x2c} | Steam {0x58,0x5c,0x64}
imm sets (* = in both, 0 common): GOG {0x4,0xc} | Steam {0x8,0x18}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004dcf50: 16-byte aligned: YES
  004dcf50  55             PUSH EBP
  004dcf51  8B EC          MOV EBP,ESP
  004dcf53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004dcf50
string literals (1):
  0x00987564  at 004dcfe6  'default'
direct callees (3):
  0x009370C7  _stricmp
  0x004DCF50  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x58 [dec 88]  x4  via EAX,ECX,EDX
  +0x64 [dec 100]  x4  via EAX,ECX,EDX
  +0x5C [dec 92]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004dcf50 FUN_004dcf50 size=298 ret=0 callers=4 callees=3

void FUN_004dcf50(int param_1,char *param_2)

{
  int iVar1;
  char local_14 [4];
  char local_10 [4];
  uint local_c;
  int local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (param_2 != (char *)0x0) {
    for (local_8 = 0; local_8 < *(int *)(param_1 + 0x5c); local_8 = local_8 + 1) {
      iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(*(int *)(param_1 + 0x58) + local_8 * 4) + 8),param_2);
      if (iVar1 == 0) goto LAB_004dd06c;
    }
    if (*(int *)(param_1 + 0x64 [dec 100]) == 0) {
      local_14[0] = s_default_00987564[0];
      local_14[1] = s_default_00987564[1];
      local_14[2] = s_default_00987564[2];
      local_14[3] = s_default_00987564[3];
      local_10[0] = s_default_00987564[4];
      local_10[1] = s_default_00987564[5];
      local_10[2] = s_default_00987564[6];
      local_10[3] = s_default_00987564[7];
      for (local_8 = 0; local_8 < *(int *)(param_1 + 0x5c); local_8 = local_8 + 1) {
        iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(*(int *)(param_1 + 0x58) + local_8 * 4) + 8),local_14);
        if (iVar1 == 0) goto LAB_004dd06c;
      }
      if (*(int *)(param_1 + 0x64 [dec 100]) != 0) {
        FUN_004dcf50(*(undefined4 *)(param_1 + 0x64 [dec 100]),local_14);
      }
    }
    else {
      FUN_004dcf50(*(undefined4 *)(param_1 + 0x64 [dec 100]),param_2);
    }
  }
LAB_004dd06c:
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x004dcf00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004dcf00: 16-byte aligned: YES
  004dcf00  55             PUSH EBP
  004dcf01  8B EC          MOV EBP,ESP
  004dcf03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004dcf00
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x28 [dec 40]  x2  via EAX
  +0x2C [dec 44]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 298 B, RET 0
- callers 4: 0x004dc920, 0x004dcf50, 0x004e3270, 0x004e3a20
- callees: 0x009370c7 _stricmp (CALL), 0x004dcf50 (CALL), 0x009196ee __security_check_cookie (CALL)
- strings: 0x00987564 "default"
- twins: none

## Card B17-11-15

GOG function: CPathfindInformation::Initialize  gog 0x006470b0  callconv=(blank)  param_size_bytes=0  gog_size=1447  gog_retn=0
Candidate Steam address: 0x006f9420 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006f9440:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00647090 CPathfindInformation::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00647660 CPathfindInformation::Shutdown  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x1c,0x20,0x24,0x28,0x2c,0x30,0x38,0x40,0x44,...(+83)} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0x44,0x108,0x114,0x170,0x17c,0x258,0x2bc,0x7f000000} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x006f9420: 16-byte aligned: YES
  006f9420  55             PUSH EBP
  006f9421  8B EC          MOV EBP,ESP
  006f9423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006f9420
string literals (0):
direct callees (1):
  0x006F99F0  CPathfindInformation::Shutdown (leaks +0x18C/+0x1B4/+0x1D8/+0x200)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006f9420 FUN_006f9420 size=19 ret=0 callers=2 callees=1
THUNK -> 0x006F99F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_006f9420(undefined4 param_1)

{
  FUN_006f99f0(param_1);
  return;
}
```

### decoy: 0x006f9440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006f9440: 16-byte aligned: YES
  006f9440  55             PUSH EBP
  006f9441  8B EC          MOV EBP,ESP
  006f9443  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006f9440
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x194 [dec 404]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
```

### Static facts (script output, report-only)
- size 19 B, RET 0
- callers 2: 0x005219d0 CSWSArea::Destructor_2, 0x005634c0 CSWSCreature::Destructor_2
- callees: 0x006f99f0 (CALL)
- twins: none

## Card B17-11-16

GOG function: PartSkin::PostProcess  gog 0x00874400  callconv=(blank)  param_size_bytes=0  gog_size=8182  gog_retn=4
Candidate Steam address: 0x004be9e0 (size 8182, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.98) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004c09e0:STR0/CALLEE0/SIG0.28/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x008742f0 Global::FindPartIndexDFS  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00876400 MdlNodeSkin::InternalGenVertices  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (28), mapped to Steam through trusted rows only:
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
  0x0081b9c0 unnamed -> Steam ?  in candidate callees: n/a
  0x008255f0 GLRender::LockPool -> Steam ?  in candidate callees: n/a
  0x00825620 GLRender::UnlockPool -> Steam ?  in candidate callees: n/a
  0x00825690 GLRender::ResizePool -> Steam ?  in candidate callees: n/a
  0x0083d9c0 MdlNodeTriMesh::BuildNeighborLists -> Steam ?  in candidate callees: n/a
  0x0083ed70 MdlNodeTriMesh::BuildVertexIndices -> Steam ?  in candidate callees: n/a
  ... 20 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 44 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x3c*,...(+30)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x3c*,...(+30)}
imm sets (* = in both, 22 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x14*,0x20*,0x24*,0x28*,0x34*,0x44*,0x4c*,...(+8)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x14*,0x20*,0x24*,0x28*,0x34*,0x44*,0x4c*,...(+8)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x004be9e0: 16-byte aligned: YES
  004be9e0  55             PUSH EBP
  004be9e1  8B EC          MOV EBP,ESP
  004be9e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004be9e0
string literals (0):
direct callees (28):
  0x00463ED0  (unnamed)
  0x00451800  (unnamed)
  0x00428F80  (unnamed)
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00463B20  (unnamed)
  0x004BE4F0  (unnamed)
  0x00429330  (unnamed)
  0x00451840  (unnamed)
  0x004BE8D0  (unnamed)
  0x009196FD  operator delete
  0x0044C940  (unnamed)
  0x0044B4F0  (unnamed)
  0x00919723  operator new
  0x004D9FF0  Material::Constructor
  0x004DA820  (unnamed)
  0x004DB420  (unnamed)
  0x004DA1B0  (unnamed)
  0x00452830  (unnamed)
  0x00432F80  (unnamed)
  0x00432EE0  (unnamed)
struct offsets touched (top 10):
  +0x14C [dec 332]  x38  via EAX,ECX,EDX
  +0x8 [dec 8]  x30  via EAX,ECX,EDX
  +0x4 [dec 4]  x28  via EAX,ECX,EDX
  +0x1A4 [dec 420]  x18  via EAX,ECX,EDX
  +0x150 [dec 336]  x14  via EAX,ECX,EDX
  +0x19C [dec 412]  x14  via EAX,ECX
  +0x180 [dec 384]  x12  via EAX,ECX,EDX
  +0xC [dec 12]  x10  via EAX,ECX,EDX
  +0x182 [dec 386]  x8  via EAX,ECX,EDX
  +0x1B4 [dec 436]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004be9e0 FUN_004be9e0 size=8182 ret=4 callers=0 callees=28

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_004be9e0(int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 *_Src [GHIDRA-AUTO-LABEL: unreliable];
  longlong lVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  float fVar9;
  float fVar10;
  float fVar11;
  char cVar12;
  uint uVar13;
  int iVar14;
  undefined4 uVar15;
  int iVar16;
  undefined4 *puVar17;
  float *pfVar18;
  int local_350;
  float local_150;
  float local_14c;
  float local_148;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  int local_a8;
  float local_a4;
  ushort local_a0;
  float local_80;
  float local_7c;
  float local_78;
  int local_74;
  int local_70[3 = byte +0xc];
  float local_64;
  float local_60;
  float local_5c;
  float local_58;
  uint local_54;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  undefined1 local_20 [4];
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094bc66;
  local_10 = ExceptionList;
  uVar13 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_24 = 0;
  local_14 = *(int *)(param_1 + 4);
  FUN_00463ed0(0);
  local_8 = 0;
  local_2c = 0;
  if (*(short *)(local_14 + 0x180) == 0) {
    FUN_00451800(0);
    local_8._0_1_ = 1;
/* ... 409 more lines truncated by --max-lines */
```

### decoy: 0x004c09e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004c09e0: 16-byte aligned: YES
  004c09e0  55             PUSH EBP
  004c09e1  8B EC          MOV EBP,ESP
  004c09e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004c09e0
string literals (0):
direct callees (9):
  0x00463E30  (unnamed)
  0x00919723  operator new
  0x004CD700  (unnamed)
  0x004CDA60  (unnamed)
  0x009196FD  operator delete
  0x004491C0  CExoPtrList::Add (doubling)
  0x0048F5A0  (unnamed)
  0x004CD470  (unnamed)
  0x00463B20  (unnamed)
struct offsets touched (top 10):
  +0x134 [dec 308]  x21  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x58 [dec 88]  x2  via EAX,ECX
  +0x64 [dec 100]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x24 [dec 36]  x1  via ECX
```

### Static facts (script output, report-only)
- size 8182 B, RET 4
- callees: 0x00463ed0 (CALL), 0x00451800 (CALL), 0x00428f80 (CALL), 0x00473e50 (CALL), 0x00463b20 (CALL), 0x004be4f0 (CALL), ...
- indirect: 0x004bfc65 slot 0x50, 0x004c09b0 slot 0x54
- twins: none

## Card B17-11-17

GOG function: Global::SceneBSPRemoveGob  gog 0x0086d8c0  callconv=(blank)  param_size_bytes=8  gog_size=133  gog_retn=0
Candidate Steam address: 0x00495fb0 (size 133, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ChildRemove 0x465c20); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00495f20:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086d830 Global::SceneBSPAddGob -> 0x00495f20  gate_class=REVIEW
- GOG-order neighbour after:  0x0086d950 Global::NodeHasGob  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x008730c0 List_T__::Remove -> Steam ?  in candidate callees: n/a
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x00945400 Global::BSPSearch -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00858000 Scene::ChildRemove -> Steam 0x00465c20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x58*,0xac*} | Steam {0x4*,0x8*,0x58*,0xac*}
imm sets (* = in both, 4 common): GOG {0x8*,0xc*,0x18*,0xa4*} | Steam {0x8*,0xc*,0x18*,0xa4*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 0)
```

### prologue
```
prologue for 0x00495fb0: 16-byte aligned: YES
  00495fb0  55             PUSH EBP
  00495fb1  8B EC          MOV EBP,ESP
  00495fb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00495fb0
string literals (0):
direct callees (3):
  0x0051A2B0  (unnamed)
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00495fb0 FUN_00495fb0 size=133 ret=0 callers=2 callees=3

void FUN_00495fb0(int param_1,int param_2)

{
  int iVar1;
  
  if (((*(int *)(param_1 + 0xac) != 0) &&
      (iVar1 = FUN_0051a2b0(*(undefined4 *)(param_1 + 0xac),*(undefined4 *)(param_2 + 0xa4),
                            *(undefined4 *)(param_2 + 0xa8),*(undefined4 *)(param_2 + 0xac),
                            FUN_00496040,param_2), iVar1 != 0)) &&
     (iVar1 = FUN_00476620(param_2), iVar1 != 0)) {
    FUN_00449170(param_2);
  }
  return;
}
```

### decoy: 0x00495f20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00495f20: 16-byte aligned: YES
  00495f20  55             PUSH EBP
  00495f21  8B EC          MOV EBP,ESP
  00495f23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00495f20
string literals (0):
direct callees (3):
  0x0051A030  (unnamed)
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 4):
  +0x80 [dec 128]  x2  via ECX,EDX
  +0x84 [dec 132]  x2  via EAX,ECX
  +0xAC [dec 172]  x2  via EAX
  +0x58 [dec 88]  x1  via EDX
```

### Static facts (script output, report-only)
- size 133 B, RET 0
- callers 2: 0x00465c20 Scene::ChildRemove, 0x00499610
- callees: 0x0051a2b0 (CALL), 0x00476620 (CALL), 0x00449170 (CALL)
- twins: none

## Card B17-11-18

GOG function: PartEmitter::RenderLinked  gog 0x008af6e0  callconv=(blank)  param_size_bytes=0  gog_size=8781  gog_retn=0
Candidate Steam address: 0x004f3b40 (size 8781, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004f5d90:STR0/CALLEE0/SIG0.075/SRV0/VT0
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
disp sets (* = in both, 33 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x44*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x6c*,...(+19)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x44*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x6c*,...(+19)}
imm sets (* = in both, 20 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x13*,0x18*,0x2c*,0x41*,0x44*,0x48*,0xb4*,...(+6)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x10*,0x13*,0x18*,0x2c*,0x41*,0x44*,0x48*,0xb4*,...(+6)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f3b40: 16-byte aligned: YES
  004f3b40  55             PUSH EBP
  004f3b41  8B EC          MOV EBP,ESP
  004f3b43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 004f3b40 FUN_004f3b40 size=8781 ret=0 callers=1 callees=27

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f3b40(int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  uint uVar5;
  float *pfVar6;
  int iVar7;
  float10 fVar8;
  float local_4d0;
  float local_4cc;
  float local_4c8;
  float local_4c4;
  float local_4c0;
  float local_4bc;
  float local_4b8;
  float local_4b4;
  float local_4b0;
  float local_4ac;
  float local_4a8;
  float local_4a4;
  float local_4a0;
  float local_49c;
  float local_498;
  float local_494;
  int local_478;
  int local_474;
  double local_470;
  float local_220;
  float local_21c;
  float local_218;
  float local_214;
  float local_210;
  float local_20c;
  float local_208;
  float local_204;
  float local_200;
  float local_1fc;
  float local_1f8;
  float local_1f4;
  float local_1f0;
  float local_1ec;
  float local_1e8;
  undefined1 local_1e4 [12];
  undefined1 local_1d8 [12];
  undefined1 local_1cc [12];
  float local_1c0;
  float local_1bc;
  float local_1b8;
  float local_1b4;
  float local_1a0;
  float local_19c;
  float local_198;
  float local_194;
  undefined1 local_180 [12];
  undefined1 local_174 [12];
  undefined1 local_168 [12];
  undefined1 local_15c [12];
  undefined1 local_150 [12];
  undefined1 local_144 [28];
  undefined1 local_128 [12];
  int local_11c;
  int local_118;
  float local_114;
  float local_110;
  float local_10c;
  uint local_104;
  float local_100;
  undefined1 local_f0 [12];
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  uint local_d8;
  float local_d4;
  float local_d0;
/* ... 662 more lines truncated by --max-lines */
```

### decoy: 0x004f5d90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f5d90: 16-byte aligned: YES
  004f5d90  55             PUSH EBP
  004f5d91  8B EC          MOV EBP,ESP
  004f5d93  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004f5d90
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x0091DBA0  _memcpy
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 8781 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x00919723 operator new (CALL), 0x004e5290 (CALL), 0x004f39b0 (CALL), 0x004f5d90 (CALL), 0x004dae60 (CALL), 0x004db1f0 (CALL), ...
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B17-11-19

GOG function: PartTriMesh::GetRenderPath  gog 0x0088dc30  callconv=(blank)  param_size_bytes=0  gog_size=420  gog_retn=0
Candidate Steam address: 0x004a7070 (size 420, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004a6bd0:STR0/CALLEE0/SIG0.143/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0088c580 Global::RenderDiffuseBumpMappedPass  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0088dde0 Global::RenderFlatVP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x005b2560 CSWGuiBarkBubble::ShouldFreeMouseOnAdd -> Steam ?  in candidate callees: n/a
  0x008192e0 Global::AurTextureGetTID -> Steam ?  in candidate callees: n/a
  0x00846bb0 MdlNode::AsMdlNodeTriMesh -> Steam ?  in candidate callees: n/a
  0x00846f70 MdlNodeTriMesh::HasTangentBases -> Steam ?  in candidate callees: n/a
  0x0086b250 Global::AurNV2xLevelShadersAvailable -> Steam ?  in candidate callees: n/a
  0x008f55b0 Material::HasBumpMap -> Steam ?  in candidate callees: n/a
  0x008f5710 Material::HasBumpyShinyMap -> Steam ?  in candidate callees: n/a
  0x008f5760 Material::HasEnvMap_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x40*,0x44*,0x48*,0x184*,0x1d4*} | Steam {0x4*,0x40*,0x44*,0x48*,0x184*,0x1d4*}
imm sets (* = in both, 8 common): GOG {0x4*,0x5*,0x6*,0x8*,0xb*,0xd*,0xf*,0x28*} | Steam {0x4*,0x5*,0x6*,0x8*,0xb*,0xd*,0xf*,0x28*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004a7070: 16-byte aligned: YES
  004a7070  55             PUSH EBP
  004a7071  8B EC          MOV EBP,ESP
  004a7073  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004a7070
string literals (0):
direct callees (8):
  0x004545A0  (unnamed)
  0x004DB420  (unnamed)
  0x004933B0  (unnamed)
  0x004DB580  (unnamed)
  0x00454960  (unnamed)
  0x004DB5D0  (unnamed)
  0x00426BE0  (unnamed)
  0x004844C0  (unnamed)
struct offsets touched (top 6):
  +0x44 [dec 68]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x48 [dec 72]  x1  via EDX
  +0x184 [dec 388]  x1  via EDX
  +0x1D4 [dec 468]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004a7070 FUN_004a7070 size=420 ret=0 callers=1 callees=8

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004a7070(int param_1)

{
  float fVar1;
  float fVar2;
  char cVar3;
  char cVar4;
  char cVar5;
  int iVar6;
  char local_17;
  char local_9;
  undefined4 local_8;
  
  iVar6 = FUN_004545a0();
  cVar3 = FUN_004db420();
  cVar4 = FUN_004933b0();
  FUN_004db580();
  cVar5 = FUN_00454960();
  local_9 = FUN_004db5d0();
  local_17 = *(char *)(iVar6 + 0x184);
  fVar1 = *(float *)(*(int *)(param_1 + 0x44) + 0x48);
  fVar2 = (float)_DAT_00986b50;
  iVar6 = *(int *)(*(int *)(param_1 + 0x44) + 4);
  if ((iVar6 != 0) && (iVar6 = FUN_00426be0(iVar6), iVar6 == 0)) {
    local_17 = '\0';
  }
  if (((local_9 == '\0') && (*(int *)(param_1 + 0x40) != 0)) &&
     (*(int *)(*(int *)(param_1 + 0x40) + 0x1d4) != 0)) {
    local_9 = '\x01';
  }
  local_8 = 1;
  if (fVar2 <= fVar1) {
    if (((cVar4 == '\0') || (cVar5 != '\0')) ||
       ((DAT_009f62ec == 0 || (cVar4 = FUN_004844c0(), cVar4 == '\0')))) {
      if (((local_17 == '\0') || (local_9 == '\0')) || (DAT_009f62f8 == 0)) {
        if (local_9 == '\0') {
          if ((local_17 == '\0') || (DAT_009f62f8 == 0)) {
            if ((cVar3 != '\0') && ((cVar5 != '\0' && (DAT_009f62ec != 0)))) {
              local_8 = 6;
            }
          }
          else {
            local_8 = 0xd;
          }
        }
        else {
          local_8 = 0xb;
        }
      }
      else {
        local_8 = 8;
      }
    }
    else {
      local_8 = 0xf;
    }
  }
  else {
    local_8 = 1;
  }
  return local_8;
}
```

### decoy: 0x004a6bd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004a6bd0: 16-byte aligned: YES
  004a6bd0  55             PUSH EBP
  004a6bd1  8B EC          MOV EBP,ESP
  004a6bd3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004a6bd0
string literals (6):
  0x00A04380  at 004a6c8b  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A04380  at 004a6d0d  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A04380  at 004a6d8f  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A04380  at 004a6e11  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A04380  at 004a6e93  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A04380  at 004a6f12  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nDP4\\tvReg0.x, vReg10, program.env[ 92 ];\\nDP4\\tvReg0.y, vReg10, program.env[ 91 ];\\nDP4\\tvReg0.z, vReg10, program.env[ 90 ];\\nDP4\\tvReg0.w, vReg10, program.env[ 89 ];\\nMUL\\tvReg10, vReg0, program.env[ 16 ].wxwx;\\nDP3\\tvReg10.w, vReg10, vReg10;\\nRSQ\\tvReg10.w, vReg10.w;\\nRCP\\tvReg10.w, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].x, vReg10.w;\\nMOV\\tresult.texcoord[ 1 ].y, vReg0.z;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
direct callees (4):
  0x00919723  operator new
  0x00506560  (unnamed)
  0x00506640  (unnamed)
  0x0049CF40  (unnamed)
struct offsets touched (top 5):
  +0x988530 [dec 9995568]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x9F4928 [dec 10438952]  x2  via EAX,ECX
  +0x2C [dec 44]  x1  via EDX
  +0x4A705C [dec 4878428]  x1  via ECX
```

### Static facts (script output, report-only)
- size 420 B, RET 0
- callers 1: 0x004a79c0
- callees: 0x004545a0 (CALL), 0x004db420 (CALL), 0x004933b0 (CALL), 0x004db580 (CALL), 0x00454960 (CALL), 0x004db5d0 (CALL), ...
- twins: none

## Card B17-11-20

GOG function: FountainEmitter::Update  gog 0x008b6860  callconv=(blank)  param_size_bytes=4  gog_size=1598  gog_retn=4
Candidate Steam address: 0x004fb270 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004fac30:STR0/CALLEE0/SIG1/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x14,0x40,0x54,0x58,0x88,0x90,0xb4,0xf0,0xf4,0x114,0x1c8,...(+5)} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x5,0x8,0xc,0x14,0x41,0x44,0x60,0x74,0x78,0xc4,0x1c8,0x1d4} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004fb270: 16-byte aligned: YES
  004fb270  55             PUSH EBP
  004fb271  8B EC          MOV EBP,ESP
  004fb273  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004fb270
string literals (0):
direct callees (1):
  0x004E8A30  PartEmitter::ctor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004fb270 FUN_004fb270 size=37 ret=4 callers=1 callees=1

undefined4 * __thiscall FUN_004fb270(undefined4 *param_1,undefined4 param_2)

{
  FUN_004e8a30(param_2);
  *param_1 = SingleEmitter::vftable;
  return param_1;
}
```

### decoy: 0x004fac30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004fac30: 16-byte aligned: YES
  004fac30  55             PUSH EBP
  004fac31  8B EC          MOV EBP,ESP
  004fac33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 37 B, RET 4
- callers 1: 0x004e54d0
- callees: 0x004e8a30 (CALL)
- twins: none

## Card B17-11-21

GOG function: CSWGuiFeatsCharGen::RemoveChosenFeat  gog 0x005fae20  callconv=(blank)  param_size_bytes=2  gog_size=395  gog_retn=4
Candidate Steam address: 0x0090bbb0 (size 244, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiFeatsCharGen::Draw | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x005fafb0:0.57>0.17
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0090bcb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
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
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1bcc,0x1bd0,0x1bde} | Steam {0x4,0x8,0x1c04}
imm sets (* = in both, 1 common): GOG {0x4*,0x30,0x1bc0,0x1bcc} | Steam {0x2,0x3,0x4*,0x8,0x78,0x1168,0x1200,0x12e0,0x1be4,0x1bec}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0090bbb0: 16-byte aligned: YES
  0090bbb0  55             PUSH EBP
  0090bbb1  8B EC          MOV EBP,ESP
  0090bbb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0090bbb0
string literals (0):
direct callees (4):
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  CSWGuiTextParams::SetColor
  0x00760EA0  (unnamed)
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x1C04 [dec 7172]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0090bbb0 FUN_0090bbb0 size=244 ret=4 callers=0 callees=4

void __thiscall FUN_0090bbb0(int param_1,undefined4 param_2)

{
  byte bVar1;
  
  bVar1 = *(byte *)(param_1 + 0x1c04);
  if (1 < bVar1) {
    if (bVar1 < 4) {
      FUN_00416e30(param_1 + 0x1bec);
      FUN_00417010(&DAT_00a7fde8);
      *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fd70;
      *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fd74;
      *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fd78;
      FUN_00760ea0(param_1 + 0x1168,0x78);
    }
    else if (bVar1 == 4) {
      FUN_00416e30(param_1 + 0x1be4);
      FUN_00417010(&DAT_00a7fde8);
      *(undefined4 *)(param_1 + 0x1200) = DAT_00a7fd70;
      *(undefined4 *)(param_1 + 0x1204) = DAT_00a7fd74;
      *(undefined4 *)(param_1 + 0x1208) = DAT_00a7fd78;
    }
  }
  FUN_0040ec90(param_2);
  return;
}
```

### decoy: 0x0090bcb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0090bcb0: 16-byte aligned: YES
  0090bcb0  55             PUSH EBP
  0090bcb1  8B EC          MOV EBP,ESP
  0090bcb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0090bcb0
string literals (0):
direct callees (1):
  0x0090C620  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 244 B, RET 4
- callees: 0x00416e30 CSWGuiTextParams::SetText (CALL), 0x00417010 CSWGuiTextParams::SetColor (CALL), 0x00760ea0 (CALL), 0x0040ec90 CSWGuiPanel::Draw (CALL)
- twins: none

## Card B17-11-22

GOG function: Global::BinaryInsertModel  gog 0x008ce5e0  callconv=(blank)  param_size_bytes=12  gog_size=148  gog_retn=0
Candidate Steam address: 0x0048e130 (size 48, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0048e090:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
  0x0048E090  (unnamed)
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
  0x0048E090  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 48 B, RET 0
- callers 3: 0x004528c0, 0x005124d0, 0x00512750
- callees: 0x0048e090 (CALL)
- twins: 0x0048e060 (distinguishing: 0x0048e154: CALL 0x0048e090)
