# Review cards B17 chunk 15

Answer each card with one TSV row in `steam_db_batches/B17/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B17-15-01

GOG function: CClientOptions::SetShadows  gog 0x00468f00  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc1b0 (size 116, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc230:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00468e80 CClientOptions::SetEnableHardwareMouseCursor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00468f30 CClientOptions::SetSoftShadows -> 0x007bc260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086ab80 Global::AurEnableShadows -> Steam ?  in candidate callees: n/a
  0x0086ab90 Global::AurDisableShadows -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2c} | Steam {0x4}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x10,0xfffffffd}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007bc1b0: 16-byte aligned: YES
  007bc1b0  55             PUSH EBP
  007bc1b1  8B EC          MOV EBP,ESP
  007bc1b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc1b0
string literals (0):
direct callees (1):
  0x00740F40  CClientExoApp::SetMouseMode
struct offsets touched (top 1):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc1b0 FUN_007bc1b0 size=116 ret=4 callers=1 callees=1

void __thiscall FUN_007bc1b0(uint *param_1,uint param_2)

{
  if ((DAT_00a1b4a4 != 0) && (*(int *)(DAT_00a1b4a4 + 4) != 0)) {
    if (param_2 == 0) {
      FUN_00740f40(1);
    }
    else {
      FUN_00740f40(0);
    }
    *param_1 = *param_1 & 0xfffffffd | (param_2 & 1) << 1;
  }
  return;
}
```

### decoy: 0x007bc230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc230: 16-byte aligned: YES
  007bc230  55             PUSH EBP
  007bc231  8B EC          MOV EBP,ESP
  007bc233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc230
string literals (0):
direct callees (2):
  0x00478CA0  (unnamed)
  0x00478CB0  Global::AurDisableShadows
struct offsets touched (top 1):
  +0x2C [dec 44]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 116 B, RET 4
- callers 1: 0x00796e00 CClientExoAppInternal::InitializeClientOptions
- callees: 0x00740f40 CClientExoApp::SetMouseMode (CALL)
- globals: 0x00a1b4a4 READ, 0x00a1b4a4 READ, 0x00a1b4a4 READ, ...
- twins: none

## Card B17-15-02

GOG function: CSWGuiSlider::HandleInputEvent  gog 0x009192c0  callconv=(blank)  param_size_bytes=8  gog_size=537  gog_retn=8
Candidate Steam address: 0x0041b220 (size 537, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0041b440:STR0/CALLEE0/SIG0.12/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00919270 CSWGuiSlider::SetCurValue  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009194e0 CSWGuiSlider::HitCheckSlider  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
  0x00916a50 CSWGuiNavigable::HandleInputEvent -> Steam ?  in candidate callees: n/a
  0x00919270 CSWGuiSlider::SetCurValue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0xc*,0x10*,0x1c*,0x28*,0x2c*,0x38*,0x74*,0x78*,0x7c*} | Steam {0xc*,0x10*,0x1c*,0x28*,0x2c*,0x38*,0x74*,0x78*,0x7c*}
imm sets (* = in both, 12 common): GOG {0x8*,0x28*,0x2f*,0x30*,0x31*,0x32*,0x3d*,0x3e*,0x3f*,0x40*,0x1f4*,0x1f5*} | Steam {0x8*,0x28*,0x2f*,0x30*,0x31*,0x32*,0x3d*,0x3e*,0x3f*,0x40*,0x1f4*,0x1f5*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0041b220: 16-byte aligned: YES
  0041b220  55             PUSH EBP
  0041b221  8B EC          MOV EBP,ESP
  0041b223  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041b220
string literals (0):
direct callees (3):
  0x0041B1D0  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x004187B0  (unnamed)
struct offsets touched (top 9):
  +0x38 [dec 56]  x10  via EAX,ECX,EDX
  +0x78 [dec 120]  x8  via EAX,ECX,EDX
  +0x28 [dec 40]  x3  via EDX
  +0x1C [dec 28]  x2  via ECX
  +0x2C [dec 44]  x2  via EDX
  +0x74 [dec 116]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
  +0x7C [dec 124]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0041b220 FUN_0041b220 size=537 ret=8 callers=0 callees=3

void __thiscall FUN_0041b220(int param_1,int param_2,int param_3)

{
  int iVar1;
  bool bVar2;
  int local_c;
  
  if (param_3 == 0) goto LAB_0041b423;
  bVar2 = false;
  if (*(int *)(param_1 + 0xc) < *(int *)(param_1 + 0x10)) {
    if (param_2 < 0x3f) {
      if (param_2 == 0x3e) {
LAB_0041b2b0:
        bVar2 = *(int *)(param_1 + 0x78) != *(int *)(param_1 + 0x74);
        FUN_0041b1d0(*(int *)(param_1 + 0x78) + 1);
      }
      else {
        if (param_2 != 0x31) {
          if (param_2 == 0x32) goto LAB_0041b2b0;
          if (param_2 != 0x3d) goto LAB_0041b35e;
        }
LAB_0041b28d:
        bVar2 = *(int *)(param_1 + 0x78) != 0;
        FUN_0041b1d0(*(int *)(param_1 + 0x78) + -1);
      }
    }
    else {
      if (param_2 == 500) goto LAB_0041b2b0;
      if (param_2 == 0x1f5) goto LAB_0041b28d;
    }
  }
  else {
    if (param_2 < 0x41) {
      if (param_2 != 0x40) {
        if (param_2 != 0x2f) {
          if (param_2 == 0x30) goto LAB_0041b338;
          if (param_2 != 0x3f) goto LAB_0041b35e;
        }
LAB_0041b315:
        bVar2 = *(int *)(param_1 + 0x78) != 0;
        FUN_0041b1d0(*(int *)(param_1 + 0x78) + -1);
        goto LAB_0041b35e;
      }
    }
    else if (param_2 != 500) {
      if (param_2 != 0x1f5) goto LAB_0041b35e;
      goto LAB_0041b315;
    }
LAB_0041b338:
    bVar2 = *(int *)(param_1 + 0x78) != *(int *)(param_1 + 0x74);
    FUN_0041b1d0(*(int *)(param_1 + 0x78) + 1);
  }
LAB_0041b35e:
  if ((bVar2) && (*(int *)(param_1 + 0x38) != 0)) {
    local_c = 0;
    iVar1 = (**(code **)(**(int **)(param_1 + 0x38) + 0x28))();
    if (iVar1 == 0) {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x38) + 0x2c))();
      if (iVar1 != 0) {
        iVar1 = (**(code **)(**(int **)(param_1 + 0x38) + 0x2c))();
        iVar1 = (**(code **)(**(int **)(iVar1 + 0x38) + 0x28))();
        local_c = *(int *)(iVar1 + 0x1c);
      }
    }
    else {
      iVar1 = (**(code **)(**(int **)(param_1 + 0x38) + 0x28))();
      local_c = *(int *)(iVar1 + 0x1c);
    }
    if (local_c != 0) {
      FUN_004122a0(*(undefined1 *)(param_1 + 0x7c));
    }
  }
LAB_0041b423:
  FUN_004187b0(param_2,param_3);
  return;
}
```

### decoy: 0x0041b440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041b440: 16-byte aligned: YES
  0041b440  55             PUSH EBP
  0041b441  8B EC          MOV EBP,ESP
  0041b443  83 EC 54       SUB ESP,0x54
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041b440
string literals (0):
direct callees (1):
  0x00414D80  CSWGuiBorder::GetInnerExtent
struct offsets touched (top 4):
  +0xC [dec 12]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
  +0x10 [dec 16]  x1  via EAX
```

### Static facts (script output, report-only)
- size 537 B, RET 8
- callees: 0x0041b1d0 (CALL), 0x004122a0 CSWGuiManager::PlayGuiSound (CALL), 0x004187b0 (CALL)
- indirect: 0x0041b38f slot 0x28, 0x0041b3a6 slot 0x28, 0x0041b3c9 slot 0x2c, 0x0041b3e0 slot 0x2c, 0x0041b3ff slot 0x28
- twins: none

## Card B17-15-03

GOG function: CResVIS::GetVISSize  gog 0x009434e0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00703bd0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: LoadArea 0x7a2fb0)
- best decoy (neighbour/other candidate) scores: 0x00703bb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009434c0 CResWave::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00943500 CResWave::OnResourceFreed  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2c*} | Steam {0x2c*}
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
prologue for 0x00703bd0: 16-byte aligned: YES
  00703bd0  55             PUSH EBP
  00703bd1  8B EC          MOV EBP,ESP
  00703bd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00703bd0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2C [dec 44]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00703bd0 FUN_00703bd0 size=17 ret=0 callers=7 callees=0

undefined4 __fastcall FUN_00703bd0(int param_1)

{
  return *(undefined4 *)(param_1 + 0x2c);
}
```

### decoy: 0x00703bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00703bb0: 16-byte aligned: YES
  00703bb0  55             PUSH EBP
  00703bb1  8B EC          MOV EBP,ESP
  00703bb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00703bb0
string literals (0):
direct callees (1):
  0x007111A0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 7: 0x005e9ee0, 0x00701fd0 CVirtualMachineInternal::ReadScriptFile, 0x0071c3d0, 0x0071f750, 0x00722d90, ...
- twins: none

## Card B17-15-04

GOG function: Global::`eh_vector_constructor_iterator'  gog 0x009258bf  callconv=__stdcall  param_size_bytes=20  gog_size=77  gog_retn=20
Candidate Steam address: 0x0091d67e (size 77, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: fail:unaligned | G3 prologue: fail:no_standard_prologue | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 34 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 34 (callers: Constructor 0x780460, LoadArea 0x7a2fb0, InitializeRepliesData 0x7c06f0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008103c0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: autolabel:`eh_vector_constructor_iterato
- GOG-order neighbour before: 0x00925530 Global::memcpy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00925936 Global::rand -> 0x008103c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0092590c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca2c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca71 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (35 of 135): the card's A-CALLEE credit may come from these, not from callees
  0x0042c9e0 CClientExoAppInternal::Constructor -> Steam 0x00780460  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004d0d80 CGuiInGame::InitializeRepliesData -> Steam 0x007c06f0  in candidate callers: yes
  0x00518840 CSWGuiMainInterface::Constructor -> Steam 0x00747210  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x10*,0x14*,0xfffffffe*} | Steam {0x10*,0x14*,0xfffffffe*}

### abi
```
GOG param bytes: 20; callconv: stdcall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0091d67e: 16-byte aligned: NO (addr % 16 = 14)
  0091d67e  6A 10          PUSH 0x10
  0091d680  68 08 03 9F 00 PUSH 0x9f0308
  0091d685  E8 42 77 00 00 CALL 0x00924dcc
standard prologue: NO
```

### anchors
```
anchors for 0x0091d67e
string literals (0):
direct callees (3):
  0x00924DCC  (unnamed)
  0x0091D6CB  (unnamed)
  0x00924E11  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0091d67e `eh_vector_constructor_iterator' size=77 ret=20 callers=134 callees=3

/* WARNING: Function: __SEH_prolog4 replaced with injection: SEH_prolog4 */
/* WARNING: Function: __SEH_epilog4 replaced with injection: EH_epilog3 */
/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    void __stdcall `eh vector constructor iterator'(void *,unsigned int,int,void (__thiscall*)(void
   *),void (__thiscall*)(void *))
   
   Libraries: Visual Studio 2005 Release, Visual Studio 2008 Release, Visual Studio 2010 Release */

void _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
               (void *param_1,uint param_2,int param_3,_func_void_void_ptr [GHIDRA-AUTO-LABEL: unreliable] *param_4,
               _func_void_void_ptr [GHIDRA-AUTO-LABEL: unreliable] *param_5)

{
  void *in_stack_ffffffcc;
  undefined4 local_20;
  
  for (local_20 = 0; local_20 < param_3; local_20 = local_20 + 1) {
    (*param_4)(in_stack_ffffffcc);
  }
  FUN_0091d6cb();
  return;
}
```

### decoy: 0x008103c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008103c0: 16-byte aligned: YES
  008103c0  55             PUSH EBP
  008103c1  8B EC          MOV EBP,ESP
  008103c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008103c0
string literals (0):
direct callees (7):
  0x008935A0  (unnamed)
  0x005E36A0  (unnamed)
  0x005E3DE0  (unnamed)
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007FD840  CSWCItem::Destructor_2
struct offsets touched (top 5):
  +0x34 [dec 52]  x3  via EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x84 [dec 132]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
```

### Static facts (script output, report-only)
- size 77 B, RET 20
- callers 134: 0x0040e170, 0x00451a80, 0x00464040, 0x004641a0, 0x00474230, ...
- callees: 0x00924dcc (CALL), 0x0091d6cb (CALL), 0x00924e11 (CALL)
- indirect: 0x0091d6a2 unresolved
- twins: none

## Card B17-15-05

GOG function: CExoSoundInternal::SetListenerPosition  gog 0x00637250  callconv=(blank)  param_size_bytes=4  gog_size=135  gog_retn=4
Candidate Steam address: 0x007088d0 (size 316, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.14) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00708510:STR0/CALLEE0/SIG0.097/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00637160 CExoSoundInternal::SetListenerOrientation -> 0x007081e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006372e0 CExoSoundInternal::PlayOneShotSound  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {0x5,0x34,0x35,0x48,0xa8,0xac}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0xc,0x9c} | Steam {0x2,0x4*,0x1c,0x44,0x48,0xa8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007088d0: 16-byte aligned: YES
  007088d0  55             PUSH EBP
  007088d1  8B EC          MOV EBP,ESP
  007088d3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007088d0
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x0064DDE0  (unnamed)
  0x00709010  (unnamed)
struct offsets touched (top 6):
  +0xA8 [dec 168]  x5  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX,EDX
  +0x5 [dec 5]  x2  via ECX,EDX
  +0xAC [dec 172]  x2  via EAX,EDX
  +0x34 [dec 52]  x1  via EAX
  +0x35 [dec 53]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007088d0 FUN_007088d0 size=316 ret=4 callers=4 callees=5

int __thiscall FUN_007088d0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_c;
  int local_8;
  
  local_c = FUN_007a1720();
  local_8 = FUN_00521060();
  do {
    if (local_c == 0) {
LAB_00708997:
      local_c = *(int *)(param_2 + 0x48);
      FUN_0058c370(&local_c);
      FUN_0064dde0(*(undefined4 *)(param_2 + 0x48));
      *(undefined1 *)(param_2 + 0x34) = 0;
      *(undefined4 *)(param_2 + 0x48) = 0;
      iVar2 = FUN_00709010(*(undefined1 *)(param_2 + 0x35));
      *(char *)(iVar2 + 5) = *(char *)(iVar2 + 5) + -1;
      return local_c;
    }
    if (local_8 == param_2) {
      if ((local_c == *(int *)(param_1 + 0xa8)) &&
         (FUN_0058c370(param_1 + 0xa8), *(int *)(param_1 + 0xa8) == 0)) {
        uVar1 = FUN_007a1720();
        *(undefined4 *)(param_1 + 0xa8) = uVar1;
        *(undefined4 *)(param_1 + 0xac) = 2;
        if (*(int *)(param_1 + 0xa8) == 0) {
          *(undefined4 *)(param_1 + 0xac) = 1;
          *(undefined4 *)(param_1 + 0xa8) = 0;
        }
      }
      goto LAB_00708997;
    }
    local_8 = FUN_0058c370(&local_c);
  } while( true );
}
```

### decoy: 0x00708510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00708510: 16-byte aligned: YES
  00708510  55             PUSH EBP
  00708511  8B EC          MOV EBP,ESP
  00708513  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00708510
string literals (0):
direct callees (25):
  0x00710C20  (unnamed)
  0x007109B0  (unnamed)
  0x00920830  (unnamed)
  0x0070B2F0  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x00919723  operator new
  0x0070AFD0  (unnamed)
  0x0070B350  (unnamed)
  0x0070B380  (unnamed)
  0x0070B3F0  (unnamed)
  0x0070B2A0  (unnamed)
  0x00709010  (unnamed)
  0x0070B3B0  (unnamed)
  0x0070B170  (unnamed)
  0x0070E9F0  (unnamed)
  0x0070A8D0  Source::LoadWaveResource (ret 8)
  0x0070FD80  (unnamed)
  0x007101F0  (unnamed)
  0x0070FD00  Source::SetPosition
struct offsets touched (top 8):
  +0x124 [dec 292]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0xC [dec 12]  x2  via ECX
  +0x1 [dec 1]  x1  via EDX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via EAX
  +0x74 [dec 116]  x1  via ECX
  +0x78 [dec 120]  x1  via EAX
```

### Static facts (script output, report-only)
- size 316 B, RET 4
- callers 4: 0x007051c0, 0x00705d70, 0x00709cf0, 0x0070fb90
- callees: 0x007a1720 (CALL), 0x00521060 (CALL), 0x0058c370 (CALL), 0x0064dde0 (CALL), 0x00709010 (CALL)
- twins: none

## Card B17-15-06

GOG function: CSWGuiScene::Draw  gog 0x009168b0  callconv=(blank)  param_size_bytes=4  gog_size=120  gog_retn=4
Candidate Steam address: 0x004184f0 (size 120, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004184b0:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00916870 CSWGuiScene::GetModel  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00916930 CSWGuiNavigable::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cfd70 Global::AurGUISetupViewport -> Steam ?  in candidate callees: n/a
  0x008d02d0 Global::AurGUICloseViewport -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x1c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x1c*,0x28*} | Steam {0x4*,0x1c*,0x28*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x004184f0: 16-byte aligned: YES
  004184f0  55             PUSH EBP
  004184f1  8B EC          MOV EBP,ESP
  004184f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004184f0
string literals (0):
direct callees (2):
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x004799C0  (unnamed)
struct offsets touched (top 7):
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 004184f0 FUN_004184f0 size=120 ret=4 callers=0 callees=2

void __thiscall FUN_004184f0(int param_1,undefined4 param_2)

{
  int iVar1;
  
  (**(code **)(**(int **)(param_1 + 0x14) + 0x1c))(param_2);
  iVar1 = FUN_00479460(*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 8),
                       *(undefined4 *)(param_1 + 0xc),*(undefined4 *)(param_1 + 0x10),param_1 + 0x28
                       ,1,0x3f800000);
  if (iVar1 != 0) {
    (**(code **)(**(int **)(param_1 + 0x18) + 8))();
    FUN_004799c0();
  }
  return;
}
```

### decoy: 0x004184b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004184b0: 16-byte aligned: YES
  004184b0  55             PUSH EBP
  004184b1  8B EC          MOV EBP,ESP
  004184b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004184b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
```

### Static facts (script output, report-only)
- size 120 B, RET 4
- callees: 0x00479460 GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected) (CALL), 0x004799c0 (CALL)
- indirect: 0x00418511 slot 0x1c, 0x0041855b slot 0x8
- twins: none

## Card B17-15-07

GOG function: CSWGuiRadioButton::HandleLMouseUp  gog 0x00918bf0  callconv=(blank)  param_size_bytes=0  gog_size=154  gog_retn=0
Candidate Steam address: 0x0041aac0 (size 154, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: GetSelectableParent 0x4193a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0041aaa0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00918b90 CSWGuiRadioButton::HandleLMouseDown  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00918c90 CSWGuiSlider::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x009118e0 CSWGuiManager::ReleaseMouse -> Steam ?  in candidate callees: n/a
  0x00917640 CSWGuiControl::GetSelectableParent -> Steam 0x004193a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x8*,0x14*,0x1c*,0x3c*} | Steam {0x8*,0x14*,0x1c*,0x3c*}
imm sets (* = in both, 3 common): GOG {0x10*,0x27*,0x1f9*} | Steam {0x10*,0x27*,0x1f9*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0041aac0: 16-byte aligned: YES
  0041aac0  55             PUSH EBP
  0041aac1  8B EC          MOV EBP,ESP
  0041aac3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041aac0
string literals (0):
direct callees (2):
  0x004193A0  CSWGuiControl::GetSelectableParent
  0x00413460  (unnamed)
struct offsets touched (top 4):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x3C [dec 60]  x2  via EDX
  +0x14 [dec 20]  x1  via EAX
  +0x1C [dec 28]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0041aac0 FUN_0041aac0 size=154 ret=0 callers=0 callees=2

void __fastcall FUN_0041aac0(int *param_1)

{
  int *piVar1;
  
  if (*(int *)(DAT_00a1b49c + 8) != 0) {
    if (*(int **)(DAT_00a1b49c + 8) != param_1) {
      piVar1 = (int *)FUN_004193a0(param_1,*(undefined4 *)(DAT_00a1b49c + 8));
      if (piVar1 != param_1) goto LAB_0041ab3a;
    }
    if ((*(uint *)(DAT_00a1b49c + 0x1c) & 1) != 0) {
      (**(code **)(*param_1 + 0x3c))(0x27,1);
      (**(code **)(*param_1 + 0x3c))(0x1f9,1);
    }
  }
LAB_0041ab3a:
  FUN_00413460(param_1);
  (**(code **)(*param_1 + 0x14))();
  return;
}
```

### decoy: 0x0041aaa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041aaa0: 16-byte aligned: YES
  0041aaa0  55             PUSH EBP
  0041aaa1  8B EC          MOV EBP,ESP
  0041aaa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041aaa0
string literals (0):
direct callees (1):
  0x00419060  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 154 B, RET 0
- callees: 0x004193a0 CSWGuiControl::GetSelectableParent (CALL), 0x00413460 (CALL)
- indirect: 0x0041ab24 slot 0x3c, 0x0041ab38 slot 0x3c, 0x0041ab54 slot 0x14
- globals: 0x00a1b49c READ, 0x00a1b49c READ, 0x00a1b49c READ
- twins: none

## Card B17-15-08

GOG function: CClientOptions::SetShadows  gog 0x00468f00  callconv=(blank)  param_size_bytes=4  gog_size=43  gog_retn=4
Candidate Steam address: 0x007bc260 (size 43, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007bc230:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00468e80 CClientOptions::SetEnableHardwareMouseCursor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00468f30 CClientOptions::SetSoftShadows -> 0x007bc260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086ab80 Global::AurEnableShadows -> Steam ?  in candidate callees: n/a
  0x0086ab90 Global::AurDisableShadows -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2c} | Steam {0x30}
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
prologue for 0x007bc260: 16-byte aligned: YES
  007bc260  55             PUSH EBP
  007bc261  8B EC          MOV EBP,ESP
  007bc263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007bc260
string literals (0):
direct callees (2):
  0x00478CF0  Global::AurEnableSoftShadows
  0x00478D10  Global::AurDisableSoftShadows
struct offsets touched (top 1):
  +0x30 [dec 48]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007bc260 FUN_007bc260 size=43 ret=4 callers=3 callees=2

void __thiscall FUN_007bc260(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x30) = param_2;
  if (*(int *)(param_1 + 0x30) == 0) {
    FUN_00478d10();
  }
  else {
    FUN_00478cf0();
  }
  return;
}
```

### decoy: 0x007bc230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007bc230: 16-byte aligned: YES
  007bc230  55             PUSH EBP
  007bc231  8B EC          MOV EBP,ESP
  007bc233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007bc230
string literals (0):
direct callees (2):
  0x00478CA0  (unnamed)
  0x00478CB0  Global::AurDisableShadows
struct offsets touched (top 1):
  +0x2C [dec 44]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 43 B, RET 4
- callers 3: 0x007b88b0 ClientOptions ini reader ([Game Options] Auto Level Up NPCs etc.), 0x007d5be0, 0x007d75f0
- callees: 0x00478cf0 (CALL), 0x00478d10 (CALL)
- twins: none

## Card B17-15-09

GOG function: CSWRoomSurfaceMesh::TransformAABBToWorld  gog 0x00714f60  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=8
Candidate Steam address: 0x00652930 (size 133, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.20) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00652810:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00714f10 CSWRoomSurfaceMesh::TransformToWorld -> 0x006527c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00715080 CSWRoomSurfaceMesh::GetEdgeVertex  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00714f60 CSWRoomSurfaceMesh::TransformAABBToWorld -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*,0x24,0x28,0xac} | Steam {0x4*,0x8*,0x54,0x60,0x8c,0x90}
imm sets (* = in both, 2 common): GOG {0x8*,0xc*,0x2c,0x74} | Steam {0x8*,0xc*,0x14}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00652930: 16-byte aligned: YES
  00652930  55             PUSH EBP
  00652931  8B EC          MOV EBP,ESP
  00652933  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00652930
string literals (0):
direct callees (0):
struct offsets touched (top 6):
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x54 [dec 84]  x1  via ECX
  +0x60 [dec 96]  x1  via ECX
  +0x8C [dec 140]  x1  via EAX
  +0x90 [dec 144]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00652930 FUN_00652930 size=133 ret=8 callers=1 callees=0

bool __thiscall FUN_00652930(int param_1,int param_2,undefined4 *param_3)

{
  bool bVar1;
  undefined4 *puVar2;
  
  bVar1 = *(int *)(param_1 + 0x90) <= param_2;
  if (bVar1) {
    *param_3 = 0;
    param_3[1 = byte +0x4] = 0;
    param_3[2 = byte +0x8] = 0;
  }
  else {
    puVar2 = (undefined4 *)
             (*(int *)(param_1 + 0x54) +
             *(int *)(*(int *)(param_1 + 0x60) +
                     *(int *)(*(int *)(param_1 + 0x8c) + param_2 * 8) * 4) * 0xc);
    *param_3 = *puVar2;
    param_3[1 = byte +0x4] = puVar2[1 = byte +0x4];
    param_3[2 = byte +0x8] = puVar2[2 = byte +0x8];
  }
  return !bVar1;
}
```

### decoy: 0x00652810 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00652810: 16-byte aligned: YES
  00652810  55             PUSH EBP
  00652811  8B EC          MOV EBP,ESP
  00652813  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00652810
string literals (0):
direct callees (2):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00652810  Global::AurGrassAddInfo
struct offsets touched (top 3):
  +0xAC [dec 172]  x6  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
```

### Static facts (script output, report-only)
- size 133 B, RET 8
- callers 1: 0x00652b40
- twins: none

## Card B17-15-10

GOG function: Global::GetResolutionTextScalar  gog 0x008d03c0  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x004799c0 (size 235, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00479ab0:STR0/CALLEE6/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d02d0 Global::AurGUICloseViewport -> 0x004799c0  gate_class=REVIEW
- GOG-order neighbour after:  0x008d0420 CAurGUIStringInternal::RefreshResolutionTextScalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 43): the card's A-CALLEE credit may come from these, not from callees
  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> Steam 0x00752f50  in candidate callers: no
  0x00564590 CSWGuiMapHider::Constructor -> Steam 0x00895620  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: yes
  0x005af4f0 CSWGuiDialogLetterbox::SetBottom -> Steam 0x008bada0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0xa*,0xc,0x3fe,0x3ff} | Steam {0x2,0xa*,0xb,0x10}

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

### decoy: 0x00479ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00479ab0: 16-byte aligned: YES
  00479ab0  55             PUSH EBP
  00479ab1  8B EC          MOV EBP,ESP
  00479ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00479ab0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 235 B, RET 0
- callers 12: 0x0040ec90 CSWGuiPanel::Draw, 0x004113a0, 0x004184f0, 0x0041bf40, 0x0041f980, ...
- callees: 0x0000004f (CALL), 0x0000000e (CALL), 0x0000001c (CALL)
- globals: 0x00a309c0 READ, 0x00a309c0 READ, 0x00a309c0 READ, ...
- imports: glDisable (OPENGL32.DLL), glEnable (OPENGL32.DLL), glViewport (OPENGL32.DLL)
- twins: none

## Card B17-15-11

GOG function: Global::BSPTraverseVolumeApprox  gog 0x00944cf0  callconv=(blank)  param_size_bytes=20  gog_size=1154  gog_retn=0
Candidate Steam address: 0x00519ba0 (size 1154, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051a030:STR0/CALLEE0/SIG0.365/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00944a50 Global::BSPTraverse  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00945180 Global::BSPFitSphere  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00944a50 Global::BSPTraverse -> Steam ?  in candidate callees: n/a
  0x00944c60 unnamed -> Steam ?  in candidate callees: n/a
  0x00944cf0 Global::BSPTraverseVolumeApprox -> Steam ?  in candidate callees: n/a
  0x00945840 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x38*,0x44*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x38*,0x44*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x34*,0x40*,0x41*,0x4c*,0x58*} | Steam {0x2*,0x3*,0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x34*,0x40*,0x41*,0x4c*,0x58*}

### abi
```
GOG param bytes: 20; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 20 bytes -> x86 slots 20; ASM RET 0)
```

### prologue
```
prologue for 0x00519ba0: 16-byte aligned: YES
  00519ba0  55             PUSH EBP
  00519ba1  8B EC          MOV EBP,ESP
  00519ba3  83 EC 58       SUB ESP,0x58
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00519ba0
string literals (0):
direct callees (5):
  0x00519B10  (unnamed)
  0x0051A6F0  (unnamed)
  0x0044B8A0  (unnamed)
  0x00519BA0  (unnamed)
  0x00519900  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x17  via EAX,ECX,EDX
  +0x8 [dec 8]  x16  via EAX,ECX,EDX
  +0xC [dec 12]  x13  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX,ECX
  +0x44 [dec 68]  x3  via ECX,EDX
  +0x10 [dec 16]  x2  via EDX
  +0x14 [dec 20]  x2  via EAX,ECX
  +0x18 [dec 24]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00519ba0 FUN_00519ba0 size=1154 ret=0 callers=2 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00519ba0(int param_1,undefined4 *param_2,int *param_3,code *param_4,undefined4 param_5)

{
  float fVar1;
  bool bVar2;
  bool bVar3;
  bool bVar4;
  int iVar5;
  int iVar6;
  undefined4 *puVar7;
  float10 fVar8;
  float10 fVar9;
  int local_5c;
  int local_58;
  int local_50;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  int local_14;
  
  if ((param_1 != 0) && ((*param_4)(param_1,param_5,param_3), *(int *)(param_1 + 0x18) == 0)) {
    bVar3 = false;
    bVar2 = false;
    *(undefined4 *)(param_1 + 0x38) = 0;
    *(undefined4 *)(param_1 + 0x44) = 0;
    local_14 = param_3[1 = byte +0x4];
    do {
      iVar5 = local_14 + -1;
      if (local_14 == 0) {
        fVar9 = (float10)FUN_0044b8a0(param_2,param_1);
        fVar1 = (float)(fVar9 + (float10)*(float *)(param_1 + 0xc));
        if (fVar1 <= (float)_DAT_00986b38) {
          local_44 = *(undefined4 *)(param_1 + 0x10);
        }
        else {
          local_44 = *(undefined4 *)(param_1 + 0x14);
        }
        if (fVar1 <= (float)_DAT_00986b38) {
          local_48 = *(undefined4 *)(param_1 + 0x14);
        }
        else {
          local_48 = *(undefined4 *)(param_1 + 0x10);
        }
        if (fVar1 <= (float)_DAT_00986b38) {
          local_4c = *(int *)(param_1 + 0x38);
        }
        else {
          local_4c = *(int *)(param_1 + 0x44);
        }
        if (fVar1 <= (float)_DAT_00986b38) {
          local_50 = *(int *)(param_1 + 0x44);
        }
        else {
          local_50 = *(int *)(param_1 + 0x38);
        }
        bVar4 = bVar3;
        if ((float)_DAT_00986b38 < fVar1) {
          bVar4 = bVar2;
        }
        if ((float)_DAT_00986b38 < fVar1) {
          bVar2 = bVar3;
        }
        if (!bVar4) {
          if (local_4c == 0) {
            FUN_00519900(local_44,*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],param_4,param_5);
          }
          else {
            if (fVar1 <= (float)_DAT_00986b38) {
              local_58 = param_1 + 0x34;
            }
            else {
              local_58 = param_1 + 0x40;
            }
            FUN_00519ba0(local_44,param_2,local_58,param_4,param_5);
          }
        }
        if (bVar2) {
/* ... 57 more lines truncated by --max-lines */
```

### decoy: 0x0051a030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051a030: 16-byte aligned: YES
  0051a030  55             PUSH EBP
  0051a031  8B EC          MOV EBP,ESP
  0051a033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051a030
string literals (0):
direct callees (2):
  0x0044B8A0  (unnamed)
  0x0051A030  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
```

### Static facts (script output, report-only)
- size 1154 B, RET 0
- callers 2: 0x00499610, 0x00519ba0
- callees: 0x00519b10 (CALL), 0x0051a6f0 (CALL), 0x0044b8a0 (CALL), 0x00519ba0 (CALL), 0x00519900 (CALL)
- indirect: 0x00519bbd unresolved
- twins: none

## Card B17-15-12

GOG function: Global::GetResolutionTextScalar  gog 0x008d03c0  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x00479b10 (size 78, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurGUIStringInternal::RefreshResolutionTextScalar | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00479ab0:STR0/CALLEE6/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008d02d0 Global::AurGUICloseViewport -> 0x004799c0  gate_class=REVIEW
- GOG-order neighbour after:  0x008d0420 CAurGUIStringInternal::RefreshResolutionTextScalar  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 43): the card's A-CALLEE credit may come from these, not from callees
  0x005246d0 CSWGuiMainInterface::SetupPauseGuiExtent -> Steam 0x00752f50  in candidate callers: no
  0x00564590 CSWGuiMapHider::Constructor -> Steam 0x00895620  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: no
  0x005af4f0 CSWGuiDialogLetterbox::SetBottom -> Steam 0x008bada0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x54,0x5c}
imm sets (* = in both, 0 common): GOG {0xa,0xc,0x3fe,0x3ff} | Steam {0x8,0x44}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00479b10: 16-byte aligned: YES
  00479b10  55             PUSH EBP
  00479b11  8B EC          MOV EBP,ESP
  00479b13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00479b10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x5C [dec 92]  x4  via EAX,ECX,EDX
  +0x54 [dec 84]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00479b10 FUN_00479b10 size=78 ret=0 callers=3 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00479b10(int param_1)

{
  float fVar1;
  
  if (_DAT_009f5f64 != *(float *)(param_1 + 0x5c)) {
    fVar1 = *(float *)(param_1 + 0x5c);
    *(float *)(param_1 + 0x5c) = _DAT_009f5f64;
    *(float *)(param_1 + 0x54) = (*(float *)(param_1 + 0x5c) / fVar1) * *(float *)(param_1 + 0x54);
  }
  return;
}
```

### decoy: 0x00479ab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00479ab0: 16-byte aligned: YES
  00479ab0  55             PUSH EBP
  00479ab1  8B EC          MOV EBP,ESP
  00479ab3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00479ab0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 78 B, RET 0
- callers 3: 0x0047aca0, 0x0047af30, 0x0047b500
- twins: none

## Card B17-15-13

GOG function: PartSkin::PostProcess  gog 0x00874400  callconv=(blank)  param_size_bytes=0  gog_size=8182  gog_retn=4
Candidate Steam address: 0x004be460 (size 144, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004be4f0:STR0/CALLEE0/SIG0.187/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x10,0x14,0x18,0x1c,0x20,0x24,0x28,0x2c,0x30,0x34,0x3c,...(+30)} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5,0x8,0xc,0x10,0x14*,0x20,0x24,0x28,0x34,0x44,0x4c,...(+8)} | Steam {0x4*,0x14*,0x98}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x004be460: 16-byte aligned: YES
  004be460  55             PUSH EBP
  004be461  8B EC          MOV EBP,ESP
  004be463  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004be460
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x00456FB0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004be460 FUN_004be460 size=144 ret=4 callers=1 callees=2

undefined4 * __fastcall FUN_004be460(undefined4 param_1)

{
  undefined4 *local_24;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00949bbb;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = (undefined4 *)FUN_00919723(0x98,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (local_24 == (undefined4 *)0x0) {
    local_24 = (undefined4 *)0x0;
  }
  else {
    FUN_00456fb0(param_1);
    *local_24 = PartSkin::vftable;
  }
  ExceptionList = local_10;
  return local_24;
}
```

### decoy: 0x004be4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004be4f0: 16-byte aligned: YES
  004be4f0  55             PUSH EBP
  004be4f1  8B EC          MOV EBP,ESP
  004be4f3  81 EC 84 00 00 00 SUB ESP,0x84
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004be4f0
string literals (0):
direct callees (4):
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x00429330  (unnamed)
  0x00451840  (unnamed)
  0x004BE4F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x13  via EAX,ECX
  +0x8 [dec 8]  x13  via EAX,ECX
  +0xC [dec 12]  x6  via EAX,ECX,EDX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
```

### Static facts (script output, report-only)
- size 144 B, RET 4
- callers 1: 0x00453ef0
- callees: 0x00919723 operator new (CALL), 0x00456fb0 (CALL)
- twins: none

## Card B17-15-14

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00710b70 (size 164, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710c20:STR0/CALLEE9/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (9 of 40): the card's A-CALLEE credit may come from these, not from callees
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

## Card B17-15-15

GOG function: Global::setSBUpLow  gog 0x0092ccaf  callconv=__cdecl  param_size_bytes=4  gog_size=403  gog_retn=0
Candidate Steam address: 0x0092504f (size 403, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: fail:unaligned | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00924feb:STR0/CALLEE0/SIG0.312/SRV0/VT0
- flags: autolabel:setSBUpLow
- GOG-order neighbour before: 0x0092cc4b Global::setSBCS  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0092cee6 Global::getSystemCP  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
  0x0093634e unnamed -> Steam ?  in candidate callees: n/a
  0x00939793 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0xc*,0x1d*,0x20*,0x11d*} | Steam {0x4*,0xc*,0x1d*,0x20*,0x11d*}
imm sets (* = in both, 11 common): GOG {0x2*,0xc*,0x10*,0x19*,0x20*,0x24*,0x44*,0x100*,0x200*,0x51c*,0xffffff9f*} | Steam {0x2*,0xc*,0x10*,0x19*,0x20*,0x24*,0x44*,0x100*,0x200*,0x51c*,0xffffff9f*}

### abi
```
GOG param bytes: 4; callconv: cdecl
expected_ret = 0 (cdecl: caller cleans the stack)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0092504f: 16-byte aligned: NO (addr % 16 = 15)
  0092504f  8B FF          MOV EDI,EDI
  00925051  55             PUSH EBP
  00925052  8B EC          MOV EBP,ESP
standard prologue: YES - hot-patch MOV EDI,EDI; PUSH EBP; MOV EBP,ESP (MSVC CRT style)
```

### anchors
```
anchors for 0x0092504f
string literals (0):
direct callees (4):
  0x0091C630  (unnamed)
  0x00933CC4  (unnamed)
  0x0092C438  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via ESI
  +0xC [dec 12]  x3  via ESI
  +0x20 [dec 32]  x1  via EDX
  +0x11D [dec 285]  x1  via ESI
```

### annotate (candidate, first 80 lines)
```
// FN 0092504f setSBUpLow size=403 ret=0 callers=1 callees=5

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    void __cdecl setSBUpLow(struct threadmbcinfostruct *)
   
   Library: Visual Studio 2008 Release */

void __cdecl setSBUpLow(threadmbcinfostruct *param_1)

{
  byte *pbVar1;
  char *pcVar2;
  BOOL BVar3;
  uint uVar4;
  CHAR CVar5;
  char cVar6;
  BYTE *pBVar7;
  int unaff_ESI;
  _cpinfo [GHIDRA-AUTO-LABEL: unreliable] local_51c;
  WORD local_508 [256];
  CHAR local_308 [256];
  CHAR local_208 [256];
  CHAR local_108 [256];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  BVar3 = GetCPInfo(*(UINT *)(unaff_ESI + 4),&local_51c);
  if (BVar3 == 0) {
    uVar4 = 0;
    do {
      pcVar2 = (char *)(unaff_ESI + 0x11d + uVar4);
      if (pcVar2 + (-0x61 - (unaff_ESI + 0x11d)) + 0x20 < (char *)0x1a) {
        pbVar1 = (byte *)(unaff_ESI + 0x1d + uVar4);
        *pbVar1 = *pbVar1 | 0x10;
        cVar6 = (char)uVar4 + ' ';
LAB_009251c8:
        *pcVar2 = cVar6;
      }
      else {
        if (pcVar2 + (-0x61 - (unaff_ESI + 0x11d)) < (char *)0x1a) {
          pbVar1 = (byte *)(unaff_ESI + 0x1d + uVar4);
          *pbVar1 = *pbVar1 | 0x20;
          cVar6 = (char)uVar4 + -0x20;
          goto LAB_009251c8;
        }
        *pcVar2 = '\0';
      }
      uVar4 = uVar4 + 1;
    } while (uVar4 < 0x100);
  }
  else {
    uVar4 = 0;
    do {
      local_108[uVar4] = (CHAR)uVar4;
      uVar4 = uVar4 + 1;
    } while (uVar4 < 0x100);
    local_108[0] = ' ';
    if (local_51c.LeadByte[0] != 0) {
      pBVar7 = local_51c.LeadByte + 1;
      do {
        uVar4 = (uint)local_51c.LeadByte[0];
        if (uVar4 <= *pBVar7) {
          _memset [GHIDRA-AUTO-LABEL: unreliable](local_108 + uVar4,0x20,(*pBVar7 - uVar4) + 1);
        }
        local_51c.LeadByte[0] = pBVar7[1];
        pBVar7 = pBVar7 + 2;
      } while (local_51c.LeadByte[0] != 0);
    }
    ___crtGetStringTypeA [GHIDRA-AUTO-LABEL: unreliable]
              ((_locale_t [GHIDRA-AUTO-LABEL: unreliable])0x0,1,local_108,0x100,local_508,*(int *)(unaff_ESI + 4),
               *(BOOL *)(unaff_ESI + 0xc));
    ___crtLCMapStringA [GHIDRA-AUTO-LABEL: unreliable]((_locale_t [GHIDRA-AUTO-LABEL: unreliable])0x0,*(LPCWSTR *)(unaff_ESI + 0xc),0x100,local_108,0x100,local_208,
                       0x100,*(int *)(unaff_ESI + 4),0);
    ___crtLCMapStringA [GHIDRA-AUTO-LABEL: unreliable]((_locale_t [GHIDRA-AUTO-LABEL: unreliable])0x0,*(LPCWSTR *)(unaff_ESI + 0xc),0x200,local_108,0x100,local_308,
                       0x100,*(int *)(unaff_ESI + 4),0);
    uVar4 = 0;
    do {
      if ((local_508[uVar4] & 1) == 0) {
        if ((local_508[uVar4] & 2) != 0) {
          pbVar1 = (byte *)(unaff_ESI + 0x1d + uVar4);
          *pbVar1 = *pbVar1 | 0x20;
/* ... 18 more lines truncated by --max-lines */
```

### decoy: 0x00924feb (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00924feb: 16-byte aligned: NO (addr % 16 = 11)
  00924feb  8B FF          MOV EDI,EDI
  00924fed  56             PUSH ESI
  00924fee  57             PUSH EDI
standard prologue: NO
anchors for 0x00924feb
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 6):
  +0x1C [dec 28]  x2  via ESI
  +0x4 [dec 4]  x1  via ESI
  +0x8 [dec 8]  x1  via ESI
  +0xC [dec 12]  x1  via ESI
  +0x10 [dec 16]  x1  via ESI
  +0x11D [dec 285]  x1  via ESI
```

### Static facts (script output, report-only)
- size 403 B, RET 0
- callers 1: 0x00925302
- callees: 0x000000af (CALL), 0x0091c630 (CALL), 0x00933cc4 (CALL), 0x0092c438 (CALL), 0x009196ee __security_check_cookie (CALL)
- imports: GetCPInfo (KERNEL32.DLL)
- twins: none

## Card B17-15-16

GOG function: Global::rand  gog 0x00925936  callconv=__cdecl  param_size_bytes=0  gog_size=34  gog_retn=0
Candidate Steam address: 0x008103c0 (size 328, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0091eb8b:STR0/CALLEE52/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009258bf Global::`eh_vector_constructor_iterator' -> 0x0091d67e  gate_class=REVIEW
- GOG-order neighbour after:  0x009259f0 Global::sin  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0092c66c unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (52 of 104): the card's A-CALLEE credit may come from these, not from callees
  0x004102b0 CSWCCreature::Constructor -> Steam 0x007645f0  in candidate callers: no
  0x00411b60 CSWCCreature::PlayFootstep -> Steam 0x00765e90  in candidate callers: no
  0x00412180 CSWCCreature::AIUpdate_2 -> Steam 0x007664b0  in candidate callers: no
  0x0041b0c0 CSWCCreature::ResolveAttackIndex -> Steam 0x0076f250  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x14} | Steam {0x4,0x10,0x34,0x84,0x94}
imm sets (* = in both, 1 common): GOG {0x10*,0x7fff,0x343fd,0x269ec3} | Steam {0x10*,0x28}

### abi
```
GOG param bytes: 0; callconv: cdecl
expected_ret = 0 (cdecl: caller cleans the stack)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008103c0: 16-byte aligned: YES
  008103c0  55             PUSH EBP
  008103c1  8B EC          MOV EBP,ESP
  008103c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008103c0
string literals (0):
direct callees (7):
  0x008935A0  (unnamed)
  0x005E36A0  (unnamed)
  0x005E3DE0  (unnamed)
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007FD840  CSWCItem::Destructor_2
struct offsets touched (top 5):
  +0x34 [dec 52]  x3  via EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x84 [dec 132]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008103c0 FUN_008103c0 size=328 ret=0 callers=1 callees=7

undefined4 FUN_008103c0(void)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  int iVar5;
  int *local_c;
  int local_8;
  
  uVar2 = FUN_008935a0();
  local_8 = FUN_005e36a0();
  iVar3 = FUN_005e3de0(1);
  if (iVar3 != 0) {
    return 0;
  }
  FUN_0073f4b0();
  if (local_8 == 1) {
    cVar1 = FUN_0053ded0(uVar2,&local_c);
    if (cVar1 != DAT_0099447f) {
      return 0;
    }
    if (local_c == (int *)0x0) {
      return 0;
    }
    iVar3 = (**(code **)(*local_c + 0x34))();
    if ((iVar3 != 0) &&
       (puVar4 = (undefined4 *)(**(code **)(*local_c + 0x34))(), puVar4 != (undefined4 *)0x0)) {
      (**(code **)*puVar4)(1);
    }
  }
  else {
    cVar1 = FUN_0053dfb0(uVar2,&local_c);
    if (cVar1 != DAT_0099447f) {
      return 0;
    }
    iVar3 = (**(code **)(*local_c + 0x34))();
    if (iVar3 == 0) {
      return 0;
    }
    (**(code **)(*(int *)(iVar3 + 0x10) + 0x84))(0);
    iVar5 = FUN_007fd840();
    if (iVar5 == 0) {
      (**(code **)(*(int *)(iVar3 + 0x10) + 0x94))(0);
    }
  }
  return 1;
}
```

### decoy: 0x0091eb8b (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0091eb8b: 16-byte aligned: NO (addr % 16 = 11)
  0091eb8b  E8 7A 5E 00 00 CALL 0x00924a0a
  0091eb90  8B 48 14       MOV ECX,dword ptr [EAX + 0x14]
  0091eb93  69 C9 FD 43 03 00 IMUL ECX,ECX,0x343fd
standard prologue: NO
anchors for 0x0091eb8b
string literals (0):
direct callees (1):
  0x00924A0A  (unnamed)
struct offsets touched (top 1):
  +0x14 [dec 20]  x2  via EAX
```

### Static facts (script output, report-only)
- size 328 B, RET 0
- callers 1: 0x00813120
- callees: 0x008935a0 (CALL), 0x005e36a0 (CALL), 0x005e3de0 (CALL), 0x0073f4b0 CClientExoApp::GetGameObjectArray (CALL), 0x0053ded0 (CALL), 0x0053dfb0 CGameObjectArray::GetGameObject (CALL), ...
- indirect: 0x0081044d slot 0x34, 0x0081045e slot 0x34, 0x0081047b slot 0x0, 0x008104b8 slot 0x34, 0x008104db slot 0x84, ...
- globals: 0x00a1b4a4 READ
- twins: none

## Card B17-15-17

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
GOG callers mapped through trusted rows (9 of 40): the card's A-CALLEE credit may come from these, not from callees
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

## Card B17-15-18

GOG function: CSWGuiProgressBarReveal::Draw  gog 0x00919f50  callconv=(blank)  param_size_bytes=4  gog_size=137  gog_retn=4
Candidate Steam address: 0x0041bf40 (size 137, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0041bda0:STR0/CALLEE0/SIG0.519/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00919db0 CSWGuiProgressBarReveal::SetExtent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00919fe0 CSWGuiScrollBar::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x008cfd70 Global::AurGUISetupViewport -> Steam ?  in candidate callees: n/a
  0x008d02d0 Global::AurGUICloseViewport -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0xc*,0x6c*,0xe4*,0x15c*,0x160*,0x164*,0x168*} | Steam {0xc*,0x6c*,0xe4*,0x15c*,0x160*,0x164*,0x168*}
imm sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x6c*,0xe4*} | Steam {0x4*,0x1c*,0x6c*,0xe4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0041bf40: 16-byte aligned: YES
  0041bf40  55             PUSH EBP
  0041bf41  8B EC          MOV EBP,ESP
  0041bf43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0041bf40
string literals (0):
direct callees (2):
  0x00479460  GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected)
  0x004799C0  (unnamed)
struct offsets touched (top 7):
  +0xC [dec 12]  x2  via EDX
  +0x6C [dec 108]  x1  via EAX
  +0xE4 [dec 228]  x1  via EAX
  +0x15C [dec 348]  x1  via ECX
  +0x160 [dec 352]  x1  via EDX
  +0x164 [dec 356]  x1  via EAX
  +0x168 [dec 360]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0041bf40 FUN_0041bf40 size=137 ret=4 callers=0 callees=2

void __thiscall FUN_0041bf40(int param_1,undefined4 param_2)

{
  int iVar1;
  
  (**(code **)(*(int *)(param_1 + 0x6c) + 0xc))(param_2);
  iVar1 = FUN_00479460(*(undefined4 *)(param_1 + 0x15c),*(undefined4 *)(param_1 + 0x160),
                       *(undefined4 *)(param_1 + 0x164),*(undefined4 *)(param_1 + 0x168),
                       &DAT_00a1ba3c,0,0x3f800000);
  if (iVar1 != 0) {
    (**(code **)(*(int *)(param_1 + 0xe4) + 0xc))(param_2);
    FUN_004799c0();
  }
  return;
}
```

### decoy: 0x0041bda0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0041bda0: 16-byte aligned: YES
  0041bda0  55             PUSH EBP
  0041bda1  8B EC          MOV EBP,ESP
  0041bda3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0041bda0
string literals (0):
direct callees (1):
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x4 [dec 4]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x164 [dec 356]  x4  via ECX,EDX
  +0x168 [dec 360]  x4  via EAX,EDX
  +0x60 [dec 96]  x3  via ECX,EDX
  +0x64 [dec 100]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x15C [dec 348]  x2  via EAX,ECX
  +0x160 [dec 352]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 137 B, RET 4
- callees: 0x00479460 GUI fill rect (glViewport + coloured unit quad; returns 0 if rejected) (CALL), 0x004799c0 (CALL)
- indirect: 0x0041bf5d slot 0xc, 0x0041bfbc slot 0xc
- twins: none

## Card B17-15-19

GOG function: CExoSoundInternal::SetListenerPosition  gog 0x00637250  callconv=(blank)  param_size_bytes=4  gog_size=135  gog_retn=4
Candidate Steam address: 0x00707050 (size 2472, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CExoSoundInternal::OcclusionRender | G7 mac: pass(0.12) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00707a00:STR0/CALLEE0/SIG0.196/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00637160 CExoSoundInternal::SetListenerOrientation -> 0x007081e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006372e0 CExoSoundInternal::PlayOneShotSound  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080f810 Global::equal -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*,0x1c,0x28,0xa0,0xa4,0xa8,0xac,0xb0,0xb4,0xb8,0xbc,0xc0,0xc4,...(+6)}
imm sets (* = in both, 3 common): GOG {0x4*,0x8,0xc*,0x9c*} | Steam {0x2,0x4*,0xc*,0x44,0x48,0x80,0x94,0x9c*,0xc4,0x160,0x174,0x3e8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00707050: 16-byte aligned: YES
  00707050  55             PUSH EBP
  00707051  8B EC          MOV EBP,ESP
  00707053  81 EC C4 00 00 00 SUB ESP,0xc4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00707050
string literals (0):
direct callees (11):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0070F950  Source::IsPlaying
  0x00707A00  GetClosestRecentSound (occlusion cache)
  0x00521110  (unnamed)
  0x00710680  (unnamed)
  0x0058C370  (unnamed)
  0x0070DAA0  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 10):
  +0x4 [dec 4]  x21  via EAX,ECX,EDX
  +0xA8 [dec 168]  x14  via EAX,ECX,EDX
  +0xC8 [dec 200]  x13  via EAX,ECX,EDX
  +0x8 [dec 8]  x12  via EAX,ECX,EDX
  +0xCC [dec 204]  x12  via EAX,ECX,EDX
  +0xB0 [dec 176]  x7  via EAX,ECX,EDX
  +0xAC [dec 172]  x5  via EAX,ECX,EDX
  +0xBC [dec 188]  x4  via EAX,EDX
  +0xC0 [dec 192]  x4  via EAX,EDX
  +0xC4 [dec 196]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00707050 FUN_00707050 size=2472 ret=4 callers=1 callees=11

void __thiscall FUN_00707050(int param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  float10 fVar3;
  undefined8 uVar4;
  float local_74;
  int local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  int local_54;
  float local_50;
  int local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  uint local_8;
  
  local_28 = *(int *)(param_1 + 0xa8);
  local_18 = 0;
  local_2c = 1;
  uVar4 = FUN_00736220();
  local_10 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar4,1000,0);
  *(int *)(param_1 + 0xb0) = *(int *)(param_1 + 0xb0) + 1;
  local_c = local_10;
  if (*(int *)(param_1 + 0xa8) == 0) {
    uVar1 = FUN_007a1720();
    *(undefined4 *)(param_1 + 0xa8) = uVar1;
    *(undefined4 *)(param_1 + 0xac) = 1;
  }
  if (*(int *)(param_1 + 0xa8) == 0) {
    *(undefined4 *)(param_1 + 0xb0) = 0;
  }
  local_8 = *(uint *)(param_1 + 0xc8 [dec 200]);
  if (*(uint *)(param_1 + 0xb8) < local_8) {
    *(uint *)(param_1 + 0xb8) = local_8;
  }
  local_20 = 0;
  local_14 = 0;
  do {
    if (*(uint *)(param_1 + 0xc4) <= *(uint *)(param_1 + 0xc8 [dec 200])) {
LAB_00707991:
      *(undefined4 *)(param_1 + 0xc8 [dec 200]) = 0;
      if (*(uint *)(param_1 + 0xb4) < *(uint *)(param_1 + 0xb0)) {
        *(undefined4 *)(param_1 + 0xb4) = *(undefined4 *)(param_1 + 0xb0);
      }
      return;
    }
    if (*(int *)(param_1 + 0xac) == 1) {
      local_30 = *(int *)(param_1 + 0xa8);
      do {
        if ((local_30 == 0) || (*(uint *)(param_1 + 0xc4) <= *(uint *)(param_1 + 0xc8 [dec 200])))
        goto LAB_0070749d;
        *(undefined4 *)(param_1 + 0xb0) = 0;
        local_1c = FUN_00561430(local_30);
        if ((local_1c != 0) &&
           (((iVar2 = FUN_0070f950(), iVar2 != 0 && (*(int *)(local_1c + 0x1c) != 0)) &&
            (*(int *)(local_1c + 0x28) == 0)))) {
          local_14 = local_14 + 1;
          local_48 = *(undefined4 *)(local_1c + 0x80);
          local_44 = *(undefined4 *)(local_1c + 0x84);
/* ... 146 more lines truncated by --max-lines */
```

### decoy: 0x00707a00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00707a00: 16-byte aligned: YES
  00707a00  55             PUSH EBP
  00707a01  8B EC          MOV EBP,ESP
  00707a03  81 EC D8 00 00 00 SUB ESP,0xd8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00707a00
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 8):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EAX
  +0xA0 [dec 160]  x1  via ECX
  +0xA4 [dec 164]  x1  via EDX
  +0x15C [dec 348]  x1  via EAX
  +0x180 [dec 384]  x1  via ECX
  +0x184 [dec 388]  x1  via EDX
```

### Static facts (script output, report-only)
- size 2472 B, RET 4
- callers 1: 0x00705d70
- callees: 0x00736220 (CALL), 0x0091d220 (CALL), 0x007a1720 (CALL), 0x00561430 (CALL), 0x0070f950 (CALL), 0x00707a00 (CALL), ...
- globals: 0x00a1b488 READ, 0x00a1b488 READ, 0x00a1b488 READ
- twins: none
