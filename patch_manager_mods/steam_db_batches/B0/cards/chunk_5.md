# Review cards B0 chunk 5

Answer each card with one TSV row in `steam_db_batches/B0/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B0-5-01

GOG function: CSWSDoor::GetIsLinked  gog 0x00768410  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x0061d040 (size 193, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWSDoor::RemoveFromArea
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0061d000:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00768380 CSWSDoor::GetDisarmDC -> 0x0061cf70  gate_class=AUTO
- GOG-order neighbour after:  0x00768450 CSWSDoor::RemoveFromArea -> 0x0061d040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x3d4} | Steam {0x4}
imm sets (* = in both, 0 common): GOG {0x2} | Steam {0x20,0x130}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0061d040: 16-byte aligned: YES
  0061d040  55             PUSH EBP
  0061d041  8B EC          MOV EBP,ESP
  0061d043  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061d040
string literals (0):
direct callees (7):
  0x005453C0  CSWSObject::GetArea
  0x00523090  CSWSArea::RemoveObjectFromArea
  0x00476620  (unnamed)
  0x0052E4E0  (unnamed)
  0x005E8320  Global::RemoveStaticBoundingBox
  0x00545420  CSWSObject::SetArea
  0x00543F50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x4  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0061d040 FUN_0061d040 size=193 ret=0 callers=3 callees=7

void __fastcall FUN_0061d040(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar1 = FUN_005453c0();
  if (iVar1 != 0) {
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_00523090(uVar2);
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0();
    iVar1 = FUN_00476620(uVar2);
    if (iVar1 == 1) {
      uVar2 = *(undefined4 *)(param_1 + 4);
      FUN_005453c0();
      FUN_0052e4e0(uVar2);
    }
    uVar2 = *(undefined4 *)(param_1 + 4);
    FUN_005453c0(uVar2);
    FUN_005e8320(uVar2);
  }
  FUN_00545420(0);
  local_10 = 0;
  local_c = 0;
  local_8 = 0;
  FUN_00543f50(&local_10,1,1,0);
  return;
}
```

### decoy: 0x0061d000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061d000: 16-byte aligned: YES
  0061d000  55             PUSH EBP
  0061d001  8B EC          MOV EBP,ESP
  0061d003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0061d000
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3D4 [dec 980]  x2  via EAX,EDX
```

## Card B0-5-02

GOG function: CSWSItemPropertyHandler::RemoveChangedSavingThrow  gog 0x007c60c0  callconv=(blank)  param_size_bytes=16  gog_size=402  gog_retn=16
Candidate Steam address: 0x005aea60 (size 180, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveReducedSavingThrow
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005aeb20:STR0/CALLEE0/SIG0.448/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> 0x005aeb20  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6260 CSWSItemPropertyHandler::ApplySpecialWalk -> 0x005aee50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2,0x4,0x8,0xa,0x14,0x18,0x122} | Steam {0x6,0x130}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x7,0x10*,0x44,0x148} | Steam {0x10*,0x14,0x18}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005aea60: 16-byte aligned: YES
  005aea60  55             PUSH EBP
  005aea61  8B EC          MOV EBP,ESP
  005aea63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005aea60
string literals (1):
  0x00992E8C  at 005aea88  'Value'
direct callees (5):
  0x00733570  CExoString::CExoString_char
  0x006F0190  CTwoDimArrays::GetIPRPCostTable
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x005AECB0  CSWSItemPropertyHandler::RemoveChangedSavingThrow
struct offsets touched (top 2):
  +0x6 [dec 6]  x1  via EAX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005aea60 FUN_005aea60 size=180 ret=16 callers=0 callees=5

undefined4 FUN_005aea60(undefined4 param_1,int param_2,undefined4 param_3)

{
  uint uVar1;
  uint uVar2;
  undefined1 *puVar3;
  int *piVar4;
  undefined4 extraout_var;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00952ee8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("Value");
  local_8 = 0;
  uVar6 = *(undefined4 *)(DAT_00a1b4d0 + 0x130);
  piVar4 = &local_14;
  puVar3 = local_1c;
  uVar2 = (uint)*(ushort *)(param_2 + 6);
  uVar5 = extraout_var;
  FUN_006f0190(0x14);
  FUN_0071bcf0(uVar2,puVar3,piVar4);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1,uVar5,uVar6);
  if (local_14 != 0) {
    FUN_005aecb0(local_14,param_1,param_2,param_3);
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005aeb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aeb20: 16-byte aligned: YES
  005aeb20  55             PUSH EBP
  005aeb21  8B EC          MOV EBP,ESP
  005aeb23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aeb20
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 5):
  +0x8 [dec 8]  x2  via EAX
  +0xA [dec 10]  x2  via ECX
  +0x2 [dec 2]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x122 [dec 290]  x1  via ECX
```

## Card B0-5-03

GOG function: CServerExoApp::GetWorldTimer  gog 0x0064bd90  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c370 (size 31, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (MainLoop 0x781be0,LoadGame 0x853a00,SetDialogDelay 0x545470)
- best decoy (neighbour/other candidate) scores: 0x0051bf80:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd70 CServerExoApp::GetActiveTimer -> 0x0051c350  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bdb0 CServerExoApp::GetTimestopTimer  (not in this batch, no trusted mapping)

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
prologue for 0x0051c370: 16-byte aligned: YES
  0051c370  55             PUSH EBP
  0051c371  8B EC          MOV EBP,ESP
  0051c373  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c370
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c370 FUN_0051c370 size=31 ret=0 callers=70 callees=0

undefined4 __fastcall FUN_0051c370(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10048);
}
```

### decoy: 0x0051bf80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bf80: 16-byte aligned: YES
  0051bf80  55             PUSH EBP
  0051bf81  8B EC          MOV EBP,ESP
  0051bf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B8 [dec 65720]  x1  via ECX
```

## Card B0-5-04

GOG function: CVirtualMachineInternal::RunScriptSituation  gog 0x00645830  callconv=(blank)  param_size_bytes=12  gog_size=689  gog_retn=12
Candidate Steam address: 0x00702590 (size 689, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007022a0:STR0/CALLEE0/SIG0.52/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645549 CVirtualMachineInternal::RunScript_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00645af0 CVirtualMachineInternal::RunScriptFile -> 0x00702850  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00641680 CVirtualMachineStack::Destructor -> Steam ?  in candidate callees: n/a
  0x00641b20 CVirtualMachineStack::CopyFromStack -> Steam ?  in candidate callees: n/a
  0x00642920 CVirtualMachineScript::Destructor -> Steam ?  in candidate callees: n/a
  0x00642b70 CVirtualMachineInternal::DeleteScript -> Steam ?  in candidate callees: n/a
  0x00645af0 CVirtualMachineInternal::RunScriptFile -> Steam ?  in candidate callees: n/a
  0x00645c90 CVirtualMachineInternal::SetUpScriptSituation -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 13 common): GOG {0x4*,0x8*,0x24*,0x28*,0x2c*,0x50*,0x16c*,0x18c*,0x1ac*,0x1b4*,0x1c0*,0x1c4*,0x3d4*} | Steam {0x4*,0x8*,0x24*,0x28*,0x2c*,0x50*,0x16c*,0x18c*,0x1ac*,0x1b4*,0x1c0*,0x1c4*,0x3d4*}
imm sets (* = in both, 5 common): GOG {0x4*,0xc*,0x28*,0x5c*,0x1ac*} | Steam {0x4*,0xc*,0x28*,0x5c*,0x1ac*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x00702590: 16-byte aligned: YES
  00702590  55             PUSH EBP
  00702591  8B EC          MOV EBP,ESP
  00702593  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702590
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x006FE8E0  CVirtualMachineStack::CopyFromStack
  0x009196FD  operator delete
  0x007029F0  CVirtualMachineInternal::SetUpScriptSituation
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x006FF6E0  CVirtualMachineScript::Destructor
struct offsets touched (top 9):
  +0x28 [dec 40]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x1AC [dec 428]  x1  via EDX
  +0x1B4 [dec 436]  x1  via EAX
  +0x1C0 [dec 448]  x1  via EDX
  +0x1C4 [dec 452]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00702590 FUN_00702590 size=689 ret=12 callers=1 callees=7

undefined4 __thiscall FUN_00702590(int param_1,int *param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  undefined4 uVar2;
  
  FUN_006fe440();
  *(undefined4 *)(param_1 + 0x1ac) = *(undefined4 *)*param_2;
  *(undefined4 *)(param_1 + 0x1b4) = *(undefined4 *)(*param_2 + 0x8 [dec 8] {int*: byte +0x20});
  *(int *)(param_1 + 0x1c0) = param_1;
  FUN_006fe8e0(*param_2,0,0);
  if (*param_2 != 0) {
    iVar1 = *param_2;
    if (iVar1 != 0) {
      FUN_006fe440();
      FUN_009196fd(iVar1);
    }
    *param_2 = 0;
  }
  *(undefined4 *)(param_1 + 0x1c4) = 0;
  *(undefined4 *)(param_1 + 0x24) = 0;
  iVar1 = FUN_007029f0(param_2);
  if (iVar1 == 0) {
    if (*(int *)(param_1 + 0x50 + *(int *)(param_1 + 0x28) * 0x28) == 0) {
      *(undefined4 *)(param_1 + 0x16c + *(int *)(param_1 + 0x28) * 4) = param_4;
      *(undefined4 *)(param_1 + 0x18c + *(int *)(param_1 + 0x28) * 4) = param_3;
      if (*(int *)(param_1 + 0x3d4) != 0) {
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 4) =
             *(undefined4 *)(param_1 + 0x16c + *(int *)(param_1 + 0x28) * 4);
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 8) =
             *(undefined4 *)(param_1 + 0x18c + *(int *)(param_1 + 0x28) * 4);
      }
      iVar1 = FUN_00702850(param_2[2 = byte +0x8]);
      FUN_006ff930(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28);
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      if (param_2 != (int *)0x0) {
        FUN_006ff6e0();
        FUN_009196fd(param_2);
      }
      if (iVar1 == 0) {
        uVar2 = 1;
      }
      else {
        uVar2 = 0;
      }
    }
    else {
      FUN_006ff930(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28);
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      if (param_2 != (int *)0x0) {
        FUN_006ff6e0();
        FUN_009196fd(param_2);
      }
      uVar2 = 0;
    }
  }
  else {
    if (param_2 != (int *)0x0) {
      FUN_006ff6e0();
      FUN_009196fd(param_2);
    }
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x007022a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007022a0: 16-byte aligned: YES
  007022a0  55             PUSH EBP
  007022a1  8B EC          MOV EBP,ESP
  007022a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007022a0
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x00734010  CExoString::CStr
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 10):
  +0x28 [dec 40]  x18  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via ECX
```

## Card B0-5-05

GOG function: CServerExoApp::GetCreatureByGameObjectID  gog 0x0064bb20  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c100 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:R9-Phase2_(GOG_delta/size_mapper,_Opus_spot-chec+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 8 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 8 (ShowPartySelection 0x7cb920,OnPanelAdded 0x894320,AIActionPhysicalAttacks 0x6d7390)
- best decoy (neighbour/other candidate) scores: 0x0051c280:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bb00 CServerExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064bb40 CServerExoApp::GetAreaByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c100: 16-byte aligned: YES
  0051c100  55             PUSH EBP
  0051c101  8B EC          MOV EBP,ESP
  0051c103  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c100
string literals (0):
direct callees (1):
  0x00537B70  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c100 FUN_0051c100 size=28 ret=4 callers=395 callees=1
THUNK -> 0x00537B70 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c100(undefined4 param_1)

{
  FUN_00537b70(param_1);
  return;
}
```

### decoy: 0x0051c280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c280: 16-byte aligned: YES
  0051c280  55             PUSH EBP
  0051c281  8B EC          MOV EBP,ESP
  0051c283  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c280
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10044 [dec 65604]  x1  via ECX
```

## Card B0-5-06

GOG function: CServerExoApp::GetActiveTimer  gog 0x0064bd70  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c350 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c240:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bd40 CServerExoApp::GetServerMode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064bd90 CServerExoApp::GetWorldTimer -> 0x0051c370  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc050 CServerExoAppInternal::GetActiveTimer -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c350: 16-byte aligned: YES
  0051c350  55             PUSH EBP
  0051c351  8B EC          MOV EBP,ESP
  0051c353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c350
string literals (0):
direct callees (1):
  0x00538E00  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c350 FUN_0051c350 size=28 ret=4 callers=12 callees=1
THUNK -> 0x00538E00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c350(undefined4 param_1)

{
  FUN_00538e00(param_1);
  return;
}
```

### decoy: 0x0051c240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c240: 16-byte aligned: YES
  0051c240  55             PUSH EBP
  0051c241  8B EC          MOV EBP,ESP
  0051c243  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c240
string literals (0):
direct callees (1):
  0x00538040  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-5-07

GOG function: CVirtualMachine::StackPushFloat  gog 0x00640c20  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x006fd9e0 (size 28, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CVirtualMachine::StackPopFloat
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fd9c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c00 CVirtualMachine::StackPopFloat -> 0x006fd9e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c40 CVirtualMachine::StackPopVector -> 0x006fda20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645f50 CVirtualMachineInternal::StackPushFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fd9e0: 16-byte aligned: YES
  006fd9e0  55             PUSH EBP
  006fd9e1  8B EC          MOV EBP,ESP
  006fd9e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fd9e0
string literals (0):
direct callees (1):
  0x00702C30  CVirtualMachineInternal::StackPopFloat
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fd9e0 FUN_006fd9e0 size=28 ret=4 callers=43 callees=1
THUNK -> 0x00702C30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fd9e0(undefined4 param_1)

{
  FUN_00702c30(param_1);
  return;
}
```

### decoy: 0x006fd9c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9c0: 16-byte aligned: YES
  006fd9c0  55             PUSH EBP
  006fd9c1  8B EC          MOV EBP,ESP
  006fd9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9c0
string literals (0):
direct callees (1):
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-08

GOG function: CServerExoApp::SaveGame  gog 0x0064b810  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051bdf0 (size 40, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-savegame+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051be20:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b7d0 CServerExoApp::GetModuleName  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064b840 CServerExoApp::LoadGame -> 0x0051be20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2990 CServerExoAppInternal::SaveGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  StartSave(slot,name) (screenshot + save-state flags)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051bdf0 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=2 callees=1
THUNK -> 0x0052F610 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052f610(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051be20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-5-09

GOG function: CVirtualMachine::StackPopCommand  gog 0x00640d50  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fdb30 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdb10:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640d30 CVirtualMachine::StackPushObject -> 0x006fdb10  gate_class=REVIEW
- GOG-order neighbour after:  0x00640d70 CVirtualMachine::SaveScriptSituation -> 0x006fdb50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006463e0 CVirtualMachineInternal::StackPopCommand -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdb30: 16-byte aligned: YES
  006fdb30  55             PUSH EBP
  006fdb31  8B EC          MOV EBP,ESP
  006fdb33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdb30
string literals (0):
direct callees (1):
  0x00703140  CVirtualMachineInternal::StackPopCommand
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdb30 FUN_006fdb30 size=28 ret=4 callers=3 callees=1
THUNK -> 0x00703140 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdb30(undefined4 param_1)

{
  FUN_00703140(param_1);
  return;
}
```

### decoy: 0x006fdb10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdb10: 16-byte aligned: YES
  006fdb10  55             PUSH EBP
  006fdb11  8B EC          MOV EBP,ESP
  006fdb13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdb10
string literals (0):
direct callees (1):
  0x007030F0  CVirtualMachineInternal::StackPushObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-10

GOG function: CVirtualMachine::StackPopObject  gog 0x00640d10  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fdaf0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (ExecuteCommandSetCreatureAILevel 0x66b640,ExecuteCommandIsObjectPartyMember 0x68bbe0,ExecuteCommandGetNPCAIStyle 0x696190)
- best decoy (neighbour/other candidate) scores: 0x006fd9c0:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640cf0 CVirtualMachine::StackPushEngineStructure -> 0x006fdad0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640d30 CVirtualMachine::StackPushObject -> 0x006fdb10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646310 CVirtualMachineInternal::StackPopObject -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdaf0: 16-byte aligned: YES
  006fdaf0  55             PUSH EBP
  006fdaf1  8B EC          MOV EBP,ESP
  006fdaf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdaf0
string literals (0):
direct callees (1):
  0x00703070  CVirtualMachineInternal::StackPopObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdaf0 FUN_006fdaf0 size=28 ret=4 callers=311 callees=1
THUNK -> 0x00703070 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdaf0(undefined4 param_1)

{
  FUN_00703070(param_1);
  return;
}
```

### decoy: 0x006fd9c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fd9c0: 16-byte aligned: YES
  006fd9c0  55             PUSH EBP
  006fd9c1  8B EC          MOV EBP,ESP
  006fd9c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fd9c0
string literals (0):
direct callees (1):
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-11

GOG function: CVirtualMachine::DeleteScriptSituation  gog 0x00640dd0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fdbb0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdb30:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640da0 CVirtualMachine::LoadScriptSituation -> 0x006fdb80  gate_class=REVIEW
- GOG-order neighbour after:  0x00640df0 CScriptCompilerIdListEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646a60 CVirtualMachineInternal::DeleteScriptSituation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdbb0: 16-byte aligned: YES
  006fdbb0  55             PUSH EBP
  006fdbb1  8B EC          MOV EBP,ESP
  006fdbb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdbb0
string literals (0):
direct callees (1):
  0x007037C0  CVirtualMachineInternal::DeleteScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdbb0 FUN_006fdbb0 size=28 ret=4 callers=3 callees=1
THUNK -> 0x007037C0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdbb0(undefined4 param_1)

{
  FUN_007037c0(param_1);
  return;
}
```

### decoy: 0x006fdb30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdb30: 16-byte aligned: YES
  006fdb30  55             PUSH EBP
  006fdb31  8B EC          MOV EBP,ESP
  006fdb33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdb30
string literals (0):
direct callees (1):
  0x00703140  CVirtualMachineInternal::StackPopCommand
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-12

GOG function: CServerExoApp::SetLoadFromSaveGame  gog 0x0064c820  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051ce00 (size 36, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-savegame+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (DoQuickLoad 0x7d0f50,LoadGame 0x853a00)
- best decoy (neighbour/other candidate) scores: 0x0051ce50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c800 CServerExoApp::GetLoadFromSaveGame -> 0x0051cde0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c850 CServerExoApp::GetButtonMashCheck  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051ce00: 16-byte aligned: YES
  0051ce00  55             PUSH EBP
  0051ce01  8B EC          MOV EBP,ESP
  0051ce03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ce00
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100C4 [dec 65732]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051ce00 FUN_0051ce00 size=36 ret=4 callers=3 callees=0

void __thiscall FUN_0051ce00(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100c4) = param_2;
  return;
}
```

### decoy: 0x0051ce50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce50: 16-byte aligned: YES
  0051ce50  55             PUSH EBP
  0051ce51  8B EC          MOV EBP,ESP
  0051ce53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100D8 [dec 65752]  x1  via EDX
```

## Card B0-5-13

GOG function: CServerExoApp::GetGameObject  gog 0x0064bac0  callconv=__thiscall  param_size_bytes=4  gog_size=62  gog_retn=4
Candidate Steam address: 0x0051c0a0 (size 62, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:ai-review-2026-10-01+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 9 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 9 (MainLoop 0x781be0,GetServerObject 0x7f2540,UpdateDialog 0x7cd0f0)
- best decoy (neighbour/other candidate) scores: 0x0051c1a0:STR0/CALLEE2/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064baa0 CServerExoApp::GetObjectArray -> 0x0051c080  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bb00 CServerExoApp::GetItemByGameObjectID  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0051c0a0: 16-byte aligned: YES
  0051c0a0  55             PUSH EBP
  0051c0a1  8B EC          MOV EBP,ESP
  0051c0a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c0a0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1005C [dec 65628]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c0a0 FUN_0051c0a0 size=62 ret=4 callers=213 callees=1

undefined4 FUN_0051c0a0(undefined4 param_1)

{
  undefined4 local_8;
  
  local_8 = 0;
  FUN_0053dfb0(param_1,&local_8);
  return local_8;
}
```

### decoy: 0x0051c1a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c1a0: 16-byte aligned: YES
  0051c1a0  55             PUSH EBP
  0051c1a1  8B EC          MOV EBP,ESP
  0051c1a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c1a0
string literals (0):
direct callees (1):
  0x00537DA0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-5-14

GOG function: CVirtualMachineInternal::StackPushVector  gog 0x00646010  callconv=(blank)  param_size_bytes=12  gog_size=84  gog_retn=12
Candidate Steam address: 0x00702d70 (size 84, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00702d00:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645fa0 CVirtualMachineInternal::StackPopVector -> 0x00702d00  gate_class=AUTO
- GOG-order neighbour after:  0x00646070 CVirtualMachineInternal::StackPopString -> 0x00702dd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645f50 CVirtualMachineInternal::StackPushFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00702d70: 16-byte aligned: YES
  00702d70  55             PUSH EBP
  00702d71  8B EC          MOV EBP,ESP
  00702d73  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702d70
string literals (0):
direct callees (1):
  0x00702CB0  CVirtualMachineInternal::StackPushFloat
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00702d70 FUN_00702d70 size=84 ret=12 callers=2 callees=1

int FUN_00702d70(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 local_8;
  
  local_8 = FUN_00702cb0(param_1);
  if (local_8 == 1) {
    local_8 = FUN_00702cb0(param_2);
    if (local_8 == 1) {
      local_8 = FUN_00702cb0(param_3);
    }
  }
  return local_8;
}
```

### decoy: 0x00702d00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702d00: 16-byte aligned: YES
  00702d00  55             PUSH EBP
  00702d01  8B EC          MOV EBP,ESP
  00702d03  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702d00
string literals (0):
direct callees (1):
  0x00702C30  CVirtualMachineInternal::StackPopFloat
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B0-5-15

GOG function: CSWSEffectListHandler::Destructor  gog 0x0064ceb0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0051d4b0 (size 44, retn 4)
Candidate sources: branch+name_map:high:batch8_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d480:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ccf0 CServerAIMaster::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064cee0 CSWSItemPropertyHandler::Destructor -> 0x0051d4e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00736240 CSWSEffectListHandler::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051d4b0: 16-byte aligned: YES
  0051d4b0  55             PUSH EBP
  0051d4b1  8B EC          MOV EBP,ESP
  0051d4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d4b0
string literals (0):
direct callees (2):
  0x00593960  CSWSEffectListHandler::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051d4b0 FUN_0051d4b0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0051d4b0(undefined4 param_1,uint param_2)

{
  FUN_00593960();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0051d480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d480: 16-byte aligned: YES
  0051d480  55             PUSH EBP
  0051d481  8B EC          MOV EBP,ESP
  0051d483  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d480
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-5-16

GOG function: CVirtualMachine::StackPopEngineStructure  gog 0x00640cd0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x006fdab0 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdad0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640cb0 CVirtualMachine::StackPushString -> 0x006fda90  gate_class=REVIEW
- GOG-order neighbour after:  0x00640cf0 CVirtualMachine::StackPushEngineStructure -> 0x006fdad0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006461c0 CVirtualMachineInternal::StackPopEngineStructure -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdab0: 16-byte aligned: YES
  006fdab0  55             PUSH EBP
  006fdab1  8B EC          MOV EBP,ESP
  006fdab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdab0
string literals (0):
direct callees (1):
  0x00702F20  CVirtualMachineInternal::StackPopEngineStructure
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdab0 FUN_006fdab0 size=32 ret=8 callers=38 callees=1
THUNK -> 0x00702F20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdab0(undefined4 param_1,undefined4 param_2)

{
  FUN_00702f20(param_1,param_2);
  return;
}
```

### decoy: 0x006fdad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdad0: 16-byte aligned: YES
  006fdad0  55             PUSH EBP
  006fdad1  8B EC          MOV EBP,ESP
  006fdad3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdad0
string literals (0):
direct callees (1):
  0x00702FE0  CVirtualMachineInternal::StackPushEngineStructure
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

## Card B0-5-17

GOG function: CVirtualMachineInternal::SetUpScriptSituation  gog 0x00645c90  callconv=(blank)  param_size_bytes=4  gog_size=353  gog_retn=4
Candidate Steam address: 0x007029f0 (size 353, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702850:STR0/CALLEE0/SIG0.268/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645c50 CVirtualMachineInternal::GetRunScriptReturnValue -> 0x007029b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00645e00 CVirtualMachineInternal::StackPopInteger -> 0x00702b60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00645270 CVirtualMachineInternal::ReadScriptFile -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x20*,0x24*,0x28*,0x2c*,0x30*,0x34*,0x38*,0x3c*,0x40*,...(+3)}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x18*,0x28*,0xffffffa2*} | Steam {0x4*,0x8*,0xc*,0x18*,0x28*,0xffffffa2*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007029f0: 16-byte aligned: YES
  007029f0  55             PUSH EBP
  007029f1  8B EC          MOV EBP,ESP
  007029f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007029f0
string literals (0):
direct callees (2):
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x007337C0  CExoString::operator=
struct offsets touched (top 8):
  +0x28 [dec 40]  x14  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007029f0 FUN_007029f0 size=353 ret=4 callers=1 callees=2

int __thiscall FUN_007029f0(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = param_2[8 = byte +0x20];
  if (param_2[4 = byte +0x10] == 0) {
    iVar1 = FUN_00701fd0(param_2 + 0x6 [dec 6] {undefined4*: byte +0x18});
    if (iVar1 != 0) {
      return iVar1;
    }
    local_8 = 0;
  }
  else {
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + 1;
    if (7 < *(int *)(param_1 + 0x28)) {
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      return -0x5e;
    }
  }
  *(undefined4 *)(param_1 + 0x4c + *(int *)(param_1 + 0x28) * 0x28) = local_8;
  FUN_007337c0(param_2 + 0x6 [dec 6] {undefined4*: byte +0x18});
  *(undefined4 *)(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28) = *param_2;
  *(undefined4 *)(param_1 + 0x30 + *(int *)(param_1 + 0x28) * 0x28) = param_2[1 = byte +0x4];
  *(undefined4 *)(param_1 + 0x34 + *(int *)(param_1 + 0x28) * 0x28) = param_2[2 = byte +0x8];
  *(undefined4 *)(param_1 + 0x38 + *(int *)(param_1 + 0x28) * 0x28) = param_2[3 = byte +0xc];
  *(undefined4 *)(param_1 + 0x50 + *(int *)(param_1 + 0x28) * 0x28) = param_2[9 = byte +0x24];
  if (param_2[4 = byte +0x10] != 0) {
    *(undefined4 *)(param_1 + 0x3c + *(int *)(param_1 + 0x28) * 0x28) = param_2[4 = byte +0x10];
    *(undefined4 *)(param_1 + 0x40 + *(int *)(param_1 + 0x28) * 0x28) = param_2[5 = byte +0x14];
  }
  *param_2 = 0;
  param_2[4 = byte +0x10] = 0;
  return 0;
}
```

### decoy: 0x00702850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702850: 16-byte aligned: YES
  00702850  55             PUSH EBP
  00702851  8B EC          MOV EBP,ESP
  00702853  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00702850
string literals (0):
direct callees (7):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00701F80  CVirtualMachineInternal::PushInstructionPtr
  0x006FF9F0  CVirtualMachineInternal::ExecuteCode
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FF450  CVirtualMachineStack::SetStackPointer
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x28 [dec 40]  x3  via EAX,ECX,EDX
  +0x1C4 [dec 452]  x3  via EAX,EDX
  +0x1AC [dec 428]  x2  via ECX
  +0x3D4 [dec 980]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
```

## Card B0-5-18

GOG function: CVirtualMachineInternal::StackPushString  gog 0x00646100  callconv=(blank)  param_size_bytes=4  gog_size=188  gog_retn=4
Candidate Steam address: 0x00702e60 (size 188, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702cb0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00646070 CVirtualMachineInternal::StackPopString -> 0x00702dd0  gate_class=REVIEW
- GOG-order neighbour after:  0x006461c0 CVirtualMachineInternal::StackPopEngineStructure -> 0x00702f20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x006416a0 CVirtualMachineStack::AddToTopOfStack -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1bc*} | Steam {0x1bc*}
imm sets (* = in both, 5 common): GOG {0x4*,0x5*,0x8*,0x18*,0x1ac*} | Steam {0x4*,0x5*,0x8*,0x18*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702e60: 16-byte aligned: YES
  00702e60  55             PUSH EBP
  00702e61  8B EC          MOV EBP,ESP
  00702e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00702e60
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00702e60 FUN_00702e60 size=188 ret=4 callers=2 callees=5

undefined4 __fastcall FUN_00702e60(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_28;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f1cb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = FUN_00919723(8,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (iVar1 == 0) {
    local_28 = 0;
  }
  else {
    uVar2 = FUN_00734010();
    local_28 = FUN_00733570(uVar2);
  }
  local_8 = 0xffffffff;
  FUN_006fe460(5);
  iVar1 = FUN_006ff440();
  *(undefined4 *)(*(int *)(param_1 + 0x1bc) + -4 + iVar1 * 4) = local_28;
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x00702cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702cb0: 16-byte aligned: YES
  00702cb0  55             PUSH EBP
  00702cb1  8B EC          MOV EBP,ESP
  00702cb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702cb0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

## Card B0-5-19

GOG function: CServerExoApp::GetObjectArray  gog 0x0064baa0  callconv=__thiscall  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051c080 (size 31, retn 0)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (StopDialog 0x546900,GetNearestObjectByName 0x546bd0,GetAIStateReputation 0x5e1680)
- best decoy (neighbour/other candidate) scores: 0x0051bd90:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ba80 CServerExoApp::LoadCharacterFinish  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064bac0 CServerExoApp::GetGameObject -> 0x0051c0a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c080: 16-byte aligned: YES
  0051c080  55             PUSH EBP
  0051c081  8B EC          MOV EBP,ESP
  0051c083  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c080
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x1005C [dec 65628]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c080 FUN_0051c080 size=31 ret=0 callers=261 callees=0

undefined4 __fastcall FUN_0051c080(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x1005c);
}
```

### decoy: 0x0051bd90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bd90: 16-byte aligned: YES
  0051bd90  55             PUSH EBP
  0051bd91  8B EC          MOV EBP,ESP
  0051bd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bd90
string literals (0):
direct callees (1):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-5-20

GOG function: CServerExoApp::GetLoadFromSaveGame  gog 0x0064c800  callconv=(blank)  param_size_bytes=0  gog_size=31  gog_retn=0
Candidate Steam address: 0x0051cde0 (size 31, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-savegame+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051ce30:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c7d0 CServerExoApp::ComputeSafeLocation  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064c820 CServerExoApp::SetLoadFromSaveGame -> 0x0051ce00  gate_class=REVIEW

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

### decoy: 0x0051ce30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051ce30: 16-byte aligned: YES
  0051ce30  55             PUSH EBP
  0051ce31  8B EC          MOV EBP,ESP
  0051ce33  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051ce30
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100D8 [dec 65752]  x1  via ECX
```

## Card B0-5-21

GOG function: CSWSEffectListHandler::OnApplyCurse  gog 0x00744fe0  callconv=(blank)  param_size_bytes=12  gog_size=1376  gog_retn=12
Candidate Steam address: 0x005a2c70 (size 536, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplySilence
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.57 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a2710:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744f70 CSWSEffectListHandler::OnRemoveModifyNumAttacks -> 0x005a26a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00745540 CSWSEffectListHandler::OnApplySilence -> 0x005a2c70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x18*,0x30*,0xfc*,0x1198*} | Steam {0x4*,0x8*,0x18*,0x30*,0xfc*,0xfd0,0x1198*}
imm sets (* = in both, 5 common): GOG {0x2*,0x3,0x4*,0x5,0x6,0x7,0xc*,0x11,0x25,0x34*,0x8c*,0x8d,0xac} | Steam {0x2*,0x4*,0xb,0xc*,0x17,0x34*,0x50,0x8c*,0x91,0xfffffff7}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a2c70: 16-byte aligned: YES
  005a2c70  55             PUSH EBP
  005a2c71  8B EC          MOV EBP,ESP
  005a2c73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005a2c70
string literals (0):
direct callees (10):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x30 [dec 48]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0xFC [dec 252]  x1  via ECX
  +0xFD0 [dec 4048]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2c70 FUN_005a2c70 size=536 ret=12 callers=0 callees=10

undefined4 FUN_005a2c70(int *param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int local_60;
  undefined4 local_5c;
  undefined4 local_58;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009526a1;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  iVar1 = (**(code **)(*param_1 + 0x30))(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar1 != 0) {
    iVar2 = FUN_0051c100(*(undefined4 *)(param_2 + 0x18));
    iVar3 = FUN_006bc570(0xb,iVar2);
    if (iVar3 != 0) {
      iVar1 = FUN_00919723(0x34);
      local_8 = 0;
      if (iVar1 == 0) {
        local_58 = 0;
      }
      else {
        local_58 = FUN_005e2540();
      }
      local_8 = 0xffffffff;
      FUN_005e1c70(0,param_1[1 = byte +0x4]);
      if (iVar2 != 0) {
        iVar1 = FUN_00919723(0x34);
        local_8 = 1;
        if (iVar1 == 0) {
          local_5c = 0;
        }
        else {
          local_5c = FUN_005e2540();
        }
        local_8 = 0xffffffff;
        FUN_005e2be0(local_5c);
        FUN_00584da0(0x91,local_5c);
      }
      uVar4 = 0x91;
      (**(code **)(*param_1 + 0x30))(0x91,local_58);
      FUN_00584da0(uVar4,local_58);
      ExceptionList = local_10;
      return 1;
    }
    if (*(int *)(iVar1 + 0xfc) != 0) {
      ExceptionList = local_10;
      return 1;
    }
    iVar2 = FUN_00919723(0x8c);
    local_8 = 2;
    if (iVar2 == 0) {
      local_60 = 0;
    }
    else {
      local_60 = FUN_005e4ac0(param_2);
    }
    local_8 = 0xffffffff;
    *(undefined2 *)(local_60 + 8) = 0x17;
    FUN_005e4e80(0,0xfffffff7);
    FUN_00544210(local_60,param_3,0);
    *(undefined4 *)(iVar1 + 0xfd0) = 1;
  }
  ExceptionList = local_10;
  return 0;
}
```

### decoy: 0x005a2710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2710: 16-byte aligned: YES
  005a2710  55             PUSH EBP
  005a2711  8B EC          MOV EBP,ESP
  005a2713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2710
string literals (0):
direct callees (13):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x006BC570  CSWSCreatureStats::GetEffectImmunity
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1C70  (unnamed)
  0x005E2BE0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4E80  CGameEffect::SetInteger
  0x005E4E60  CGameEffect::GetInteger
  0x00544210  CSWSObject::ApplyEffect
  0x0051C280  (unnamed)
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 6):
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0xFC [dec 252]  x1  via ECX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B0-5-22

GOG function: CVirtualMachine::StackPushObject  gog 0x00640d30  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fdb10 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdaf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640d10 CVirtualMachine::StackPopObject -> 0x006fdaf0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640d50 CVirtualMachine::StackPopCommand -> 0x006fdb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646390 CVirtualMachineInternal::StackPushObject -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdb10: 16-byte aligned: YES
  006fdb10  55             PUSH EBP
  006fdb11  8B EC          MOV EBP,ESP
  006fdb13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdb10
string literals (0):
direct callees (1):
  0x007030F0  CVirtualMachineInternal::StackPushObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdb10 FUN_006fdb10 size=28 ret=4 callers=76 callees=1
THUNK -> 0x007030F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdb10(undefined4 param_1)

{
  FUN_007030f0(param_1);
  return;
}
```

### decoy: 0x006fdaf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdaf0: 16-byte aligned: YES
  006fdaf0  55             PUSH EBP
  006fdaf1  8B EC          MOV EBP,ESP
  006fdaf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdaf0
string literals (0):
direct callees (1):
  0x00703070  CVirtualMachineInternal::StackPopObject
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-23

GOG function: CServerExoApp::GetPlayerCreatureId  gog 0x0064c310  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c8f0 (size 22, retn 0)
Candidate sources: branch+mapper:vote+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (ShowPartySelection 0x7cb920,UpdateDialog 0x7cd0f0,OnPanelAdded 0x894320)
- best decoy (neighbour/other candidate) scores: 0x0051c8d0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> 0x0051c8d0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c330 CServerExoApp::TestDirectLine  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcfb0 CServerExoAppInternal::GetPlayerCreatureId -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c8f0: 16-byte aligned: YES
  0051c8f0  55             PUSH EBP
  0051c8f1  8B EC          MOV EBP,ESP
  0051c8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c8f0
string literals (0):
direct callees (1):
  0x00539D60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c8f0 FUN_0051c8f0 size=22 ret=0 callers=117 callees=1
THUNK -> 0x00539D60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0051c8f0(undefined4 param_1)

{
  FUN_00539d60(param_1);
  return;
}
```

### decoy: 0x0051c8d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-5-24

GOG function: CVirtualMachineInternal::DeleteScript  gog 0x00642b70  callconv=(blank)  param_size_bytes=4  gog_size=192  gog_retn=4
Candidate Steam address: 0x006ff930 (size 192, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x006ff8a0:STR0/CALLEE0/SIG0.629/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00642ae0 CVirtualMachineInternal::InitializeScript -> 0x006ff8a0  gate_class=AUTO
- GOG-order neighbour after:  0x00642c30 CVirtualMachineInternal::ExecuteCode -> 0x006ff9f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00641680 CVirtualMachineStack::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x24*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x24*}
imm sets (* = in both, 3 common): GOG {0x4*,0xd*,0x14*} | Steam {0x4*,0xd*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006ff930: 16-byte aligned: YES
  006ff930  55             PUSH EBP
  006ff931  8B EC          MOV EBP,ESP
  006ff933  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006ff930
string literals (0):
direct callees (2):
  0x006FE440  CVirtualMachineStack::Destructor
  0x009196FD  operator delete
struct offsets touched (top 6):
  +0x10 [dec 16]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006ff930 FUN_006ff930 size=192 ret=4 callers=4 callees=2

undefined4 FUN_006ff930(int *param_1)

{
  int iVar1;
  
  if (*param_1 != 0) {
    iVar1 = *param_1;
    if (iVar1 != 0) {
      FUN_006fe440();
      FUN_009196fd(iVar1);
    }
    *param_1 = 0;
  }
  if (param_1[4 = byte +0x10] != 0) {
    FUN_009196fd(param_1[4 = byte +0x10]);
    param_1[4 = byte +0x10] = 0;
  }
  param_1[1 = byte +0x4] = 0;
  param_1[2 = byte +0x8] = 0xd;
  param_1[3 = byte +0xc] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[9 = byte +0x24] = 0;
  return 1;
}
```

### decoy: 0x006ff8a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ff8a0: 16-byte aligned: YES
  006ff8a0  55             PUSH EBP
  006ff8a1  8B EC          MOV EBP,ESP
  006ff8a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006ff8a0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 7):
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

## Card B0-5-25

GOG function: CVirtualMachineStack::GetStackPointer  gog 0x00642680  callconv=(blank)  param_size_bytes=0  gog_size=16  gog_retn=0
Candidate Steam address: 0x006ff440 (size 16, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006ff2a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006424e0 CVirtualMachineStack::ClearStack_2 -> 0x006ff2a0  gate_class=AUTO
- GOG-order neighbour after:  0x00642690 CVirtualMachineStack::SetStackPointer -> 0x006ff450  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x006ff440: 16-byte aligned: YES
  006ff440  55             PUSH EBP
  006ff441  8B EC          MOV EBP,ESP
  006ff443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006ff440
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006ff440 FUN_006ff440 size=16 ret=0 callers=14 callees=0

undefined4 __fastcall FUN_006ff440(undefined4 *param_1)

{
  return *param_1;
}
```

### decoy: 0x006ff2a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ff2a0: 16-byte aligned: YES
  006ff2a0  55             PUSH EBP
  006ff2a1  8B EC          MOV EBP,ESP
  006ff2a3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006ff2a0
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 7):
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x10 [dec 16]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x3D4 [dec 980]  x1  via ECX
```

## Card B0-5-26

GOG function: CServerExoApp::GetGlobalVariableTable  gog 0x0064c2b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c890 (size 22, retn 0)
Candidate sources: branch+mapper:vote+name_map:medium:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c870:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c290 CServerExoApp::GetFirstPCObject  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> 0x0051c8b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x100fc*} | Steam {0x100fc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c890: 16-byte aligned: YES
  0051c890  55             PUSH EBP
  0051c891  8B EC          MOV EBP,ESP
  0051c893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c890
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c890 FUN_0051c890 size=22 ret=0 callers=35 callees=0

int __fastcall FUN_0051c890(int param_1)

{
  return *(int *)(param_1 + 4) + 0x100fc;
}
```

### decoy: 0x0051c870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c870: 16-byte aligned: YES
  0051c870  55             PUSH EBP
  0051c871  8B EC          MOV EBP,ESP
  0051c873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c870
string literals (0):
direct callees (1):
  0x00539BB0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-5-27

GOG function: Scene::ChildAdd  gog 0x00857f90  callconv=__thiscall  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x00465c20 (size 112, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=Scene::ChildRemove
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465bb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857ec0 Scene::UnlistSpecialParts  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00858000 Scene::ChildRemove -> 0x00465c20  gate_class=REVIEW

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
prologue for 0x00465c20: 16-byte aligned: YES
  00465c20  55             PUSH EBP
  00465c21  8B EC          MOV EBP,ESP
  00465c23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 00465c20 FUN_00465c20 size=112 ret=4 callers=0 callees=3

void __thiscall FUN_00465c20(undefined4 param_1,int *param_2)

{
  int iVar1;
  
  if (param_2[0x21 = byte +0x84] != 0) {
    FUN_00495fb0(param_1,param_2);
    FUN_00465ae0(param_2[0x26 = byte +0x98]);
  }
  FUN_00449170(param_2);
  iVar1 = (**(code **)(*param_2 + 0x1a8))();
  if (iVar1 != 0) {
    FUN_00449170(param_2);
  }
  return;
}
```

### decoy: 0x00465bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465bb0: 16-byte aligned: YES
  00465bb0  55             PUSH EBP
  00465bb1  8B EC          MOV EBP,ESP
  00465bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B0-5-28

GOG function: CSWSItemPropertyHandler::RemoveChangedSavingThrow  gog 0x007c60c0  callconv=(blank)  param_size_bytes=16  gog_size=402  gog_retn=16
Candidate Steam address: 0x005af020 (size 392, retn 16)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSItemPropertyHandler::RemoveSpecialWalk
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.55 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.55
- best decoy (neighbour/other candidate) scores: 0x005aee50:STR0/CALLEE0/SIG0.286/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007c5f30 CSWSItemPropertyHandler::ApplyChangedSavingThrow -> 0x005aeb20  gate_class=REVIEW
- GOG-order neighbour after:  0x007c6260 CSWSItemPropertyHandler::ApplySpecialWalk -> 0x005aee50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006ab470 CSWSObject::RemoveEffectById -> Steam ?  in candidate callees: n/a
  0x00734b20 CGameEffect::GetInteger -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x122} | Steam {0x2*,0x4*,0x8*,0xa*,0x14*,0x18*,0x15c,0x1198}
imm sets (* = in both, 3 common): GOG {0x2,0x3,0x7*,0x10*,0x44,0x148*} | Steam {0x7*,0x10*,0x4c,0x148*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x005af020: 16-byte aligned: YES
  005af020  55             PUSH EBP
  005af021  8B EC          MOV EBP,ESP
  005af023  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005af020
string literals (0):
direct callees (2):
  0x005E4E60  CGameEffect::GetInteger
  0x00544790  CSWSObject::RemoveEffectById
struct offsets touched (top 8):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0xA [dec 10]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
  +0x15C [dec 348]  x1  via EAX
  +0x1198 [dec 4504]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005af020 FUN_005af020 size=392 ret=16 callers=0 callees=2

undefined4 FUN_005af020(int param_1,int param_2,int param_3)

{
  ushort uVar1;
  undefined4 *puVar2;
  uint uVar3;
  uint local_10;
  
  local_10 = (uint)*(ushort *)(*(int *)(param_3 + 0x1198) + 0x15c);
  while( true ) {
    if (*(int *)(param_3 + 0x14c) <= (int)local_10) {
      return 0;
    }
    if ((int)DAT_00996008 <
        (int)(uint)*(ushort *)(*(int *)(*(int *)(param_3 + 0x148) + local_10 * 4) + 8)) break;
    puVar2 = *(undefined4 **)(*(int *)(param_3 + 0x148) + local_10 * 4);
    if ((((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996004) && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14))) &&
        (uVar1 = *(ushort *)(param_2 + 2), uVar3 = FUN_005e4e60(0), uVar3 == uVar1)) &&
       ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
      FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
    }
    else if (((*(ushort *)(puVar2 + 0x2 [dec 2] {undefined4*: byte +0x8}) == DAT_00996008) && (puVar2[6 = byte +0x18] == *(int *)(param_1 + 0x14)))
            && ((*(ushort *)((int)puVar2 + 0xa [dec 10]) & 7) == DAT_009967b0)) {
      FUN_00544790(*puVar2,puVar2[1 = byte +0x4]);
      return 0;
    }
    local_10 = local_10 + 1;
  }
  return 0;
}
```

### decoy: 0x005aee50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005aee50: 16-byte aligned: YES
  005aee50  55             PUSH EBP
  005aee51  8B EC          MOV EBP,ESP
  005aee53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005aee50
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005E4920  CGameEffect::Constructor
  0x005E5EA0  CGameEffect::SetCreator
  0x005E4E80  CGameEffect::SetInteger
  0x00544210  CSWSObject::ApplyEffect
struct offsets touched (top 4):
  +0xA [dec 10]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x2 [dec 2]  x1  via EDX
```

## Card B0-5-29

GOG function: Scene::ChildAdd  gog 0x00857f90  callconv=__thiscall  param_size_bytes=4  gog_size=112  gog_retn=4
Candidate Steam address: 0x00465ae0 (size 203, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x00465bb0
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00465bb0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00857ec0 Scene::UnlistSpecialParts  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00858000 Scene::ChildRemove -> 0x00465c20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00857dd0 Scene::ListSpecialPartsR -> Steam ?  in candidate callees: n/a
  0x0086d830 Global::SceneBSPAddGob -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x84,0x98,0x1a8} | Steam {0x10,0x24,0x30,0x34,0x8c}
imm sets (* = in both, 2 common): GOG {0x4*,0x8*,0x54,0x10c} | Steam {0x4*,0x8*,0x10,0xa0}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00465ae0: 16-byte aligned: YES
  00465ae0  55             PUSH EBP
  00465ae1  8B EC          MOV EBP,ESP
  00465ae3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00465ae0
string literals (0):
direct callees (3):
  0x004964F0  (unnamed)
  0x00449170  (unnamed)
  0x00465AE0  (unnamed)
struct offsets touched (top 5):
  +0x34 [dec 52]  x2  via EDX
  +0x8C [dec 140]  x2  via ECX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00465ae0 FUN_00465ae0 size=203 ret=4 callers=2 callees=3

void __thiscall FUN_00465ae0(int param_1,int *param_2)

{
  int iVar1;
  int local_10;
  
  if (param_2 != (int *)0x0) {
    iVar1 = (**(code **)(*param_2 + 0x24))();
    if (iVar1 != 0) {
      (**(code **)(**(int **)(param_1 + 0x8c) + 0x10))(iVar1,0);
      FUN_004964f0(param_1,iVar1);
    }
    iVar1 = (**(code **)(*param_2 + 0x34))();
    if (iVar1 != 0) {
      FUN_00449170(iVar1);
      FUN_004964f0(param_1,iVar1);
    }
    for (local_10 = 0; local_10 < param_2[0xd = byte +0x34]; local_10 = local_10 + 1) {
      FUN_00465ae0(*(undefined4 *)(param_2[0xc = byte +0x30] + local_10 * 4));
    }
  }
  return;
}
```

### decoy: 0x00465bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00465bb0: 16-byte aligned: YES
  00465bb0  55             PUSH EBP
  00465bb1  8B EC          MOV EBP,ESP
  00465bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B0-5-30

GOG function: CSWSItem::Constructor  gog 0x006634d0  callconv=(blank)  param_size_bytes=4  gog_size=772  gog_retn=4
Candidate Steam address: 0x005ffd60 (size 772, retn 4)
Candidate sources: branch+name_map:high:gog_item_cluster+near    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (AddObject 0x51e040); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00600070:STR0/CALLEE0/SIG0.025/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663170 CSWSCreature::SetAttackSummaryFeedbackEffectData  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006637e0 CSWSObject::GetDialogResref -> 0x0053ef30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004ecb70 CSWItem::Constructor -> Steam ?  in candidate callees: n/a
  0x00607350 CExoLocString::Constructor -> Steam ?  in candidate callees: n/a
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064da60 CServerAIMaster::AddObject -> Steam 0x0051e040  in candidate callees: yes
  0x006a55e0 CSWSObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 26 common): GOG {0x8*,0x10*,0xe8*,0x278*,0x279*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x2a0*,0x2a4*,0x2a5*,...(+12)} | Steam {0x8*,0x10*,0xe8*,0x278*,0x279*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x2a0*,0x2a4*,0x2a5*,...(+12)}
imm sets (* = in both, 20 common): GOG {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x18*,0x32*,0x2b0*,0x2b8*,0x2c0*,0x2f4*,0x2711*,0xfffffeff*,...(+6)} | Steam {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x18*,0x32*,0x2b0*,0x2b8*,0x2c0*,0x2f4*,0x2711*,0xfffffeff*,...(+6)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005ffd60: 16-byte aligned: YES
  005ffd60  55             PUSH EBP
  005ffd61  8B EC          MOV EBP,ESP
  005ffd63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005ffd60
string literals (0):
direct callees (6):
  0x006D6DA0  (unnamed)
  0x0053E830  CSWSObject::Constructor
  0x00735240  (unnamed)
  0x0051C280  (unnamed)
  0x0051E040  CServerAIMaster::AddObject
  0x0091C630  (unnamed)
struct offsets touched (top 10):
  +0x2C8 [dec 712]  x18  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0xE8 [dec 232]  x1  via EAX
  +0x278 [dec 632]  x1  via EAX
  +0x284 [dec 644]  x1  via EDX
  +0x288 [dec 648]  x1  via EAX
  +0x28C [dec 652]  x1  via ECX
  +0x290 [dec 656]  x1  via EDX
  +0x294 [dec 660]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005ffd60 FUN_005ffd60 size=772 ret=4 callers=29 callees=6

undefined4 * __thiscall FUN_005ffd60(undefined4 *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 *local_20;
  byte local_11;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00954d7d;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_006d6da0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_0053e830(DAT_00994475,param_2,0);
  local_8._0_1_ = 1;
  *param_1 = CSWSItem::vftable;
  param_1[4 = byte +0x10] = CSWSItem::vftable;
  FUN_00735240();
  local_8._0_1_ = 2;
  FUN_00735240();
  local_8._0_1_ = 3;
  FUN_00735240();
  local_8 = CONCAT31(local_8._1_3_,4);
  param_1[0xab = byte +0x2ac] = 0;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xfffffffd;
  if (param_1 == (undefined4 *)0x0) {
    local_20 = (undefined4 *)0x0;
  }
  else {
    local_20 = param_1 + 0x4 [dec 4] {undefined4*: byte +0x10};
  }
  uVar1 = 0;
  FUN_0051c280(local_20,0);
  FUN_0051e040(local_20,uVar1);
  param_1[0x3a = byte +0xe8] = 0x2711;
  param_1[0xaa = byte +0x2a8] = DAT_00994484;
  param_1[0xa2 = byte +0x288] = 0;
  param_1[0xa3 = byte +0x28c] = 0;
  param_1[0xa4 = byte +0x290] = 0;
  param_1[0xa5 = byte +0x294] = 0;
  param_1[0xa6 = byte +0x298] = 0x32;
  param_1[0xa8 = byte +0x2a0] = 0;
  *(undefined1 *)(param_1 + 0x9e) = 0;
  for (local_11 = 0; local_11 < 8; local_11 = local_11 + 1) {
    *(undefined1 *)((int)param_1 + local_11 + 0x279) = 0;
  }
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xfffffffe;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xfffffffb;
  param_1[0xa1 = byte +0x284] = 0;
  param_1[0xb3 = byte +0x2cc] = 1;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xffffffdf;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] | 8;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] | 0x10;
  *(undefined1 *)(param_1 + 0xa9) = 1;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xffffffbf;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xffffff7f;
  param_1[0xb2 = byte +0x2c8] = param_1[0xb2 = byte +0x2c8] & 0xfffffeff;
  param_1[0xb5 = byte +0x2d4] = 0;
  *(undefined1 *)((int)param_1 + 0x2a5) = 0;
  *(undefined1 *)((int)param_1 + 0x2a6) = 0;
  param_1[0xb7 = byte +0x2dc] = 0;
  param_1[0xb8 = byte +0x2e0] = 0;
  param_1[0xbc = byte +0x2f0] = 0;
  param_1[0xba = byte +0x2e8] = 0;
  param_1[0xb9 = byte +0x2e4] = 0;
  param_1[0xbb = byte +0x2ec] = 0;
  _memset [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0xbd,0,0x18);
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00600070 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00600070: 16-byte aligned: YES
  00600070  55             PUSH EBP
  00600071  8B EC          MOV EBP,ESP
  00600073  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00600070
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B0-5-31

GOG function: CVirtualMachineInternal::StackPushFloat  gog 0x00645f50  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x00702cb0 (size 72, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702be0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645ed0 CVirtualMachineInternal::StackPopFloat -> 0x00702c30  gate_class=REVIEW
- GOG-order neighbour after:  0x00645fa0 CVirtualMachineInternal::StackPopVector -> 0x00702d00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006416a0 CVirtualMachineStack::AddToTopOfStack -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1bc*} | Steam {0x1bc*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x1ac*} | Steam {0x4*,0x8*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702cb0: 16-byte aligned: YES
  00702cb0  55             PUSH EBP
  00702cb1  8B EC          MOV EBP,ESP
  00702cb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702cb0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00702cb0 FUN_00702cb0 size=72 ret=4 callers=3 callees=2

undefined4 __thiscall FUN_00702cb0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  FUN_006fe460(4);
  iVar1 = FUN_006ff440();
  *(undefined4 *)(*(int *)(param_1 + 0x1bc) + -4 + iVar1 * 4) = param_2;
  return 1;
}
```

### decoy: 0x00702be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702be0: 16-byte aligned: YES
  00702be0  55             PUSH EBP
  00702be1  8B EC          MOV EBP,ESP
  00702be3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702be0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

## Card B0-5-32

GOG function: CVirtualMachine::SaveScriptSituation  gog 0x00640d70  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x006fdb50 (size 36, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdb80:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x00640d50 CVirtualMachine::StackPopCommand -> 0x006fdb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00640da0 CVirtualMachine::LoadScriptSituation -> 0x006fdb80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646610 CVirtualMachineInternal::SaveScriptSituation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdb50: 16-byte aligned: YES
  006fdb50  55             PUSH EBP
  006fdb51  8B EC          MOV EBP,ESP
  006fdb53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdb50
string literals (0):
direct callees (1):
  0x00703370  CVirtualMachineInternal::SaveScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdb50 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x00703370 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00703370(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x006fdb80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdb80: 16-byte aligned: YES
  006fdb80  55             PUSH EBP
  006fdb81  8B EC          MOV EBP,ESP
  006fdb83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdb80
string literals (0):
direct callees (1):
  0x00703530  CVirtualMachineInternal::LoadScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

## Card B0-5-33

GOG function: CVirtualMachineInternal::GetRunScriptReturnValue  gog 0x00645c50  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x007029b0 (size 53, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702850:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645af0 CVirtualMachineInternal::RunScriptFile -> 0x00702850  gate_class=REVIEW
- GOG-order neighbour after:  0x00645c90 CVirtualMachineInternal::SetUpScriptSituation -> 0x007029f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x20*} | Steam {0x1c*,0x20*}
imm sets (* = in both, 2 common): GOG {0x3*,0x8*} | Steam {0x3*,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007029b0: 16-byte aligned: YES
  007029b0  55             PUSH EBP
  007029b1  8B EC          MOV EBP,ESP
  007029b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007029b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x20 [dec 32]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007029b0 FUN_007029b0 size=53 ret=8 callers=1 callees=0

bool __thiscall FUN_007029b0(int param_1,undefined4 *param_2,undefined4 *param_3)

{
  bool bVar1;
  
  bVar1 = *(int *)(param_1 + 0x1c) == 3;
  if (bVar1) {
    *param_2 = *(undefined4 *)(param_1 + 0x1c);
    *param_3 = *(undefined4 *)(param_1 + 0x20);
  }
  return bVar1;
}
```

### decoy: 0x00702850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702850: 16-byte aligned: YES
  00702850  55             PUSH EBP
  00702851  8B EC          MOV EBP,ESP
  00702853  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00702850
string literals (0):
direct callees (7):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00701F80  CVirtualMachineInternal::PushInstructionPtr
  0x006FF9F0  CVirtualMachineInternal::ExecuteCode
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FF450  CVirtualMachineStack::SetStackPointer
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x28 [dec 40]  x3  via EAX,ECX,EDX
  +0x1C4 [dec 452]  x3  via EAX,EDX
  +0x1AC [dec 428]  x2  via ECX
  +0x3D4 [dec 980]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
```

## Card B0-5-34

GOG function: CVirtualMachineInternal::RunScript  gog 0x00645540  callconv=(blank)  param_size_bytes=12  gog_size=9  gog_retn=None
Candidate Steam address: 0x007022a0 (size 740, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00701fd0:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006452b0 CVirtualMachineInternal::ReadScriptFile_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00645549 CVirtualMachineInternal::RunScript_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1c,0x20,0x24,0x28,0x2c,0x50,0x16c,0x18c,0x1b8,0x1bc,0x1c0,...(+2)}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x3,0xc,0x10*,0x28,0x1ac}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x007022a0: 16-byte aligned: YES
  007022a0  55             PUSH EBP
  007022a1  8B EC          MOV EBP,ESP
  007022a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007022a0
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x00734010  CExoString::CStr
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 10):
  +0x28 [dec 40]  x18  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007022a0 FUN_007022a0 size=740 ret=12 callers=1 callees=7

undefined4 __thiscall FUN_007022a0(int param_1,int *param_2,undefined4 param_3,undefined4 param_4)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  
  if (param_2 != (int *)0x0) {
    if ((*param_2 == 0) || (*(char *)*param_2 == '\0')) {
      bVar1 = true;
    }
    else {
      bVar1 = false;
    }
    if (!bVar1) {
      if (*(int *)(param_1 + 0x28) == -1) {
        FUN_006fe440();
        *(int *)(param_1 + 0x1c0) = param_1;
        *(undefined4 *)(param_1 + 0x24) = 0;
        *(undefined4 *)(param_1 + 0x1c4) = 0;
      }
      iVar2 = FUN_00701fd0(param_2);
      if (iVar2 != 0) {
        if (*(int *)(param_1 + 0x28) != -1) {
          FUN_00734010();
        }
        return 0;
      }
      *(undefined4 *)(param_1 + 0x50 + *(int *)(param_1 + 0x28) * 0x28) = 0;
      *(undefined4 *)(param_1 + 0x16c + *(int *)(param_1 + 0x28) * 4) = param_4;
      *(undefined4 *)(param_1 + 0x18c + *(int *)(param_1 + 0x28) * 4) = param_3;
      if (*(int *)(param_1 + 0x3d4) != 0) {
        (**(code **)(**(int **)(param_1 + 0x3d4) + 0xc))(param_2);
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 4) =
             *(undefined4 *)(param_1 + 0x16c + *(int *)(param_1 + 0x28) * 4);
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 8) =
             *(undefined4 *)(param_1 + 0x18c + *(int *)(param_1 + 0x28) * 4);
      }
      iVar2 = FUN_006ff440();
      iVar3 = FUN_00702850(0);
      FUN_006ff930(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28);
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      if ((*(int *)(param_1 + 0x28) != -1) && (*(int *)(param_1 + 0x3d4) != 0)) {
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 4) =
             *(undefined4 *)(param_1 + 0x16c + *(int *)(param_1 + 0x28) * 4);
        *(undefined4 *)(*(int *)(param_1 + 0x3d4) + 8) =
             *(undefined4 *)(param_1 + 0x18c + *(int *)(param_1 + 0x28) * 4);
      }
      if (iVar3 == 0) {
        if (*(int *)(param_1 + 0x28) != -1) {
          FUN_00734010();
        }
        iVar3 = FUN_006ff440();
        if (iVar3 == iVar2 + 1) {
          if (*(char *)(*(int *)(param_1 + 0x1b8) + iVar2) == '\x03') {
            *(undefined4 *)(param_1 + 0x1c) = 3;
            *(undefined4 *)(param_1 + 0x20) = *(undefined4 *)(*(int *)(param_1 + 0x1bc) + iVar2 * 4)
            ;
          }
          FUN_006ff450(iVar2);
          if (*(int *)(param_1 + 0x28) == -1) {
            FUN_006fe440();
          }
          return 1;
        }
        iVar3 = FUN_006ff440();
        if (iVar3 == iVar2) {
          if (*(int *)(param_1 + 0x28) == -1) {
            FUN_006fe440();
          }
          return 1;
        }
        FUN_006ff450(iVar2);
        if (*(int *)(param_1 + 0x28) == -1) {
          FUN_006fe440();
        }
        return 0;
      }
      if (*(int *)(param_1 + 0x28) != -1) {
/* ... 7 more lines truncated by --max-lines */
```

### decoy: 0x00701fd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00701fd0: 16-byte aligned: YES
  00701fd0  55             PUSH EBP
  00701fd1  8B EC          MOV EBP,ESP
  00701fd3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00701fd0
string literals (0):
direct callees (9):
  0x00710840  (unnamed)
  0x00703950  (unnamed)
  0x00710810  (unnamed)
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
  0x00703BD0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007112B0  CRes::Release
  0x006FF8A0  CVirtualMachineInternal::InitializeScript
struct offsets touched (top 9):
  +0x28 [dec 40]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x5 [dec 5]  x3  via EAX,ECX,EDX
  +0x7 [dec 7]  x3  via EAX,ECX,EDX
  +0x1 [dec 1]  x2  via EAX,ECX
  +0x2 [dec 2]  x2  via EAX,EDX
  +0x3 [dec 3]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
```

## Card B0-5-35

GOG function: CVirtualMachineStack::Destructor  gog 0x00641680  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x006fe440 (size 19, retn 0)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fe410:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006415a0 2009_::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006416a0 CVirtualMachineStack::AddToTopOfStack -> 0x006fe460  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006424e0 CVirtualMachineStack::ClearStack_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x006fe440: 16-byte aligned: YES
  006fe440  55             PUSH EBP
  006fe441  8B EC          MOV EBP,ESP
  006fe443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fe440
string literals (0):
direct callees (1):
  0x006FF2A0  CVirtualMachineStack::ClearStack_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006fe440 FUN_006fe440 size=19 ret=0 callers=9 callees=1
THUNK -> 0x006FF2A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_006fe440(undefined4 param_1)

{
  FUN_006ff2a0(param_1);
  return;
}
```

### decoy: 0x006fe410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fe410: 16-byte aligned: YES
  006fe410  55             PUSH EBP
  006fe411  8B EC          MOV EBP,ESP
  006fe413  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fe410
string literals (0):
direct callees (2):
  0x006FE360  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-5-36

GOG function: CVirtualMachine::LoadScriptSituation  gog 0x00640da0  callconv=__thiscall  param_size_bytes=12  gog_size=36  gog_retn=12
Candidate Steam address: 0x006fdb80 (size 36, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdb50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x00640d70 CVirtualMachine::SaveScriptSituation -> 0x006fdb50  gate_class=REVIEW
- GOG-order neighbour after:  0x00640dd0 CVirtualMachine::DeleteScriptSituation -> 0x006fdbb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006467d0 CVirtualMachineInternal::LoadScriptSituation -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdb80: 16-byte aligned: YES
  006fdb80  55             PUSH EBP
  006fdb81  8B EC          MOV EBP,ESP
  006fdb83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdb80
string literals (0):
direct callees (1):
  0x00703530  CVirtualMachineInternal::LoadScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdb80 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x00703530 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00703530(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x006fdb50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdb50: 16-byte aligned: YES
  006fdb50  55             PUSH EBP
  006fdb51  8B EC          MOV EBP,ESP
  006fdb53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdb50
string literals (0):
direct callees (1):
  0x00703370  CVirtualMachineInternal::SaveScriptSituation
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

## Card B0-5-37

GOG function: CVirtualMachineInternal::ReadScriptFile  gog 0x00645270  callconv=(blank)  param_size_bytes=4  gog_size=64  gog_retn=None
Candidate Steam address: 0x00701fd0 (size 716, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007022a0:STR0/CALLEE0/SIG0.031/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645220 CVirtualMachineInternal::PushInstructionPtr -> 0x00701f80  gate_class=REVIEW
- GOG-order neighbour after:  0x006452b0 CVirtualMachineInternal::ReadScriptFile_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x28*} | Steam {0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x28*,0x2c,0x44}
imm sets (* = in both, 3 common): GOG {0x8*,0x44*,0xffffffa2*} | Steam {0x4,0x8*,0x9,0xa,0x10,0x18,0x20,0x28,0x2e,0x30,0x31,0x39,0x42,0x43,...(+11)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00701fd0: 16-byte aligned: YES
  00701fd0  55             PUSH EBP
  00701fd1  8B EC          MOV EBP,ESP
  00701fd3  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00701fd0
string literals (0):
direct callees (9):
  0x00710840  (unnamed)
  0x00703950  (unnamed)
  0x00710810  (unnamed)
  0x007111E0  CRes::Demand
  0x006F3D90  (unnamed)
  0x00703BD0  (unnamed)
  0x007337C0  CExoString::operator=
  0x007112B0  CRes::Release
  0x006FF8A0  CVirtualMachineInternal::InitializeScript
struct offsets touched (top 9):
  +0x28 [dec 40]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x5 [dec 5]  x3  via EAX,ECX,EDX
  +0x7 [dec 7]  x3  via EAX,ECX,EDX
  +0x1 [dec 1]  x2  via EAX,ECX
  +0x2 [dec 2]  x2  via EAX,EDX
  +0x3 [dec 3]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x6 [dec 6]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00701fd0 FUN_00701fd0 size=716 ret=4 callers=3 callees=9

undefined4 __thiscall FUN_00701fd0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 local_44 [16];
  undefined1 local_34 [16];
  undefined1 local_24 [16];
  int local_14;
  int local_10;
  int local_c;
  char *local_8;
  
  *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + 1;
  if (*(int *)(param_1 + 0x28) < 8) {
    if (param_2 == 0) {
      FUN_00710840(&DAT_00986925);
      FUN_00703950(local_24,0);
    }
    else {
      FUN_00710810(param_2);
      FUN_00703950(local_34,0);
    }
    iVar2 = FUN_007111e0();
    if (iVar2 == 0) {
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
      uVar1 = 0xffffffa1;
    }
    else {
      local_8 = (char *)FUN_006f3d90();
      local_c = FUN_00703bd0();
      FUN_007337c0(param_2);
      if (((((*local_8 == 'N') && (local_8[1] == 'C')) && (local_8[2] == 'S')) &&
          ((local_8[3] == ' ' && (local_8[4] == 'V')))) && (local_8[6] == '.')) {
        local_10 = 0;
        if (('0' < local_8[5]) && (local_8[5] < ':')) {
          local_10 = (local_8[5] + -0x30) * 10;
        }
        if (('0' < local_8[7]) && (local_8[7] < ':')) {
          local_10 = local_10 + -0x30 + (int)local_8[7];
        }
        if (local_10 == 10) {
          if (local_8[8] == 'B') {
            local_14 = (uint)(byte)local_8[0xc] +
                       (uint)(byte)local_8[0xb] * 0x100 +
                       (uint)(byte)local_8[10] * 0x10000 + local_8[9] * 0x1000000;
            local_8 = local_8 + 0xd;
            local_c = local_c + -0xd;
            FUN_006ff8a0(param_1 + 0x2c + *(int *)(param_1 + 0x28) * 0x28,local_8,local_c);
            FUN_007112b0();
            FUN_00710840(&DAT_00986925);
            FUN_00703950(local_44,0);
            uVar1 = 0;
          }
          else {
            *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
            FUN_007112b0();
            uVar1 = 0xffffffa0;
          }
        }
        else {
          *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
          FUN_007112b0();
          uVar1 = 0xffffffa0;
        }
      }
      else {
        *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
        FUN_007112b0();
        uVar1 = 0xffffffa0;
      }
    }
  }
  else {
    *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + -1;
    uVar1 = 0xffffffa2;
  }
  return uVar1;
}
```

### decoy: 0x007022a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007022a0: 16-byte aligned: YES
  007022a0  55             PUSH EBP
  007022a1  8B EC          MOV EBP,ESP
  007022a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007022a0
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x00701FD0  CVirtualMachineInternal::ReadScriptFile
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x00734010  CExoString::CStr
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 10):
  +0x28 [dec 40]  x18  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x8  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via ECX
  +0x1B8 [dec 440]  x1  via EDX
  +0x1BC [dec 444]  x1  via ECX
```

## Card B0-5-38

GOG function: CVirtualMachineInternal::DeleteScriptSituation  gog 0x00646a60  callconv=__thiscall  param_size_bytes=4  gog_size=78  gog_retn=4
Candidate Steam address: 0x007037c0 (size 78, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00703810:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006467d0 CVirtualMachineInternal::LoadScriptSituation -> 0x00703530  gate_class=AUTO
- GOG-order neighbour after:  0x00646ab0 CVirtualMachineInternal::SetCommandImplementer -> 0x00703810  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00642920 CVirtualMachineScript::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c*} | Steam {0x4*,0x1c*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007037c0: 16-byte aligned: YES
  007037c0  55             PUSH EBP
  007037c1  8B EC          MOV EBP,ESP
  007037c3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007037c0
string literals (0):
direct callees (2):
  0x006FF6E0  CVirtualMachineScript::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007037c0 FUN_007037c0 size=78 ret=4 callers=1 callees=2

void FUN_007037c0(int param_1)

{
  if (param_1 != 0) {
    FUN_006ff6e0();
    FUN_009196fd(param_1);
  }
  return;
}
```

### decoy: 0x00703810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00703810: 16-byte aligned: YES
  00703810  55             PUSH EBP
  00703811  8B EC          MOV EBP,ESP
  00703813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00703810
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3D4 [dec 980]  x4  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
```

## Card B0-5-39

GOG function: CVirtualMachineInternal::StackPopObject  gog 0x00646310  callconv=(blank)  param_size_bytes=4  gog_size=127  gog_retn=4
Candidate Steam address: 0x00703070 (size 127, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702dd0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00646280 CVirtualMachineInternal::StackPushEngineStructure -> 0x00702fe0  gate_class=AUTO
- GOG-order neighbour after:  0x00646390 CVirtualMachineInternal::StackPushObject -> 0x007030f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1b8*,0x1bc*} | Steam {0x1b8*,0x1bc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x6*,0xc*,0x1ac*} | Steam {0x4*,0x6*,0xc*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00703070: 16-byte aligned: YES
  00703070  55             PUSH EBP
  00703071  8B EC          MOV EBP,ESP
  00703073  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00703070
string literals (0):
direct callees (2):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00703070 FUN_00703070 size=127 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_00703070(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006ff440();
  if (iVar1 < 1) {
    uVar2 = 0;
  }
  else {
    iVar1 = iVar1 + -1;
    if (*(char *)(*(int *)(param_1 + 0x1b8) + iVar1) == '\x06') {
      *param_2 = *(undefined4 *)(*(int *)(param_1 + 0x1bc) + iVar1 * 4);
      FUN_006ff450(iVar1);
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00702dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702dd0: 16-byte aligned: YES
  00702dd0  55             PUSH EBP
  00702dd1  8B EC          MOV EBP,ESP
  00702dd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702dd0
string literals (0):
direct callees (3):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x007337C0  CExoString::operator=
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

## Card B0-5-40

GOG function: CSWSObject::GetDialogResref  gog 0x006637e0  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x0053ef30 (size 29, retn 4)
Candidate sources: branch+name_map:high:steam-db-pr3-phase2    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004597d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006634d0 CSWSItem::Constructor -> 0x005ffd60  gate_class=REVIEW
- GOG-order neighbour after:  0x00663800 CSWSObject::GetMaxHitPoints  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
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
prologue for 0x0053ef30: 16-byte aligned: YES
  0053ef30  55             PUSH EBP
  0053ef31  8B EC          MOV EBP,ESP
  0053ef33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053ef30
string literals (0):
direct callees (1):
  0x00710840  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053ef30 FUN_0053ef30 size=29 ret=4 callers=0 callees=1

undefined4 FUN_0053ef30(undefined4 param_1)

{
  FUN_00710840(&DAT_00986925);
  return param_1;
}
```

### decoy: 0x004597d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004597d0: 16-byte aligned: YES
  004597d0  55             PUSH EBP
  004597d1  8B EC          MOV EBP,ESP
  004597d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004597d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1D8 [dec 472]  x1  via EAX
```

## Card B0-5-41

GOG function: CVirtualMachineInternal::ExecuteCode  gog 0x00642c30  callconv=(blank)  param_size_bytes=12  gog_size=1214  gog_retn=None
Candidate Steam address: 0x006ff9f0 (size 9336, retn 12)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.46 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006ff8a0:STR0/CALLEE0/SIG0.104/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00642b70 CVirtualMachineInternal::DeleteScript -> 0x006ff930  gate_class=REVIEW
- GOG-order neighbour after:  0x006430ee CVirtualMachineInternal::ExecuteCode_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00641870 CVirtualMachineStack::AssignLocationToLocation -> Steam ?  in candidate callees: n/a
  0x006430ee CVirtualMachineInternal::ExecuteCode_2 -> Steam ?  in candidate callees: n/a
  0x00643793 CVirtualMachineInternal::ExecuteCode_3 -> Steam ?  in candidate callees: n/a
  0x00645091 unnamed -> Steam ?  in candidate callees: n/a
  0x00645e80 CVirtualMachineInternal::StackPushInteger -> Steam ?  in candidate callees: n/a
  0x00645f50 CVirtualMachineInternal::StackPushFloat -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x1*,0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x24*,0x1ac*,0x1b0*} | Steam {0x1*,0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8,0x9,0x1c,0x24*,0x28,0x16c,0x18c,...(+8)}
imm sets (* = in both, 25 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0x10*,0x18*,0x19*,0x26*,0x27*,0x2b*,0xff*,0x1ac*,...(+11)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x8*,0xa,0xb,0xc,0xd,0xe,0xf,0x10*,0x14,...(+40)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x006ff9f0: 16-byte aligned: YES
  006ff9f0  55             PUSH EBP
  006ff9f1  8B EC          MOV EBP,ESP
  006ff9f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006ff9f0
string literals (0):
direct callees (25):
  0x006FE630  CVirtualMachineStack::AssignLocationToLocation
  0x00702BE0  CVirtualMachineInternal::StackPushInteger
  0x00702CB0  CVirtualMachineInternal::StackPushFloat
  0x00733570  CExoString::CExoString_char
  0x00702E60  CVirtualMachineInternal::StackPushString
  0x00733780  CExoString::~CExoString
  0x007030F0  CVirtualMachineInternal::StackPushObject
  0x00702FE0  CVirtualMachineInternal::StackPushEngineStructure
  0x00733540  CExoString::CExoString(empty)
  0x00919723  operator new
  0x00734270  CExoString::Format
  0x009196FD  operator delete
  0x00702B60  CVirtualMachineInternal::StackPopInteger
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00734D60  CExoString::CompareNoCase
  0x006FF450  CVirtualMachineStack::SetStackPointer
  0x00702C30  CVirtualMachineInternal::StackPopFloat
  0x00702DD0  CVirtualMachineInternal::StackPopString
  0x00733DF0  CExoString::operator+
  0x00702D00  CVirtualMachineInternal::StackPopVector
struct offsets touched (top 10):
  +0x1AC [dec 428]  x7  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x1B0 [dec 432]  x6  via EAX,ECX,EDX
  +0x1BC [dec 444]  x6  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x6  via EAX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x24 [dec 36]  x3  via EAX,ECX
  +0x28 [dec 40]  x2  via EDX
  +0x3C8 [dec 968]  x2  via EAX,EDX
  +0x3CC [dec 972]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006ff9f0 FUN_006ff9f0 size=9336 ret=12 callers=1 callees=25

/* WARNING: Removing unreachable block (ram,0x00701e56) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __thiscall FUN_006ff9f0(int param_1,int *param_2,int param_3,int param_4)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 *puVar3;
  int iVar4;
  uint local_238;
  undefined1 local_1f4 [12];
  float local_1e8;
  float local_1e4;
  float local_1e0;
  float local_1dc;
  float local_1d8;
  float local_1d4;
  float local_1d0;
  float local_1cc;
  float local_1c8;
  undefined1 local_1c4 [8];
  int local_1bc;
  int local_1b8;
  int local_1b4;
  int local_1b0;
  int local_1ac;
  int local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  int local_19c;
  int local_198;
  int local_194;
  int local_190;
  short local_18c;
  short local_188;
  int local_184;
  short local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  int local_16c;
  int local_168;
  int local_164;
  int local_160;
  int local_15c;
  int local_158;
  uint local_154;
  float local_150;
  int local_14c;
  int local_148;
  int local_144;
  float local_140;
  float local_13c;
  float local_138;
  float local_134;
  float local_130;
  float local_12c;
  float local_128;
  float local_124;
  float local_120;
  float local_11c;
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  float local_108;
  int local_104;
  int local_100;
  float local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  undefined1 local_ec [4];
  int local_e8;
  undefined1 local_e4 [4];
  int local_e0;
  undefined1 local_dc [4];
  float local_d8;
  float local_d4;
/* ... 979 more lines truncated by --max-lines */
```

### decoy: 0x006ff8a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ff8a0: 16-byte aligned: YES
  006ff8a0  55             PUSH EBP
  006ff8a1  8B EC          MOV EBP,ESP
  006ff8a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006ff8a0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 7):
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EDX
```

## Card B0-5-42

GOG function: CVirtualMachineStack::LoadStack  gog 0x006421c0  callconv=(blank)  param_size_bytes=8  gog_size=455  gog_retn=None
Candidate Steam address: 0x006fef80 (size 784, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass
- anchors: A-STR 3 | A-CALLEE 1 | A-SIG 0.54 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (ReadFieldCHAR 0x718a80); SIG 0.54
- best decoy (neighbour/other candidate) scores: 0x006fed80:STR3/CALLEE0/SIG0.369/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00641fc0 CVirtualMachineStack::SaveStack -> 0x006fed80  gate_class=AUTO
- GOG-order neighbour after:  0x00642387 CVirtualMachineStack::LoadStack_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: yes
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: yes
  0x00624be0 CResGFF::ReadFieldCHAR -> Steam 0x00718a80  in candidate callees: yes
  0x00624de0 CResGFF::ReadFieldINT -> Steam 0x00718c80  in candidate callees: yes
  0x00624e60 CResGFF::ReadFieldFLOAT -> Steam 0x00718d00  in candidate callees: yes
  0x00642387 CVirtualMachineStack::LoadStack_2 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG strings (5): 'Value'; 'Stack'; 'TotalSize'; 'StackPointer' ...
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x10*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14,0x28,0x3d4}
imm sets (* = in both, 3 common): GOG {0x3*,0x4*,0x68*} | Steam {0x2,0x3*,0x4*,0x8,0x10,0x68*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x006fef80: 16-byte aligned: YES
  006fef80  55             PUSH EBP
  006fef81  8B EC          MOV EBP,ESP
  006fef83  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x006fef80
string literals (8):
  0x0099B640  at 006fefb1  'BasePointer'
  0x0099B630  at 006fefce  'StackPointer'
  0x0099B624  at 006fefea  'TotalSize'
  0x0099B61C  at 006ff054  'Stack'
  0x00992E8C  at 006ff0f3  'Value'
  0x00992E8C  at 006ff11f  'Value'
  0x00992E8C  at 006ff1a4  'Value'
  0x00992E8C  at 006ff204  'Value'
direct callees (13):
  0x00718C80  CResGFF::ReadFieldINT
  0x00919723  operator new
  0x00718760  CResGFF::GetList
  0x00718890  CResGFF::GetListElement
  0x00718A80  CResGFF::ReadFieldCHAR
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x00733540  CExoString::CExoString(empty)
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00718C00  CResGFF::ReadFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x10 [dec 16]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x28 [dec 40]  x1  via EDX
  +0x3D4 [dec 980]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fef80 FUN_006fef80 size=784 ret=8 callers=1 callees=13

void __thiscall FUN_006fef80(int *param_1,undefined4 param_2,undefined4 param_3)

{
  undefined1 uVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined4 local_6c;
  undefined1 local_5c [8];
  undefined1 local_54 [8];
  int local_4c;
  undefined4 local_48;
  int local_44;
  int local_40;
  int local_3c;
  undefined1 local_38 [4];
  undefined4 local_34;
  float local_30;
  undefined1 local_2c [4];
  undefined1 local_28 [20];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f01b;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = uVar2;
  iVar3 = FUN_00718c80(param_3,"BasePointer",local_2c,0);
  param_1[1 = byte +0x4] = iVar3;
  iVar3 = FUN_00718c80(param_3,"StackPointer",local_2c,0);
  *param_1 = iVar3;
  iVar3 = FUN_00718c80(param_3,"TotalSize",local_2c,0);
  param_1[2 = byte +0x8] = iVar3;
  if (0 < param_1[2 = byte +0x8]) {
    local_40 = FUN_00919723(param_1[2 = byte +0x8],uVar2);
    param_1[3 = byte +0xc] = local_40;
    local_44 = FUN_00919723(-(uint)((int)((ulonglong)(uint)param_1[2 = byte +0x8] * 4 >> 0x20) != 0) |
                            (uint)((ulonglong)(uint)param_1[2 = byte +0x8] * 4));
    param_1[4 = byte +0x10] = local_44;
    FUN_00718760(local_28,param_3,"Stack");
    for (local_3c = 0; local_3c < *param_1; local_3c = local_3c + 1) {
      FUN_00718890(local_38,local_28,local_3c);
      uVar1 = FUN_00718a80(local_38,&DAT_00994500,local_2c,0);
      *(undefined1 *)(param_1[3 = byte +0xc] + local_3c) = uVar1;
      switch(*(undefined1 *)(param_1[3 = byte +0xc] + local_3c)) {
      case 3:
        uVar4 = FUN_00718c80(local_38,"Value",local_2c,0);
        *(undefined4 *)(param_1[4 = byte +0x10] + local_3c * 4) = uVar4;
        break;
      case 4:
        fVar5 = (float10)FUN_00718d00(local_38,"Value",local_2c,0);
        local_30 = (float)fVar5;
        *(float *)(param_1[4 = byte +0x10] + local_3c * 4) = local_30;
        break;
      case 5:
        local_4c = FUN_00919723(8);
        local_8 = 0;
        if (local_4c == 0) {
          local_6c = 0;
        }
        else {
          local_6c = FUN_00733540();
        }
        local_48 = local_6c;
        local_8 = 0xffffffff;
        local_34 = local_6c;
        FUN_00733570(&DAT_00986925);
        local_8 = 1;
        uVar4 = FUN_00718f90(local_5c,local_38,"Value",local_2c,local_54);
        local_8._0_1_ = 2;
        FUN_007337c0(uVar4);
        local_8 = CONCAT31(local_8._1_3_,1);
        FUN_00733780();
        local_8 = 0xffffffff;
/* ... 18 more lines truncated by --max-lines */
```

### decoy: 0x006fed80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fed80: 16-byte aligned: YES
  006fed80  55             PUSH EBP
  006fed81  8B EC          MOV EBP,ESP
  006fed83  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fed80
string literals (8):
  0x0099B640  at 006fed93  'BasePointer'
  0x0099B630  at 006fedab  'StackPointer'
  0x0099B624  at 006fedc2  'TotalSize'
  0x0099B61C  at 006fede7  'Stack'
  0x00992E8C  at 006fee7c  'Value'
  0x00992E8C  at 006fee9f  'Value'
  0x00992E8C  at 006feec5  'Value'
  0x00992E8C  at 006feee5  'Value'
direct callees (8):
  0x00719BF0  CResGFF::WriteFieldINT
  0x00719740  CResGFF::AddList
  0x00719800  CResGFF::AddListElement
  0x00719A40  CResGFF::WriteFieldCHAR
  0x00719CE0  CResGFF::WriteFieldFLOAT
  0x00719E50  CResGFF::WriteFieldCExoString
  0x00719B90  CResGFF::WriteFieldDWORD
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x10 [dec 16]  x5  via EAX,ECX,EDX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX
  +0x4 [dec 4]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x24 [dec 36]  x1  via EAX
  +0x3D4 [dec 980]  x1  via EDX
```

## Card B0-5-43

GOG function: CServerExoApp::ClientToServerObjectId  gog 0x0064c2d0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c8b0 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-phase2+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 8 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 8 (CreatureAcquireItem 0x73f9e0,MainLoop 0x781be0,ClearAllActions 0x7f24c0)
- best decoy (neighbour/other candidate) scores: 0x0051c8f0:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2b0 CServerExoApp::GetGlobalVariableTable -> 0x0051c890  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> 0x0051c8d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bcf80 CServerExoAppInternal::ClientToServerObjectId -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051c8b0: 16-byte aligned: YES
  0051c8b0  55             PUSH EBP
  0051c8b1  8B EC          MOV EBP,ESP
  0051c8b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c8b0
string literals (0):
direct callees (1):
  0x00539D30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c8b0 FUN_0051c8b0 size=28 ret=4 callers=84 callees=1
THUNK -> 0x00539D30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c8b0(undefined4 param_1)

{
  FUN_00539d30(param_1);
  return;
}
```

### decoy: 0x0051c8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8f0: 16-byte aligned: YES
  0051c8f0  55             PUSH EBP
  0051c8f1  8B EC          MOV EBP,ESP
  0051c8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8f0
string literals (0):
direct callees (1):
  0x00539D60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-5-44

GOG function: CServerExoApp::LoadGame  gog 0x0064b840  callconv=__thiscall  param_size_bytes=16  gog_size=40  gog_retn=16
Candidate Steam address: 0x0051be20 (size 40, retn 16)
Candidate sources: branch+mapper:vote+name_map:high:steam-db-pr3-savegame+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bdf0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b810 CServerExoApp::SaveGame -> 0x0051bdf0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b870 CServerExoApp::SetMoveToModulePending  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2f00 CServerExoAppInternal::LoadGame -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0051be20: 16-byte aligned: YES
  0051be20  55             PUSH EBP
  0051be21  8B EC          MOV EBP,ESP
  0051be23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be20
string literals (0):
direct callees (1):
  0x0052FB80  CServerExoAppInternal::LoadGame (SAVES:%06d - %s; partytable, globals 0x656710, LoadModule)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be20 Concurrency::details::SchedulerProxy::DoHillClimbing size=40 ret=16 callers=1 callees=1
THUNK -> 0x0052FB80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: unsigned int __thiscall Concurrency::details::SchedulerProxy::DoHillClimbing(unsigned
   int,unsigned int,unsigned int,unsigned int)
   
   Libraries: Visual Studio 2010 Debug, Visual Studio 2012 Debug, Visual Studio 2015 Debug, Visual
   Studio 2015 Release */

uint __thiscall
Concurrency::details::SchedulerProxy::DoHillClimbing
          (SchedulerProxy *this,uint param_1,uint param_2,uint param_3,uint param_4)

{
  uint uVar1;
  
  uVar1 = FUN_0052fb80(param_1,param_2,param_3,param_4);
  return uVar1;
}
```

### decoy: 0x0051bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051bdf0: 16-byte aligned: YES
  0051bdf0  55             PUSH EBP
  0051bdf1  8B EC          MOV EBP,ESP
  0051bdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051bdf0
string literals (0):
direct callees (1):
  0x0052F610  StartSave(slot,name) (screenshot + save-state flags)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B0-5-45

GOG function: CVirtualMachine::StackPushString  gog 0x00640cb0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x006fda90 (size 28, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fda70:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c90 CVirtualMachine::StackPopString -> 0x006fda70  gate_class=REVIEW
- GOG-order neighbour after:  0x00640cd0 CVirtualMachine::StackPopEngineStructure -> 0x006fdab0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646100 CVirtualMachineInternal::StackPushString -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fda90: 16-byte aligned: YES
  006fda90  55             PUSH EBP
  006fda91  8B EC          MOV EBP,ESP
  006fda93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fda90
string literals (0):
direct callees (1):
  0x00702E60  CVirtualMachineInternal::StackPushString
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fda90 FUN_006fda90 size=28 ret=4 callers=26 callees=1
THUNK -> 0x00702E60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fda90(undefined4 param_1)

{
  FUN_00702e60(param_1);
  return;
}
```

### decoy: 0x006fda70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fda70: 16-byte aligned: YES
  006fda70  55             PUSH EBP
  006fda71  8B EC          MOV EBP,ESP
  006fda73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fda70
string literals (0):
direct callees (1):
  0x00702DD0  CVirtualMachineInternal::StackPopString
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

## Card B0-5-46

GOG function: CVirtualMachineInternal::PopInstructionPtr  gog 0x006451c0  callconv=(blank)  param_size_bytes=4  gog_size=92  gog_retn=4
Candidate Steam address: 0x00701f20 (size 92, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00701f80:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645160 CVirtualMachineInternal::ExecuteCode_5  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00645220 CVirtualMachineInternal::PushInstructionPtr -> 0x00701f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c4*,0x1c8*} | Steam {0x1c4*,0x1c8*}
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
prologue for 0x00701f20: 16-byte aligned: YES
  00701f20  55             PUSH EBP
  00701f21  8B EC          MOV EBP,ESP
  00701f23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00701f20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C4 [dec 452]  x5  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00701f20 FUN_00701f20 size=92 ret=4 callers=1 callees=0

bool __thiscall FUN_00701f20(int param_1,undefined4 *param_2)

{
  bool bVar1;
  
  *(int *)(param_1 + 0x1c4) = *(int *)(param_1 + 0x1c4) + -1;
  bVar1 = -1 < *(int *)(param_1 + 0x1c4);
  if (bVar1) {
    *param_2 = *(undefined4 *)(param_1 + 0x1c8 + *(int *)(param_1 + 0x1c4) * 4);
  }
  else {
    *(undefined4 *)(param_1 + 0x1c4) = 0;
  }
  return bVar1;
}
```

### decoy: 0x00701f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00701f80: 16-byte aligned: YES
  00701f80  55             PUSH EBP
  00701f81  8B EC          MOV EBP,ESP
  00701f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00701f80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C4 [dec 452]  x4  via EAX,ECX
```

## Card B0-5-47

GOG function: CVirtualMachineInternal::Destructor  gog 0x006429f0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x006ff7b0 (size 44, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006ff6e0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00642920 CVirtualMachineScript::Destructor -> 0x006ff6e0  gate_class=AUTO
- GOG-order neighbour after:  0x00642a20 CVirtualMachineInternal::Destructor_2 -> 0x006ff7e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00642a20 CVirtualMachineInternal::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x006ff7b0: 16-byte aligned: YES
  006ff7b0  55             PUSH EBP
  006ff7b1  8B EC          MOV EBP,ESP
  006ff7b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006ff7b0
string literals (0):
direct callees (2):
  0x006FF7E0  CVirtualMachineInternal::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006ff7b0 FUN_006ff7b0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_006ff7b0(undefined4 param_1,uint param_2)

{
  FUN_006ff7e0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x006ff6e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006ff6e0: 16-byte aligned: YES
  006ff6e0  55             PUSH EBP
  006ff6e1  8B EC          MOV EBP,ESP
  006ff6e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x006ff6e0
string literals (0):
direct callees (3):
  0x006FE440  CVirtualMachineStack::Destructor
  0x009196FD  operator delete
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x10 [dec 16]  x3  via ECX,EDX
```

## Card B0-5-48

GOG function: CVirtualMachineInternal::StackPushObject  gog 0x00646390  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x007030f0 (size 72, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702e60:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00646310 CVirtualMachineInternal::StackPopObject -> 0x00703070  gate_class=REVIEW
- GOG-order neighbour after:  0x006463e0 CVirtualMachineInternal::StackPopCommand -> 0x00703140  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006416a0 CVirtualMachineStack::AddToTopOfStack -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1bc*} | Steam {0x1bc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x6*,0x8*,0x1ac*} | Steam {0x4*,0x6*,0x8*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007030f0: 16-byte aligned: YES
  007030f0  55             PUSH EBP
  007030f1  8B EC          MOV EBP,ESP
  007030f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007030f0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007030f0 FUN_007030f0 size=72 ret=4 callers=2 callees=2

undefined4 __thiscall FUN_007030f0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  FUN_006fe460(6);
  iVar1 = FUN_006ff440();
  *(undefined4 *)(*(int *)(param_1 + 0x1bc) + -4 + iVar1 * 4) = param_2;
  return 1;
}
```

### decoy: 0x00702e60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702e60: 16-byte aligned: YES
  00702e60  55             PUSH EBP
  00702e61  8B EC          MOV EBP,ESP
  00702e63  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00702e60
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EDX
```

## Card B0-5-49

GOG function: CVirtualMachineInternal::StackPopInteger  gog 0x00645e00  callconv=(blank)  param_size_bytes=4  gog_size=119  gog_retn=4
Candidate Steam address: 0x00702b60 (size 119, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702be0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645c90 CVirtualMachineInternal::SetUpScriptSituation -> 0x007029f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00645e80 CVirtualMachineInternal::StackPushInteger -> 0x00702be0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1b8*,0x1bc*} | Steam {0x1b8*,0x1bc*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x8*,0x1ac*} | Steam {0x3*,0x4*,0x8*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702b60: 16-byte aligned: YES
  00702b60  55             PUSH EBP
  00702b61  8B EC          MOV EBP,ESP
  00702b63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702b60
string literals (0):
direct callees (2):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00702b60 FUN_00702b60 size=119 ret=4 callers=2 callees=2

undefined4 __thiscall FUN_00702b60(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006ff440();
  if (iVar1 < 1) {
    uVar2 = 0;
  }
  else {
    iVar1 = iVar1 + -1;
    if (*(char *)(*(int *)(param_1 + 0x1b8) + iVar1) == '\x03') {
      *param_2 = *(undefined4 *)(*(int *)(param_1 + 0x1bc) + iVar1 * 4);
      FUN_006ff450(iVar1);
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00702be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702be0: 16-byte aligned: YES
  00702be0  55             PUSH EBP
  00702be1  8B EC          MOV EBP,ESP
  00702be3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702be0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

## Card B0-5-50

GOG function: CSWSItemPropertyHandler::Destructor  gog 0x0064cee0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0051d4e0 (size 44, retn 4)
Candidate sources: branch+name_map:high:batch8_reverse_engineering    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d4b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064ceb0 CSWSEffectListHandler::Destructor -> 0x0051d4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064cf10 CServerAIMaster::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007c17d0 CSWSItemPropertyHandler::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
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
prologue for 0x0051d4e0: 16-byte aligned: YES
  0051d4e0  55             PUSH EBP
  0051d4e1  8B EC          MOV EBP,ESP
  0051d4e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d4e0
string literals (0):
direct callees (2):
  0x005AA3C0  CSWSItemPropertyHandler::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051d4e0 FUN_0051d4e0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_0051d4e0(undefined4 param_1,uint param_2)

{
  FUN_005aa3c0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x0051d4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d4b0: 16-byte aligned: YES
  0051d4b0  55             PUSH EBP
  0051d4b1  8B EC          MOV EBP,ESP
  0051d4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d4b0
string literals (0):
direct callees (2):
  0x00593960  CSWSEffectListHandler::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B0-5-51

GOG function: CVirtualMachineInternal::RunScriptFile  gog 0x00645af0  callconv=(blank)  param_size_bytes=4  gog_size=345  gog_retn=4
Candidate Steam address: 0x00702850 (size 345, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702590:STR0/CALLEE0/SIG0.355/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645830 CVirtualMachineInternal::RunScriptSituation -> 0x00702590  gate_class=REVIEW
- GOG-order neighbour after:  0x00645c50 CVirtualMachineInternal::GetRunScriptReturnValue -> 0x007029b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642c30 CVirtualMachineInternal::ExecuteCode -> Steam ?  in candidate callees: n/a
  0x00645220 CVirtualMachineInternal::PushInstructionPtr -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x10*,0x1c*,0x20*,0x28*,0x3c*,0x40*,0x44*,0x1ac*,0x1c4*,0x3d4*} | Steam {0x10*,0x1c*,0x20*,0x28*,0x3c*,0x40*,0x44*,0x1ac*,0x1c4*,0x3d4*}
imm sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x28*,0x1ac*} | Steam {0x4*,0x1c*,0x28*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702850: 16-byte aligned: YES
  00702850  55             PUSH EBP
  00702851  8B EC          MOV EBP,ESP
  00702853  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00702850
string literals (0):
direct callees (7):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x00701F80  CVirtualMachineInternal::PushInstructionPtr
  0x006FF9F0  CVirtualMachineInternal::ExecuteCode
  0x00734010  CExoString::CStr
  0x00733570  CExoString::CExoString_char
  0x006FF450  CVirtualMachineStack::SetStackPointer
  0x00733780  CExoString::~CExoString
struct offsets touched (top 7):
  +0x28 [dec 40]  x3  via EAX,ECX,EDX
  +0x1C4 [dec 452]  x3  via EAX,EDX
  +0x1AC [dec 428]  x2  via ECX
  +0x3D4 [dec 980]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00702850 FUN_00702850 size=345 ret=4 callers=2 callees=7

int __thiscall FUN_00702850(int param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined1 local_28 [8];
  undefined4 local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0095f198;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_14 = FUN_006ff440(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_1c = *(int *)(param_1 + 0x1c4);
  *(undefined4 *)(param_1 + 0x1c) = 0;
  *(undefined4 *)(param_1 + 0x20) = 0;
  FUN_00701f80(0xffffffff);
  local_20 = param_2;
  local_18 = FUN_006ff9f0(&local_20,
                          *(undefined4 *)(param_1 + 0x3c + *(int *)(param_1 + 0x28) * 0x28),
                          *(undefined4 *)(param_1 + 0x40 + *(int *)(param_1 + 0x28) * 0x28));
  if ((local_18 < 0) || (local_1c != *(int *)(param_1 + 0x1c4))) {
    uVar1 = FUN_00734010();
    FUN_00733570(uVar1);
    local_8 = 0;
    (**(code **)(**(int **)(param_1 + 0x3d4) + 0x10))(local_28,-local_18);
    if (local_14 < *(int *)(param_1 + 0x1ac)) {
      FUN_006ff450(local_14);
    }
    else {
      *(int *)(param_1 + 0x1ac) = local_14;
    }
    *(int *)(param_1 + 0x1c4) = local_1c;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return local_18;
}
```

### decoy: 0x00702590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702590: 16-byte aligned: YES
  00702590  55             PUSH EBP
  00702591  8B EC          MOV EBP,ESP
  00702593  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702590
string literals (0):
direct callees (7):
  0x006FE440  CVirtualMachineStack::Destructor
  0x006FE8E0  CVirtualMachineStack::CopyFromStack
  0x009196FD  operator delete
  0x007029F0  CVirtualMachineInternal::SetUpScriptSituation
  0x00702850  CVirtualMachineInternal::RunScriptFile
  0x006FF930  CVirtualMachineInternal::DeleteScript
  0x006FF6E0  CVirtualMachineScript::Destructor
struct offsets touched (top 9):
  +0x28 [dec 40]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x3D4 [dec 980]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x1AC [dec 428]  x1  via EDX
  +0x1B4 [dec 436]  x1  via EAX
  +0x1C0 [dec 448]  x1  via EDX
  +0x1C4 [dec 452]  x1  via EDX
```

## Card B0-5-52

GOG function: CSWSEffectListHandler::OnApplyCurse  gog 0x00744fe0  callconv=(blank)  param_size_bytes=12  gog_size=1376  gog_retn=12
Candidate Steam address: 0x005a2640 (size 96, retn 12)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSEffectListHandler::OnApplyModifyNumAttacks
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005a2530:STR0/CALLEE0/SIG0.267/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00744f70 CSWSEffectListHandler::OnRemoveModifyNumAttacks -> 0x005a26a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00745540 CSWSEffectListHandler::OnApplySilence -> 0x005a2c70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00549d60 CSWCCMessageData::SetObjectID -> Steam ?  in candidate callees: n/a
  0x0054a2c0 CSWCCMessageData::CopyTo -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam ?  in candidate callees: n/a
  0x0064db90 CServerAIMaster::AddEventDeltaTime -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x006aaef0 CSWSObject::ApplyEffect -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x18,0x30*,0xfc,0x1198} | Steam {0x30*,0x10dc}
imm sets (* = in both, 1 common): GOG {0x2,0x3,0x4,0x5,0x6,0x7,0xc*,0x11,0x25,0x34,0x8c,0x8d,0xac} | Steam {0xc*,0x10}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005a2640: 16-byte aligned: YES
  005a2640  55             PUSH EBP
  005a2641  8B EC          MOV EBP,ESP
  005a2643  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005a2640
string literals (0):
direct callees (3):
  0x005922A0  CSWSCombatRound::GetActualBonusEffectAttacks
  0x005E4E60  CGameEffect::GetInteger
  0x00592280  CSWSCombatRound::SetBonusEffectAttacks
struct offsets touched (top 2):
  +0x10DC [dec 4316]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005a2640 FUN_005a2640 size=96 ret=12 callers=0 callees=3

undefined4 FUN_005a2640(int *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = (**(code **)(*param_1 + 0x30))();
  if (iVar1 != 0) {
    iVar1 = FUN_005922a0();
    iVar2 = FUN_005e4e60(0);
    FUN_00592280(iVar1 + iVar2);
  }
  return 0;
}
```

### decoy: 0x005a2530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005a2530: 16-byte aligned: YES
  005a2530  55             PUSH EBP
  005a2531  8B EC          MOV EBP,ESP
  005a2533  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005a2530
string literals (0):
direct callees (5):
  0x005E4E60  CGameEffect::GetInteger
  0x00919723  operator new
  0x005E4AC0  CGameEffect::Constructor_2
  0x005E4CA0  CGameEffect::Destructor
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
```

## Card B0-5-53

GOG function: CVirtualMachine::StackPushEngineStructure  gog 0x00640cf0  callconv=(blank)  param_size_bytes=8  gog_size=32  gog_retn=8
Candidate Steam address: 0x006fdad0 (size 32, retn 8)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fdab0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640cd0 CVirtualMachine::StackPopEngineStructure -> 0x006fdab0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640d10 CVirtualMachine::StackPopObject -> 0x006fdaf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00646280 CVirtualMachineInternal::StackPushEngineStructure -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fdad0: 16-byte aligned: YES
  006fdad0  55             PUSH EBP
  006fdad1  8B EC          MOV EBP,ESP
  006fdad3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fdad0
string literals (0):
direct callees (1):
  0x00702FE0  CVirtualMachineInternal::StackPushEngineStructure
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 006fdad0 FUN_006fdad0 size=32 ret=8 callers=83 callees=1
THUNK -> 0x00702FE0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fdad0(undefined4 param_1,undefined4 param_2)

{
  FUN_00702fe0(param_1,param_2);
  return;
}
```

### decoy: 0x006fdab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fdab0: 16-byte aligned: YES
  006fdab0  55             PUSH EBP
  006fdab1  8B EC          MOV EBP,ESP
  006fdab3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fdab0
string literals (0):
direct callees (1):
  0x00702F20  CVirtualMachineInternal::StackPopEngineStructure
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EDX
```

## Card B0-5-54

GOG function: CVirtualMachineInternal::StackPushInteger  gog 0x00645e80  callconv=(blank)  param_size_bytes=4  gog_size=72  gog_retn=4
Candidate Steam address: 0x00702be0 (size 72, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702cb0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645e00 CVirtualMachineInternal::StackPopInteger -> 0x00702b60  gate_class=REVIEW
- GOG-order neighbour after:  0x00645ed0 CVirtualMachineInternal::StackPopFloat -> 0x00702c30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006416a0 CVirtualMachineStack::AddToTopOfStack -> Steam ?  in candidate callees: n/a
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1bc*} | Steam {0x1bc*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x8*,0x1ac*} | Steam {0x3*,0x4*,0x8*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702be0: 16-byte aligned: YES
  00702be0  55             PUSH EBP
  00702be1  8B EC          MOV EBP,ESP
  00702be3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702be0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00702be0 FUN_00702be0 size=72 ret=4 callers=2 callees=2

undefined4 __thiscall FUN_00702be0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  FUN_006fe460(3);
  iVar1 = FUN_006ff440();
  *(undefined4 *)(*(int *)(param_1 + 0x1bc) + -4 + iVar1 * 4) = param_2;
  return 1;
}
```

### decoy: 0x00702cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702cb0: 16-byte aligned: YES
  00702cb0  55             PUSH EBP
  00702cb1  8B EC          MOV EBP,ESP
  00702cb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702cb0
string literals (0):
direct callees (2):
  0x006FE460  CVirtualMachineStack::AddToTopOfStack
  0x006FF440  CVirtualMachineStack::GetStackPointer
struct offsets touched (top 1):
  +0x1BC [dec 444]  x1  via EAX
```

## Card B0-5-55

GOG function: CSWSDoor::GetIsLinked  gog 0x00768410  callconv=(blank)  param_size_bytes=0  gog_size=50  gog_retn=0
Candidate Steam address: 0x0061cf70 (size 144, retn 0)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSDoor::GetDisarmDC+name_held_elsewhere=0x0061d000
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0061d000:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00768380 CSWSDoor::GetDisarmDC -> 0x0061cf70  gate_class=AUTO
- GOG-order neighbour after:  0x00768450 CSWSDoor::RemoveFromArea -> 0x0061d040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x3d4} | Steam {0x4,0x33c}
imm sets (* = in both, 1 common): GOG {0x2*} | Steam {0x2*,0x5,0x20,0xfffffffb}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0061cf70: 16-byte aligned: YES
  0061cf70  55             PUSH EBP
  0061cf71  8B EC          MOV EBP,ESP
  0061cf73  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0061cf70
string literals (0):
direct callees (1):
  0x0072FB00  OptionsHolder::GetGameOptions (returns *(*(this+4)+4); +8 bit0 = Auto Level Up NPCs)
struct offsets touched (top 2):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x33C [dec 828]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0061cf70 FUN_0061cf70 size=144 ret=0 callers=8 callees=1

undefined4 __fastcall FUN_0061cf70(int param_1)

{
  int iVar1;
  undefined3 uVar2;
  undefined1 local_24;
  undefined1 local_8;
  
  local_24 = *(char *)(param_1 + 0x33c);
  local_8 = '\0';
  iVar1 = FUN_0072fb00();
  uVar2 = (undefined3)((uint)iVar1 >> 8);
  if (*(char *)(iVar1 + 4) == '\0') {
    local_8 = -5;
  }
  else if (*(char *)(iVar1 + 4) == '\x02') {
    local_8 = '\x05';
  }
  local_24 = local_24 + local_8;
  if (local_24 == '\0') {
    local_24 = '\x01';
  }
  else {
    uVar2 = 0;
  }
  return CONCAT31(uVar2,local_24);
}
```

### decoy: 0x0061d000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0061d000: 16-byte aligned: YES
  0061d000  55             PUSH EBP
  0061d001  8B EC          MOV EBP,ESP
  0061d003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0061d000
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3D4 [dec 980]  x2  via EAX,EDX
```

## Card B0-5-56

GOG function: CVirtualMachineInternal::StackPopString  gog 0x00646070  callconv=(blank)  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x00702dd0 (size 129, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702c30:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00646010 CVirtualMachineInternal::StackPushVector -> 0x00702d70  gate_class=REVIEW
- GOG-order neighbour after:  0x00646100 CVirtualMachineInternal::StackPushString -> 0x00702e60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1b8*,0x1bc*} | Steam {0x1b8*,0x1bc*}
imm sets (* = in both, 4 common): GOG {0x4*,0x5*,0xc*,0x1ac*} | Steam {0x4*,0x5*,0xc*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702dd0: 16-byte aligned: YES
  00702dd0  55             PUSH EBP
  00702dd1  8B EC          MOV EBP,ESP
  00702dd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702dd0
string literals (0):
direct callees (3):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x007337C0  CExoString::operator=
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00702dd0 FUN_00702dd0 size=129 ret=4 callers=2 callees=3

undefined4 __fastcall FUN_00702dd0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006ff440();
  if (iVar1 < 1) {
    uVar2 = 0;
  }
  else {
    iVar1 = iVar1 + -1;
    if (*(char *)(*(int *)(param_1 + 0x1b8) + iVar1) == '\x05') {
      FUN_007337c0(*(undefined4 *)(*(int *)(param_1 + 0x1bc) + iVar1 * 4));
      FUN_006ff450(iVar1);
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00702c30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702c30: 16-byte aligned: YES
  00702c30  55             PUSH EBP
  00702c31  8B EC          MOV EBP,ESP
  00702c33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702c30
string literals (0):
direct callees (2):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

## Card B0-5-57

GOG function: CVirtualMachineInternal::PushInstructionPtr  gog 0x00645220  callconv=(blank)  param_size_bytes=4  gog_size=80  gog_retn=4
Candidate Steam address: 0x00701f80 (size 80, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00701f20:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006451c0 CVirtualMachineInternal::PopInstructionPtr -> 0x00701f20  gate_class=REVIEW
- GOG-order neighbour after:  0x00645270 CVirtualMachineInternal::ReadScriptFile -> 0x00701fd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c4*,0x1c8*} | Steam {0x1c4*,0x1c8*}
imm sets (* = in both, 2 common): GOG {0x4*,0x80*} | Steam {0x4*,0x80*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00701f80: 16-byte aligned: YES
  00701f80  55             PUSH EBP
  00701f81  8B EC          MOV EBP,ESP
  00701f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00701f80
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C4 [dec 452]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00701f80 FUN_00701f80 size=80 ret=4 callers=2 callees=0

bool __thiscall FUN_00701f80(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x1c8 + *(int *)(param_1 + 0x1c4) * 4) = param_2;
  *(int *)(param_1 + 0x1c4) = *(int *)(param_1 + 0x1c4) + 1;
  return *(int *)(param_1 + 0x1c4) < 0x80;
}
```

### decoy: 0x00701f20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00701f20: 16-byte aligned: YES
  00701f20  55             PUSH EBP
  00701f21  8B EC          MOV EBP,ESP
  00701f23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00701f20
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1C4 [dec 452]  x5  via EAX,ECX,EDX
```

## Card B0-5-58

GOG function: CTwoDimArrays::GetSwoopUpgradeRowProvider  gog 0x0064c2f0  callconv=__thiscall  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051c8d0 (size 22, retn 0)
Candidate sources: branch+name_map:high:gog_2da_cluster    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (ShowGalaxyMapGui 0x7ca730,ShowPartySelection 0x7cb920,OnPanelAdded 0x894320)
- best decoy (neighbour/other candidate) scores: 0x0051c8f0:STR0/CALLEE3/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c2d0 CServerExoApp::ClientToServerObjectId -> 0x0051c8b0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c310 CServerExoApp::GetPlayerCreatureId -> 0x0051c8f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0x1f0b4*} | Steam {0x1f0b4*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051c8d0: 16-byte aligned: YES
  0051c8d0  55             PUSH EBP
  0051c8d1  8B EC          MOV EBP,ESP
  0051c8d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c8d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c8d0 FUN_0051c8d0 size=22 ret=0 callers=175 callees=0

int __fastcall FUN_0051c8d0(int param_1)

{
  return *(int *)(param_1 + 4) + 0x1f0b4;
}
```

### decoy: 0x0051c8f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c8f0: 16-byte aligned: YES
  0051c8f0  55             PUSH EBP
  0051c8f1  8B EC          MOV EBP,ESP
  0051c8f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c8f0
string literals (0):
direct callees (1):
  0x00539D60  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B0-5-59

GOG function: CVirtualMachine::StackPushFloat  gog 0x00640c20  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x006fda20 (size 28, retn 4)
Candidate sources: card    mapper tag: -

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CVirtualMachine::StackPopVector
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006fda00:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00640c00 CVirtualMachine::StackPopFloat -> 0x006fd9e0  gate_class=REVIEW
- GOG-order neighbour after:  0x00640c40 CVirtualMachine::StackPopVector -> 0x006fda20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00645f50 CVirtualMachineInternal::StackPushFloat -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}
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
prologue for 0x006fda20: 16-byte aligned: YES
  006fda20  55             PUSH EBP
  006fda21  8B EC          MOV EBP,ESP
  006fda23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006fda20
string literals (0):
direct callees (1):
  0x00702D00  CVirtualMachineInternal::StackPopVector
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006fda20 FUN_006fda20 size=28 ret=4 callers=14 callees=1
THUNK -> 0x00702D00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_006fda20(undefined4 param_1)

{
  FUN_00702d00(param_1);
  return;
}
```

### decoy: 0x006fda00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006fda00: 16-byte aligned: YES
  006fda00  55             PUSH EBP
  006fda01  8B EC          MOV EBP,ESP
  006fda03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006fda00
string literals (0):
direct callees (1):
  0x00702CB0  CVirtualMachineInternal::StackPushFloat
struct offsets touched (top 1):
  +0x1C [dec 28]  x1  via EAX
```

## Card B0-5-60

GOG function: CVirtualMachineInternal::StackPopFloat  gog 0x00645ed0  callconv=(blank)  param_size_bytes=4  gog_size=127  gog_retn=4
Candidate Steam address: 0x00702c30 (size 127, retn 4)
Candidate sources: branch+mapper:vote+name_map:high:batch6_reverse_engineering+near    mapper tag: vote

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00702dd0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00645e80 CVirtualMachineInternal::StackPushInteger -> 0x00702be0  gate_class=REVIEW
- GOG-order neighbour after:  0x00645f50 CVirtualMachineInternal::StackPushFloat -> 0x00702cb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00642680 CVirtualMachineStack::GetStackPointer -> Steam ?  in candidate callees: n/a
  0x00642690 CVirtualMachineStack::SetStackPointer -> Steam ?  in candidate callees: n/a
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1b8*,0x1bc*} | Steam {0x1b8*,0x1bc*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x1ac*} | Steam {0x4*,0xc*,0x1ac*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00702c30: 16-byte aligned: YES
  00702c30  55             PUSH EBP
  00702c31  8B EC          MOV EBP,ESP
  00702c33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00702c30
string literals (0):
direct callees (2):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00702c30 FUN_00702c30 size=127 ret=4 callers=3 callees=2

undefined4 __thiscall FUN_00702c30(int param_1,undefined4 *param_2)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_006ff440();
  if (iVar1 < 1) {
    uVar2 = 0;
  }
  else {
    iVar1 = iVar1 + -1;
    if (*(char *)(*(int *)(param_1 + 0x1b8) + iVar1) == '\x04') {
      *param_2 = *(undefined4 *)(*(int *)(param_1 + 0x1bc) + iVar1 * 4);
      FUN_006ff450(iVar1);
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x00702dd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00702dd0: 16-byte aligned: YES
  00702dd0  55             PUSH EBP
  00702dd1  8B EC          MOV EBP,ESP
  00702dd3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00702dd0
string literals (0):
direct callees (3):
  0x006FF440  CVirtualMachineStack::GetStackPointer
  0x007337C0  CExoString::operator=
  0x006FF450  CVirtualMachineStack::SetStackPointer
struct offsets touched (top 2):
  +0x1B8 [dec 440]  x1  via ECX
  +0x1BC [dec 444]  x1  via EDX
```
