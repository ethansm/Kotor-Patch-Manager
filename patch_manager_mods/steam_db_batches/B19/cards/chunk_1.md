# Review cards B19 chunk 1

Answer each card with one TSV row in `steam_db_batches/B19/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B19-1-01

GOG function: PartEmitter::RenderMotionBlurred  gog 0x008b1b50  callconv=(blank)  param_size_bytes=0  gog_size=1008  gog_retn=None
Candidate Steam address: 0x004f5df0 (size 340, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.93) | G8 mac_rotation: rotation:0x008b1f40:0.85>0.40
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f5f50:STR0/CALLEE0/SIG0.411/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008af6e0 PartEmitter::RenderLinked -> 0x004f3b40  trusted
- GOG-order neighbour after:  0x008b1f40 PartEmitter::RenderStandard -> 0x004f5f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x008af550 unnamed -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0x40*,0x68,0x84,0x94*,0x19c*,0x1c8*,0x1cc*,0x274} | Steam {0x4*,0x8*,0xc,0x14,0x40*,0x5c,0x60,0x6c,0x74,0x88,0x94*,0x19c*,0x1c8*,0x1cc*}
imm sets (* = in both, 3 common): GOG {0x2,0x4,0x5*,0x8*,0xc*,0xb4,0x4f8,0x5dc0} | Steam {0x5*,0x8*,0xc*,0x10,0x38,0x44,0x48}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5df0: 16-byte aligned: YES
  004f5df0  55             PUSH EBP
  004f5df1  8B EC          MOV EBP,ESP
  004f5df3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004f5df0
string literals (0):
direct callees (2):
  0x004F39B0  (unnamed)
  0x005195A0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x74 [dec 116]  x5  via EAX,ECX,EDX
  +0x40 [dec 64]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x19C [dec 412]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x5C [dec 92]  x1  via EDX
  +0x60 [dec 96]  x1  via EDX
  +0x6C [dec 108]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004f5df0 FUN_004f5df0 size=340 ret=0 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5df0(int param_1)

{
  undefined4 *puVar1;
  undefined1 local_38 [32];
  undefined1 local_18 [12];
  int local_c;
  int local_8;
  
  if (((*(int *)(param_1 + 0x40) == 0) || (*(int *)(*(int *)(param_1 + 0x40) + 0x19c) == 0)) ||
     ((float)_DAT_00986b50 <= *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0x19c) + 0x94))) {
    if (DAT_00a7bd50 != 0) {
      FUN_004f39b0();
    }
    for (local_c = 0; local_c < *(int *)(param_1 + 0x1cc); local_c = local_c + 1) {
      local_8 = *(int *)(*(int *)(param_1 + 0x1c8) + local_c * 4);
      if (((*(int *)(local_8 + 0x74) != 0) &&
          ((**(code **)(**(int **)(local_8 + 0x74) + 0x5c))
                     (local_18,*(undefined4 *)(local_8 + 8),*(undefined4 *)(local_8 + 0xc),
                      *(undefined4 *)(local_8 + 0x10)),
          *(float *)(param_1 + 0x88) != (float)_DAT_00986b38)) &&
         (*(char *)(local_8 + 0x6c) == '\0')) {
        puVar1 = (undefined4 *)
                 FUN_005195a0(*(undefined4 *)(local_8 + 0x48),*(undefined4 *)(local_8 + 0x4c),
                              *(undefined4 *)(local_8 + 0x50),*(undefined4 *)(local_8 + 0x14));
        (**(code **)(**(int **)(local_8 + 0x74) + 0x60))
                  (local_38,*puVar1,puVar1[1 = byte +0x4],puVar1[2 = byte +0x8],puVar1[3 = byte +0xc]);
      }
    }
  }
  return;
}
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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
- size 340 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x004f39b0 (CALL), 0x005195a0 (CALL)
- indirect: 0x004f5eb9 slot 0x5c, 0x004f5f39 slot 0x60
- twins: none

## Card B19-1-02

GOG function: PartEmitter::RenderStandard  gog 0x008b1f40  callconv=(blank)  param_size_bytes=0  gog_size=7354  gog_retn=0
Candidate Steam address: 0x004f5f50 (size 8362, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.84) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.89 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.89
- best decoy (neighbour/other candidate) scores: 0x004f8000:STR0/CALLEE2/SIG0.872/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b1b50 PartEmitter::RenderMotionBlurred -> 0x004f5f50  gate_class=REVIEW
- GOG-order neighbour after:  0x008b3c00 PartEmitter::RenderBoundingSphere -> 0x004f8000  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00811430 Quaternion::Constructor -> Steam ?  in candidate callees: n/a
  0x00823860 GLRender::RenderObjectsVTCQuadUP -> Steam 0x00431140  in candidate callees: yes
  0x008a4d80 PartEmitter::GetTileAlpha -> Steam ?  in candidate callees: n/a
  0x008a4db0 unnamed -> Steam ?  in candidate callees: n/a
  0x008aeed0 PartEmitter::updateParticleFrame -> Steam ?  in candidate callees: n/a
  0x008b1930 unnamed -> Steam ?  in candidate callees: n/a
  0x008b1990 unnamed -> Steam ?  in candidate callees: n/a
  0x008f4ff0 Material::BindTexture0 -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 28 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+14)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+17)}
imm sets (* = in both, 14 common): GOG {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0xb4*,0x11c*,0x17c*,0x188*,0xc00*} | Steam {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0xb4*,0x11c*,0x17c*,0x188*,0x4f8,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5f50: 16-byte aligned: YES
  004f5f50  55             PUSH EBP
  004f5f51  8B EC          MOV EBP,ESP
  004f5f53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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

### annotate (candidate, first 80 lines)
```
// FN 004f5f50 FUN_004f5f50 size=8362 ret=0 callers=1 callees=24

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5f50(int *param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  undefined4 *puVar7;
  float *pfVar8;
  float *pfVar9;
  float10 fVar10;
  int local_4f8;
  int local_4e8;
  int local_4d8;
  uint local_4c8;
  float local_4b0;
  float local_4ac;
  float local_4a8;
  float local_4a4;
  float local_4a0;
  float local_49c;
  float local_498;
  float local_494;
  float local_490;
  float local_48c;
  float local_488;
  float local_484;
  float local_480;
  float local_47c;
  float local_478;
  float local_474;
  int local_458;
  int local_454;
  double local_450;
  undefined1 local_17c [12];
  undefined1 local_170 [12];
  undefined1 local_164 [12];
  int local_158;
  int local_154;
  float local_150;
  float local_14c;
  float local_148;
  uint local_144;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  float local_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  float local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
/* ... 525 more lines truncated by --max-lines */
```

### decoy: 0x004f8000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f8000: 16-byte aligned: YES
  004f8000  55             PUSH EBP
  004f8001  8B EC          MOV EBP,ESP
  004f8003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x004f8000
string literals (0):
direct callees (19):
  0x00919723  operator new
  0x0046CDA0  (unnamed)
  0x0046CE40  (unnamed)
  0x004F39B0  (unnamed)
  0x004F5D90  (unnamed)
  0x004DAE60  GUI image texture bind (MakeCurrent via vtbl +0xB8)
  0x004DB1F0  Material::SetBlendingMode
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
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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

### Static facts (script output, report-only)
- size 8362 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x00919723 operator new (CALL), 0x0046cda0 (CALL), 0x0046ce40 (CALL), 0x00463120 (CALL), 0x00428f80 (CALL), 0x00473e50 (CALL), ...
- indirect: 0x004f61ea slot 0x84
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B19-1-03

GOG function: PartEmitter::RenderMotionBlurred  gog 0x008b1b50  callconv=(blank)  param_size_bytes=0  gog_size=1008  gog_retn=None
Candidate Steam address: 0x004f5f50 (size 8362, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.87) | G8 mac_rotation: rotation:0x008b1f40:0.85>0.40
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.41 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f5df0:STR0/CALLEE0/SIG0.331/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008af6e0 PartEmitter::RenderLinked -> 0x004f3b40  trusted
- GOG-order neighbour after:  0x008b1f40 PartEmitter::RenderStandard -> 0x004f5f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x008af550 unnamed -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0x8*,0x40*,0x68*,0x84*,0x94*,0x19c*,0x1c8*,0x1cc*,0x274*} | Steam {0x4*,0x8*,0xc,0x10,0x14,0x3c,0x40*,0x4c,0x50,0x54,0x64,0x68*,0x70,0x78,...(+17)}
imm sets (* = in both, 8 common): GOG {0x2*,0x4*,0x5*,0x8*,0xc*,0xb4*,0x4f8*,0x5dc0*} | Steam {0x2*,0x4*,0x5*,0x8*,0xc*,0x10,0x18,0x41,0x44,0xb4*,0x11c,0x17c,0x188,0x4f8*,...(+2)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5f50: 16-byte aligned: YES
  004f5f50  55             PUSH EBP
  004f5f51  8B EC          MOV EBP,ESP
  004f5f53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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

### annotate (candidate, first 80 lines)
```
// FN 004f5f50 FUN_004f5f50 size=8362 ret=0 callers=1 callees=24

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5f50(int *param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  uint uVar6;
  undefined4 *puVar7;
  float *pfVar8;
  float *pfVar9;
  float10 fVar10;
  int local_4f8;
  int local_4e8;
  int local_4d8;
  uint local_4c8;
  float local_4b0;
  float local_4ac;
  float local_4a8;
  float local_4a4;
  float local_4a0;
  float local_49c;
  float local_498;
  float local_494;
  float local_490;
  float local_48c;
  float local_488;
  float local_484;
  float local_480;
  float local_47c;
  float local_478;
  float local_474;
  int local_458;
  int local_454;
  double local_450;
  undefined1 local_17c [12];
  undefined1 local_170 [12];
  undefined1 local_164 [12];
  int local_158;
  int local_154;
  float local_150;
  float local_14c;
  float local_148;
  uint local_144;
  undefined4 local_140;
  undefined4 local_13c;
  undefined4 local_138;
  undefined4 local_134;
  undefined4 local_130;
  undefined4 local_12c;
  float local_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  float local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  float local_108;
  float local_104;
  float local_100;
  float local_fc;
  float local_f8;
  float local_f4;
  float local_f0;
  float local_ec;
  float local_e8;
  float local_e4;
  float local_e0;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
/* ... 525 more lines truncated by --max-lines */
```

### decoy: 0x004f5df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004f5df0: 16-byte aligned: YES
  004f5df0  55             PUSH EBP
  004f5df1  8B EC          MOV EBP,ESP
  004f5df3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004f5df0
string literals (0):
direct callees (2):
  0x004F39B0  (unnamed)
  0x005195A0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x74 [dec 116]  x5  via EAX,ECX,EDX
  +0x40 [dec 64]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x19C [dec 412]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x5C [dec 92]  x1  via EDX
  +0x60 [dec 96]  x1  via EDX
  +0x6C [dec 108]  x1  via EDX
```

### Static facts (script output, report-only)
- size 8362 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x00919723 operator new (CALL), 0x0046cda0 (CALL), 0x0046ce40 (CALL), 0x00463120 (CALL), 0x00428f80 (CALL), 0x00473e50 (CALL), ...
- indirect: 0x004f61ea slot 0x84
- imports: glBlendFunc (OPENGL32.DLL), glDepthMask (OPENGL32.DLL), glDisable (OPENGL32.DLL), ...
- twins: none

## Card B19-1-04

GOG function: PartEmitter::RenderStandard  gog 0x008b1f40  callconv=(blank)  param_size_bytes=0  gog_size=7354  gog_retn=0
Candidate Steam address: 0x004f5df0 (size 340, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.93) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.27 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f5f50:STR0/CALLEE2/SIG0.889/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b1b50 PartEmitter::RenderMotionBlurred -> 0x004f5f50  gate_class=REVIEW
- GOG-order neighbour after:  0x008b3c00 PartEmitter::RenderBoundingSphere -> 0x004f8000  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00811430 Quaternion::Constructor -> Steam ?  in candidate callees: n/a
  0x00823860 GLRender::RenderObjectsVTCQuadUP -> Steam 0x00431140  in candidate callees: no
  0x008a4d80 PartEmitter::GetTileAlpha -> Steam ?  in candidate callees: n/a
  0x008a4db0 unnamed -> Steam ?  in candidate callees: n/a
  0x008aeed0 PartEmitter::updateParticleFrame -> Steam ?  in candidate callees: n/a
  0x008b1930 unnamed -> Steam ?  in candidate callees: n/a
  0x008b1990 unnamed -> Steam ?  in candidate callees: n/a
  0x008f4ff0 Material::BindTexture0 -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x10,0x14*,0x3c,0x40*,0x4c,0x50,0x54,0x64,0x68,0x70,0x78,...(+14)} | Steam {0x4*,0x8*,0xc*,0x14*,0x40*,0x5c,0x60,0x6c,0x74,0x88,0x94*,0x19c,0x1c8*,0x1cc*}
imm sets (* = in both, 5 common): GOG {0x2,0x4,0x5*,0x8*,0xc*,0x10*,0x18,0x41,0x44*,0xb4,0x11c,0x17c,0x188,0xc00} | Steam {0x5*,0x8*,0xc*,0x10*,0x38,0x44*,0x48}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004f5df0: 16-byte aligned: YES
  004f5df0  55             PUSH EBP
  004f5df1  8B EC          MOV EBP,ESP
  004f5df3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004f5df0
string literals (0):
direct callees (2):
  0x004F39B0  (unnamed)
  0x005195A0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x74 [dec 116]  x5  via EAX,ECX,EDX
  +0x40 [dec 64]  x3  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x19C [dec 412]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via ECX
  +0x5C [dec 92]  x1  via EDX
  +0x60 [dec 96]  x1  via EDX
  +0x6C [dec 108]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004f5df0 FUN_004f5df0 size=340 ret=0 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f5df0(int param_1)

{
  undefined4 *puVar1;
  undefined1 local_38 [32];
  undefined1 local_18 [12];
  int local_c;
  int local_8;
  
  if (((*(int *)(param_1 + 0x40) == 0) || (*(int *)(*(int *)(param_1 + 0x40) + 0x19c) == 0)) ||
     ((float)_DAT_00986b50 <= *(float *)(*(int *)(*(int *)(param_1 + 0x40) + 0x19c) + 0x94))) {
    if (DAT_00a7bd50 != 0) {
      FUN_004f39b0();
    }
    for (local_c = 0; local_c < *(int *)(param_1 + 0x1cc); local_c = local_c + 1) {
      local_8 = *(int *)(*(int *)(param_1 + 0x1c8) + local_c * 4);
      if (((*(int *)(local_8 + 0x74) != 0) &&
          ((**(code **)(**(int **)(local_8 + 0x74) + 0x5c))
                     (local_18,*(undefined4 *)(local_8 + 8),*(undefined4 *)(local_8 + 0xc),
                      *(undefined4 *)(local_8 + 0x10)),
          *(float *)(param_1 + 0x88) != (float)_DAT_00986b38)) &&
         (*(char *)(local_8 + 0x6c) == '\0')) {
        puVar1 = (undefined4 *)
                 FUN_005195a0(*(undefined4 *)(local_8 + 0x48),*(undefined4 *)(local_8 + 0x4c),
                              *(undefined4 *)(local_8 + 0x50),*(undefined4 *)(local_8 + 0x14));
        (**(code **)(**(int **)(local_8 + 0x74) + 0x60))
                  (local_38,*puVar1,puVar1[1 = byte +0x4],puVar1[2 = byte +0x8],puVar1[3 = byte +0xc]);
      }
    }
  }
  return;
}
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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
- size 340 B, RET 0
- callers 1: 0x004e92a0
- callees: 0x004f39b0 (CALL), 0x005195a0 (CALL)
- indirect: 0x004f5eb9 slot 0x5c, 0x004f5f39 slot 0x60
- twins: none

## Card B19-1-05

GOG function: PartEmitter::RenderStandard  gog 0x008b1f40  callconv=(blank)  param_size_bytes=0  gog_size=7354  gog_retn=0
Candidate Steam address: 0x004f8000 (size 7939, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=PartEmitter::RenderBoundingSphere | G7 mac: pass(0.89) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.87 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f5f50:STR0/CALLEE2/SIG0.889/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008b1b50 PartEmitter::RenderMotionBlurred -> 0x004f5f50  gate_class=REVIEW
- GOG-order neighbour after:  0x008b3c00 PartEmitter::RenderBoundingSphere -> 0x004f8000  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00811430 Quaternion::Constructor -> Steam ?  in candidate callees: n/a
  0x00823860 GLRender::RenderObjectsVTCQuadUP -> Steam 0x00431140  in candidate callees: yes
  0x008a4d80 PartEmitter::GetTileAlpha -> Steam ?  in candidate callees: n/a
  0x008a4db0 unnamed -> Steam ?  in candidate callees: n/a
  0x008aeed0 PartEmitter::updateParticleFrame -> Steam ?  in candidate callees: n/a
  0x008b1930 unnamed -> Steam ?  in candidate callees: n/a
  0x008b1990 unnamed -> Steam ?  in candidate callees: n/a
  0x008f4ff0 Material::BindTexture0 -> Steam ?  in candidate callees: n/a
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 28 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+14)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x3c*,0x40*,0x4c*,0x50*,0x54*,0x64*,0x68*,0x70*,0x78*,...(+15)}
imm sets (* = in both, 14 common): GOG {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0xb4*,0x11c*,0x17c*,0x188*,0xc00*} | Steam {0x2*,0x3,0x4*,0x5*,0x8*,0xc*,0x10*,0x18*,0x41*,0x44*,0x60,0xb4*,0x11c*,0x17c*,...(+4)}

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
  0x004DB1F0  Material::SetBlendingMode
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
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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

## Card B19-1-06

GOG function: PartEmitter::RenderMotionBlurred  gog 0x008b1b50  callconv=(blank)  param_size_bytes=0  gog_size=1008  gog_retn=None
Candidate Steam address: 0x004f8000 (size 7939, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: fail:trusted_holder=PartEmitter::RenderBoundingSphere | G7 mac: pass(0.92) | G8 mac_rotation: rotation:0x008b1f40:0.85>0.40
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004f5f50:STR0/CALLEE0/SIG0.411/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008af6e0 PartEmitter::RenderLinked -> 0x004f3b40  trusted
- GOG-order neighbour after:  0x008b1f40 PartEmitter::RenderStandard -> 0x004f5f50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0042a8f0 Quaternion::GetZAxis -> Steam ?  in candidate callees: n/a
  0x004309a0 Quaternion::GetYAxis -> Steam ?  in candidate callees: n/a
  0x00457ac0 CExoArrayList_Vector_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00457b00 CExoArrayList_Vector_::Allocate -> Steam ?  in candidate callees: n/a
  0x0055f8f0 unnamed -> Steam ?  in candidate callees: n/a
  0x008af550 unnamed -> Steam ?  in candidate callees: n/a
  0x008fe2a0 List_SafePointer_WindManager___::Destructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0x40*,0x68*,0x84,0x94*,0x19c*,0x1c8*,0x1cc*,0x274} | Steam {0x4*,0x8*,0xc,0x10,0x14,0x3c,0x40*,0x4c,0x50,0x54,0x64,0x68*,0x70,0x78,...(+15)}
imm sets (* = in both, 7 common): GOG {0x2*,0x4*,0x5*,0x8*,0xc*,0xb4*,0x4f8,0x5dc0*} | Steam {0x2*,0x3,0x4*,0x5*,0x8*,0xc*,0x10,0x18,0x41,0x44,0x60,0xb4*,0x11c,0x17c,...(+4)}

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
  0x004DB1F0  Material::SetBlendingMode
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
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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
  0x004DB1F0  Material::SetBlendingMode
  0x004F3330  (unnamed)
  0x004E9210  (unnamed)
  0x004E91E0  (unnamed)
  0x004DE5D0  (unnamed)
  0x005195A0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0091CB24  (unnamed)
  0x00431140  GLRender::RenderObjectsVTCQuadUP
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
