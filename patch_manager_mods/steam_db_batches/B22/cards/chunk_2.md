# Review cards B22 chunk 2

Answer each card with one TSV row in `steam_db_batches/B22/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B22-2-01

GOG function: CSWCModule::Destructor  gog 0x0046a080  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x007f3640 (size 297, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f3040:STR0/CALLEE0/SIG0.038/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00469cd0 CSWCModule::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046a0b0 Global::CSWCModule_local_camera_target_Part_cleanup_helper  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0046a0d0 CSWCModule::Destructor_2 -> Steam 0x007f2eb0  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x1c,0x3c,0x40,0x44,0x48,0x70,0x80,0x8c,0xb8}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x24}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f3640: 16-byte aligned: YES
  007f3640  55             PUSH EBP
  007f3641  8B EC          MOV EBP,ESP
  007f3643  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f3640
string literals (0):
direct callees (4):
  0x007F51C0  (unnamed)
  0x00784440  CClientExoAppInternal::GetActivePauseState
  0x007A6760  CSWCArea::Update
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 10):
  +0x40 [dec 64]  x5  via ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x8C [dec 140]  x2  via EAX,EDX
  +0xB8 [dec 184]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
  +0x44 [dec 68]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f3640 FUN_007f3640 size=297 ret=4 callers=2 callees=4

void __thiscall FUN_007f3640(int param_1,float param_2)

{
  char cVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 local_c;
  undefined4 local_8;
  
  if ((*(int *)(param_1 + 0x48) != 0) && (*(int *)(param_1 + 0x40) != 0)) {
    if (*(int *)(param_1 + 0x8c) != 0) {
      iVar2 = FUN_007f51c0(param_2,&local_c,&local_8);
      if (((iVar2 != 0) &&
          (piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x40) + 0x80))(0xffffffff),
          piVar3 != (int *)0x0)) && (iVar2 = (**(code **)(*piVar3 + 0x1c))(), iVar2 != 0)) {
        *(undefined4 *)(iVar2 + 0x3c) = local_c;
        *(undefined4 *)(iVar2 + 0x44) = local_8;
      }
      *(undefined4 *)(param_1 + 0x8c) = 0;
    }
    uVar5 = *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 4) + 4);
    cVar1 = FUN_00784440();
    if (cVar1 == '\0') {
      *(float *)(param_1 + 0xb8) = *(float *)(param_1 + 0xb8) + param_2;
    }
    FUN_007a6760(param_2);
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar2 = FUN_0073fea0();
    if (*(char *)(iVar2 + 0x70) == '\0') {
      (**(code **)(**(int **)(param_1 + 0x40) + 8))(param_1,iVar2,uVar4,uVar5);
    }
  }
  return;
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
- size 297 B, RET 4
- callers 2: 0x007a23a0, 0x008025d0
- callees: 0x007f51c0 (CALL), 0x00784440 CClientExoAppInternal::GetActivePauseState (CALL), 0x007a6760 CSWCArea::Update (CALL), 0x0073fea0 CClientExoApp::GetSWGuiManager (CALL)
- indirect: 0x007f369d slot 0x80, 0x007f36b3 slot 0x1c, 0x007f3761 slot 0x8
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ
- twins: none

## Card B22-2-02

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

## Card B22-2-03

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

## Card B22-2-04

GOG function: CSWCModule::Destructor  gog 0x0046a080  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x007f2910 (size 235, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::AcquireAnimationBase | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f26b0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00469cd0 CSWCModule::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0046a0b0 Global::CSWCModule_local_camera_target_Part_cleanup_helper  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0046a0d0 CSWCModule::Destructor_2 -> Steam 0x007f2eb0  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x8,0x2c,0x68,0x74,0x7c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x18,0xff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007f2910: 16-byte aligned: YES
  007f2910  55             PUSH EBP
  007f2911  8B EC          MOV EBP,ESP
  007f2913  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2910
string literals (0):
direct callees (1):
  0x0077F580  CSWCCreature::UnRegisterCallbacks
struct offsets touched (top 5):
  +0x68 [dec 104]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x2C [dec 44]  x2  via EDX
  +0x74 [dec 116]  x1  via EAX
  +0x7C [dec 124]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2910 FUN_007f2910 size=235 ret=4 callers=1 callees=1

void __thiscall FUN_007f2910(int param_1,int *param_2)

{
  int iVar1;
  int *piVar2;
  
  if (param_2 != (int *)0x0) {
    if ((*(int *)(param_1 + 0x68) != 0) && (*(undefined4 **)(param_1 + 0x68) != (undefined4 *)0x0))
    {
      (**(code **)**(undefined4 **)(param_1 + 0x68))(1);
    }
    iVar1 = (**(code **)(*param_2 + 0x2c))();
    if (iVar1 != 0) {
      (**(code **)(*param_2 + 0x2c))();
      FUN_0077f580();
    }
    *(int *)(param_1 + 0x68) = param_2[0x1a = byte +0x68];
    piVar2 = (int *)(**(code **)(*(int *)param_2[0x1a = byte +0x68] + 8))(0xff);
    (**(code **)(*piVar2 + 0x74))(param_1);
    piVar2 = (int *)(**(code **)(*(int *)param_2[0x1a = byte +0x68] + 8))(0xff);
    (**(code **)(*piVar2 + 0x7c))(*(undefined1 *)(param_1 + 8));
    param_2[0x1a = byte +0x68] = 0;
  }
  return;
}
```

### decoy: 0x007f26b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f26b0: 16-byte aligned: YES
  007f26b0  55             PUSH EBP
  007f26b1  8B EC          MOV EBP,ESP
  007f26b3  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f26b0
string literals (1):
  0x0099EDC4  at 007f271c  'lookathook'
direct callees (4):
  0x0073F490  CClientExoApp::GetModuleCamera
  0x00463120  (unnamed)
  0x0044B8A0  Vector::Dot
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via EDX
  +0x6C [dec 108]  x1  via EAX
  +0x6E [dec 110]  x1  via EAX
  +0xDC [dec 220]  x1  via EDX
```

### Static facts (script output, report-only)
- size 235 B, RET 4
- callers 1: 0x0080d050 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add
- callees: 0x0077f580 (CALL)
- indirect: 0x007f294e slot 0x0, 0x007f2967 slot 0x2c, 0x007f2978 slot 0x2c, 0x007f29a3 slot 0x8, 0x007f29b7 slot 0x74, ...
- twins: none
