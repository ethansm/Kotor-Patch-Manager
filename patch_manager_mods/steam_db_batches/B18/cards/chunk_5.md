# Review cards B18 chunk 5

Answer each card with one TSV row in `steam_db_batches/B18/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B18-5-01

GOG function: Global::`eh_vector_constructor_iterator'  gog 0x009258bf  callconv=__stdcall  param_size_bytes=20  gog_size=77  gog_retn=20
Candidate Steam address: 0x0091d67e (size 77, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: fail:unaligned | G3 prologue: fail:no_standard_prologue | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 48 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 48 (callers: Constructor 0x780460, LoadArea 0x7a2fb0, InitializeRepliesData 0x7c06f0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008103c0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: autolabel:`eh_vector_constructor_iterato
- GOG-order neighbour before: 0x00925530 Global::memcpy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00925936 Global::rand -> 0x0091eb8b  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0092590c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca2c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca71 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (49 of 135): the card's A-CALLEE credit may come from these, not from callees
  0x0042c9e0 CClientExoAppInternal::Constructor -> Steam 0x00780460  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004d0d80 CGuiInGame::InitializeRepliesData -> Steam 0x007c06f0  in candidate callers: yes
  0x0050c520 CSWRules::LoadClassInfo -> Steam 0x0069fee0  in candidate callers: yes
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
  0x008935A0  CSWCMessage::ReadOBJECTIDClient
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

## Card B18-5-02

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458980 (size 253, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084af50 PartLight::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b0b0 Gob::SetScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0084b0b0 Gob::SetScene -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1c,0x20,0x1a4}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0x64} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458980: 16-byte aligned: YES
  00458980  55             PUSH EBP
  00458981  8B EC          MOV EBP,ESP
  00458983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00458980
string literals (0):
direct callees (5):
  0x00476620  (unnamed)
  0x004142E0  CExoArrayList::IndexOf
  0x0049B840  (unnamed)
  0x00463B50  (unnamed)
  0x00456EA0  (unnamed)
struct offsets touched (top 3):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x1A4 [dec 420]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00458980 FUN_00458980 size=253 ret=0 callers=1 callees=5

void __fastcall FUN_00458980(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00949d08;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = PartLight::vftable;
  local_8 = 0;
  for (local_14 = 0; local_14 < DAT_00a1ffa0; local_14 = local_14 + 1) {
    iVar1 = *(int *)(*(int *)(DAT_00a1ff9c + local_14 * 4) + 0x1a4);
    if (iVar1 != 0) {
      iVar3 = FUN_00476620(param_1);
      if (iVar3 != 0) {
        iVar3 = FUN_004142e0(param_1);
        if (-1 < iVar3) {
          FUN_0049b840(iVar3);
          FUN_00463b50(iVar3);
        }
      }
    }
    if (*(undefined4 **)(iVar1 + 0x1c) == param_1) {
      *(undefined4 *)(iVar1 + 0x1c) = 0;
      *(undefined4 *)(iVar1 + 0x20) = 0;
    }
  }
  local_8 = 0xffffffff;
  FUN_00456ea0(uVar2);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00458a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 253 B, RET 0
- callers 1: 0x00458950
- callees: 0x00476620 (CALL), 0x004142e0 CExoArrayList::IndexOf (CALL), 0x0049b840 (CALL), 0x00463b50 (CALL), 0x00456ea0 (CALL)
- twins: none

## Card B18-5-03

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458ae0 (size 130, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0084af50 PartLight::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0084b0b0 Gob::SetScene  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0084b0b0 Gob::SetScene -> Steam ?  in candidate callees: n/a
  0x0093e852 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4,0x8*,0x64*} | Steam {0x8*,0x14,0x64*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458ae0: 16-byte aligned: YES
  00458ae0  55             PUSH EBP
  00458ae1  8B EC          MOV EBP,ESP
  00458ae3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00458ae0
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00458ae0 FUN_00458ae0 size=130 ret=0 callers=2 callees=1

int FUN_00458ae0(char *param_1)

{
  int iVar1;
  int iVar2;
  int local_c;
  int local_8;
  
  local_c = 0;
  local_8 = DAT_00a1ffa0 + -1;
  while( true ) {
    while( true ) {
      if (local_8 < local_c) {
        return local_c;
      }
      iVar2 = local_8 + local_c >> 1;
      iVar1 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(DAT_00a1ff9c + iVar2 * 4) + 0x64 [dec 100]),param_1);
      if (iVar1 < 1) break;
      local_8 = iVar2 + -1;
    }
    if (-1 < iVar1) break;
    local_c = iVar2 + 1;
  }
  return iVar2;
}
```

### decoy: 0x00458a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 130 B, RET 0
- callers 2: 0x00458a80, 0x00458b70
- callees: 0x009370c7 _stricmp (CALL)
- twins: none

## Card B18-5-04

GOG function: InitControl::ScaleToEdgeHelper_2  gog 0x0090deb0  callconv=(blank)  param_size_bytes=12  gog_size=79  gog_retn=0
Candidate Steam address: 0x0040f5a0 (size 117, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiPanel::StopLoadFromLayout | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040f620:STR0/CALLEE0/SIG0.042/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090de70 InitControl::ScaleToEdgeHelper  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> 0x0040fa80  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090df00 InitControl::ScaleToEdgeHelper_3 -> Steam 0x0040fa80  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x30,0x48}
imm sets (* = in both, 0 common): GOG {0x2,0x5} | Steam {0x10,0xfffffffd}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040f5a0: 16-byte aligned: YES
  0040f5a0  55             PUSH EBP
  0040f5a1  8B EC          MOV EBP,ESP
  0040f5a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040f5a0
string literals (0):
direct callees (2):
  0x007112B0  CRes::Release
  0x0040FB70  (unnamed)
struct offsets touched (top 2):
  +0x30 [dec 48]  x3  via EAX,ECX,EDX
  +0x48 [dec 72]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0040f5a0 FUN_0040f5a0 size=117 ret=0 callers=73 callees=2

void __fastcall FUN_0040f5a0(int param_1)

{
  undefined4 uVar1;
  
  if ((*(uint *)(param_1 + 0x48) >> 1 & 1) != 0) {
    FUN_007112b0();
    if (*(undefined4 **)(param_1 + 0x30) == (undefined4 *)0x0) {
      uVar1 = 0;
    }
    else {
      uVar1 = (**(code **)**(undefined4 **)(param_1 + 0x30))(1);
    }
    *(undefined4 *)(param_1 + 0x30) = 0;
    *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffffd;
    FUN_0040fb70(uVar1);
  }
  return;
}
```

### decoy: 0x0040f620 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040f620: 16-byte aligned: YES
  0040f620  55             PUSH EBP
  0040f621  8B EC          MOV EBP,ESP
  0040f623  81 EC AC 00 00 00 SUB ESP,0xac
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040f620
string literals (1):
  0x00987480  at 0040f90d  'LBL_BAR'
direct callees (9):
  0x00418DF0  CSWGuiControl::Load_2
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
  0x0040F9F0  (unnamed)
  0x0040FA80  InitControl::ScaleToEdgeHelper_3
  0x0040FB40  (unnamed)
  0x00734010  CExoString::CStr
  0x0091D6F0  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 10):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x20 [dec 32]  x2  via EAX,EDX
  +0x28 [dec 40]  x2  via EAX,EDX
  +0x6C [dec 108]  x2  via EAX
  +0x6E [dec 110]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 117 B, RET 0
- callers 73: 0x0040e860 CSWGuiPanel::Destructor, 0x00747210, 0x00754ed0, 0x00757c40 CSWGuiInGameMessages::Constructor, 0x0075ae40, ...
- callees: 0x007112b0 CRes::Release (CALL), 0x0040fb70 (CALL)
- indirect: 0x0040f5e2 slot 0x0
- twins: none
