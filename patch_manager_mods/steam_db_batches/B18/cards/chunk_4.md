# Review cards B18 chunk 4

Answer each card with one TSV row in `steam_db_batches/B18/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B18-4-01

GOG function: CAurGUIStringInternal::SetSizeBoundingRect_2  gog 0x008d10b0  callconv=(blank)  param_size_bytes=8  gog_size=89  gog_retn=8
Candidate Steam address: 0x0047a7a0 (size 89, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0047a740:STR0/CALLEE0/SIG1/SRV0/VT0
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
prologue for 0x0047a7a0: 16-byte aligned: YES
  0047a7a0  55             PUSH EBP
  0047a7a1  8B EC          MOV EBP,ESP
  0047a7a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0047a7a0 FUN_0047a7a0 size=89 ret=8 callers=0 callees=1

void __fastcall FUN_0047a7a0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  iVar2 = (int)*(short *)(&DAT_00a309cc + DAT_00a309c0 * 10);
  uVar1 = FUN_0091c860();
  *(undefined4 *)(param_1 + 0xc) = uVar1;
  uVar1 = FUN_0091c860((int)(short)(&DAT_00a309ce)[DAT_00a309c0 * 5],iVar2);
  *(undefined4 *)(param_1 + 0x10) = uVar1;
  return;
}
```

### decoy: 0x0047a740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0047a740: 16-byte aligned: YES
  0047a740  55             PUSH EBP
  0047a741  8B EC          MOV EBP,ESP
  0047a743  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0047a740
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0xA309CC [dec 10684876]  x1  via ECX
  +0xA309CE [dec 10684878]  x1  via EDX
```

### Static facts (script output, report-only)
- size 89 B, RET 8
- callees: 0x0091c860 (CALL)
- globals: 0x00a309c0 READ
- twins: none

## Card B18-4-02

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2380 (size 21, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2360:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: no
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: no
  0x00578ec0 CSWCProjectile::Constructor -> Steam 0x008e54e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a2380: 16-byte aligned: YES
  007a2380  55             PUSH EBP
  007a2381  8B EC          MOV EBP,ESP
  007a2383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2380
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2380 FUN_007a2380 size=21 ret=4 callers=2 callees=0

void __thiscall FUN_007a2380(undefined4 *param_1,undefined4 param_2)

{
  *param_1 = param_2;
  return;
}
```

### decoy: 0x007a2360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2360: 16-byte aligned: YES
  007a2360  55             PUSH EBP
  007a2361  8B EC          MOV EBP,ESP
  007a2363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2360
string literals (0):
direct callees (1):
  0x007A2040  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 21 B, RET 4
- callers 2: 0x005363a0 CServerExoAppInternal::StartServices, 0x00788d90 CClientExoAppInternal::StartServices
- twins: none

## Card B18-4-03

GOG function: Global::AurMultiTextureAvailable  gog 0x0086b4a0  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x00484690 (size 121, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484710:STR0/CALLEE4/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b420 Global::AurOnePassEnvMapAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b4e0 Global::AurVertexProgramsNVAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: no
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: no
  0x008de4b0 GLRender::EnableFragmentShadersARB -> Steam 0x004b50c0  in candidate callers: no
  0x008e85f0 VertexPrimitiveFlat::ReconnectArraysVAR -> Steam 0x004ce460  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00484690: 16-byte aligned: YES
  00484690  55             PUSH EBP
  00484691  8B EC          MOV EBP,ESP
  00484693  83 3D 08 61 9F 00 FF CMP dword ptr [0x009f6108],-0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484690
string literals (0):
direct callees (2):
  0x00484480  (unnamed)
  0x004843C0  GL_HasFragmentShaderBit
struct offsets touched (top 1):
  +0x988528 [dec 9995560]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00484690 FUN_00484690 size=121 ret=0 callers=1 callees=3

uint FUN_00484690(void)

{
  char cVar1;
  int iVar2;
  
  if (DAT_009f6108 == 0xffffffff) {
    cVar1 = FUN_00484480();
    if (cVar1 == '\0') {
      DAT_009f6108 = 0;
    }
    else {
      iVar2 = FUN_004843c0();
      if (iVar2 == 0) {
        glGetIntegerv(DAT_0098852c,&DAT_009f6108);
        DAT_009f6108 = (uint)(1 < (int)DAT_009f6108);
      }
      else {
        DAT_009f6108 = 1;
      }
    }
  }
  return DAT_009f6108;
}
```

### decoy: 0x00484710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484710: 16-byte aligned: YES
  00484710  55             PUSH EBP
  00484711  8B EC          MOV EBP,ESP
  00484713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484710
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 121 B, RET 0
- callers 1: 0x004a1070
- callees: 0x00484480 (CALL), 0x004843c0 (CALL), 0x00000027 (CALL)
- imports: glGetIntegerv (OPENGL32.DLL)
- twins: none

## Card B18-4-04

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

## Card B18-4-05

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

## Card B18-4-06

GOG function: CExoFileInternal::Read  gog 0x00610130  callconv=(blank)  param_size_bytes=8  gog_size=231  gog_retn=8
Candidate Steam address: 0x0073d710 (size 376, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073d650:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006100a0 CExoFileInternal::GetSize -> 0x0073dba0  trusted
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0x10}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x10,0x18} | Steam {0x4*,0x5,0x8*,0x1c}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0073d710: 16-byte aligned: YES
  0073d710  55             PUSH EBP
  0073d711  8B EC          MOV EBP,ESP
  0073d713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0073d710
string literals (1):
  0x0099D540  at 0073d790  'saves'
direct callees (8):
  0x00733A60  CExoString::operator==_2
  0x00919F10  _strlen
  0x00733570  CExoString::CExoString_char
  0x00734FC0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x0091D3BA  (unnamed)
  0x0073D650  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073d710 FUN_0073d710 size=376 ret=8 callers=1 callees=8

undefined * __thiscall FUN_0073d710(undefined *param_1,int *param_2,size_t param_3)

{
  char cVar1;
  uint uVar2;
  int iVar3;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  size_t local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961de6;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar3 = FUN_00733a60(&DAT_00986925);
  if (iVar3 == 0) {
    local_18 = param_3;
    if (param_3 == 0) {
      if (*param_2 == 0) {
        local_18 = 0;
      }
      else {
        local_18 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
      }
    }
    if (DAT_00a7ea88 != '\0') {
      FUN_00733570("saves");
      local_8 = 0;
      local_20 = FUN_00734fc0(local_28,5);
      local_8 = 0xffffffff;
      FUN_00733780(uVar2);
      if (local_20 != 0) {
        if ((DAT_00a7eac4 & 1) == 0) {
          DAT_00a7eac4 = DAT_00a7eac4 | 1;
          local_8 = 1;
          FUN_00733540();
          _atexit [GHIDRA-AUTO-LABEL: unreliable](FUN_00985340);
          local_8 = 0xffffffff;
        }
        cVar1 = FUN_0073d650(&DAT_00a7eabc);
        if (cVar1 != '\0') {
          ExceptionList = local_10;
          return &DAT_00a7eabc;
        }
      }
    }
    local_1c = 0;
    local_14 = *(int *)(param_1 + 8);
    while ((local_1c == 0 && (local_14 != 0))) {
      iVar3 = FUN_00734fc0(param_2,local_18);
      if (iVar3 == 1) {
        local_1c = 1;
      }
      else {
        local_14 = *(int *)(local_14 + 0x10);
      }
    }
    if (local_1c != 0) {
      param_1 = (undefined *)(local_14 + 8);
    }
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0073d650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073d650: 16-byte aligned: YES
  0073d650  55             PUSH EBP
  0073d651  8B EC          MOV EBP,ESP
  0073d653  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073d650
string literals (1):
  0x0099D538  at 0073d6ce  '%s%d/'
direct callees (5):
  0x0073D640  (unnamed)
  0x00919671  (unnamed)
  0x009216B0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x009196EE  __security_check_cookie
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EDX
```

### Static facts (script output, report-only)
- size 376 B, RET 8
- callers 1: 0x00739790
- callees: 0x00733a60 (CALL), 0x00919f10 _strlen (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00734fc0 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), ...
- strings: 0x0099d540 "saves"
- twins: none

## Card B18-4-07

GOG function: Global::AurGUICloseViewport  gog 0x008d02d0  callconv=(blank)  param_size_bytes=0  gog_size=235  gog_retn=0
Candidate Steam address: 0x004799c0 (size 235, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00479460:STR0/CALLEE6/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008cfd70 Global::AurGUISetupViewport  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008d03c0 Global::GetResolutionTextScalar -> 0x00479ab0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 12): the card's A-CALLEE credit may come from these, not from callees
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

### Static facts (script output, report-only)
- size 235 B, RET 0
- callers 12: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x004184f0 CSWGuiScene::Draw, 0x0041bf40 CSWGuiProgressBarReveal::Draw, 0x0041f980, ...
- callees: 0x0000004f (CALL), 0x0000000e (CALL), 0x0000001c (CALL)
- globals: 0x00a309c0 READ, 0x00a309c0 READ, 0x00a309c0 READ, ...
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), glViewport (OPENGL32.DLL)
- twins: none

## Card B18-4-08

GOG function: Global::AurMultiTextureAvailable  gog 0x0086b4a0  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x00484750 (size 53, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484710:STR0/CALLEE4/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b420 Global::AurOnePassEnvMapAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b4e0 Global::AurVertexProgramsNVAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (4 of 47): the card's A-CALLEE credit may come from these, not from callees
  0x00819810 CAurTextureBasic::AsNormalizeVectorCubeMap -> Steam 0x00427110  in candidate callers: no
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: no
  0x008de4b0 GLRender::EnableFragmentShadersARB -> Steam 0x004b50c0  in candidate callers: no
  0x008e85f0 VertexPrimitiveFlat::ReconnectArraysVAR -> Steam 0x004ce460  in candidate callers: no
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
prologue for 0x00484750: 16-byte aligned: YES
  00484750  55             PUSH EBP
  00484751  8B EC          MOV EBP,ESP
  00484753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484750
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484750 FUN_00484750 size=53 ret=0 callers=1 callees=0

uint FUN_00484750(void)

{
  if (DAT_009f6110 == 0xffffffff) {
    DAT_009f6110 = (uint)((DAT_009f6088 & DAT_00a32df8) == DAT_009f6088);
  }
  return DAT_009f6110;
}
```

### decoy: 0x00484710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484710: 16-byte aligned: YES
  00484710  55             PUSH EBP
  00484711  8B EC          MOV EBP,ESP
  00484713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484710
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 53 B, RET 0
- callers 1: 0x004c0dd0
- twins: 0x00484710, 0x00484a20 (distinguishing: 0x00484754: CMP dword ptr [0x009f6110],-0x1)

## Card B18-4-09

GOG function: CSWCollisionMesh::NormalLocalToWorld  gog 0x00728b10  callconv=(blank)  param_size_bytes=12  gog_size=331  gog_retn=12
Candidate Steam address: 0x005ef570 (size 6521, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWSPlaceable::LoadPlaceable | G7 mac: pass(0.02) | G8 mac_rotation: rotation:0x007286d0:0.80>0.36
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005ef3d0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007286d0 CSWCollisionMesh::LocalToWorld  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00728c60 Global::AddModuleOverrideResourceDirectories  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00723da0 CSWCollisionMesh::GetVertex -> Steam ?  in candidate callees: n/a
  0x00723e60 CSWCollisionMesh::LoadDefaultMesh -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007627d0 CSWSDoor::GetMeshBoundingBox -> Steam 0x00617320  in candidate callers: no
  0x0076ce50 CSWSPlaceable::AddToArea -> Steam 0x005f2a80  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1c,0x58,0x64} | Steam {0x4,0x8,0x48,0x7c,0xcc,0xd4,0xe4,0xfc,0x130,0x228,0x22c,0x244,0x251,0x270,...(+34)}
imm sets (* = in both, 1 common): GOG {0xc*,0x50,0x55ffff00} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc*,0xd,0xe,0xf,...(+19)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005ef570: 16-byte aligned: YES
  005ef570  55             PUSH EBP
  005ef571  8B EC          MOV EBP,ESP
  005ef573  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ef570
string literals (67):
  0x0099364C  at 005ef673  'TemplateResRef'
  0x00996E14  at 005ef6dc  'LocName'
  0x00996E04  at 005ef746  'AutoRemoveKey'
  0x00996DFC  at 005ef778  'Faction'
  0x00993EBC  at 005ef7bf  'Invulnerable'
  0x00996DE4  at 005ef82f  'NotBlastable'
  0x00996DDC  at 005ef879  'Min1HP'
  0x00996DCC  at 005ef8ab  'PartyInteract'
  0x00996DC0  at 005ef8dd  'OpenLockDC'
  0x00996DB0  at 005ef90c  'OpenLockDiff'
  0x00996DA0  at 005ef93b  'OpenLockDiffMod'
  0x00996D98  at 005ef983  'KeyName'
direct callees (37):
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x00734690  CExoString::ToLower
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710840  (unnamed)
  0x00718E40  CResGFF::ReadFieldCResRef
  0x00710CA0  (unnamed)
  0x00735240  (unnamed)
  0x007190E0  CResGFF::ReadFieldCExoLocString
  0x00735400  (unnamed)
  0x00735360  CExoLocString::Destructor
  0x00718A00  CResGFF::ReadFieldBYTE
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718A80  CResGFF::ReadFieldCHAR
  0x00718C80  CResGFF::ReadFieldINT
  0x00718B80  CResGFF::ReadFieldSHORT
  0x0054A610  CSWSObject::SetCurrentHitPoints
  0x00718B00  CResGFF::ReadFieldWORD
struct offsets touched (top 10):
  +0x7C [dec 124]  x3  via EDX
  +0xFC [dec 252]  x3  via EAX,EDX
  +0x2E0 [dec 736]  x3  via ECX,EDX
  +0x228 [dec 552]  x2  via EAX
  +0x22C [dec 556]  x2  via EDX
  +0x251 [dec 593]  x2  via EAX,EDX
  +0x294 [dec 660]  x2  via ECX
  +0x380 [dec 896]  x2  via EAX,EDX
  +0x390 [dec 912]  x2  via ECX,EDX
  +0x3EC [dec 1004]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005ef570 FUN_005ef570 size=6521 ret=12 callers=2 callees=37

/* WARNING: Removing unreachable block (ram,0x005f08ab) */
/* WARNING: Removing unreachable block (ram,0x005f0864) */
/* WARNING: Removing unreachable block (ram,0x005f091e) */

void __thiscall FUN_005ef570(int *param_1,undefined4 param_2,undefined4 param_3,int param_4)

{
  bool bVar1;
  undefined1 uVar2;
  short sVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  undefined1 *puVar7;
  undefined4 *local_49c;
  undefined1 local_415 [16];
  undefined1 local_405;
  uint local_404;
  uint local_400;
  uint local_3fc;
  uint local_3f8;
  undefined1 local_3f4;
  undefined1 local_3f3;
  undefined1 local_3f2;
  undefined1 local_3f1;
  int local_3f0;
  int local_3ec;
  undefined1 local_3e5;
  int local_3e4;
  int local_3e0;
  int local_3dc;
  uint local_3d8;
  undefined1 local_3d2;
  undefined1 local_3d1;
  uint local_3d0;
  uint local_3cc;
  int local_3c8;
  undefined1 local_3c2;
  undefined1 local_3c1;
  uint local_3c0;
  uint local_3bc;
  undefined4 local_3b8;
  undefined1 local_3b3;
  undefined1 local_3b2;
  undefined1 local_3b1;
  uint local_3b0;
  uint local_3ac;
  uint local_3a8;
  uint local_3a4;
  uint local_3a0;
  uint local_39c;
  uint local_398;
  int local_394;
  uint local_390;
  undefined4 local_38c;
  undefined4 local_388;
  undefined1 local_384 [16];
  undefined4 local_374;
  undefined1 local_370 [8];
  undefined4 local_368;
  undefined4 *local_364;
  undefined4 *local_360;
  undefined4 *local_35c;
  undefined4 *local_358;
  undefined1 local_354 [16];
  int local_344;
  undefined4 *local_340;
  undefined1 local_33c [16];
  undefined1 local_32c [16];
  undefined1 local_31c [8];
  undefined1 local_314 [16];
  undefined1 local_304 [16];
  undefined1 local_2f4 [16];
  undefined1 local_2e4 [16];
  undefined1 local_2d4 [16];
  undefined1 local_2c4 [16];
  undefined1 local_2b4 [16];
  undefined1 local_2a4 [16];
  undefined1 local_294 [16];
/* ... 445 more lines truncated by --max-lines */
```

### decoy: 0x005ef3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ef3d0: 16-byte aligned: YES
  005ef3d0  55             PUSH EBP
  005ef3d1  8B EC          MOV EBP,ESP
  005ef3d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005ef3d0
string literals (1):
  0x00996B14  at 005ef4c3  "Item template %s doesn't exist.\\n"
direct callees (9):
  0x00919723  operator new
  0x00716FD0  CResGFF::Constructor_2
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x007189D0  CResGFF::GetTopLevelStruct
  0x005EF570  CSWSPlaceable::LoadPlaceable
struct offsets touched (top 1):
  +0x94 [dec 148]  x1  via EAX
```

### Static facts (script output, report-only)
- size 6521 B, RET 12
- callers 2: 0x00527890 CSWSArea::LoadPlaceables, 0x005ef3d0 CSWSPlaceable::LoadFromTemplate
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00733570 CExoString::CStrConstructor (CALL), 0x00718f90 CResGFF::ReadFieldCExoString (CALL), 0x00734690 CExoString::ToLower (CALL), 0x007337c0 CExoString::operator= (CALL), 0x00733780 CExoString::Destructor (CALL), ...
- indirect: 0x005eff57 slot 0xcc, 0x005eff74 slot 0xd4, 0x005f0b8c slot 0x7c, 0x005f0bc8 slot 0x7c, 0x005f0c63 slot 0x7c, ...
- strings: 0x009953ac "Animation", 0x00996b40 "AnimationState", 0x00996b68 "IsCorpse", ...
- globals: 0x00a1b4a4 READ, 0x00a1b4d0 READ
- twins: none

## Card B18-4-10

GOG function: Global::AurSetVSyncState  gog 0x0086ac50  callconv=(blank)  param_size_bytes=1  gog_size=43  gog_retn=0
Candidate Steam address: 0x00478d50 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCArea::SetGlobalLightAmbient | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478d70:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ac30 Global::AurSetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ac80 Global::AurSetPauseOverlayColor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b270 Global::AurBumpyShinyAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004691f0 CClientOptions::SetVSync -> Steam 0x007bc520  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478d50: 16-byte aligned: YES
  00478d50  55             PUSH EBP
  00478d51  8B EC          MOV EBP,ESP
  00478d53  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478d50
string literals (0):
direct callees (1):
  0x00478FB0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478d50 FUN_00478d50 size=22 ret=0 callers=1 callees=1

undefined4 FUN_00478d50(undefined4 param_1)

{
  FUN_00478fb0(param_1);
  return DAT_00a309b0;
}
```

### decoy: 0x00478d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478d70: 16-byte aligned: YES
  00478d70  55             PUSH EBP
  00478d71  8B EC          MOV EBP,ESP
  00478d73  E8 68 B7 00 00 CALL 0x004844e0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478d70
string literals (0):
direct callees (1):
  0x004844E0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 22 B, RET 0
- callers 1: 0x007bc4f0
- callees: 0x00478fb0 (CALL)
- globals: 0x00a309b0 READ
- twins: none

## Card B18-4-11

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00710b70 (size 164, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710c20:STR0/CALLEE10/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 40): the card's A-CALLEE credit may come from these, not from callees
  0x00411610 CSWCCreature::SetSoundSet -> Steam 0x00765940  in candidate callers: no
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: no
  0x00475550 CSWCObject::SetEnvMap -> Steam 0x007f03f0  in candidate callers: no
  0x004756d0 CSWCObject::SetEnvMap_2 -> Steam 0x007f0570  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0xc,0x10*,0x18}

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

### decoy: 0x00710c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710c20: 16-byte aligned: YES
  00710c20  55             PUSH EBP
  00710c21  8B EC          MOV EBP,ESP
  00710c23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710c20
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 164 B, RET 4
- callers 1: 0x00710aa0
- callees: 0x00734010 (CALL), 0x00919f10 _strlen (CALL), 0x0091ac7c (CALL)
- twins: none

## Card B18-4-12

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

## Card B18-4-13

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040fa30 (size 79, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ScaleToEdgeHelper_3 0x40fa80)
- best decoy (neighbour/other candidate) scores: 0x0040f9f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> Steam 0x0040fa80  in candidate callers: yes
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
- callers 2: 0x0040fa80 InitControl::ScaleToEdgeHelper_3, 0x0040fb40
- twins: none

## Card B18-4-14

GOG function: CSWCollisionMesh::not_GetWalkableMaterialMask  gog 0x00713530  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00650d40 (size 158, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Global::QueryTimer | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00650de0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713490 Global::QueryTimer -> 0x00650d40  trusted
- GOG-order neighbour after:  0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00713490 Global::QueryTimer -> Steam 0x00650d40  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0xf4240}

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

## Card B18-4-15

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00711170 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CRes::Destructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007110d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 40): the card's A-CALLEE credit may come from these, not from callees
  0x00411610 CSWCCreature::SetSoundSet -> Steam 0x00765940  in candidate callers: no
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: no
  0x00475550 CSWCObject::SetEnvMap -> Steam 0x007f03f0  in candidate callers: no
  0x004756d0 CSWCObject::SetEnvMap_2 -> Steam 0x007f0570  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00711170: 16-byte aligned: YES
  00711170  55             PUSH EBP
  00711171  8B EC          MOV EBP,ESP
  00711173  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00711170
string literals (0):
direct callees (2):
  0x007111A0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00711170 FUN_00711170 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00711170(undefined4 param_1,uint param_2)

{
  FUN_007111a0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007110d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007110d0: 16-byte aligned: YES
  007110d0  55             PUSH EBP
  007110d1  8B EC          MOV EBP,ESP
  007110d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007110d0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x6 [dec 6]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x007111a0 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B18-4-16

GOG function: Global::AurSetVSyncState  gog 0x0086ac50  callconv=(blank)  param_size_bytes=1  gog_size=43  gog_retn=0
Candidate Steam address: 0x00478da0 (size 175, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478d70:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ac30 Global::AurSetAnisotropy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ac80 Global::AurSetPauseOverlayColor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b270 Global::AurBumpyShinyAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004691f0 CClientOptions::SetVSync -> Steam 0x007bc520  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x10,0x18,0x40,0xc00}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 1 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478da0: 16-byte aligned: YES
  00478da0  55             PUSH EBP
  00478da1  8B EC          MOV EBP,ESP
  00478da3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478da0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478da0 FUN_00478da0 size=175 ret=0 callers=1 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00478da0(float param_1,float param_2,float param_3,float param_4)

{
  undefined4 local_44;
  undefined4 local_34;
  undefined4 local_24;
  undefined4 local_14;
  
  local_14 = (uint)(longlong)ROUND(param_1 * (float)_DAT_009884a0);
  local_24 = (int)(longlong)ROUND(param_2 * (float)_DAT_009884a0);
  local_34 = (int)(longlong)ROUND(param_3 * (float)_DAT_009884a0);
  local_44 = (int)(longlong)ROUND(param_4 * (float)_DAT_009884a0);
  _DAT_009f5f58 = local_14 | local_24 << 8 | local_34 << 0x10 | local_44 << 0x18;
  return;
}
```

### decoy: 0x00478d70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478d70: 16-byte aligned: YES
  00478d70  55             PUSH EBP
  00478d71  8B EC          MOV EBP,ESP
  00478d73  E8 68 B7 00 00 CALL 0x004844e0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478d70
string literals (0):
direct callees (1):
  0x004844E0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 175 B, RET 0
- callers 1: 0x00407920
- twins: none
