# Review cards CAL chunk 2

Answer each card with one TSV row in `steam_db_batches/calibration/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card CAL-2-01

GOG function: CSWGuiListBox::SetPadding  gog 0x0091c970  callconv=__thiscall  param_size_bytes=4  gog_size=128  gog_retn=4
Candidate Steam address: 0x0041f980 (size 676, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0041f7f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091c940 CSWGuiListBox::SetActiveControl  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0091c9f0 CSWGuiListBox::OrganizeControls  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0091c9f0 CSWGuiListBox::OrganizeControls -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x2a4*,0x2a8*,0x2cc*,0x2d0} | Steam {0x4,0x8,0xc,0x10,0x38,0x3c,0x74,0x8c,0x90,0x94,0x10c,0x284,0x29c,0x2a0,...(+5)}
imm sets (* = in both, 1 common): GOG {0x4*,0x8} | Steam {0x2,0x4*,0x5,0xa,0xb,0x1c,0x44,0x94,0x10c,0x2e0}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0041f980: 16-byte aligned: YES
  0041f980  55             PUSH EBP
  0041f981  8B EC          MOV EBP,ESP
  0041f983  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041f980
string literals (0):
direct callees (5):
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x0041E790  (unnamed)
  0x004799C0  (unnamed)
  0x0040FED0  CSWGuiPanel::GetLocalMouseCoords
  0x0041CB50  (unnamed)
struct offsets touched (top 10):
  +0x90 [dec 144]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,EDX
  +0x2AC [dec 684]  x4  via EAX,EDX
  +0x2A8 [dec 680]  x3  via EAX,ECX,EDX
  +0x2CC [dec 716]  x3  via ECX,EDX
  +0xC [dec 12]  x2  via EDX
  +0x8C [dec 140]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0041f980 FUN_0041f980 size=676 ret=4 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0041f980(int *param_1,float param_2)

{
  int iVar1;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = param_1[0xac = byte +0x2b0];
  if (((uint)param_1[0xb3 = byte +0x2cc] >> 5 & 1) != 0) {
    if (((uint)param_1[0xb3 = byte +0x2cc] >> 0xb & 1) != 0) {
      param_1[0xa1 = byte +0x284] = _DAT_009f45fc;
    }
    (**(code **)(param_1[0x43 = byte +0x10c] + 0x38))(param_2);
  }
  (**(code **)(param_1[0x25 = byte +0x94] + 0xc))(param_2);
  iVar1 = FUN_00479460(param_1[0xa7 = byte +0x29c],param_1[0xa8 = byte +0x2a0],param_1[0xa9 = byte +0x2a4],param_1[0xaa = byte +0x2a8],param_1 + 0xb8,0,
                       0x3f800000);
  if (iVar1 != 0) {
    iVar1 = FUN_0041e790();
    if (iVar1 == 0) {
      for (local_8 = 0; local_8 < local_c; local_8 = local_8 + 1) {
        local_10 = *(int *)(*(int *)(param_1[0xab = byte +0x2ac] + local_8 * 4) + 8);
        iVar1 = *(int *)(param_1[0xab = byte +0x2ac] + local_8 * 4);
        local_14 = *(int *)(iVar1 + 8) + *(int *)(iVar1 + 0x10);
        if (((0 < local_14) && (local_10 <= param_1[0xaa = byte +0x2a8])) &&
           ((((uint)param_1[0xb3 = byte +0x2cc] >> 10 & 1) == 0 || ((local_14 < param_1[0xaa = byte +0x2a8] && (-1 < local_10)))
            ))) {
          (**(code **)(**(int **)(param_1[0xab = byte +0x2ac] + local_8 * 4) + 0x38))(param_2);
        }
      }
    }
    else {
      (**(code **)(**(int **)param_1[0xab = byte +0x2ac] + 0x38))(param_2);
    }
    FUN_004799c0();
  }
  if ((*(int **)(DAT_00a1b49c + 0x10) == param_1) && (param_1[0x23 = byte +0x8c] != 0)) {
    local_1c = param_1[0xe = byte +0x38];
    FUN_0040fed0(&local_18,&local_20);
    iVar1 = FUN_0041cb50(local_18,local_20);
    if (iVar1 == param_1[0x1d = byte +0x74]) {
      param_1[0x24 = byte +0x90] = (int)((float)param_1[0x24 = byte +0x90] - param_2);
      while ((float)param_1[0x24 = byte +0x90] < (float)_DAT_00986b38) {
        param_1[0x24 = byte +0x90] = (int)((float)param_1[0x24 = byte +0x90] + (float)_DAT_00987df0);
        (**(code **)(*param_1 + 0x3c))(param_1[0x23 = byte +0x8c],1);
      }
    }
  }
  return;
}
```

### decoy: 0x0041f7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041f7f0: 16-byte aligned: YES
  0041f7f0  55             PUSH EBP
  0041f7f1  8B EC          MOV EBP,ESP
  0041f7f3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041f7f0
string literals (0):
direct callees (3):
  0x0041E800  (unnamed)
  0x0041C7C0  (unnamed)
  0x0091C860  _ftol2_sse
struct offsets touched (top 8):
  +0x2C4 [dec 708]  x4  via EAX,ECX
  +0x2D0 [dec 720]  x4  via EAX,ECX,EDX
  +0x2D2 [dec 722]  x4  via EAX,ECX,EDX
  +0x2A8 [dec 680]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x2A4 [dec 676]  x1  via ECX
  +0x2AC [dec 684]  x1  via EAX
  +0x2C8 [dec 712]  x1  via ECX
```

## Card CAL-2-02

GOG function: CSWSObject::SetAnimation  gog 0x006a8070  callconv=(blank)  param_size_bytes=4  gog_size=35  gog_retn=4
Candidate Steam address: 0x00541390 (size 35, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005413c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a7d60 CSWSObject::ClearAllActions -> 0x00541080  trusted
- GOG-order neighbour after:  0x006a80a0 CSWSObject::GetCurrentHitPoints  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1110 CSWSObject::SetDirtyFlag -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xd8*} | Steam {0xd8*}
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
prologue for 0x00541390: 16-byte aligned: YES
  00541390  55             PUSH EBP
  00541391  8B EC          MOV EBP,ESP
  00541393  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00541390
string literals (0):
direct callees (1):
  0x0054A430  CSWSObject::SetDirtyFlag
struct offsets touched (top 1):
  +0xD8 [dec 216]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00541390 FUN_00541390 size=35 ret=4 callers=1 callees=1

void __thiscall FUN_00541390(int param_1,undefined4 param_2)

{
  FUN_0054a430(0);
  *(undefined4 *)(param_1 + 0xd8) = param_2;
  return;
}
```

### decoy: 0x005413c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005413c0: 16-byte aligned: YES
  005413c0  55             PUSH EBP
  005413c1  8B EC          MOV EBP,ESP
  005413c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005413c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xE0 [dec 224]  x2  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

## Card CAL-2-03

GOG function: CSWGuiPanel::Destructor  gog 0x0090cce0  callconv=__thiscall  param_size_bytes=0  gog_size=276  gog_retn=0
Candidate Steam address: 0x0040e7d0 (size 13, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e7b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090cb30 CSWGuiPanel::Constructor -> 0x0040e5a0  trusted
- GOG-order neighbour after:  0x0090ce00 CSWGuiPanel::SetExtent -> 0x0040e980  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x0090da20 CSWGuiPanel::StopLoadFromLayout -> Steam 0x0040f5a0  in candidate callees: no
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1c,0x28,0x30,0x60} | Steam {}
imm sets (* = in both, 0 common): GOG {0x20,0x24} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040e7d0: 16-byte aligned: YES
  0040e7d0  55             PUSH EBP
  0040e7d1  8B EC          MOV EBP,ESP
  0040e7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e7d0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040e7d0 FUN_0040e7d0 size=13 ret=0 callers=0 callees=0

undefined1 FUN_0040e7d0(void)

{
  return 1;
}
```

### decoy: 0x0040e7b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e7b0: 16-byte aligned: YES
  0040e7b0  55             PUSH EBP
  0040e7b1  8B EC          MOV EBP,ESP
  0040e7b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e7b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x20 [dec 32]  x1  via EAX
```

## Card CAL-2-04

GOG function: CSWSPlaceable::RemoveFromArea  gog 0x0076d320  callconv=(blank)  param_size_bytes=0  gog_size=143  gog_retn=0
Candidate Steam address: 0x005f2f50 (size 143, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005f2a80:STR0/CALLEE0/SIG0.111/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0076ce50 CSWSPlaceable::AddToArea  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0076d3b0 CSWSPlaceable::EventHandler  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0055a8f0 Global::RemoveStaticBoundingBox -> Steam ?  in candidate callees: n/a
  0x006aac30 CSWSObject::SetPosition -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x006ac100 CSWSObject::SetArea -> Steam ?  in candidate callees: n/a
  0x00718240 CSWSArea::RemoveObjectFromArea -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x39c*} | Steam {0x4*,0x39c*}
imm sets (* = in both, 1 common): GOG {0x14*} | Steam {0x14*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005f2f50: 16-byte aligned: YES
  005f2f50  55             PUSH EBP
  005f2f51  8B EC          MOV EBP,ESP
  005f2f53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005f2f50
string literals (0):
direct callees (5):
  0x005453C0  CSWSObject::GetArea
  0x005E8320  Global::RemoveStaticBoundingBox
  0x00523090  CSWSArea::RemoveObjectFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x39C [dec 924]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005f2f50 FUN_005f2f50 size=143 ret=0 callers=3 callees=5

void __fastcall FUN_005f2f50(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    if (-1 < *(int *)(param_1 + 0x39c)) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0(uVar2);
      FUN_005e8320(uVar2);
      *(undefined4 *)(param_1 + 0x39c) = 0xffffffff;
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### decoy: 0x005f2a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005f2a80: 16-byte aligned: YES
  005f2a80  55             PUSH EBP
  005f2a81  8B EC          MOV EBP,ESP
  005f2a83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005f2a80
string literals (3):
  0x00996E64  at 005f2af9  'PreciseUse'
  0x00996E5C  at 005f2b8d  'Hostile'
  0x00996E50  at 005f2c21  'ModelName'
direct callees (19):
  0x005F2F50  CSWSPlaceable::RemoveFromArea
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0071B1D0  (unnamed)
  0x00710810  (unnamed)
  0x00710CA0  (unnamed)
  0x005E9EE0  (unnamed)
  0x00515D50  (unnamed)
  0x005195A0  (unnamed)
  0x005E9A10  (unnamed)
  0x005EEBF0  (unnamed)
  0x005E8120  Global::AddStaticBoundingBox
  0x005453C0  CSWSObject::GetArea
  0x00522BF0  CSWSArea::AddObjectToArea
  0x009196EE  __security_check_cookie
struct offsets touched (top 9):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX
  +0x130 [dec 304]  x3  via ECX
  +0x270 [dec 624]  x3  via EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x394 [dec 916]  x1  via EAX
  +0x398 [dec 920]  x1  via EAX
  +0x39C [dec 924]  x1  via EDX
```

## Card CAL-2-05

GOG function: CClientExoApp::SetTexturePack  gog 0x0040d0d0  callconv=(blank)  param_size_bytes=1  gog_size=31  gog_retn=4
Candidate Steam address: 0x0073fbf0 (size 31, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbd0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d0b0 CClientExoApp::SetVideoMode -> 0x0073fbd0  trusted
- GOG-order neighbour after:  0x0040d0f0 CClientExoApp::GetSoundSetList  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00439c70 CClientExoAppInternal::SetTexturePack -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0073fbf0: 16-byte aligned: YES
  0073fbf0  55             PUSH EBP
  0073fbf1  8B EC          MOV EBP,ESP
  0073fbf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbf0
string literals (0):
direct callees (1):
  0x0078D6A0  CClientExoApp::SetTexturePack (texpacks.2da row: Texture, Gui, DynMemRatio, Mem)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbf0 FUN_0073fbf0 size=31 ret=4 callers=1 callees=1
THUNK -> 0x0078D6A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fbf0(undefined1 param_1)

{
  FUN_0078d6a0(param_1,1);
  return;
}
```

### decoy: 0x0073fbd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fbd0: 16-byte aligned: YES
  0073fbd0  55             PUSH EBP
  0073fbd1  8B EC          MOV EBP,ESP
  0073fbd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fbd0
string literals (0):
direct callees (1):
  0x0078DBF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-2-06

GOG function: CAurTriangleBin::DestroyGrassPolys  gog 0x0089c850  callconv=(blank)  param_size_bytes=0  gog_size=177  gog_retn=0
Candidate Steam address: 0x0050e6e0 (size 203, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0050dac0:STR0/CALLEE0/SIG0.135/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0089c7b0 CAurTriangleBin::BuildGrassPolys  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0089c910 CAurTriangleBin::RenderGrassPolys  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00457b60 CExoArrayList_CSWCMapPin__::Allocate -> Steam ?  in candidate callees: n/a
  0x00843ab0 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2,0x38,0x3c} | Steam {0x4,0x8}
imm sets (* = in both, 3 common): GOG {0x4*,0x8,0xc*,0x58*,0x64} | Steam {0x4*,0xc*,0x14,0x20,0x2c,0x44,0x58*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0050e6e0: 16-byte aligned: YES
  0050e6e0  55             PUSH EBP
  0050e6e1  8B EC          MOV EBP,ESP
  0050e6e3  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0050e6e0
string literals (0):
direct callees (4):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00436200  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x4  via EAX,EDX
  +0x4 [dec 4]  x3  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0050e6e0 FUN_0050e6e0 size=203 ret=0 callers=1 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_0050e6e0(int param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  float10 fVar3;
  undefined1 local_28 [12];
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined1 local_10 [12];
  
  puVar1 = (undefined4 *)FUN_0044c8a0(local_10,param_1 + 0x14,param_1 + 0x20);
  if (_DAT_0098bd68 == (float)_DAT_00986b38) {
    local_1c = *puVar1;
    local_18 = puVar1[1 = byte +0x4];
    local_14 = puVar1[2 = byte +0x8];
  }
  else {
    FUN_00436200(&local_1c,puVar1,1.0 / _DAT_0098bd68);
  }
  *(undefined4 *)(param_1 + 0x2c) = local_1c;
  *(undefined4 *)(param_1 + 0x30) = local_18;
  *(undefined4 *)(param_1 + 0x34) = local_14;
  uVar2 = FUN_004452b0(local_28,param_1 + 0x14,param_1 + 0x20);
  fVar3 = (float10)FUN_005142f0(uVar2);
  *(float *)(param_1 + 8) = (float)(fVar3 / (float10)_DAT_00987490);
  return;
}
```

### decoy: 0x0050dac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0050dac0: 16-byte aligned: YES
  0050dac0  55             PUSH EBP
  0050dac1  8B EC          MOV EBP,ESP
  0050dac3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0050dac0
string literals (2):
  0x009F7B80  at 0050de1d  '!!ARBvp1.0PARAM mvMatrix[4] = { state.matrix.modelview[0] };\\nPARAM mvpMatrix[4] = { state.matrix.mvp };\\nTEMP vTemp;\\nTEMP vReg0, vReg1, vReg2, vReg3;\\nTEMP vReg10;\\nTEMP vNormalWorld;\\nALIAS vFinalColour = vTemp;\\nDP4   result.position.x, mvpMatrix[0], vertex.position;\\nDP4   result.position.y, mvpMatrix[1], vertex.position;\\nDP4   result.position.z, mvpMatrix[2], vertex.position;\\nDP4   result.position.w, mvpMatrix[3], vertex.position;\\nDP4   vReg10.x, mvMatrix[0], vertex.position;\\nDP4   vReg10.y, mvMatrix[1], vertex.position;\\nDP4   vReg10.z, mvMatrix[2], vertex.position;\\nDP4   vReg10.w, mvMatrix[3], vertex.position;\\nDP3   vNormalWorld.x, mvMatrix[0], vertex.normal;\\nDP3   vNormalWorld.y, mvMatrix[1], vertex.normal;\\nDP3   vNormalWorld.z, mvMatrix[2], vertex.normal;\\nDP3\\tvReg0.x, vReg10, vReg10;\\nRSQ\\tvReg0.x, vReg0.x;\\nRCP\\tvReg0.x, vReg0.x;\\nMOV result.fogcoord, vReg0.x;\\nMOV   result.texcoord[0], vertex.texcoord[0];\\nMOV   result.texcoord[1], vertex.texcoord[1];\\nMOV   result.texcoord[2], vertex.texcoord[2];\\nMOV   result.texcoord[3], vertex.texcoord[3];\\nMOV result.color, program.env[15].wwwz;\\n'
  0x00A06AF0  at 0050de29  '!!VP1.0 # This is the standard vertex program.\\nDP4   o[HPOS].x, c[0], v[OPOS];DP4   o[HPOS].y, c[1], v[OPOS];DP4   o[HPOS].z, c[2], v[OPOS];DP4   o[HPOS].w, c[3], v[OPOS];DP4   R10.x, c[8], v[OPOS];DP4   R10.y, c[9], v[OPOS];DP4   R10.z, c[10], v[OPOS];DP4   R10.w, c[11], v[OPOS];DP3   R1.w, R10, R10;RSQ   R1.w, R1.w;RCP   R1.w, R1.w;MOV   o[FOGC], R1.w;DP3   R11.x, c[8], v[NRML];DP3   R11.y, c[9], v[NRML];DP3   R11.z, c[10], v[NRML];MOV R9, c[15].wwwz;MOV   o[TEX0], v[TEX0];MOV   o[TEX1], v[TEX1];MOV   o[TEX2], v[TEX2];MOV   o[TEX3], v[TEX3];MOV o[COL0], c[15].wwwz;'
direct callees (8):
  0x00484480  (unnamed)
  0x00484710  (unnamed)
  0x00510F70  CAurTriangleBin::UpdateGrassForWind
  0x00484550  (unnamed)
  0x00919723  operator new
  0x00506560  (unnamed)
  0x00506640  (unnamed)
  0x004369A0  (unnamed)
struct offsets touched (top 10):
  +0x988658 [dec 9995864]  x7  via EAX,ECX,EDX
  +0x9F4928 [dec 10438952]  x7  via EAX,ECX,EDX
  +0x54 [dec 84]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,EDX
  +0x38 [dec 56]  x4  via ECX,EDX
  +0x58 [dec 88]  x4  via EAX,EDX
  +0x3C [dec 60]  x3  via EAX,ECX
  +0x64 [dec 100]  x3  via EAX,EDX
  +0xB8 [dec 184]  x3  via EAX,EDX
```

## Card CAL-2-07

GOG function: CSWSModule::LoadModuleFinish  gog 0x0072ce00  callconv=__thiscall  param_size_bytes=0  gog_size=390  gog_retn=0
Candidate Steam address: 0x0055a650 (size 390, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (GetTopLevelStruct 0x7189d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0055a7e0:STR0/CALLEE0/SIG0.181/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0072cc10 CSWSModule::LoadModuleInProgress  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0072cf90 CSWSModule::PackModuleIntoMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0042c720 Global::SetFramesPerSecond -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x0061ac10 CRes::Release -> Steam 0x007112b0  in candidate callees: yes
  0x00624b30 CResGFF::GetTopLevelStruct -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064bd00 CServerExoApp::GetSWSMessage -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170  in candidate callees: yes
  0x0064f340 CServerAIMaster::LoadReputationAdjustments -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (1): '30'
disp sets (* = in both, 5 common): GOG {0x8*,0xc*,0x14*,0x20*,0x1f8*} | Steam {0x8*,0xc*,0x14*,0x20*,0x1f8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x11*,0x34*,0x38*} | Steam {0x4*,0x11*,0x34*,0x38*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0055a650: 16-byte aligned: YES
  0055a650  55             PUSH EBP
  0055a651  8B EC          MOV EBP,ESP
  0055a653  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0055a650
string literals (0):
direct callees (11):
  0x007189D0  CResGFF::GetTopLevelStruct
  0x0051C2E0  (unnamed)
  0x00637270  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
  0x0051F920  (unnamed)
  0x007112B0  CRes::Release
  0x007801A0  (unnamed)
struct offsets touched (top 5):
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x20 [dec 32]  x2  via ECX,EDX
  +0xC [dec 12]  x1  via ECX
  +0x1F8 [dec 504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0055a650 FUN_0055a650 size=390 ret=0 callers=1 callees=11

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0055a650(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined2 *local_48;
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00957b5b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_007189d0(local_14);
  uVar5 = *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 0x14) + 0xc);
  *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 0x14) + 8) = uVar5;
  uVar7 = 0xffffffff;
  FUN_0051c2e0(uVar5,0xffffffff,uVar1);
  FUN_00637270(uVar5,uVar7);
  FUN_00623320();
  iVar2 = FUN_00919723(0x34);
  local_8 = 0;
  if (iVar2 == 0) {
    local_48 = (undefined2 *)0x0;
  }
  else {
    local_48 = (undefined2 *)FUN_005e1a60();
  }
  local_8 = 0xffffffff;
  *local_48 = 0x11;
  uVar5 = *(undefined4 *)(param_1 + 0x20);
  uVar7 = *(undefined4 *)(param_1 + 0x20);
  uVar4 = 0;
  uVar3 = 0;
  uVar6 = DAT_0099241c;
  FUN_0051c280(0,0,uVar7,uVar5,DAT_0099241c,local_48);
  FUN_0051e170(uVar3,uVar4,uVar7,uVar5,uVar6,local_48);
  FUN_0051c280();
  FUN_0051f920();
  FUN_007112b0();
  *(undefined4 *)(param_1 + 0x1f8) = 1;
  if (DAT_00a7eb9c != 0) {
    FUN_007801a0(&DAT_00994dc0);
    _DAT_00a1b710 = (float)DAT_00a7eb9c;
    _DAT_009f5e58 = 0;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x0055a7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055a7e0: 16-byte aligned: YES
  0055a7e0  55             PUSH EBP
  0055a7e1  8B EC          MOV EBP,ESP
  0055a7e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0055a7e0
string literals (0):
direct callees (9):
  0x0051C2E0  (unnamed)
  0x00734010  CExoString::CStr
  0x0091D5C0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x005E46A0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00633190  (unnamed)
  0x005E41B0  (unnamed)
  0x005E4630  (unnamed)
struct offsets touched (top 4):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x1C0 [dec 448]  x1  via EAX
```

## Card CAL-2-08

GOG function: CSWSDoor::MoveToNextOpenState  gog 0x00768bf0  callconv=(blank)  param_size_bytes=0  gog_size=167  gog_retn=0
Candidate Steam address: 0x0061d620 (size 179, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSDoor::GetLinkedObject
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0061d6e0:STR0/CALLEE0/SIG0.245/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00768b20 CSWSDoor::SetOpenState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00768ca0 CSWSDoor::SetOrientation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00721af0 CSWSArea::BudgeCreatures -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x31c,0x31d} | Steam {0x8,0x10,0x3e8}
imm sets (* = in both, 0 common): GOG {0x3,0x14,0x94,0x388,0x394} | Steam {0x18,0x3d8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0061d620: 16-byte aligned: YES
  0061d620  55             PUSH EBP
  0061d621  8B EC          MOV EBP,ESP
  0061d623  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061d620
string literals (0):
direct callees (5):
  0x0061D000  CSWSDoor::GetIsLinked
  0x0051C1A0  (unnamed)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0055EB90  (unnamed)
  0x0051C0A0  CServerExoApp::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x3E8 [dec 1000]  x3  via ECX,EDX
  +0x10 [dec 16]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0061d620 FUN_0061d620 size=179 ret=0 callers=6 callees=5

int FUN_0061d620(void)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  int extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_0061d000();
  if (iVar1 != 0) {
    uVar6 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    iVar1 = extraout_var;
    local_8 = FUN_0051c1a0(*(undefined4 *)(extraout_var + 0x3e8 [dec 1000]));
    if (local_8 == 0) {
      uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      uVar4 = 0;
      iVar2 = iVar1 + 0x3d8;
      FUN_0051bd90(iVar2,0);
      uVar4 = FUN_0055eb90(iVar2,uVar4);
      *(undefined4 *)(iVar1 + 0x3e8 [dec 1000]) = uVar4;
      uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
      piVar3 = (int *)FUN_0051c0a0(*(undefined4 *)(iVar1 + 0x3e8 [dec 1000]));
      if (piVar3 != (int *)0x0) {
        local_8 = (**(code **)(*piVar3 + 0x10))(iVar1,uVar4,uVar5,uVar6);
      }
    }
  }
  return local_8;
}
```

### decoy: 0x0061d6e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061d6e0: 16-byte aligned: YES
  0061d6e0  55             PUSH EBP
  0061d6e1  8B EC          MOV EBP,ESP
  0061d6e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0061d6e0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x7C [dec 124]  x4  via EAX,EDX
  +0x31C [dec 796]  x2  via EAX,EDX
  +0x31D [dec 797]  x2  via ECX
  +0x358 [dec 856]  x1  via EAX
```

## Card CAL-2-09

GOG function: CClientExoApp::GetTargetChanging  gog 0x0040e340  callconv=(blank)  param_size_bytes=0  gog_size=57  gog_retn=0
Candidate Steam address: 0x00740e30 (size 32, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::ResetModuleResourcesLoaded
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.42 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740df0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e310 CClientExoApp::GetModuleResourceLoaded  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040e380 CClientExoApp::GetTogglePauseAtEndOfFrame  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x378} | Steam {0x4*,0x4a0}
imm sets (* = in both, 1 common): GOG {0x8*,0x41} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00740e30: 16-byte aligned: YES
  00740e30  55             PUSH EBP
  00740e31  8B EC          MOV EBP,ESP
  00740e33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740e30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x4A0 [dec 1184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740e30 FUN_00740e30 size=32 ret=0 callers=1 callees=0

void __fastcall FUN_00740e30(int param_1)

{
  *(undefined1 *)(*(int *)(param_1 + 4) + 0x4a0) = 0;
  return;
}
```

### decoy: 0x00740df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740df0: 16-byte aligned: YES
  00740df0  55             PUSH EBP
  00740df1  8B EC          MOV EBP,ESP
  00740df3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740df0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4A0 [dec 1184]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-2-10

GOG function: CSWSMessage::HandlePlayerToServerLevelUpMessage  gog 0x008079d0  callconv=(blank)  param_size_bytes=5  gog_size=1059  gog_retn=8
Candidate Steam address: 0x00661c30 (size 1059, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (SendServerToPlayerLevelUp_Confirmation 0x64c450); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00662060:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00807980 CSWSMessage::HandlePlayerToServerServerStatusMessage  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00807e00 CSWSMessage::HandlePlayerToServerJournalMessage  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (23), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x0054ae00 CSWMessage::ReadBYTE -> Steam ?  in candidate callees: n/a
  0x0054ae80 CSWMessage::ReadCHAR -> Steam ?  in candidate callees: n/a
  0x0054af10 CSWMessage::ReadWORD -> Steam ?  in candidate callees: n/a
  0x0054b030 CSWMessage::ReadDWORD -> Steam ?  in candidate callees: n/a
  0x0054b4c0 CSWMessage::MessageReadOverflow -> Steam ?  in candidate callees: n/a
  0x0054b520 CSWMessage::MessageReadUnderflow -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
  ... 15 more GOG callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0xc*,0x28*,0x2a*,0x2b*,0x2c*,0x2d*,0x30*,0x54*,0x55*,0x123*,0x124*,0x1198*,...(+1)} | Steam {0x4*,0x8*,0xc*,0x28*,0x2a*,0x2b*,0x2c*,0x2d*,0x30*,0x54*,0x55*,0x123*,0x124*,0x1198*,...(+1)}
imm sets (* = in both, 12 common): GOG {0x4*,0x8*,0xb*,0xc*,0x10*,0x20*,0x30*,0x7c*,0x238*,0x7f000000*,0x80000003*,0xfffffffc*} | Steam {0x4*,0x8*,0xb*,0xc*,0x10*,0x20*,0x30*,0x7c*,0x238*,0x7f000000*,0x80000003*,0xfffffffc*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00661c30: 16-byte aligned: YES
  00661c30  55             PUSH EBP
  00661c31  8B EC          MOV EBP,ESP
  00661c33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00661c30
string literals (0):
direct callees (23):
  0x005E3DE0  (unnamed)
  0x00632A20  (unnamed)
  0x00919723  operator new
  0x006F58B0  PendingLevelUp::Constructor
  0x005E3720  (unnamed)
  0x006A4E00  CSWSCreatureStats::GetLevel
  0x005E3950  (unnamed)
  0x005E37A0  (unnamed)
  0x006F5B00  PendingLevelUp::SetSkillDelta
  0x005E3830  (unnamed)
  0x006F5B40  PendingLevelUp::AddFeat
  0x0083EA60  CExoArrayList<uint32>::Add
  0x005E3E40  (unnamed)
  0x006B9640  CSWSCreatureStats::ValidateLevelUp
  0x006F59B0  PendingLevelUp::Destructor
  0x009196FD  operator delete
  0x0064C450  CSWSMessage::SendServerToPlayerLevelUp_Confirmation
  0x006B9790  CSWSCreatureStats::CanLevelUp
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
struct offsets touched (top 10):
  +0x1198 [dec 4504]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
  +0x2A [dec 42]  x1  via EDX
  +0x2B [dec 43]  x1  via ECX
  +0x2D [dec 45]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x54 [dec 84]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00661c30 FUN_00661c30 size=1059 ret=8 callers=1 callees=23

/* WARNING: Removing unreachable block (ram,0x00661d2c) */

undefined4 FUN_00661c30(int param_1,char param_2)

{
  bool bVar1;
  undefined1 uVar2;
  undefined2 uVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int local_88;
  int local_38;
  char local_34;
  char local_33;
  byte local_31;
  char local_21;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009595fb;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = 0;
  iVar5 = FUN_005e3de0(1);
  if (iVar5 == 0) {
    if (param_2 == '\0') {
      piVar6 = (int *)FUN_00632a20(uVar4);
      iVar5 = (**(code **)(*piVar6 + 0x30))();
      if (iVar5 == 0) {
        local_14 = 0;
      }
      else {
        iVar7 = FUN_00919723(0x30);
        local_8 = 0;
        if (iVar7 == 0) {
          local_88 = 0;
        }
        else {
          local_88 = FUN_006f58b0();
        }
        local_8 = 0xffffffff;
        uVar2 = FUN_005e3720(8);
        *(undefined1 *)(local_88 + 0x2c) = uVar2;
        uVar4 = FUN_006a4e00(0);
        if (((uVar4 & 0xff) + 1 & 0x80000003) == 0) {
          uVar2 = FUN_005e3720(8);
          *(undefined1 *)(local_88 + 0x2a) = uVar2;
        }
        iVar7 = (uint)*(byte *)(local_88 + 0x2c) * 0x238 + *(int *)(DAT_00a1b4d0 + 0x124);
        *(undefined1 *)(local_88 + 0x2b) = *(undefined1 *)(iVar7 + 0x54);
        *(undefined1 *)(local_88 + 0x2d) = *(undefined1 *)(iVar7 + 0x55);
        uVar4 = FUN_005e3950(0x20);
        for (local_31 = 0; local_31 < *(byte *)(DAT_00a1b4d0 + 0x123); local_31 = local_31 + 1) {
          if ((1 << (local_31 & 0x1f) & uVar4) != 0) {
            uVar2 = FUN_005e37a0(8);
            FUN_006f5b00(local_31,uVar2);
          }
        }
        uVar3 = FUN_005e3830(0x10);
        *(undefined2 *)(local_88 + 0x28) = uVar3;
        for (local_21 = FUN_005e3720(8); local_21 != '\0'; local_21 = local_21 + -1) {
          uVar3 = FUN_005e3830(0x10);
          FUN_006f5b40(uVar3);
        }
        bVar1 = false;
        while (!bVar1) {
          for (local_34 = FUN_005e3720(8); local_34 != '\0'; local_34 = local_34 + -1) {
            uVar8 = FUN_005e3950(0x20);
            FUN_0083ea60(uVar8);
          }
          for (local_33 = FUN_005e3720(8); local_33 != '\0'; local_33 = local_33 + -1) {
/* ... 42 more lines truncated by --max-lines */
```

### decoy: 0x00662060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00662060: 16-byte aligned: YES
  00662060  55             PUSH EBP
  00662061  8B EC          MOV EBP,ESP
  00662063  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00662060
string literals (0):
direct callees (17):
  0x00733540  CExoString::CExoString(empty)
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
  0x0064BC80  (unnamed)
  0x0064DC90  (unnamed)
  0x005E3C90  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x005E3DE0  (unnamed)
  0x005E3E40  (unnamed)
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0055F4A0  (unnamed)
  0x0055EDE0  (unnamed)
  0x005E39D0  (unnamed)
  0x005E36A0  (unnamed)
  0x0055F2E0  (unnamed)
  0x0055F1D0  (unnamed)
struct offsets touched (top 2):
  +0x4C [dec 76]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
```

## Card CAL-2-11

GOG function: Gob::DisableForceAnimations  gog 0x0084c360  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x00459d80 (size 18, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459d60:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c360 Global::DisableForceAnimations -> 0x00459d80  gate_class=REVIEW
- GOG-order neighbour after:  0x0084c380 Gob::IsForceAnimated  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00459d80: 16-byte aligned: YES
  00459d80  55             PUSH EBP
  00459d81  8B EC          MOV EBP,ESP
  00459d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459d80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459d80 FUN_00459d80 size=18 ret=0 callers=0 callees=0

void __fastcall FUN_00459d80(int param_1)

{
  *(undefined1 *)(param_1 + 0x10) = 0;
  return;
}
```

### decoy: 0x00459d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459d60: 16-byte aligned: YES
  00459d60  55             PUSH EBP
  00459d61  8B EC          MOV EBP,ESP
  00459d63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459d60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

## Card CAL-2-12

GOG function: CResGFF::WriteGFFData  gog 0x006267d0  callconv=(blank)  param_size_bytes=8  gog_size=562  gog_retn=8
Candidate Steam address: 0x0071a5a0 (size 198, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CResGFF::WriteGFFFile
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0071a670:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00626700 CResGFF::WriteGFFFile -> 0x0071a5a0  trusted
- GOG-order neighbour after:  0x00626a10 CResGFF::Pack -> 0x0071a8b0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060bac0 CExoFile::Write -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x40,0x44,...(+5)} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4,0x8*,0xc,0x38} | Steam {0x2,0x8*,0x1c}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0071a5a0: 16-byte aligned: YES
  0071a5a0  55             PUSH EBP
  0071a5a1  8B EC          MOV EBP,ESP
  0071a5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0071a5a0
string literals (0):
direct callees (7):
  0x00733570  CExoString::CExoString_char
  0x007397F0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007398E0  (unnamed)
  0x00739880  (unnamed)
  0x0071A8B0  CResGFF::Pack
  0x0071A670  CResGFF::WriteGFFData
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0071a5a0 FUN_0071a5a0 size=198 ret=8 callers=19 callees=7

undefined4 FUN_0071a5a0(undefined4 param_1,undefined2 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  undefined1 local_20 [8];
  undefined1 local_18 [4];
  undefined1 local_14 [4];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095fff0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570(&DAT_0098ba1c);
  local_8 = 0;
  uVar3 = extraout_var;
  FUN_007397f0(param_1,param_2,local_20);
  local_8 = CONCAT31(local_8._1_3_,2);
  FUN_00733780(uVar1,uVar3);
  iVar2 = FUN_007398e0();
  if (iVar2 == 0) {
    local_8 = 0xffffffff;
    FUN_00739880();
    uVar3 = 0;
  }
  else {
    FUN_0071a8b0(0,0);
    uVar3 = FUN_0071a670(local_14,local_18);
    local_8 = 0xffffffff;
    FUN_00739880();
  }
  ExceptionList = local_10;
  return uVar3;
}
```

### decoy: 0x0071a670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0071a670: 16-byte aligned: YES
  0071a670  55             PUSH EBP
  0071a671  8B EC          MOV EBP,ESP
  0071a673  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0071a670
string literals (0):
direct callees (1):
  0x00739A30  (unnamed)
struct offsets touched (top 10):
  +0x40 [dec 64]  x20  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
```

## Card CAL-2-13

GOG function: Gob::DisableRenderGobBBox  gog 0x0084e4f0  callconv=(blank)  param_size_bytes=0  gog_size=23  gog_retn=0
Candidate Steam address: 0x0045bf10 (size 23, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0045bed0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084e4b0 Gob::EnableRenderGobBBox  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084e510 Global::PartHasMaterial  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf4*} | Steam {0xf4*}
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
prologue for 0x0045bf10: 16-byte aligned: YES
  0045bf10  55             PUSH EBP
  0045bf11  8B EC          MOV EBP,ESP
  0045bf13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0045bf10
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF4 [dec 244]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0045bf10 FUN_0045bf10 size=23 ret=0 callers=0 callees=0

void __fastcall FUN_0045bf10(int param_1)

{
  *(undefined2 *)(param_1 + 0xf4) = 0;
  return;
}
```

### decoy: 0x0045bed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0045bed0: 16-byte aligned: YES
  0045bed0  55             PUSH EBP
  0045bed1  8B EC          MOV EBP,ESP
  0045bed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0045bed0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xF4 [dec 244]  x1  via ECX
```

## Card CAL-2-14

GOG function: Gob::SetMotionBlurred  gog 0x0084bf50  callconv=(blank)  param_size_bytes=1  gog_size=25  gog_retn=4
Candidate Steam address: 0x00459970 (size 25, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bf30 Gob::SetRenderPersonalSpace  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bf70 Gob::GetMotionBlurred -> 0x00459990  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x181*} | Steam {0x181*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00459970: 16-byte aligned: YES
  00459970  55             PUSH EBP
  00459971  8B EC          MOV EBP,ESP
  00459973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459970
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x181 [dec 385]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459970 FUN_00459970 size=25 ret=4 callers=0 callees=0

void __thiscall FUN_00459970(int param_1,undefined1 param_2)

{
  *(undefined1 *)(param_1 + 0x181) = param_2;
  return;
}
```

### decoy: 0x00459950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459950: 16-byte aligned: YES
  00459950  55             PUSH EBP
  00459951  8B EC          MOV EBP,ESP
  00459953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459950
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x180 [dec 384]  x1  via EAX
```

## Card CAL-2-15

GOG function: CSWGuiPanel::Constructor  gog 0x0090cb30  callconv=__thiscall  param_size_bytes=4  gog_size=423  gog_retn=4
Candidate Steam address: 0x0040e5a0 (size 423, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (Constructor 0x414a50,Constructor 0x8ba980,Constructor 0x850770); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0040e480:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090ca10 CSWGuiExtent::Union  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090cce0 CSWGuiPanel::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00912e20 CSWGuiBorder::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0xc*,0x18*,0x1c*,0x20*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*} | Steam {0x4*,0x8*,0xc*,0x18*,0x1c*,0x20*,0x30*,0x48*,0x4c*,0x50*,0x60*,0x64*}
imm sets (* = in both, 11 common): GOG {0x2*,0x4*,0x18*,0x24*,0x40*,0x54*,0x78*,0xfffffcff*,0xffffff7f*,0xffffffcf*,0xfffffffd*} | Steam {0x2*,0x4*,0x18*,0x24*,0x40*,0x54*,0x78*,0xfffffcff*,0xffffff7f*,0xffffffcf*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0040e5a0: 16-byte aligned: YES
  0040e5a0  55             PUSH EBP
  0040e5a1  8B EC          MOV EBP,ESP
  0040e5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0040e5a0 FUN_0040e5a0 size=423 ret=4 callers=75 callees=3

undefined4 * __thiscall FUN_0040e5a0(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009482ae;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiObject::vftable;
  param_1[4 = byte +0x10] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[1 = byte +0x4] = 0;
  local_8 = 0;
  *param_1 = CSWGuiPanel::vftable;
  FUN_0084a640(0);
  local_8._0_1_ = 1;
  param_1[0x15 = byte +0x54] = 0;
  param_1[0x16 = byte +0x58] = 0;
  param_1[0x17 = byte +0x5c] = 0;
  iVar2 = FUN_00919723(0x78,uVar1);
  local_8 = CONCAT31(local_8._1_3_,2);
  if (iVar2 == 0) {
    local_28 = 0;
  }
  else {
    local_28 = FUN_00414a50();
  }
  param_1[0x18 = byte +0x60] = local_28;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xfffffffd;
  param_1[8 = byte +0x20] = 0;
  param_1[7 = byte +0x1c] = param_2;
  param_1[0xc = byte +0x30] = 0;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 4;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 0x40;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xffffffcf;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xfffffcff;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] & 0xffffff7f;
  param_1[0x14 = byte +0x50] = 0x3f800000;
  *(undefined1 *)(param_1 + 0x13) = 0;
  param_1[0x12 = byte +0x48] = param_1[0x12 = byte +0x48] | 1;
  *(undefined1 *)(param_1 + 0x19) = 0xff;
  param_1[6 = byte +0x18] = 1;
  ExceptionList = local_10;
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

## Card CAL-2-16

GOG function: CSWCCreature::CreateUsableItemEntry  gog 0x00427c70  callconv=(blank)  param_size_bytes=13  gog_size=1209  gog_retn=16
Candidate Steam address: 0x0077bcf0 (size 1209, retn 16)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR '%s (%s)'@0099b694; '%s (%s) (%d)'@0099f30c; SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077c1b0:STR0/CALLEE0/SIG0.417/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00427b10 CSWCCreature::IsHostileUsableItem  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00428130 CSWCCreature::CreateHostileItemEntry  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam ?  in candidate callees: n/a
  0x0040d070 CClientExoApp::GetSWParty -> Steam ?  in candidate callees: n/a
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam ?  in candidate callees: n/a
  0x004275f0 CSWCCreature::IsLeaderUsableItem -> Steam ?  in candidate callees: n/a
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x004ecc00 CSWItem::GetBaseItem -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG strings (3): '%s (%s)'; '%s (%d)'; '%s (%s) (%d)'
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x30*,0x3c*,0x58*,0x5c*,0xac*,0x2cc*} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x30*,0x3c*,0x58*,0x5c*,0xac*,0x2cc*}
imm sets (* = in both, 14 common): GOG {0x2*,0xa*,0xc*,0x10*,0x14*,0x20*,0x30*,0x3c*,0x7b*,0xcc*,0x2c0*,0x9475*,0x40000000*,0xfffffffe*} | Steam {0x2*,0xa*,0xc*,0x10*,0x14*,0x20*,0x30*,0x3c*,0x7b*,0xcc*,0x2c0*,0x9475*,0x40000000*,0xfffffffe*}

### abi
```
GOG param bytes: 13; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(13/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 13 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0077bcf0: 16-byte aligned: YES
  0077bcf0  55             PUSH EBP
  0077bcf1  8B EC          MOV EBP,ESP
  0077bcf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077bcf0
string literals (3):
  0x0099F30C  at 0077be09  '%s (%s) (%d)'
  0x0099D78C  at 0077be47  '%s (%d)'
  0x0099B694  at 0077beb1  '%s (%s)'
direct callees (19):
  0x0077B670  GuiInGame::IsItemUsableByCurrent
  0x00733540  CExoString::CExoString(empty)
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x007408F0  CClientExoApp::GetGUIString
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x00734190  CExoString::Find_2
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x007FEEC0  CSWCItem::non_virtual_thunk_to_dtor_CSWCItem
  0x0071EBD0  (unnamed)
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007337C0  CExoString::operator=
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x006D6E30  (unnamed)
  0x006042E0  CSWSItem::GetPropertyByType
struct offsets touched (top 9):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX
  +0x2CC [dec 716]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x5C [dec 92]  x2  via EAX,EDX
  +0xAC [dec 172]  x2  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x3C [dec 60]  x1  via ECX
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077bcf0 FUN_0077bcf0 size=1209 ret=16 callers=1 callees=19

void __thiscall FUN_0077bcf0(int param_1,int *param_2,int param_3,int param_4,char param_5)

{
  undefined4 *puVar1;
  uint *puVar2;
  undefined1 uVar3;
  uint uVar4;
  int iVar5;
  undefined4 uVar6;
  undefined1 *puVar7;
  undefined4 uVar8;
  undefined1 local_80 [16];
  undefined1 local_70 [16];
  code *local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  code *local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  int local_30;
  undefined4 local_2c;
  int local_28;
  undefined1 local_24 [8];
  int local_1c;
  uint local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965338;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = param_2[1 = byte +0x4];
  iVar5 = FUN_0077b670(param_3,param_4,param_5);
  if (iVar5 != 0) {
    FUN_00733540(uVar4);
    local_8 = 0;
    uVar8 = 0;
    puVar7 = local_24;
    uVar6 = FUN_0073f350(puVar7,0);
    FUN_007356b0(uVar6,puVar7,uVar8);
    if (*(int *)(param_3 + 0x2cc) < 2) {
      if (param_4 == 0) {
        uVar6 = FUN_00734010();
        FUN_00734270(local_24,&DAT_0098beb4,uVar6);
      }
      else {
        FUN_007408f0(local_40,0x9475);
        local_8._0_1_ = 2;
        uVar6 = FUN_00734010();
        uVar6 = FUN_00734010(uVar6);
        FUN_00734270(local_24,"%s (%s)",uVar6);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
      }
    }
    else if (param_4 == 0) {
      uVar6 = FUN_00734010(*(undefined4 *)(param_3 + 0x2cc));
      FUN_00734270(local_24,"%s (%d)",uVar6);
    }
    else {
      FUN_007408f0(local_38,0x9475);
      local_8._0_1_ = 1;
      uVar6 = FUN_00734010(*(undefined4 *)(param_3 + 0x2cc));
      uVar6 = FUN_00734010(uVar6);
      FUN_00734270(local_24,"%s (%s) (%d)",uVar6);
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
    local_28 = FUN_00734190(0x3c,0);
    local_1c = FUN_00734190(0x7b,0);
    if ((local_28 != -1) || (local_1c != -1)) {
/* ... 50 more lines truncated by --max-lines */
```

### decoy: 0x0077c1b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077c1b0: 16-byte aligned: YES
  0077c1b0  55             PUSH EBP
  0077c1b1  8B EC          MOV EBP,ESP
  0077c1b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0077c1b0
string literals (0):
direct callees (9):
  0x0077BB90  CSWCCreature::IsHostileUsableItem
  0x00733540  CExoString::CExoString(empty)
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007337C0  CExoString::operator=
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x2CC [dec 716]  x1  via ECX
```

## Card CAL-2-17

GOG function: CGuiInGame::NextSWInGameGui  gog 0x004da930  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007c9850 (size 996, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::UnloadStuntModelsForDialog
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.14 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c9c40:STR0/CALLEE0/SIG0.212/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004da760 CGuiInGame::HideSWInGameGui  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004daac0 CGuiInGame::PrevSWInGameGui -> 0x007ca3c0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'k_pend_screenchg'
disp sets (* = in both, 3 common): GOG {0x4*,0x8,0xc*,0x2c*,0x30,0x128} | Steam {0x4*,0xc*,0x18,0x24,0x2c*,0x40,0x4c,0x98,0x9c,0xd8,0x1ac,0x1b0,0x1b4}
imm sets (* = in both, 1 common): GOG {0x2*,0x8,0x1c,0x7f000000} | Steam {0x2*,0x3,0x4,0x78,0xfe,0xff,0x2710}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007c9850: 16-byte aligned: YES
  007c9850  55             PUSH EBP
  007c9851  8B EC          MOV EBP,ESP
  007c9853  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007c9850
string literals (0):
direct callees (8):
  0x0073F4D0  ClientApp::GetClientObject
  0x00733540  CExoString::CExoString(empty)
  0x007ED830  CSWCObject::GetAnimationBase
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00842D00  CSWCPlaceable::GetServerPlaceable
  0x00734010  CExoString::CStr
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x4C [dec 76]  x6  via EDX
  +0x18 [dec 24]  x4  via EDX
  +0x1B0 [dec 432]  x4  via EAX,ECX,EDX
  +0x40 [dec 64]  x3  via EAX,EDX
  +0x1B4 [dec 436]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EDX
  +0x2C [dec 44]  x2  via EDX
  +0x98 [dec 152]  x2  via EDX
  +0x9C [dec 156]  x2  via EAX
  +0x1AC [dec 428]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007c9850 FUN_007c9850 size=996 ret=0 callers=1 callees=8

void __fastcall FUN_007c9850(int param_1)

{
  uint uVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined2 local_5c;
  undefined1 local_4c [8];
  undefined1 local_44 [8];
  undefined1 local_3c [8];
  int local_34;
  int *local_30;
  int *local_2c;
  undefined2 local_28;
  int *local_1c;
  int *local_18;
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00969a70;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  for (local_14 = 0; local_14 < *(uint *)(param_1 + 0x1ac); local_14 = local_14 + 1) {
    local_18 = (int *)FUN_0073f4d0(*(undefined4 *)(*(int *)(param_1 + 0x1b0) + local_14 * 4));
    local_1c = (int *)0x0;
    if (local_18 != (int *)0x0) {
      local_1c = (int *)(**(code **)(*local_18 + 0xc))();
    }
    if (local_1c != (int *)0x0) {
      local_30 = (int *)(**(code **)(*local_1c + 0x98))(0xff,0);
      if (local_30 != (int *)0x0) {
        (**(code **)(*local_30 + 0x18))(0,0x3f800000,0,0);
        (**(code **)(*local_30 + 0x24))();
      }
      local_2c = (int *)(**(code **)(*local_1c + 0x98))(0xfe,0);
      if (local_2c != (int *)0x0) {
        (**(code **)(*local_2c + 0x18))(0,0x3f800000,0,0);
        (**(code **)(*local_2c + 0x24))();
      }
      FUN_00733540();
      local_8 = 0;
      iVar2 = (**(code **)(*local_1c + 0x2c))();
      if (iVar2 != 0) {
        piVar3 = (int *)FUN_007ed830();
        uVar4 = (**(code **)(*piVar3 + 0x40))(local_3c,10000);
        local_8._0_1_ = 1;
        FUN_007337c0(uVar4);
        local_8 = (uint)local_8._1_3_ << 8;
        FUN_00733780();
        local_28 = 10000;
      }
      iVar2 = (**(code **)(*local_1c + 0x4c))();
      if (iVar2 != 0) {
        (**(code **)(*local_1c + 0x4c))();
        local_34 = FUN_00842d00();
        if (local_34 == 0) {
          (**(code **)(*local_1c + 0x4c))();
          piVar3 = (int *)FUN_007ed830();
          uVar4 = (**(code **)(*piVar3 + 0x40))(local_4c,10000);
          local_8._0_1_ = 3;
          FUN_007337c0(uVar4);
          local_8 = (uint)local_8._1_3_ << 8;
          FUN_00733780();
          local_28 = 10000;
        }
        else {
          local_5c = (undefined2)*(undefined4 *)(local_34 + 0xd8);
          local_28 = local_5c;
          (**(code **)(*local_1c + 0x4c))();
          piVar3 = (int *)FUN_007ed830();
          uVar4 = (**(code **)(*piVar3 + 0x40))(local_44,local_28);
          local_8._0_1_ = 2;
          FUN_007337c0(uVar4);
          local_8 = (uint)local_8._1_3_ << 8;
/* ... 36 more lines truncated by --max-lines */
```

### decoy: 0x007c9c40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c9c40: 16-byte aligned: YES
  007c9c40  55             PUSH EBP
  007c9c41  8B EC          MOV EBP,ESP
  007c9c43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c9c40
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00410670  (unnamed)
  0x0075BAA0  (unnamed)
struct offsets touched (top 6):
  +0x38 [dec 56]  x10  via EAX,EDX
  +0x34 [dec 52]  x8  via EAX,ECX,EDX
  +0x7C [dec 124]  x6  via EAX,EDX
  +0x98 [dec 152]  x5  via EAX,ECX
  +0x128 [dec 296]  x1  via ECX
  +0xF1C [dec 3868]  x1  via ECX
```

## Card CAL-2-18

GOG function: CClientExoAppInternal::SetMouseCursor  gog 0x00460480  callconv=(blank)  param_size_bytes=12  gog_size=86  gog_retn=12
Candidate Steam address: 0x007b3870 (size 86, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (HandleMouseOver 0x7b3a00)
- best decoy (neighbour/other candidate) scores: 0x007b38d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00460460 CClientExoAppInternal::PerformZMouseMoveAction  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004604e0 CClientExoAppInternal::HideMouse  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00616850 CExoInput::SetMouseCursor -> Steam ?  in candidate callees: n/a
  0x00911f00 CSWGuiManager::SetSoftwareMouseCursor -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x274*} | Steam {0x274*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007b3870: 16-byte aligned: YES
  007b3870  55             PUSH EBP
  007b3871  8B EC          MOV EBP,ESP
  007b3873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007b3870
string literals (0):
direct callees (2):
  0x0072FC50  (unnamed)
  0x00413A80  (unnamed)
struct offsets touched (top 1):
  +0x274 [dec 628]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007b3870 FUN_007b3870 size=86 ret=12 callers=2 callees=2

void __thiscall FUN_007b3870(int param_1,uint *param_2,uint *param_3,uint *param_4)

{
  uint *extraout_EAX;
  
  if (DAT_00a1b48c != (SchedulerResourceManagement *)0x0) {
    Concurrency::details::SchedulerResourceManagement::Statistics
              (DAT_00a1b48c,param_2,param_3,param_4);
    param_2 = extraout_EAX;
  }
  if (*(int *)(param_1 + 0x274) != 0) {
    FUN_00413a80(param_2,param_3,param_4);
  }
  return;
}
```

### decoy: 0x007b38d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007b38d0: 16-byte aligned: YES
  007b38d0  55             PUSH EBP
  007b38d1  8B EC          MOV EBP,ESP
  007b38d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007b38d0
string literals (0):
direct callees (2):
  0x0072FCB0  (unnamed)
  0x00413A40  (unnamed)
struct offsets touched (top 2):
  +0x3D8 [dec 984]  x3  via EAX,EDX
  +0x274 [dec 628]  x2  via ECX,EDX
```

## Card CAL-2-19

GOG function: CResGFF::WriteFieldBYTE  gog 0x00625b30  callconv=(blank)  param_size_bytes=12  gog_size=97  gog_retn=12
Candidate Steam address: 0x007199d0 (size 97, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00719a40:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625aa0 CResGFF::AddStructToStruct -> 0x00719940  trusted
- GOG-order neighbour after:  0x00625ba0 CResGFF::WriteFieldCHAR -> 0x00719a40  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00623c00 CResGFF::InitializeForWriting -> Steam ?  in candidate callees: n/a
  0x00624190 CResGFF::AddField -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x98*} | Steam {0x8*,0x98*}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007199d0: 16-byte aligned: YES
  007199d0  55             PUSH EBP
  007199d1  8B EC          MOV EBP,ESP
  007199d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007199d0
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007199d0 FUN_007199d0 size=97 ret=12 callers=35 callees=2

undefined4 __thiscall FUN_007199d0(int param_1,int param_2,byte param_3,int param_4)

{
  undefined4 uVar1;
  int iVar2;
  
  if ((param_2 == 0) || (param_4 == 0)) {
    uVar1 = 0;
  }
  else {
    if (*(int *)(param_1 + 0x98) != 0) {
      FUN_00717aa0();
    }
    iVar2 = FUN_00718030(param_2,param_4,0);
    if (iVar2 == 0) {
      uVar1 = 0;
    }
    else {
      *(uint *)(iVar2 + 8) = (uint)param_3;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00719a40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00719a40: 16-byte aligned: YES
  00719a40  55             PUSH EBP
  00719a41  8B EC          MOV EBP,ESP
  00719a43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00719a40
string literals (0):
direct callees (2):
  0x00717AA0  CResGFF::InitializeForWriting
  0x00718030  CResGFF::AddField
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via ECX
  +0x98 [dec 152]  x1  via EAX
```

## Card CAL-2-20

GOG function: Gob::GetName  gog 0x0084bcd0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x004596f0 (size 17, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459670:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084bc50 CAurObjectLightContainer::`scalar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bcf0 Gob::GetPosition -> 0x00459710  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x64*} | Steam {0x64*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004596f0: 16-byte aligned: YES
  004596f0  55             PUSH EBP
  004596f1  8B EC          MOV EBP,ESP
  004596f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004596f0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004596f0 FUN_004596f0 size=17 ret=0 callers=0 callees=0

int __fastcall FUN_004596f0(int param_1)

{
  return param_1 + 0x64 [dec 100];
}
```

### decoy: 0x00459670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459670: 16-byte aligned: YES
  00459670  55             PUSH EBP
  00459671  8B EC          MOV EBP,ESP
  00459673  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00459670
string literals (0):
direct callees (3):
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x00463B20  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-2-21

GOG function: CSWCObject::AnimationHideEquippedItems  gog 0x00474540  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef910 (size 129, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004744b0 CSWCObject::AnimationPlayOutOfPlace  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004745d0 CSWCObject::AnimationWalking  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xcc*,0xe0*,0x130*} | Steam {0x68*,0xcc*,0xe0*,0x130*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef910: 16-byte aligned: YES
  007ef910  55             PUSH EBP
  007ef911  8B EC          MOV EBP,ESP
  007ef913  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef910
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef910 FUN_007ef910 size=129 ret=4 callers=3 callees=1

bool __thiscall FUN_007ef910(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe50,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef880
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card CAL-2-22

GOG function: CExoString::Format  gog 0x00606380  callconv=(blank)  param_size_bytes=9  gog_size=380  gog_retn=0
Candidate Steam address: 0x005160c0 (size 362, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00515e60:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006062a0 CExoString::Find_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00606500 CExoString::Insert  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00924d20 unnamed -> Steam ?  in candidate callees: n/a
  0x00927cad unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0xc}
imm sets (* = in both, 0 common): GOG {0x4,0xc,0x10,0x1c,0x400} | Steam {0x8,0x28,0x41}

### abi
```
GOG param bytes: 9; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(9/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 9 bytes -> x86 slots 12; ASM RET 0)
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

## Card CAL-2-23

GOG function: CSWSMessage::SendServerToPlayerLevelUp_Confirmation  gog 0x007e2b70  callconv=(blank)  param_size_bytes=8  gog_size=61  gog_retn=8
Candidate Steam address: 0x0064c450 (size 61, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (HandlePlayerToServerLevelUpMessage 0x661c30); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0064c490:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007e2ae0 CSWSMessage::SendServerToPlayerJournalUpdated  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007e2bb0 CSWSMessage::SendServerToPlayerDialogReplyChosen  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007dd770 CSWSMessage::SendServerToPlayerMessage -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x1d*} | Steam {0x2*,0x8*,0x1d*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0064c450: 16-byte aligned: YES
  0064c450  55             PUSH EBP
  0064c451  8B EC          MOV EBP,ESP
  0064c453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0064c450
string literals (0):
direct callees (1):
  0x00647050  CSWSMessage::SendServerToPlayerMessage (builds 0x50,major,minor)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0064c450 FUN_0064c450 size=61 ret=8 callers=1 callees=1

void FUN_0064c450(undefined4 param_1,int param_2)

{
  if (param_2 == 0) {
    FUN_00647050(param_1,0x1d,2,0,0);
  }
  else {
    FUN_00647050(param_1,0x1d,1,0,0);
  }
  return;
}
```

### decoy: 0x0064c490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0064c490: 16-byte aligned: YES
  0064c490  55             PUSH EBP
  0064c491  8B EC          MOV EBP,ESP
  0064c493  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0064c490
string literals (0):
direct callees (5):
  0x005E3F20  (unnamed)
  0x005E41B0  (unnamed)
  0x005E43B0  (unnamed)
  0x005E47E0  (unnamed)
  0x00647050  CSWSMessage::SendServerToPlayerMessage (builds 0x50,major,minor)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-2-24

GOG function: CClientExoApp::SetCombatMode  gog 0x0040dd40  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x00740880 (size 28, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007408a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040dd20 CClientExoApp::GetCombatMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040dd60 CClientExoApp::PlayerFlourishWeapons  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00449ce0 CClientExoAppInternal::SetCombatMode -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x00740880: 16-byte aligned: YES
  00740880  55             PUSH EBP
  00740881  8B EC          MOV EBP,ESP
  00740883  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740880
string literals (0):
direct callees (1):
  0x0079D6E0  CClientExoAppInternal::SetCombatMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740880 FUN_00740880 size=28 ret=4 callers=3 callees=1
THUNK -> 0x0079D6E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00740880(undefined4 param_1)

{
  FUN_0079d6e0(param_1);
  return;
}
```

### decoy: 0x007408a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007408a0: 16-byte aligned: YES
  007408a0  55             PUSH EBP
  007408a1  8B EC          MOV EBP,ESP
  007408a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007408a0
string literals (0):
direct callees (1):
  0x0079F430  CClientExoAppInternal::PlayerFlourishWeapons
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-2-25

GOG function: Scene::SetFog  gog 0x00857010  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00464c30 (size 22, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464bf0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00856fd0 Global::CAurScene_scalar_deleting_destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00857030 Scene::GetFogRange  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x60*} | Steam {0x60*}
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
prologue for 0x00464c30: 16-byte aligned: YES
  00464c30  55             PUSH EBP
  00464c31  8B EC          MOV EBP,ESP
  00464c33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464c30
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x60 [dec 96]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00464c30 FUN_00464c30 size=22 ret=4 callers=0 callees=0

void __thiscall FUN_00464c30(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x60) = param_2;
  return;
}
```

### decoy: 0x00464bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464bf0: 16-byte aligned: YES
  00464bf0  55             PUSH EBP
  00464bf1  8B EC          MOV EBP,ESP
  00464bf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464bf0
string literals (0):
direct callees (2):
  0x00473EB0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card CAL-2-26

GOG function: Gob::SetOrientation  gog 0x00853a70  callconv=(blank)  param_size_bytes=20  gog_size=80  gog_retn=20
Candidate Steam address: 0x00460c20 (size 552, retn 20)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00460bb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00853a20 Gob::SetPosition -> 0x00461490  trusted
- GOG-order neighbour after:  0x00853ac0 Gob::GetBehavior  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*,0x8,0xc} | Steam {0x4*,0x44,0x98,0x1d4}
imm sets (* = in both, 1 common): GOG {0x14*,0xb0} | Steam {0x8,0xc,0x14*,0x44c}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00460c20: 16-byte aligned: YES
  00460c20  55             PUSH EBP
  00460c21  8B EC          MOV EBP,ESP
  00460c23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00460c20
string literals (0):
direct callees (8):
  0x0091C90B  (unnamed)
  0x00463ED0  (unnamed)
  0x00460BB0  (unnamed)
  0x0044B350  (unnamed)
  0x004DAFC0  (unnamed)
  0x00422E20  CAurTexture::SetIsEnvironmentMapped (SetFlagDE)
  0x00461ED0  Gob::SetProcessFlag
  0x00463B20  (unnamed)
struct offsets touched (top 4):
  +0x44 [dec 68]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x98 [dec 152]  x1  via ECX
  +0x1D4 [dec 468]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00460c20 FUN_00460c20 size=552 ret=20 callers=0 callees=8

undefined4 __thiscall
FUN_00460c20(int param_1,char *param_2,undefined4 param_3,int param_4,int param_5)

{
  uint uVar1;
  undefined4 uVar2;
  int local_42c;
  uint local_428[257 = byte +0x404];
  char *local_24;
  undefined1 local_20 [4];
  int local_1c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949fc8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (DAT_00a1ff8c != (FILE *)0x0) {
    _fprintf [GHIDRA-AUTO-LABEL: unreliable](DAT_00a1ff8c,"%s\n",param_3,uVar1);
    for (local_42c = 0; local_42c < param_4; local_42c = local_42c + 1) {
      _fprintf [GHIDRA-AUTO-LABEL: unreliable](DAT_00a1ff8c,"%i ",(uint)*(ushort *)(param_5 + local_42c * 2),uVar1);
    }
    _fprintf [GHIDRA-AUTO-LABEL: unreliable](DAT_00a1ff8c,"\n");
  }
  for (local_14 = 0; local_14 < param_4; local_14 = local_14 + 1) {
    local_428[local_14] = (uint)*(ushort *)(param_5 + local_14 * 2);
  }
  local_24 = (char *)0x0;
  if ((param_2 != (char *)0x0) && (*param_2 != '\0')) {
    local_24 = param_2;
  }
  FUN_00463ed0(0);
  local_8 = 0;
  FUN_00460bb0(*(undefined4 *)(param_1 + 0x98),local_20);
  for (local_14 = 0; local_14 < local_1c; local_14 = local_14 + 1) {
    uVar2 = FUN_0044b350();
    FUN_004dafc0(local_24,param_3,uVar2,local_428,param_4);
    if (*(int *)(param_1 + 0x1d4) != 0) {
      FUN_00422e20(1);
    }
  }
  FUN_00461ed0();
  local_8 = 0xffffffff;
  FUN_00463b20();
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x00460bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00460bb0: 16-byte aligned: YES
  00460bb0  55             PUSH EBP
  00460bb1  8B EC          MOV EBP,ESP
  00460bb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00460bb0
string literals (0):
direct callees (2):
  0x00473F40  vector<T*>::push_back (`ret 4`)
  0x00460BB0  (unnamed)
struct offsets touched (top 3):
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
```

## Card CAL-2-27

GOG function: CClientExoApp::GetCharacterChangeInProgress  gog 0x0040e480  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x00740fc0 (size 31, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GetCharacterChangeInProgress
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (ShowLevelUpGUI 0x84fcd0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00740fe0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e460 CClientExoApp::ShowMouse  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040e4a0 CClientExoApp::SetCharacterChangeInProgress  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x4ac*} | Steam {0x4*,0x4ac*}
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
prologue for 0x00740fc0: 16-byte aligned: YES
  00740fc0  55             PUSH EBP
  00740fc1  8B EC          MOV EBP,ESP
  00740fc3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740fc0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x4AC [dec 1196]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00740fc0 FUN_00740fc0 size=31 ret=0 callers=3 callees=0

undefined4 __fastcall FUN_00740fc0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x4ac);
}
```

### decoy: 0x00740fe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740fe0: 16-byte aligned: YES
  00740fe0  55             PUSH EBP
  00740fe1  8B EC          MOV EBP,ESP
  00740fe3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740fe0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x4AC [dec 1196]  x1  via EDX
```

## Card CAL-2-28

GOG function: CClientExoApp::SendLoadGameRequest  gog 0x0040cbe0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x0073f720 (size 36, retn 12)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073f750:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040cbb0 CClientExoApp::SendSaveGameRequest  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040cc10 CClientExoApp::GetInGameGui  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00431c80 CClientExoAppInternal::SendLoadGameRequest -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0073f720: 16-byte aligned: YES
  0073f720  55             PUSH EBP
  0073f721  8B EC          MOV EBP,ESP
  0073f723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f720
string literals (0):
direct callees (1):
  0x00785660  CClientExoAppInternal::SendLoadGameRequest
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f720 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x00785660 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00785660(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0073f750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f750: 16-byte aligned: YES
  0073f750  55             PUSH EBP
  0073f751  8B EC          MOV EBP,ESP
  0073f753  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via ECX
```

## Card CAL-2-29

GOG function: Gob::AddAttachment  gog 0x008557b0  callconv=(blank)  param_size_bytes=4  gog_size=55  gog_retn=4
Candidate Steam address: 0x004632c0 (size 55, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00463300:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008556b0 Gob::CollectLights_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008557f0 Gob::RemoveAttachment  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00873110 List_T__::Count -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1e8*} | Steam {0x4*,0x1e8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004632c0: 16-byte aligned: YES
  004632c0  55             PUSH EBP
  004632c1  8B EC          MOV EBP,ESP
  004632c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004632c0
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004632c0 FUN_004632c0 size=55 ret=4 callers=1 callees=2

void FUN_004632c0(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_00476620(param_1);
  if (iVar1 == 0) {
    FUN_00473f40(param_1);
  }
  return;
}
```

### decoy: 0x00463300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463300: 16-byte aligned: YES
  00463300  55             PUSH EBP
  00463301  8B EC          MOV EBP,ESP
  00463303  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00463300
string literals (0):
direct callees (2):
  0x00476620  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 0):
```

## Card CAL-2-30

GOG function: CResGFF::ReadFieldQuaternion  gog 0x00625530  callconv=(blank)  param_size_bytes=20  gog_size=326  gog_retn=20
Candidate Steam address: 0x00718e40 (size 336, retn 20)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CResGFF::ReadFieldCResRef
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.60 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.60
- best decoy (neighbour/other candidate) scores: 0x00718d80:STR0/CALLEE0/SIG0.267/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00625240 CResGFF::ReadFieldCExoLocString -> 0x007190e0  trusted
- GOG-order neighbour after:  0x00625680 CResGFF::ReadFieldVector -> 0x00719520  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006238d0 CResGFF::GetField -> Steam ?  in candidate callees: n/a
  0x00623980 CResGFF::GetDataField -> Steam ?  in candidate callees: n/a
  0x00623a40 CResGFF::GetFieldByLabel -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x10,0x14*,0x24} | Steam {0xb,0x14*,0x18}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x00718e40: 16-byte aligned: YES
  00718e40  55             PUSH EBP
  00718e41  8B EC          MOV EBP,ESP
  00718e43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00718e40
string literals (0):
direct callees (4):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
  0x007108A0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00718e40 FUN_00718e40 size=336 ret=20 callers=37 callees=4

undefined4 *
FUN_00718e40(undefined4 *param_1,int param_2,int param_3,undefined4 *param_4,undefined4 *param_5)

{
  undefined4 uVar1;
  uint local_14;
  byte *local_10;
  int *local_c;
  byte local_5;
  
  if ((param_2 == 0) || (param_3 == 0)) {
    *param_4 = 0;
    *param_1 = *param_5;
    param_1[1 = byte +0x4] = param_5[1 = byte +0x4];
    param_1[2 = byte +0x8] = param_5[2 = byte +0x8];
    param_1[3 = byte +0xc] = param_5[3 = byte +0xc];
  }
  else {
    uVar1 = FUN_007178e0(param_2,param_3);
    local_c = (int *)FUN_00717770(param_2,uVar1);
    if ((local_c == (int *)0x0) || (*local_c != 0xb)) {
      *param_4 = 0;
      *param_1 = *param_5;
      param_1[1 = byte +0x4] = param_5[1 = byte +0x4];
      param_1[2 = byte +0x8] = param_5[2 = byte +0x8];
      param_1[3 = byte +0xc] = param_5[3 = byte +0xc];
    }
    else {
      local_10 = (byte *)FUN_00717820(local_c,&local_14);
      if ((local_10 == (byte *)0x0) || (local_14 == 0)) {
        *param_4 = 0;
        *param_1 = *param_5;
        param_1[1 = byte +0x4] = param_5[1 = byte +0x4];
        param_1[2 = byte +0x8] = param_5[2 = byte +0x8];
        param_1[3 = byte +0xc] = param_5[3 = byte +0xc];
      }
      else {
        local_5 = *local_10;
        if (local_14 < local_5 + 1) {
          *param_4 = 0;
          *param_1 = *param_5;
          param_1[1 = byte +0x4] = param_5[1 = byte +0x4];
          param_1[2 = byte +0x8] = param_5[2 = byte +0x8];
          param_1[3 = byte +0xc] = param_5[3 = byte +0xc];
        }
        else {
          *param_4 = 1;
          FUN_007108a0(local_10 + 1,local_5);
        }
      }
    }
  }
  return param_1;
}
```

### decoy: 0x00718d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00718d80: 16-byte aligned: YES
  00718d80  55             PUSH EBP
  00718d81  8B EC          MOV EBP,ESP
  00718d83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00718d80
string literals (0):
direct callees (3):
  0x007178E0  CResGFF::GetFieldByLabel
  0x00717770  CResGFF::GetField
  0x00717820  CResGFF::GetDataField
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card CAL-2-31

GOG function: CSWGuiListBox::SetExtent  gog 0x0091b720  callconv=__thiscall  param_size_bytes=4  gog_size=241  gog_retn=4
Candidate Steam address: 0x0041d710 (size 241, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (GetInnerExtent 0x414d80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0041d810:STR0/CALLEE0/SIG0.393/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091b5a0 CSWGuiListBox::InitializeScrollBar  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0091b820 CSWGuiListBox::SetScrollBarExtent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00913150 CSWGuiBorder::GetInnerExtent -> Steam ?  in candidate callees: n/a
  0x0091b820 CSWGuiListBox::SetScrollBarExtent -> Steam ?  in candidate callees: n/a
  0x0091c9f0 CSWGuiListBox::OrganizeControls -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x94*,0x118*,0x2cc*} | Steam {0x4*,0x8*,0xc*,0x94*,0x118*,0x2cc*}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x28*,0x94*,0x29c*} | Steam {0x4*,0x8*,0x28*,0x94*,0x29c*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0041d710: 16-byte aligned: YES
  0041d710  55             PUSH EBP
  0041d711  8B EC          MOV EBP,ESP
  0041d713  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041d710
string literals (0):
direct callees (3):
  0x00414D80  CSWGuiBorder::GetInnerExtent
  0x0041D810  CSWGuiListBox::LayoutScrollBar (SPECULATIVE name)
  0x0041EA50  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x6  via EAX,ECX
  +0x8 [dec 8]  x5  via EAX,ECX
  +0xC [dec 12]  x5  via EAX,ECX
  +0x2CC [dec 716]  x3  via EAX,ECX,EDX
  +0x94 [dec 148]  x1  via EDX
  +0x118 [dec 280]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0041d710 FUN_0041d710 size=241 ret=4 callers=0 callees=3

void __thiscall FUN_0041d710(int param_1,undefined4 *param_2)

{
  undefined4 *puVar1;
  undefined1 local_28 [16];
  int local_18;
  int local_14;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  
  *(undefined4 *)(param_1 + 4) = *param_2;
  *(undefined4 *)(param_1 + 8) = param_2[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0xc) = param_2[2 = byte +0x8];
  *(undefined4 *)(param_1 + 0x10) = param_2[3 = byte +0xc];
  local_14 = *(int *)(param_1 + 4);
  local_10 = *(undefined4 *)(param_1 + 8);
  local_8 = *(undefined4 *)(param_1 + 0x10);
  local_18 = *(int *)(param_1 + 0x118);
  if ((*(uint *)(param_1 + 0x2cc) >> 4 & 1) != 0) {
    local_14 = local_14 + local_18;
  }
  local_c = *(int *)(param_1 + 0xc) - local_18;
  (**(code **)(*(int *)(param_1 + 0x94) + 4))(&local_14);
  puVar1 = (undefined4 *)FUN_00414d80(local_28);
  *(undefined4 *)(param_1 + 0x29c) = *puVar1;
  *(undefined4 *)(param_1 + 0x2a0) = puVar1[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0x2a4) = puVar1[2 = byte +0x8];
  *(undefined4 *)(param_1 + 0x2a8) = puVar1[3 = byte +0xc];
  FUN_0041d810(local_18,0);
  *(uint *)(param_1 + 0x2cc) = *(uint *)(param_1 + 0x2cc) | 8;
  FUN_0041ea50();
  return;
}
```

### decoy: 0x0041d810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041d810: 16-byte aligned: YES
  0041d810  55             PUSH EBP
  0041d811  8B EC          MOV EBP,ESP
  0041d813  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041d810
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x4 [dec 4]  x4  via EAX,EDX
  +0xC [dec 12]  x4  via EAX,ECX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via EDX
  +0x10C [dec 268]  x1  via EAX
  +0x2CC [dec 716]  x1  via ECX
```

## Card CAL-2-32

GOG function: CGuiInGame::PrevSWInGameGui  gog 0x004daac0  callconv=(blank)  param_size_bytes=0  gog_size=389  gog_retn=0
Candidate Steam address: 0x007ca3c0 (size 389, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ca230:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004da930 CGuiInGame::NextSWInGameGui  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004dac50 CGuiInGame::SwitchToSWInGameGui  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004e0e60 CGuiInGame::UpdateCreatedInGameGUI -> Steam ?  in candidate callees: n/a
  0x00527f60 CSWGuiInGameMenu::SetActiveControlID -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG strings (1): 'k_pend_screenchg'
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*} | Steam {0x4*,0x8*,0xc*,0x2c*,0x30*,0x128*}
imm sets (* = in both, 4 common): GOG {0x2*,0x7*,0x1c*,0x7f000000*} | Steam {0x2*,0x7*,0x1c*,0x7f000000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ca3c0: 16-byte aligned: YES
  007ca3c0  55             PUSH EBP
  007ca3c1  8B EC          MOV EBP,ESP
  007ca3c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ca3c0
string literals (1):
  0x009A175C  at 007ca4fd  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ca3c0 FUN_007ca3c0 size=389 ret=0 callers=1 callees=9

void __fastcall FUN_007ca3c0(int param_1)

{
  undefined4 local_2c;
  undefined1 local_20 [8];
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096fe28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x128) != 0) {
    local_18 = FUN_0073fea0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    if (((DAT_00a10760 != 0) && (*(int *)(param_1 + 0x30) != 0)) && (local_18 != 0)) {
      FUN_00410670(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
    }
    local_14 = *(undefined4 *)(param_1 + 0x2c);
    *(int *)(param_1 + 0x2c) = *(int *)(param_1 + 0x2c) + -1;
    if (*(int *)(param_1 + 0x2c) < 0) {
      *(undefined4 *)(param_1 + 0x2c) = 7;
    }
    if ((*(int *)(param_1 + 0x30) == 0) || (local_18 == 0)) {
      local_2c = 0xffffffff;
    }
    else {
      local_2c = *(undefined4 *)(param_1 + 0x2c);
    }
    FUN_007d0760(local_14,local_2c);
    if ((*(int *)(param_1 + 0x30) != 0) && (local_18 != 0)) {
      FUN_00410530(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4),2,1);
      FUN_00410780(*(undefined4 *)(param_1 + 0xc + *(int *)(param_1 + 0x2c) * 4));
      FUN_00756860(*(undefined4 *)(param_1 + 0x2c),1);
      if (DAT_00a1b4a8 != 0) {
        FUN_00733570("k_pend_screenchg");
        local_8 = 0;
        FUN_006fd8d0(local_20,0x7f000000,0);
        local_8 = 0xffffffff;
        FUN_00733780();
      }
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007ca230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ca230: 16-byte aligned: YES
  007ca230  55             PUSH EBP
  007ca231  8B EC          MOV EBP,ESP
  007ca233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ca230
string literals (1):
  0x009A175C  at 007ca36d  'k_pend_screenchg'
direct callees (9):
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x00410670  (unnamed)
  0x007D0760  CGuiInGame::UpdateCreatedInGameGUI
  0x00410530  CSWGuiManager::AddPanel
  0x00410780  (unnamed)
  0x00756860  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x006FD8D0  CVirtualMachine::RunScript
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x2C [dec 44]  x10  via EAX,ECX,EDX
  +0x30 [dec 48]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x128 [dec 296]  x1  via EAX
```

## Card CAL-2-33

GOG function: CServerExoApp::GetLoadFromSaveGame  gog 0x0064c800  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cde0 (size 31, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ce00:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c7d0 CServerExoApp::ComputeSafeLocation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064c820 CServerExoApp::SetLoadFromSaveGame -> 0x0051ce00  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051cde0: 16-byte aligned: YES
  0051cde0  55             PUSH EBP
  0051cde1  8B EC          MOV EBP,ESP
  0051cde3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cde0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cde0 FUN_0051cde0 size=31 ret=0 callers=5 callees=0

undefined4 __fastcall FUN_0051cde0(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x100c4);
}
```

### decoy: 0x0051ce00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce00: 16-byte aligned: YES
  0051ce00  55             PUSH EBP
  0051ce01  8B EC          MOV EBP,ESP
  0051ce03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via EDX
```

## Card CAL-2-34

GOG function: CResTPC::GetTPCAttrib  gog 0x00944400  callconv=(blank)  param_size_bytes=32  gog_size=192  gog_retn=32
Candidate Steam address: 0x00945db0 (size 192, retn 32)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00945e70:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009443e0 CResTPC::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009444c0 CResTPC::GetTPCSize  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xa*,0xc*,0xd*,0x30*,0x34*,0x38*,0x40*} | Steam {0x4*,0x8*,0xa*,0xc*,0xd*,0x30*,0x34*,0x38*,0x40*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3*,0x4*,0x20*} | Steam {0x2*,0x3*,0x4*,0x20*}

### abi
```
GOG param bytes: 32; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(32/4)*4 = 32 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=32 ; asm RET = 32
Stack Slot Match: TRUE (GOG 32 bytes -> x86 slots 32; ASM RET 32)
```

### prologue
```
prologue for 0x00945db0: 16-byte aligned: YES
  00945db0  55             PUSH EBP
  00945db1  8B EC          MOV EBP,ESP
  00945db3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00945db0
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x40 [dec 64]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xA [dec 10]  x1  via EAX
  +0xD [dec 13]  x1  via ECX
  +0x30 [dec 48]  x1  via EDX
  +0x34 [dec 52]  x1  via EDX
  +0x38 [dec 56]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00945db0 FUN_00945db0 size=192 ret=32 callers=2 callees=0

undefined4 __thiscall
FUN_00945db0(int param_1,uint *param_2,uint *param_3,undefined1 *param_4,undefined4 *param_5,
            undefined4 *param_6,undefined4 *param_7,undefined4 *param_8,undefined1 *param_9)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x40) == 0) {
    uVar1 = 0;
  }
  else {
    *param_2 = (uint)*(ushort *)(*(int *)(param_1 + 0x40) + 8);
    *param_3 = (uint)*(ushort *)(*(int *)(param_1 + 0x40) + 0xa [dec 10]);
    *param_8 = *(undefined4 *)(*(int *)(param_1 + 0x40) + 4);
    *param_9 = *(undefined1 *)(*(int *)(param_1 + 0x40) + 0xd);
    *param_5 = *(undefined4 *)(param_1 + 0x34);
    *param_6 = *(undefined4 *)(param_1 + 0x30);
    *param_7 = *(undefined4 *)(param_1 + 0x38);
    if ((*(byte *)(*(int *)(param_1 + 0x40) + 0xc) & 1) == 0) {
      if ((*(byte *)(*(int *)(param_1 + 0x40) + 0xc) & 2) == 0) {
        if ((*(byte *)(*(int *)(param_1 + 0x40) + 0xc) & 4) != 0) {
          *param_4 = 4;
        }
      }
      else {
        *param_4 = 3;
      }
    }
    else {
      *param_4 = 1;
    }
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x00945e70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00945e70: 16-byte aligned: YES
  00945e70  55             PUSH EBP
  00945e71  8B EC          MOV EBP,ESP
  00945e73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00945e70
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x28 [dec 40]  x1  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x34 [dec 52]  x1  via ECX
```

## Card CAL-2-35

GOG function: CGuiInGame::ResetDialogAnimations  gog 0x004d9ad0  callconv=(blank)  param_size_bytes=0  gog_size=373  gog_retn=0
Candidate Steam address: 0x007c84e0 (size 3826, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::SetDialogAnimations
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.28 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007c93e0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004d8bd0 CGuiInGame::SetDialogAnimations  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004d9c50 CGuiInGame::ResetTalkAnimations -> 0x007c9560  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam ?  in candidate callees: n/a
  0x00472930 CSWCObject::GetCurrentAnimation -> Steam 0x007ed7d0  in candidate callees: yes
  0x004dfbe0 CGuiInGame::CanPlayDialogAnimations -> Steam ?  in candidate callees: n/a
  0x00536580 CSWCPlaceable::GetServerPlaceable -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0x2c*,0x4c*,0x54*,0x68*,0x70*,0x9c*,0xd8*,0x1bc*,0x1c8*} | Steam {0x4*,0xc,0x14,0x2c*,0x4c*,0x54*,0x68*,0x70*,0x9c*,0xa0,0xd8*,0x190,0x194,0x198,...(+8)}
imm sets (* = in both, 1 common): GOG {0x28,0x2710*} | Steam {0x3,0x120,0x2710*,0x272e,0x2736,0x27aa,0x27ab,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007c84e0: 16-byte aligned: YES
  007c84e0  55             PUSH EBP
  007c84e1  8B EC          MOV EBP,ESP
  007c84e3  81 EC 20 01 00 00 SUB ESP,0x120
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007c84e0
string literals (0):
direct callees (15):
  0x0073F4D0  ClientApp::GetClientObject
  0x007CF4E0  CGuiInGame::CanPlayDialogAnimations
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x00842D00  CSWCPlaceable::GetServerPlaceable
  0x007CAE60  (unnamed)
  0x00773BF0  CSWCCreature::LookAt
  0x007CB250  (unnamed)
  0x007F0300  CSWCObject::AbortFireForgetAnimation
  0x007CB010  (unnamed)
  0x007CB220  (unnamed)
  0x007F02C0  CSWCObject::RemoveAllFireForgetAnimations
  0x00777E30  CSWCCreature::AnimateFireAndForget
  0x00775A80  CSWCCreature::GetPauseReadyAnimation
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
struct offsets touched (top 10):
  +0xC [dec 12]  x28  via EAX,EDX
  +0x68 [dec 104]  x28  via EAX,ECX,EDX
  +0x9C [dec 156]  x18  via EAX,EDX
  +0x2C [dec 44]  x16  via EAX,EDX
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x70 [dec 112]  x14  via EDX
  +0x54 [dec 84]  x6  via ECX,EDX
  +0x4C [dec 76]  x4  via EDX
  +0xA0 [dec 160]  x4  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007c84e0 FUN_007c84e0 size=3826 ret=0 callers=3 callees=15

void __fastcall FUN_007c84e0(int param_1)

{
  bool bVar1;
  bool bVar2;
  bool bVar3;
  short sVar4;
  short sVar5;
  int *piVar6;
  int iVar7;
  int *piVar8;
  undefined4 uVar9;
  int local_58;
  short local_50;
  int local_2c;
  int local_8;
  
  for (local_8 = 0; local_8 < 3; local_8 = local_8 + 1) {
    piVar6 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0xff0 + local_8 * 4));
    if ((piVar6 != (int *)0x0) && (iVar7 = (**(code **)(*piVar6 + 0x2c))(), iVar7 != 0)) {
      iVar7 = (**(code **)(*piVar6 + 0x2c))();
      (**(code **)(**(int **)(iVar7 + 0x68) + 0x70))(0);
      *(undefined4 *)(param_1 + 0xff0 + local_8 * 4) = 0x7f000000;
    }
  }
  bVar3 = false;
  bVar2 = false;
  local_8 = 0;
  do {
    if (*(int *)(param_1 + 0x1bc) <= local_8) {
      if (((!bVar2) &&
          (piVar6 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x19c)), piVar6 != (int *)0x0)) &&
         (iVar7 = (**(code **)(*piVar6 + 0xc))(), iVar7 != 0)) {
        iVar7 = (**(code **)(*piVar6 + 0x2c))();
        if (iVar7 == 0) {
          (**(code **)(*piVar6 + 0xc))();
          sVar4 = FUN_007ed7d0();
          if ((sVar4 != 10000) && (iVar7 = (**(code **)(*piVar6 + 0x14))(), iVar7 == 0)) {
            piVar8 = (int *)(**(code **)(*piVar6 + 0xc))();
            (**(code **)(*piVar8 + 0x9c))(10000,0,1);
          }
          iVar7 = (**(code **)(*piVar6 + 0xc))();
          (**(code **)(**(int **)(iVar7 + 0x68) + 0x70))(0);
        }
        else {
          iVar7 = (**(code **)(*piVar6 + 0x2c))();
          iVar7 = FUN_007cf4e0(*(undefined4 *)(iVar7 + 4));
          if (iVar7 != 0) {
            (**(code **)(*piVar6 + 0xc))();
            sVar4 = FUN_007ed7d0();
            if (sVar4 != 10000) {
              piVar8 = (int *)(**(code **)(*piVar6 + 0xc))();
              (**(code **)(*piVar8 + 0x9c))(10000,0,1);
            }
            iVar7 = (**(code **)(*piVar6 + 0xc))();
            (**(code **)(**(int **)(iVar7 + 0x68) + 0x70))(0);
          }
        }
      }
      if (((!bVar3) &&
          (piVar6 = (int *)FUN_0073f4d0(*(undefined4 *)(param_1 + 0x198)), piVar6 != (int *)0x0)) &&
         (iVar7 = (**(code **)(*piVar6 + 0xc))(), iVar7 != 0)) {
        iVar7 = (**(code **)(*piVar6 + 0x2c))();
        if (iVar7 == 0) {
          (**(code **)(*piVar6 + 0xc))();
          sVar4 = FUN_007ed7d0();
          if ((sVar4 != 10000) && (iVar7 = (**(code **)(*piVar6 + 0x14))(), iVar7 == 0)) {
            piVar8 = (int *)(**(code **)(*piVar6 + 0xc))();
            (**(code **)(*piVar8 + 0x9c))(10000,0,1);
          }
          iVar7 = (**(code **)(*piVar6 + 0xc))();
          (**(code **)(**(int **)(iVar7 + 0x68) + 0x70))(0);
        }
        else {
          iVar7 = (**(code **)(*piVar6 + 0x2c))();
          iVar7 = FUN_007cf4e0(*(undefined4 *)(iVar7 + 4));
          if (iVar7 != 0) {
            (**(code **)(*piVar6 + 0xc))();
            sVar4 = FUN_007ed7d0();
/* ... 229 more lines truncated by --max-lines */
```

### decoy: 0x007c93e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007c93e0: 16-byte aligned: YES
  007c93e0  55             PUSH EBP
  007c93e1  8B EC          MOV EBP,ESP
  007c93e3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007c93e0
string literals (0):
direct callees (4):
  0x0073F4D0  ClientApp::GetClientObject
  0x007CF4E0  CGuiInGame::CanPlayDialogAnimations
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 10):
  +0x68 [dec 104]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x54 [dec 84]  x2  via ECX
  +0x70 [dec 112]  x2  via EDX
  +0x9C [dec 156]  x2  via EAX,EDX
  +0xD8 [dec 216]  x1  via EDX
  +0x1BC [dec 444]  x1  via ECX
  +0x1C8 [dec 456]  x1  via EAX
```

## Card CAL-2-36

GOG function: Gob::IsForceAnimated  gog 0x0084c380  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00459da0 (size 17, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459d80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0084c360 Gob::DisableForceAnimations -> 0x00459d80  trusted
- GOG-order neighbour after:  0x0084c3a0 Gob::EnableAlwaysRender -> 0x00459dc0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00459da0: 16-byte aligned: YES
  00459da0  55             PUSH EBP
  00459da1  8B EC          MOV EBP,ESP
  00459da3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459da0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00459da0 FUN_00459da0 size=17 ret=0 callers=0 callees=0

undefined1 __fastcall FUN_00459da0(int param_1)

{
  return *(undefined1 *)(param_1 + 0x10);
}
```

### decoy: 0x00459d80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459d80: 16-byte aligned: YES
  00459d80  55             PUSH EBP
  00459d81  8B EC          MOV EBP,ESP
  00459d83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459d80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

## Card CAL-2-37

GOG function: CSWCObject::DetachFromServerObject  gog 0x004777d0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007f2630 (size 24, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2610:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004777b0 CSWCObject::AttachmentFromServerObject -> 0x007f2610  trusted
- GOG-order neighbour after:  0x004777f0 CSWCObject::EnableDistortion  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf8*} | Steam {0xf8*}
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
prologue for 0x007f2630: 16-byte aligned: YES
  007f2630  55             PUSH EBP
  007f2631  8B EC          MOV EBP,ESP
  007f2633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2630
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2630 FUN_007f2630 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007f2630(int param_1)

{
  *(undefined4 *)(param_1 + 0xf8) = 0;
  return;
}
```

### decoy: 0x007f2610 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2610: 16-byte aligned: YES
  007f2610  55             PUSH EBP
  007f2611  8B EC          MOV EBP,ESP
  007f2613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

## Card CAL-2-38

GOG function: Scene::ChildAdd  gog 0x00857f90  callconv=__thiscall  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x00465bb0 (size 112, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465c20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857ec0 Scene::UnlistSpecialParts  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00858000 Scene::ChildRemove -> 0x00465c20  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00857dd0 Scene::ListSpecialPartsR -> Steam ?  in candidate callees: n/a
  0x0086d830 Global::SceneBSPAddGob -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x84*,0x98*,0x1a8*} | Steam {0x84*,0x98*,0x1a8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x54*,0x10c*} | Steam {0x4*,0x8*,0x54*,0x10c*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00465bb0: 16-byte aligned: YES
  00465bb0  55             PUSH EBP
  00465bb1  8B EC          MOV EBP,ESP
  00465bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00465bb0
string literals (0):
direct callees (3):
  0x00495F20  (unnamed)
  0x004659F0  (unnamed)
  0x00473F40  vector<T*>::push_back (`ret 4`)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00465bb0 FUN_00465bb0 size=112 ret=4 callers=0 callees=3

void __thiscall FUN_00465bb0(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21 = byte +0x84] != 0) {
    FUN_00495f20(param_1,param_2);
    FUN_004659f0(param_2[0x26 = byte +0x98]);
  }
  FUN_00473f40(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00473f40(param_2);
  }
  return;
}
```

### decoy: 0x00465c20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465c20: 16-byte aligned: YES
  00465c20  55             PUSH EBP
  00465c21  8B EC          MOV EBP,ESP
  00465c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00465c20
string literals (0):
direct callees (3):
  0x00495FB0  (unnamed)
  0x00465AE0  (unnamed)
  0x00449170  (unnamed)
struct offsets touched (top 3):
  +0x84 [dec 132]  x1  via EAX
  +0x98 [dec 152]  x1  via EAX
  +0x1A8 [dec 424]  x1  via EDX
```

## Card CAL-2-39

GOG function: CSWGuiButton::Draw  gog 0x00918340  callconv=(blank)  param_size_bytes=4  gog_size=110  gog_retn=4
Candidate Steam address: 0x007cf3b0 (size 117, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CGuiInGame::SetStatusSummary
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007cf430:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00918220 CSWGuiButton::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009183b0 CSWGuiButton::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0xc,0x48,0x70,0xe8,0x160} | Steam {0xb8,0xc4}
imm sets (* = in both, 1 common): GOG {0x4*,0x70,0xe8,0x160} | Steam {0x4*,0xa,0xc,0x28,0x68}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007cf3b0: 16-byte aligned: YES
  007cf3b0  55             PUSH EBP
  007cf3b1  8B EC          MOV EBP,ESP
  007cf3b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cf3b0
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 2):
  +0xC4 [dec 196]  x3  via EAX,ECX
  +0xB8 [dec 184]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007cf3b0 FUN_007cf3b0 size=117 ret=4 callers=2 callees=1

void __thiscall FUN_007cf3b0(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 *puVar3;
  
  if (*(int *)(param_1 + 0xb8) == 0) {
    if (*(int *)(param_1 + 0xc4) == 0) {
      uVar1 = FUN_00919723(0x28);
      *(undefined4 *)(param_1 + 0xc4) = uVar1;
    }
    puVar3 = *(undefined4 **)(param_1 + 0xc4);
    for (iVar2 = 10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = *param_2;
      param_2 = param_2 + 0x1 [dec 1] {undefined4*: byte +0x4};
      puVar3 = puVar3 + 0x1 [dec 1] {undefined4*: byte +0x4};
    }
  }
  else {
    puVar3 = (undefined4 *)(*(int *)(param_1 + 0xb8) + 0x68);
    for (iVar2 = 10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *puVar3 = *param_2;
      param_2 = param_2 + 0x1 [dec 1] {undefined4*: byte +0x4};
      puVar3 = puVar3 + 0x1 [dec 1] {undefined4*: byte +0x4};
    }
  }
  return;
}
```

### decoy: 0x007cf430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cf430: 16-byte aligned: YES
  007cf430  55             PUSH EBP
  007cf431  8B EC          MOV EBP,ESP
  007cf433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cf430
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x2  via EAX,ECX
```

## Card CAL-2-40

GOG function: Scene::DisableAnimations  gog 0x008575b0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x004651d0 (size 21, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004651b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857590 Scene::EnableAnimations  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008575d0 Scene::IsAnimating  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xe6*} | Steam {0xe6*}
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
prologue for 0x004651d0: 16-byte aligned: YES
  004651d0  55             PUSH EBP
  004651d1  8B EC          MOV EBP,ESP
  004651d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004651d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004651d0 FUN_004651d0 size=21 ret=0 callers=0 callees=0

void __fastcall FUN_004651d0(int param_1)

{
  *(undefined1 *)(param_1 + 0xe6) = 0;
  return;
}
```

### decoy: 0x004651b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004651b0: 16-byte aligned: YES
  004651b0  55             PUSH EBP
  004651b1  8B EC          MOV EBP,ESP
  004651b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004651b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xE6 [dec 230]  x1  via EAX
```

## Card CAL-2-41

GOG function: CSWSTrigger::OnEnterMine  gog 0x00761bc0  callconv=(blank)  param_size_bytes=4  gog_size=660  gog_retn=4
Candidate Steam address: 0x0041f980 (size 676, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0041fc30:STR0/CALLEE0/SIG0.113/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007619e0 CSWSTrigger::GetCanFireMineOnObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00761e60 CSWSTrigger::GetClientTrigger  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00605ae0 CExoString::operator== -> Steam ?  in candidate callees: n/a
  0x00640af0 CVirtualMachine::RunScript -> Steam ?  in candidate callees: n/a
  0x0064bb00 CServerExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam 0x0051e170  in candidate callees: no
  0x006694e0 CItemRepository::FindItemWithTag -> Steam ?  in candidate callees: n/a
  0x006696b0 CItemRepository::RemoveItem -> Steam ?  in candidate callees: n/a
  ... 6 more GOG callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x14,0x2dc,0x2e4,0x30c,0x310,0x1150} | Steam {0x4*,0x8*,0xc,0x10,0x38,0x3c,0x74,0x8c,0x90,0x94,0x10c,0x284,0x29c,0x2a0,...(+5)}
imm sets (* = in both, 2 common): GOG {0x4*,0x5*,0x14,0x28,0x52,0x5c,0x2bc,0x7f000000} | Steam {0x2,0x4*,0x5*,0xa,0xb,0x1c,0x44,0x94,0x10c,0x2e0}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0041f980: 16-byte aligned: YES
  0041f980  55             PUSH EBP
  0041f981  8B EC          MOV EBP,ESP
  0041f983  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041f980
string literals (0):
direct callees (5):
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x0041E790  (unnamed)
  0x004799C0  (unnamed)
  0x0040FED0  CSWGuiPanel::GetLocalMouseCoords
  0x0041CB50  (unnamed)
struct offsets touched (top 10):
  +0x90 [dec 144]  x5  via EAX,ECX,EDX
  +0x38 [dec 56]  x4  via EAX,EDX
  +0x2AC [dec 684]  x4  via EAX,EDX
  +0x2A8 [dec 680]  x3  via EAX,ECX,EDX
  +0x2CC [dec 716]  x3  via ECX,EDX
  +0xC [dec 12]  x2  via EDX
  +0x8C [dec 140]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0041f980 FUN_0041f980 size=676 ret=4 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0041f980(int *param_1,float param_2)

{
  int iVar1;
  undefined4 local_20;
  int local_1c;
  undefined4 local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_c = param_1[0xac = byte +0x2b0];
  if (((uint)param_1[0xb3 = byte +0x2cc] >> 5 & 1) != 0) {
    if (((uint)param_1[0xb3 = byte +0x2cc] >> 0xb & 1) != 0) {
      param_1[0xa1 = byte +0x284] = _DAT_009f45fc;
    }
    (**(code **)(param_1[0x43 = byte +0x10c] + 0x38))(param_2);
  }
  (**(code **)(param_1[0x25 = byte +0x94] + 0xc))(param_2);
  iVar1 = FUN_00479460(param_1[0xa7 = byte +0x29c],param_1[0xa8 = byte +0x2a0],param_1[0xa9 = byte +0x2a4],param_1[0xaa = byte +0x2a8],param_1 + 0xb8,0,
                       0x3f800000);
  if (iVar1 != 0) {
    iVar1 = FUN_0041e790();
    if (iVar1 == 0) {
      for (local_8 = 0; local_8 < local_c; local_8 = local_8 + 1) {
        local_10 = *(int *)(*(int *)(param_1[0xab = byte +0x2ac] + local_8 * 4) + 8);
        iVar1 = *(int *)(param_1[0xab = byte +0x2ac] + local_8 * 4);
        local_14 = *(int *)(iVar1 + 8) + *(int *)(iVar1 + 0x10);
        if (((0 < local_14) && (local_10 <= param_1[0xaa = byte +0x2a8])) &&
           ((((uint)param_1[0xb3 = byte +0x2cc] >> 10 & 1) == 0 || ((local_14 < param_1[0xaa = byte +0x2a8] && (-1 < local_10)))
            ))) {
          (**(code **)(**(int **)(param_1[0xab = byte +0x2ac] + local_8 * 4) + 0x38))(param_2);
        }
      }
    }
    else {
      (**(code **)(**(int **)param_1[0xab = byte +0x2ac] + 0x38))(param_2);
    }
    FUN_004799c0();
  }
  if ((*(int **)(DAT_00a1b49c + 0x10) == param_1) && (param_1[0x23 = byte +0x8c] != 0)) {
    local_1c = param_1[0xe = byte +0x38];
    FUN_0040fed0(&local_18,&local_20);
    iVar1 = FUN_0041cb50(local_18,local_20);
    if (iVar1 == param_1[0x1d = byte +0x74]) {
      param_1[0x24 = byte +0x90] = (int)((float)param_1[0x24 = byte +0x90] - param_2);
      while ((float)param_1[0x24 = byte +0x90] < (float)_DAT_00986b38) {
        param_1[0x24 = byte +0x90] = (int)((float)param_1[0x24 = byte +0x90] + (float)_DAT_00987df0);
        (**(code **)(*param_1 + 0x3c))(param_1[0x23 = byte +0x8c],1);
      }
    }
  }
  return;
}
```

### decoy: 0x0041fc30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041fc30: 16-byte aligned: YES
  0041fc30  55             PUSH EBP
  0041fc31  8B EC          MOV EBP,ESP
  0041fc33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041fc30
string literals (0):
direct callees (5):
  0x0041D290  (unnamed)
  0x004142E0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00421720  (unnamed)
  0x0041EA50  (unnamed)
struct offsets touched (top 6):
  +0x2CC [dec 716]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x38 [dec 56]  x1  via EDX
  +0x54 [dec 84]  x1  via EDX
```

## Card CAL-2-42

GOG function: CClientExoApp::QueryExitProgram  gog 0x0040d300  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe20 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2e0 CClientExoApp::GetActivePauseState -> 0x0073fe00  trusted
- GOG-order neighbour after:  0x0040d320 CClientExoApp::SetInputActive  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004431c0 CClientExoAppInternal::QueryExitProgram -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073fe20: 16-byte aligned: YES
  0073fe20  55             PUSH EBP
  0073fe21  8B EC          MOV EBP,ESP
  0073fe23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe20
string literals (0):
direct callees (1):
  0x00796BA0  CClientExoAppInternal::QueryExitProgram
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe20 FUN_0073fe20 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00796BA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fe20(undefined4 param_1)

{
  FUN_00796ba0(param_1);
  return;
}
```

### decoy: 0x0073fe00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe00: 16-byte aligned: YES
  0073fe00  55             PUSH EBP
  0073fe01  8B EC          MOV EBP,ESP
  0073fe03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe00
string literals (0):
direct callees (1):
  0x00784440  CClientExoAppInternal::GetActivePauseState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-2-43

GOG function: Scene::SetFogRange  gog 0x00857060  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x00464c80 (size 31, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00464c50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857030 Scene::GetFogRange  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00857080 Scene::SetSceneFocus  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x64*,0x68*} | Steam {0x64*,0x68*}
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
prologue for 0x00464c80: 16-byte aligned: YES
  00464c80  55             PUSH EBP
  00464c81  8B EC          MOV EBP,ESP
  00464c83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00464c80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via EAX
  +0x68 [dec 104]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00464c80 FUN_00464c80 size=31 ret=8 callers=0 callees=0

void __thiscall FUN_00464c80(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x64 [dec 100]) = param_2;
  *(undefined4 *)(param_1 + 0x68) = param_3;
  return;
}
```

### decoy: 0x00464c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00464c50: 16-byte aligned: YES
  00464c50  55             PUSH EBP
  00464c51  8B EC          MOV EBP,ESP
  00464c53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00464c50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x64 [dec 100]  x1  via ECX
  +0x68 [dec 104]  x1  via EAX
```

## Card CAL-2-44

GOG function: CGuiInGame::HidePowersFeatsSkillsDebugMenu  gog 0x004dbc40  callconv=(blank)  param_size_bytes=0  gog_size=149  gog_retn=0
Candidate Steam address: 0x007cb540 (size 149, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007cb4a0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004dbba0 CGuiInGame::HideLoadModuleDebugMenu -> 0x007cb4a0  trusted
- GOG-order neighbour after:  0x004dbce0 CGuiInGame::UnlockDialogParticipantOrientations  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040d3c0 CClientExoApp::SetInputClass -> Steam 0x0073fee0  in candidate callees: yes
  0x0040d580 CClientExoApp::GetMiniGame -> Steam ?  in candidate callees: n/a
  0x0090eaf0 CSWGuiManager::RemovePanel -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x38*,0x74*,0x128*} | Steam {0x4*,0x38*,0x74*,0x128*}
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
prologue for 0x007cb540: 16-byte aligned: YES
  007cb540  55             PUSH EBP
  007cb541  8B EC          MOV EBP,ESP
  007cb543  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007cb540
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x74 [dec 116]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007cb540 FUN_007cb540 size=149 ret=0 callers=2 callees=5

void __fastcall FUN_007cb540(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x128) != 0) {
    iVar1 = CMFCVisualManagerBitmapCache::IsCached
                      (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x38),
                       *(CSize **)(param_1 + 0x74));
    if (iVar1 != 0) {
      FUN_004109c0();
      FUN_00410670(*(undefined4 *)(param_1 + 0x74));
      iVar1 = FUN_007400a0();
      if (iVar1 == 0) {
        FUN_0073fee0(0,1);
      }
      else {
        FUN_0073fee0(1,1);
      }
    }
  }
  return;
}
```

### decoy: 0x007cb4a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007cb4a0: 16-byte aligned: YES
  007cb4a0  55             PUSH EBP
  007cb4a1  8B EC          MOV EBP,ESP
  007cb4a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007cb4a0
string literals (0):
direct callees (5):
  0x00410800  GuiManager::Contains
  0x004109C0  CSWGuiManager::PopModalPanel
  0x00410670  (unnamed)
  0x007400A0  CClientExoApp::GetMiniGame
  0x0073FEE0  CClientExoApp::SetInputClass
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x70 [dec 112]  x2  via ECX,EDX
  +0x128 [dec 296]  x1  via EAX
```

## Card CAL-2-45

GOG function: CSWCObject::AnimationPause  gog 0x00474890  callconv=(blank)  param_size_bytes=2  gog_size=68  gog_retn=4
Candidate Steam address: 0x007ef350 (size 129, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef2c0:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474800 CSWCObject::AnimationOverlay -> 0x007ef6a0  trusted
- GOG-order neighbour after:  0x004748e0 CSWCObject::AnimationDialog -> 0x007ef780  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x68,0xcc,0xe0,0x130}
imm sets (* = in both, 1 common): GOG {0x4*,0x4b0,0x52f,0x640,0x6bf} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef350
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ef350 FUN_007ef350 size=129 ret=4 callers=2 callees=1

bool __thiscall FUN_007ef350(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe2c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef2c0: 16-byte aligned: YES
  007ef2c0  55             PUSH EBP
  007ef2c1  8B EC          MOV EBP,ESP
  007ef2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef2c0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card CAL-2-46

GOG function: CClientExoApp::GetActivePauseState  gog 0x0040d2e0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073fe00 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fe20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d2b0 CClientExoApp::SetStartMenuMusicOnMovieEnd  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d300 CClientExoApp::QueryExitProgram  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430a60 CClientExoAppInternal::GetActivePauseState -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073fe00: 16-byte aligned: YES
  0073fe00  55             PUSH EBP
  0073fe01  8B EC          MOV EBP,ESP
  0073fe03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fe00
string literals (0):
direct callees (1):
  0x00784440  CClientExoAppInternal::GetActivePauseState
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fe00 FUN_0073fe00 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00784440 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073fe00(undefined4 param_1)

{
  FUN_00784440(param_1);
  return;
}
```

### decoy: 0x0073fe20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fe20: 16-byte aligned: YES
  0073fe20  55             PUSH EBP
  0073fe21  8B EC          MOV EBP,ESP
  0073fe23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fe20
string literals (0):
direct callees (1):
  0x00796BA0  CClientExoAppInternal::QueryExitProgram
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-2-47

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00764420 (size 279, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.28 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764540:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x1,0x4*,0x8,0xa,0xc,0x10,0x14,0x18,0x1a,0x1c,0x1e,0x20,0x24,0x28,...(+4)}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x2c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00764420: 16-byte aligned: YES
  00764420  55             PUSH EBP
  00764421  8B EC          MOV EBP,ESP
  00764423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764420
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x1 [dec 1]  x2  via EAX,ECX
  +0xA [dec 10]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x10 [dec 16]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,ECX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x1A [dec 26]  x2  via EAX,ECX
  +0x1C [dec 28]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00764420 FUN_00764420 size=279 ret=4 callers=1 callees=0

void __thiscall FUN_00764420(undefined1 *param_1,undefined1 *param_2)

{
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  *(undefined4 *)(param_1 + 4) = *(undefined4 *)(param_2 + 4);
  *(undefined4 *)(param_1 + 0x14) = *(undefined4 *)(param_2 + 0x14);
  *(undefined2 *)(param_1 + 8) = *(undefined2 *)(param_2 + 8);
  *(undefined2 *)(param_1 + 0xa [dec 10]) = *(undefined2 *)(param_2 + 0xa [dec 10]);
  param_1[0xc] = param_2[0xc];
  *(undefined4 *)(param_1 + 0x10) = *(undefined4 *)(param_2 + 0x10);
  *(undefined2 *)(param_1 + 0x1e) = *(undefined2 *)(param_2 + 0x1e);
  *(undefined2 *)(param_1 + 0x18) = *(undefined2 *)(param_2 + 0x18);
  *(undefined2 *)(param_1 + 0x1a) = *(undefined2 *)(param_2 + 0x1a);
  *(undefined2 *)(param_1 + 0x1c) = *(undefined2 *)(param_2 + 0x1c);
  *(undefined4 *)(param_1 + 0x2c) = *(undefined4 *)(param_2 + 0x2c);
  *(undefined4 *)(param_1 + 0x30) = *(undefined4 *)(param_2 + 0x30);
  *(undefined4 *)(param_1 + 0x34) = *(undefined4 *)(param_2 + 0x34);
  *(undefined4 *)(param_1 + 0x38) = *(undefined4 *)(param_2 + 0x38);
  *(undefined4 *)(param_1 + 0x3c) = *(undefined4 *)(param_2 + 0x3c);
  *(undefined4 *)(param_1 + 0x44) = *(undefined4 *)(param_2 + 0x44);
  *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(param_2 + 0x20);
  *(undefined4 *)(param_1 + 0x24) = *(undefined4 *)(param_2 + 0x24);
  param_1[0x28] = param_2[0x28];
  *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(param_2 + 0x40);
  return;
}
```

### decoy: 0x00764540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764540: 16-byte aligned: YES
  00764540  55             PUSH EBP
  00764541  8B EC          MOV EBP,ESP
  00764543  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00764540
string literals (0):
direct callees (2):
  0x00733540  CExoString::CExoString(empty)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card CAL-2-48

GOG function: CClientExoApp::CreatureAcquireItem  gog 0x0040cec0  callconv=(blank)  param_size_bytes=8  gog_size=68  gog_retn=8
Candidate Steam address: 0x006977a0 (size 74, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::ExecuteCommandSetButtonMashCheck
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00697750:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040cea0 CClientExoApp::MainLoop -> 0x0073f9c0  trusted
- GOG-order neighbour after:  0x0040cf10 CClientExoApp::Uninitialize  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: no
  0x0064c680 CServerExoApp::CreatureAcquireItem -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
imm sets (* = in both, 1 common): GOG {0x8*,0x10} | Steam {0x8*,0xc,0xfffff82f}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006977a0: 16-byte aligned: YES
  006977a0  55             PUSH EBP
  006977a1  8B EC          MOV EBP,ESP
  006977a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006977a0
string literals (0):
direct callees (2):
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051CE50  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006977a0 FUN_006977a0 size=74 ret=8 callers=0 callees=2

undefined4 FUN_006977a0(void)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = FUN_006fd9a0(&local_8);
  if (iVar1 == 0) {
    uVar2 = 0xfffff82f;
  }
  else {
    FUN_0051ce50(local_8);
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x00697750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00697750: 16-byte aligned: YES
  00697750  55             PUSH EBP
  00697751  8B EC          MOV EBP,ESP
  00697753  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00697750
string literals (0):
direct callees (2):
  0x0051CE30  (unnamed)
  0x006FD9C0  CVirtualMachine::StackPushInteger
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card CAL-2-49

GOG function: Scene::SetFog  gog 0x00857010  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x007a2380 (size 21, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2360:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00856fd0 Global::CAurScene_scalar_deleting_destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00857030 Scene::GetFogRange  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x60} | Steam {}
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

## Card CAL-2-50

GOG function: Gob::GetPosition  gog 0x0084bcf0  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x00459740 (size 49, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Gob::GetOrientation
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00459710:STR0/CALLEE0/SIG1/SRV0/VT1
- flags: autolabel:GetMargins
- GOG-order neighbour before: 0x0084bcd0 Gob::GetName  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084bd20 Gob::GetOrientation  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0xc}
imm sets (* = in both, 1 common): GOG {0x4*,0xa4} | Steam {0x4*,0xb0}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00459740: 16-byte aligned: YES
  00459740  55             PUSH EBP
  00459741  8B EC          MOV EBP,ESP
  00459743  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00459740
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00459740 CRichEditView::GetMargins size=49 ret=4 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: class CRect __thiscall CRichEditView::GetMargins(void)const 
   
   Library: Visual Studio 2010 Debug */

undefined4 * __thiscall CRichEditView::GetMargins(CRichEditView *this)

{
  undefined4 *in_stack_00000004;
  
  *in_stack_00000004 = *(undefined4 *)(this + 0xb0);
  in_stack_00000004[1 = byte +0x4] = *(undefined4 *)(this + 0xb4);
  in_stack_00000004[2 = byte +0x8] = *(undefined4 *)(this + 0xb8);
  in_stack_00000004[3 = byte +0xc] = *(undefined4 *)(this + 0xbc);
  return in_stack_00000004;
}
```

### decoy: 0x00459710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00459710: 16-byte aligned: YES
  00459710  55             PUSH EBP
  00459711  8B EC          MOV EBP,ESP
  00459713  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00459710
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
```

## Card CAL-2-51

GOG function: CResGFF::InitializeForWriting  gog 0x00623c00  callconv=(blank)  param_size_bytes=0  gog_size=1157  gog_retn=0
Candidate Steam address: 0x00717630 (size 234, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CResGFF::ReleaseResource
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.22 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00717720:STR0/CALLEE0/SIG0.152/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00623b60 CResGFF::GetField_2 -> 0x00717a00  trusted
- GOG-order neighbour after:  0x00624090 CResGFF::AddStruct -> 0x00717f30  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00623790 CResGFF::ReleaseResource -> Steam 0x00717630  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 14 common): GOG {0x4,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x38,...(+15)} | Steam {0x40*,0x44*,0x48*,0x4c*,0x50*,0x54*,0x58*,0x5c*,0x60*,0x64*,0x68*,0x6c,0x70*,0x74*,...(+3)}
imm sets (* = in both, 0 common): GOG {0x4,0xc,0x10,0x38,0x58,0x62,0x6e,0x286,0x72c,0xfd4} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00717630: 16-byte aligned: YES
  00717630  55             PUSH EBP
  00717631  8B EC          MOV EBP,ESP
  00717633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00717630
string literals (0):
direct callees (2):
  0x007112B0  CRes::Release
  0x00713010  CExoResMan::ReleaseResObject
struct offsets touched (top 10):
  +0x98 [dec 152]  x2  via EAX,ECX
  +0x9C [dec 156]  x2  via EAX,ECX
  +0x40 [dec 64]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
  +0x48 [dec 72]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x54 [dec 84]  x1  via ECX
  +0x58 [dec 88]  x1  via EDX
  +0x5C [dec 92]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00717630 FUN_00717630 size=234 ret=0 callers=3 callees=2

void __fastcall FUN_00717630(int param_1)

{
  if (*(int *)(param_1 + 0x98) != 0) {
    if (*(int *)(param_1 + 0x9c) != 0) {
      FUN_007112b0();
      FUN_00713010(param_1);
      *(undefined4 *)(param_1 + 0x9c) = 0;
    }
    *(undefined4 *)(param_1 + 0x98) = 0;
  }
  *(undefined4 *)(param_1 + 0x40) = 0;
  *(undefined4 *)(param_1 + 0x44) = 0;
  *(undefined4 *)(param_1 + 0x48) = 0;
  *(undefined4 *)(param_1 + 0x4c) = 0;
  *(undefined4 *)(param_1 + 0x50) = 0;
  *(undefined4 *)(param_1 + 0x54) = 0;
  *(undefined4 *)(param_1 + 0x58) = 0;
  *(undefined4 *)(param_1 + 0x5c) = 0;
  *(undefined4 *)(param_1 + 0x60) = 0;
  *(undefined4 *)(param_1 + 0x64 [dec 100]) = 0;
  *(undefined4 *)(param_1 + 0x68) = 0;
  *(undefined4 *)(param_1 + 0x6c) = 0;
  *(undefined4 *)(param_1 + 0x70) = 0;
  *(undefined4 *)(param_1 + 0x74) = 0;
  *(undefined4 *)(param_1 + 0x78) = 0;
  return;
}
```

### decoy: 0x00717720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00717720: 16-byte aligned: YES
  00717720  55             PUSH EBP
  00717721  8B EC          MOV EBP,ESP
  00717723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00717720
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x44 [dec 68]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
```

## Card CAL-2-52

GOG function: CSWSObject::SetDialogOwner  gog 0x006ada90  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00546db0 (size 22, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00546dd0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006ad8b0 CSWSObject::GetNearestObjectByName -> 0x00546bd0  trusted
- GOG-order neighbour after:  0x006adb20 CSWSObject::SetListenExpression  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x54*} | Steam {0x54*}
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
prologue for 0x00546db0: 16-byte aligned: YES
  00546db0  55             PUSH EBP
  00546db1  8B EC          MOV EBP,ESP
  00546db3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00546db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x54 [dec 84]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00546db0 FUN_00546db0 size=22 ret=4 callers=3 callees=0

void __thiscall FUN_00546db0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  return;
}
```

### decoy: 0x00546dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00546dd0: 16-byte aligned: YES
  00546dd0  55             PUSH EBP
  00546dd1  8B EC          MOV EBP,ESP
  00546dd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00546dd0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1B0 [dec 432]  x2  via EAX
  +0x1B4 [dec 436]  x1  via ECX
```

## Card CAL-2-53

GOG function: CClientExoAppInternal::MainLoop  gog 0x0042e160  callconv=__thiscall  param_size_bytes=0  gog_size=8599  gog_retn=0
Candidate Steam address: 0x00781be0 (size 8599, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 1 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Playing next anim in queue: %s'@0099f4b4; CALLEE 3 (UpdateAreaTransitionGUI 0x7ce780,ResetTimer 0x51ae30,PauseWorldTimer 0x51b050); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007819e0:STR0/CALLEE0/SIG0.103/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042df60 CClientExoAppInternal::Initialize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00430300 CClientExoAppInternal::StallServerCode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (106), mapped to Steam through trusted rows only:
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x00407440 Global::ClearLogCounters -> Steam ?  in candidate callees: n/a
  0x0040ba50 Global::RestoreResolutionFromMovie -> Steam ?  in candidate callees: n/a
  0x0040c510 CClientExoApp::EndLoadBar -> Steam ?  in candidate callees: n/a
  0x0040c7b0 CClientExoApp::GetCaptureEventPlayBack -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam ?  in candidate callees: n/a
  0x0040c8f0 CClientExoApp::GetModule -> Steam ?  in candidate callees: n/a
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  ... 98 more GOG callees
GOG strings (1): 'Playing next anim in queue: %s'
disp sets (* = in both, 64 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x38*,0x40*,...(+50)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x38*,0x40*,...(+50)}
imm sets (* = in both, 21 common): GOG {0x2*,0x3*,0x4*,0x5*,0x7*,0xc*,0x10*,0x24*,0x41*,0x44*,0x7f*,0xff*,0x158*,0x168*,...(+7)} | Steam {0x2*,0x3*,0x4*,0x5*,0x7*,0xc*,0x10*,0x24*,0x41*,0x44*,0x7f*,0xff*,0x158*,0x168*,...(+7)}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00781be0: 16-byte aligned: YES
  00781be0  55             PUSH EBP
  00781be1  8B EC          MOV EBP,ESP
  00781be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00781be0
string literals (2):
  0x0099F4B4  at 00782c55  'Playing next anim in queue: %s'
  0x0099F4B4  at 00782c71  'Playing next anim in queue: %s'
direct callees (106):
  0x007CE740  CGuiInGame::SetAreaTransition
  0x0078CEE0  (unnamed)
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
  0x0079ED60  CClientExoAppInternal::CrownFutureLeader
  0x007B3050  CClientExoAppInternal::SetInputClass
  0x00865250  (unnamed)
  0x007C9C40  CGuiInGame::SetSWGuiStatus
  0x007CEEC0  CGuiInGame::InitializeMap
  0x007B3390  CClientExoAppInternal::EnableInput
  0x00783D80  CClientExoAppInternal::StallServerCode
  0x00733780  CExoString::~CExoString
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x00798C00  IsMoviePlaying
  0x007983E0  (unnamed)
  0x007413E0  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x0093B5E0  (unnamed)
  0x0051B050  CWorldTimer::PauseWorldTimer
  0x0051AE30  CWorldTimer::ResetTimer
struct offsets touched (top 10):
  +0x40 [dec 64]  x39  via EAX,ECX,EDX
  +0x4 [dec 4]  x30  via EAX,ECX,EDX
  +0x8 [dec 8]  x17  via EAX,ECX,EDX
  +0x10 [dec 16]  x13  via EAX,ECX,EDX
  +0x18 [dec 24]  x13  via EAX,ECX,EDX
  +0x24 [dec 36]  x10  via EAX,ECX,EDX
  +0x14 [dec 20]  x9  via EAX,ECX,EDX
  +0x2C [dec 44]  x9  via EAX,ECX,EDX
  +0x16C [dec 364]  x8  via EAX,ECX,EDX
  +0x370 [dec 880]  x8  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00781be0 FUN_00781be0 size=8599 ret=0 callers=1 callees=106

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00781be0(int param_1)

{
  longlong lVar1;
  undefined8 uVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  float10 fVar9;
  undefined8 uVar10;
  undefined4 uVar11;
  undefined1 *puVar12;
  undefined4 uVar13;
  undefined1 *puVar14;
  undefined4 local_36c;
  int local_364;
  uint local_354;
  undefined4 local_2d8;
  size_t local_20c;
  size_t local_208;
  size_t local_204;
  undefined1 local_b8 [12];
  undefined1 local_ac [12];
  undefined1 local_a0 [12];
  undefined1 local_94 [8];
  undefined4 local_8c;
  undefined4 local_88;
  undefined4 local_84;
  undefined1 local_80 [8];
  undefined4 local_78;
  undefined4 local_74;
  undefined4 local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  undefined4 local_5c;
  int *local_58;
  int local_54;
  undefined1 local_50 [8];
  undefined1 local_48 [4];
  float local_44[3 = byte +0xc];
  int *local_38;
  int local_34;
  undefined1 local_30 [4];
  int *local_2c;
  undefined4 local_28;
  int local_24;
  undefined1 local_20 [7];
  undefined1 local_19;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096585b;
  local_10 = ExceptionList;
  uVar4 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = 0;
  if (*(int *)(param_1 + 0x40) != 0) {
    local_78 = 0;
    local_74 = 0;
    local_70 = 0;
    FUN_007ce740(0x7f000000,&local_78);
  }
  if (((*(int *)(param_1 + 0x288) != 0) && (*(int *)(param_1 + 0x18) != 0)) &&
     (iVar5 = FUN_0078cee0(uVar4), iVar5 != 0)) {
    iVar5 = FUN_007fae30();
    if ((iVar5 == 0) && (*(int *)(param_1 + 0x338) != 0x7f000000)) {
      FUN_0079ed60();
    }
/* ... 605 more lines truncated by --max-lines */
```

### decoy: 0x007819e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007819e0: 16-byte aligned: YES
  007819e0  55             PUSH EBP
  007819e1  8B EC          MOV EBP,ESP
  007819e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007819e0
string literals (0):
direct callees (12):
  0x00919723  operator new
  0x008850B0  (unnamed)
  0x0069DEA0  CSWRules::CSWRules (loads EXPTABLE 51 rows at +0x38)
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x007BDE80  (unnamed)
  0x00785DF0  CClientExoAppInternal::SetEventDescriptions
  0x00798C00  IsMoviePlaying
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x0072F9F0  (unnamed)
  0x0070BC60  (unnamed)
struct offsets touched (top 8):
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0xA4 [dec 164]  x2  via EAX,ECX
  +0x130 [dec 304]  x2  via EAX,ECX
  +0x278 [dec 632]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via ECX
  +0x204 [dec 516]  x1  via EAX
  +0x274 [dec 628]  x1  via ECX
```

## Card CAL-2-54

GOG function: CSWGuiLevelUpCharGen::CopyCreatureStats  gog 0x005e9ac0  callconv=(blank)  param_size_bytes=8  gog_size=2241  gog_retn=8
Candidate Steam address: 0x008f6220 (size 78, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008f61d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e90b0 CSWGuiLevelUpCharGen::AbilitiesChanged  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005ea390 CSWGuiLevelUpCharGen::NewAbilityScores  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (33), mapped to Steam through trusted rows only:
  0x0048f020 CSWCCreatureStats::ClearSkills -> Steam ?  in candidate callees: n/a
  0x0048f0d0 CSWCCreatureStats::ClearFeats -> Steam ?  in candidate callees: n/a
  0x0048f5e0 CSWCCreatureStats::GetArmorClass -> Steam ?  in candidate callees: n/a
  0x0048f9b0 CSWCLevelUpStats::GetClass -> Steam ?  in candidate callees: n/a
  0x0048f9f0 CSWCLevelUpStats::GetClassLevel -> Steam ?  in candidate callees: n/a
  0x0048fc50 CSWCCreatureStats::GetFeat -> Steam ?  in candidate callees: n/a
  0x0048ff50 CSWCLevelUpStats::AddKnownSpell -> Steam ?  in candidate callees: n/a
  0x00490060 CSWCCreatureStats::ClearKnownSpells -> Steam ?  in candidate callees: n/a
  ... 25 more GOG callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x24,0x30,0x31,0x32,0x33,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x48,0x4c,0x4e,...(+15)} | Steam {0x1c}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0x8*,0x9,0x14,0x1c,0x28*,0x64,0x70,0xb8,...(+1)} | Steam {0x8*,0x28*,0x2e}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x008f6220: 16-byte aligned: YES
  008f6220  55             PUSH EBP
  008f6221  8B EC          MOV EBP,ESP
  008f6223  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008f6220
string literals (0):
direct callees (3):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008F6270  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008f6220 FUN_008f6220 size=78 ret=8 callers=0 callees=3

void FUN_008f6220(int param_1,int param_2)

{
  if ((param_2 != 0) && ((param_1 == 0x28 || (param_1 == 0x2e)))) {
    FUN_004122a0(0);
    FUN_008f6270();
  }
  FUN_0040eaa0(param_1,param_2);
  return;
}
```

### decoy: 0x008f61d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008f61d0: 16-byte aligned: YES
  008f61d0  55             PUSH EBP
  008f61d1  8B EC          MOV EBP,ESP
  008f61d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008f61d0
string literals (0):
direct callees (3):
  0x009049B0  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 3):
  +0x70 [dec 112]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x2C9C [dec 11420]  x1  via EAX
```

## Card CAL-2-55

GOG function: CClientExoApp::GetPlayerCreature  gog 0x0040c910  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0073f450 (size 22, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetControlledObject
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (ShowSoloModeQuery 0x7cd510,ShowLevelUpGUI 0x84fcd0)
- best decoy (neighbour/other candidate) scores: 0x0073f430:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040c8f0 CClientExoApp::GetModule  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040c930 CClientExoApp::GetPlayerCreatureId  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073f450: 16-byte aligned: YES
  0073f450  55             PUSH EBP
  0073f451  8B EC          MOV EBP,ESP
  0073f453  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073f450
string literals (0):
direct callees (1):
  0x0078CEE0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073f450 FUN_0073f450 size=22 ret=0 callers=57 callees=1
THUNK -> 0x0078CEE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0073f450(undefined4 param_1)

{
  FUN_0078cee0(param_1);
  return;
}
```

### decoy: 0x0073f430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073f430: 16-byte aligned: YES
  0073f430  55             PUSH EBP
  0073f431  8B EC          MOV EBP,ESP
  0073f433  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073f430
string literals (0):
direct callees (1):
  0x00726F80  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card CAL-2-56

GOG function: CSWGuiSaveLoad::VerifyLoadGame  gog 0x005d1bf0  callconv=(blank)  param_size_bytes=4  gog_size=209  gog_retn=4
Candidate Steam address: 0x008528b0 (size 209, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.88 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (LoadGame 0x853a00); SIG 0.88
- best decoy (neighbour/other candidate) scores: 0x00852380:STR0/CALLEE0/SIG0.081/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d16c0 CSWGuiSaveLoad::PopulateGameLB  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d1cd0 Global::LoadScreenUpdate  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam ?  in candidate callees: n/a
  0x0052d090 CSWGuiMessageBox::SetAllowCancel -> Steam ?  in candidate callees: n/a
  0x0052e030 CSWGuiMessageBox::SetCallback -> Steam ?  in candidate callees: n/a
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam ?  in candidate callees: n/a
  0x0091df50 CSWGuiListBox::GetSelectedControl -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x1c*,0x50*,0x68*,0x70*,0xa0*,0x1b70} | Steam {0x4*,0x1c*,0x50*,0x68*,0x70*,0xa0*,0x1e40}
imm sets (* = in both, 4 common): GOG {0x4*,0x18*,0x970*,0x7d9b*} | Steam {0x4*,0x18*,0x970*,0x7d9b*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008528b0: 16-byte aligned: YES
  008528b0  55             PUSH EBP
  008528b1  8B EC          MOV EBP,ESP
  008528b3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008528b0
string literals (0):
direct callees (6):
  0x0041FFB0  (unnamed)
  0x00853A00  CSWGuiSaveLoad::LoadGame
  0x0073F750  ClientApp::GetGuiInGame
  0x0075BB00  (unnamed)
  0x0075CA30  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 7):
  +0x4 [dec 4]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x68 [dec 104]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
  +0xA0 [dec 160]  x1  via EDX
  +0x1E40 [dec 7744]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008528b0 FUN_008528b0 size=209 ret=4 callers=0 callees=6

void __thiscall FUN_008528b0(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  
  if ((*(int *)(param_2 + 0x50) != 0) && (iVar2 = FUN_0041ffb0(), iVar2 != 0)) {
    if (((*(uint *)(param_1 + 0x68) >> 1 & 1) == 0) && (*(int *)(param_1 + 0x1e40) == 0)) {
      iVar3 = FUN_0073f750();
      piVar1 = *(int **)(iVar3 + 0xa0);
      FUN_0075bb00(1);
      (**(code **)(*piVar1 + 0x70))(0x7d9b);
      FUN_0075ca30(param_1,FUN_00853a00,iVar2);
      FUN_00410530(piVar1,1,1);
    }
    else {
      FUN_00853a00(iVar2);
    }
  }
  return;
}
```

### decoy: 0x00852380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00852380: 16-byte aligned: YES
  00852380  55             PUSH EBP
  00852381  8B EC          MOV EBP,ESP
  00852383  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00852380
string literals (0):
direct callees (18):
  0x0084A640  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
  0x0041D290  (unnamed)
  0x007107F0  (unnamed)
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
  0x00416EA0  (unnamed)
  0x00919723  operator new
  0x008557C0  (unnamed)
  0x00852340  (unnamed)
  0x007339D0  CExoString::operator==
  0x00414280  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0041FC30  (unnamed)
  0x0041FF70  (unnamed)
  0x0041E9A0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 8):
  +0x1E2C [dec 7724]  x2  via EAX,ECX
  +0x1E30 [dec 7728]  x2  via EAX,EDX
  +0x1E34 [dec 7732]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x68 [dec 104]  x1  via ECX
  +0x1D0 [dec 464]  x1  via EDX
  +0x1E20 [dec 7712]  x1  via EAX
  +0x1E24 [dec 7716]  x1  via EAX
```

## Card CAL-2-57

GOG function: CClientExoApp::SetInputClass  gog 0x0040d3c0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x0073fee0 (size 32, retn 8)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 8 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 8 (HideGalaxyMapGui 0x7ca920,HideItemCreateMenu 0x7caa60,HideContainerGui 0x7cad30)
- best decoy (neighbour/other candidate) scores: 0x0073ff00:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d3a0 CClientExoApp::ServerToClientObjectId  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d3e0 CClientExoApp::GetInputClass  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0045fc60 CClientExoAppInternal::SetInputClass -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073fee0: 16-byte aligned: YES
  0073fee0  55             PUSH EBP
  0073fee1  8B EC          MOV EBP,ESP
  0073fee3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fee0
string literals (0):
direct callees (1):
  0x007B3050  CClientExoAppInternal::SetInputClass
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fee0 FUN_0073fee0 size=32 ret=8 callers=62 callees=1
THUNK -> 0x007B3050 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fee0(undefined4 param_1,undefined4 param_2)

{
  FUN_007b3050(param_1,param_2);
  return;
}
```

### decoy: 0x0073ff00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073ff00: 16-byte aligned: YES
  0073ff00  55             PUSH EBP
  0073ff01  8B EC          MOV EBP,ESP
  0073ff03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073ff00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x9C [dec 156]  x1  via ECX
```

## Card CAL-2-58

GOG function: CClientExoApp::SetVideoMode  gog 0x0040d0b0  callconv=(blank)  param_size_bytes=4  gog_size=30  gog_retn=4
Candidate Steam address: 0x0073fbd0 (size 30, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073fbf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d090 CClientExoApp::GetHitCheckMaterialMask  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0040d0d0 CClientExoApp::SetTexturePack  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0043a1c0 CClientExoAppInternal::SetVideoMode -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0073fbd0: 16-byte aligned: YES
  0073fbd0  55             PUSH EBP
  0073fbd1  8B EC          MOV EBP,ESP
  0073fbd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073fbd0
string literals (0):
direct callees (1):
  0x0078DBF0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0073fbd0 FUN_0073fbd0 size=30 ret=4 callers=1 callees=1
THUNK -> 0x0078DBF0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0073fbd0(undefined4 param_1)

{
  FUN_0078dbf0(param_1,0);
  return;
}
```

### decoy: 0x0073fbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073fbf0: 16-byte aligned: YES
  0073fbf0  55             PUSH EBP
  0073fbf1  8B EC          MOV EBP,ESP
  0073fbf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073fbf0
string literals (0):
direct callees (1):
  0x0078D6A0  CClientExoApp::SetTexturePack (texpacks.2da row: Texture, Gui, DynMemRatio, Mem)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card CAL-2-59

GOG function: CSWGuiControl::Constructor  gog 0x00916c30  callconv=__thiscall  param_size_bytes=0  gog_size=347  gog_retn=0
Candidate Steam address: 0x007eb460 (size 342, retn 0)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007eb5c0:STR0/CALLEE0/SIG0.105/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00916b80 CSWGuiNavigable::Load  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00916d90 CSWGuiControl::AddEvent -> 0x00418af0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e030 CExoArrayList_CSWGuiSkillStruct_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam ?  in candidate callees: n/a
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0xc,0x18,0x28,0x34,0x38,0x48,0x54,0x58,0x59,0x5c} | Steam {0x4*,0x8*,0x24,0x520,0x11e8}
imm sets (* = in both, 0 common): GOG {0x2,0x4,0x8,0x1c,0x2c,0x3c,0xffffffbf,0xffffffdf,0xffffffef,0xfffffffe} | Steam {0x40,0x88}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007eb460: 16-byte aligned: YES
  007eb460  55             PUSH EBP
  007eb461  8B EC          MOV EBP,ESP
  007eb463  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007eb460
string literals (0):
direct callees (3):
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 4):
  +0x8 [dec 8]  x4  via EDX
  +0x520 [dec 1312]  x2  via EDX
  +0x11E8 [dec 4584]  x2  via ECX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007eb460 FUN_007eb460 size=342 ret=0 callers=1 callees=3

undefined4 __fastcall FUN_007eb460(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int local_c;
  
  uVar1 = FUN_0073f470();
  uVar1 = FUN_0051c8b0(uVar1);
  iVar2 = FUN_0051c100(uVar1);
  if (iVar2 == 0) {
    uVar1 = 0;
  }
  else if ((*(int *)(iVar2 + 0x520) == 0) || (*(char *)(iVar2 + 0x11e8) != '\x01')) {
    for (local_c = 0; local_c < *param_1; local_c = local_c + 1) {
      uVar1 = FUN_0051c8b0(param_1[local_c * 0x22 + 9]);
      iVar2 = FUN_0051c100(uVar1);
      if (((iVar2 != 0) && (*(int *)(iVar2 + 0x520) != 0)) && (*(char *)(iVar2 + 0x11e8) == '\x01'))
      {
        return 1;
      }
    }
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007eb5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007eb5c0: 16-byte aligned: YES
  007eb5c0  55             PUSH EBP
  007eb5c1  8B EC          MOV EBP,ESP
  007eb5c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007eb5c0
string literals (4):
  0x009A21F8  at 007eb788  'Undefined'
  0x009A21E8  at 007eb78f  'UndefinedOption'
  0x009A21F8  at 007ecd4d  'Undefined'
  0x009A21E8  at 007ecd57  'UndefinedOption'
direct callees (1):
  0x00733540  CExoString::CExoString(empty)
struct offsets touched (top 9):
  +0x4 [dec 4]  x52  via EAX,ECX,EDX
  +0x8 [dec 8]  x50  via EAX,ECX,EDX
  +0xC [dec 12]  x50  via EAX,ECX,EDX
  +0x10C [dec 268]  x6  via EAX,ECX
  +0x90 [dec 144]  x1  via EDX
  +0x94 [dec 148]  x1  via EAX
  +0x98 [dec 152]  x1  via EDX
  +0xC4 [dec 196]  x1  via ECX
  +0x110 [dec 272]  x1  via EAX
```

## Card CAL-2-60

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00764f60 (size 22, retn 4)
Candidate sources: calib    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007645f0:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x00764f60: 16-byte aligned: YES
  00764f60  55             PUSH EBP
  00764f61  8B EC          MOV EBP,ESP
  00764f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764f60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00764f60 FUN_00764f60 size=22 ret=4 callers=0 callees=0

void __thiscall FUN_00764f60(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 4) = param_2;
  return;
}
```

### decoy: 0x007645f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007645f0: 16-byte aligned: YES
  007645f0  55             PUSH EBP
  007645f1  8B EC          MOV EBP,ESP
  007645f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007645f0
string literals (0):
direct callees (11):
  0x007ED050  (unnamed)
  0x007107F0  (unnamed)
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x00845AC0  (unnamed)
  0x00764320  (unnamed)
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0085A0D0  Global::CSWCCreatureAppearance
  0x00859470  (unnamed)
  0x0091EB8B  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x458 [dec 1112]  x4  via EAX,EDX
  +0x224 [dec 548]  x2  via EAX,EDX
  +0x3C [dec 60]  x1  via EAX
  +0x40 [dec 64]  x1  via EDX
  +0x48 [dec 72]  x1  via EDX
  +0x58 [dec 88]  x1  via EAX
  +0x12C [dec 300]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```
