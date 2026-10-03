# Review cards B16 chunk 1

Answer each card with one TSV row in `steam_db_batches/B16/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B16-1-01

GOG function: CAurTextureBasic::GetController  gog 0x00815aa0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00422ed0 (size 296, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTexture::Bind | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00423000:STR0/CALLEE0/SIG0.008/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00815700 CAurTextureBasic::Constructor -> 0x00423000  gate_class=AUTO
- GOG-order neighbour after:  0x00815ac0 Global::CAurTextureBasic_scalar_deleting_destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x34} | Steam {0x4,0x8,0xc,0x50,0xeb}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0xc,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00422ed0: 16-byte aligned: YES
  00422ed0  55             PUSH EBP
  00422ed1  8B EC          MOV EBP,ESP
  00422ed3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00422ed0
string literals (0):
direct callees (4):
  0x00421CF0  (unnamed)
  0x004226C0  (unnamed)
  0x00426BE0  (unnamed)
  0x00421CC0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0xEB [dec 235]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00422ed0 FUN_00422ed0 size=296 ret=0 callers=0 callees=4

void __fastcall FUN_00422ed0(int *param_1)

{
  char cVar1;
  undefined1 uVar2;
  int *piVar3;
  int iVar4;
  int *local_8;
  
  if (DAT_00a1bbdc == 0) {
    cVar1 = FUN_004226c0();
    if (cVar1 == '\0') {
      local_8 = (int *)(**(code **)(*param_1 + 0x4 [dec 4] {int*: byte +0x10}))();
      if (local_8 == (int *)0x0) {
        piVar3 = (int *)(**(code **)(*param_1 + 0x8 [dec 8] {int*: byte +0x20}))();
        local_8 = (int *)(**(code **)(*piVar3 + 0x4 [dec 4] {int*: byte +0x10}))();
      }
      if (local_8 == (int *)0x0) {
        FUN_00421cf0(0,0,0);
      }
      else {
        DAT_00a1bc78 = FUN_00426be0(local_8);
        iVar4 = (**(code **)(*param_1 + 0x50))();
        if (iVar4 == 0) {
          uVar2 = (**(code **)(*local_8 + 0xc))(0);
          FUN_00421cf0(DAT_00a1bc78,uVar2);
        }
        else {
          FUN_00421cf0(DAT_00a1bc78,0,1);
        }
        piVar3 = (int *)FUN_00421cc0(local_8);
        if ((piVar3 != (int *)0x0) && (DAT_009f4798 != 0)) {
          (**(code **)(*piVar3 + 0x4 [dec 4] {int*: byte +0x10}))();
        }
        *(undefined1 *)((int)local_8 + 0xeb) = 1;
      }
    }
    else {
      FUN_00421cf0(0,0,0);
    }
  }
  else {
    FUN_00421cf0(0,0,0);
  }
  return;
}
```

### decoy: 0x00423000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00423000: 16-byte aligned: YES
  00423000  55             PUSH EBP
  00423001  8B EC          MOV EBP,ESP
  00423003  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00423000
string literals (0):
direct callees (6):
  0x00421E30  CAurTexture::Constructor
  0x0091CFF0  (unnamed)
  0x00919723  operator new
  0x004236A0  (unnamed)
  0x00423820  (unnamed)
  0x00423860  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x18 [dec 24]  x1  via ECX
  +0x1C [dec 28]  x1  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via ECX
  +0x2C [dec 44]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B16-1-02

GOG function: CSWGlobalVariableTable::AddIdentifierString  gog 0x00651410  callconv=(blank)  param_size_bytes=4  gog_size=85  gog_retn=4
Candidate Steam address: 0x00654550 (size 85, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::AddString | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00654490:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006513b0 CSWGlobalVariableTable::AddIdentifierLocation -> 0x006544f0  gate_class=AUTO
- GOG-order neighbour after:  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> 0x006544f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00651190 CSWGlobalVariableTable::AddIdentifier -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xeba8*} | Steam {0xeba8*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x5*,0x8*} | Steam {0x3*,0x4*,0x5*,0x8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00654550: 16-byte aligned: YES
  00654550  55             PUSH EBP
  00654551  8B EC          MOV EBP,ESP
  00654553  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654550
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA8 [dec 60328]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00654550 FUN_00654550 size=85 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_00654550(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0xeba8) < 5) {
    local_8 = FUN_006542d0(param_2,3,*(undefined4 *)(param_1 + 0xeba8));
    *(int *)(param_1 + 0xeba8) = *(int *)(param_1 + 0xeba8) + 1;
  }
  return local_8;
}
```

### decoy: 0x00654490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654490: 16-byte aligned: YES
  00654490  55             PUSH EBP
  00654491  8B EC          MOV EBP,ESP
  00654493  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00654490
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA0 [dec 60320]  x4  via EAX,ECX
```

## Card B16-1-03

GOG function: CSWGuiInGameGameplay::OnReverse  gog 0x005e5870  callconv=(blank)  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x008e0dc0 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0d40:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e57f0 CSWGuiInGameGameplay::OnInvertCam -> 0x008e0d40  gate_class=REVIEW
- GOG-order neighbour after:  0x005e58f0 CSWGuiInGameGameplay::OnReverseIngame -> 0x008e0e40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x1c*,0xfffffff7*} | Steam {0x3*,0x4*,0x1c*,0xfffffff7*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0dc0: 16-byte aligned: YES
  008e0dc0  55             PUSH EBP
  008e0dc1  8B EC          MOV EBP,ESP
  008e0dc3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0dc0
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0dc0 FUN_008e0dc0 size=122 ret=4 callers=0 callees=1

void __thiscall FUN_008e0dc0(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar2 = FUN_0072fb00(param_1);
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = FUN_0072fb00();
    *(uint *)(iVar2 + 8) = *(uint *)(iVar2 + 8) & 0xfffffff7 | (uint)((uVar1 >> 3 & 1) == 0) << 3;
  }
  return;
}
```

### decoy: 0x008e0d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0d40: 16-byte aligned: YES
  008e0d40  55             PUSH EBP
  008e0d41  8B EC          MOV EBP,ESP
  008e0d43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0d40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-04

GOG function: CSWGuiInGameGameplay::SetUpOptions  gog 0x005e60d0  callconv=(blank)  param_size_bytes=0  gog_size=491  gog_retn=0
Candidate Steam address: 0x008e1620 (size 491, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.20) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e1280:STR0/CALLEE0/SIG0.063/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e6080 CSWGuiInGameGameplay::OnExitDiff -> 0x008e15d0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e62c0 CSWGuiInGameAutoPause::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x4*,0x8*,0x5c*,0x1d4*,0x1298*,0x1560*,0x1828*,0x1af0*,0x1db8*,0x2080*,0x2b74*} | Steam {0x4*,0x8*,0x5c*,0x1d4*,0x1298*,0x1560*,0x1828*,0x1af0*,0x1db8*,0x2080*,0x2b74*}
imm sets (* = in both, 12 common): GOG {0x2*,0x3*,0x4*,0x5*,0x40*,0x1298*,0x1560*,0x1828*,0x1af0*,0x1db8*,0x2080*,0xfffffffe*} | Steam {0x2*,0x3*,0x4*,0x5*,0x40*,0x1298*,0x1560*,0x1828*,0x1af0*,0x1db8*,0x2080*,0xfffffffe*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008e1620: 16-byte aligned: YES
  008e1620  55             PUSH EBP
  008e1621  8B EC          MOV EBP,ESP
  008e1623  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e1620
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
struct offsets touched (top 10):
  +0x1D4 [dec 468]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x5C [dec 92]  x6  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x1298 [dec 4760]  x1  via EDX
  +0x1560 [dec 5472]  x1  via EAX
  +0x1828 [dec 6184]  x1  via EDX
  +0x1AF0 [dec 6896]  x1  via EAX
  +0x1DB8 [dec 7608]  x1  via EDX
  +0x2080 [dec 8320]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1620 FUN_008e1620 size=491 ret=0 callers=2 callees=2

void __fastcall FUN_008e1620(int param_1)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  
  iVar2 = FUN_0072fb00();
  if (iVar2 != 0) {
    *(uint *)(param_1 + 0x2b74) = (uint)*(byte *)(iVar2 + 4);
    FUN_008e1280();
    uVar1 = *(uint *)(iVar2 + 8);
    iVar3 = (**(code **)(*(int *)(param_1 + 0x1298) + 0x5c))();
    *(uint *)(iVar3 + 0x1d4) = *(uint *)(iVar3 + 0x1d4) & 0xfffffffe | uVar1 & 1;
    uVar1 = *(uint *)(iVar2 + 8);
    iVar3 = (**(code **)(*(int *)(param_1 + 0x1560) + 0x5c))();
    *(uint *)(iVar3 + 0x1d4) = *(uint *)(iVar3 + 0x1d4) & 0xfffffffe | uVar1 >> 1 & 1;
    uVar1 = *(uint *)(iVar2 + 8);
    iVar3 = (**(code **)(*(int *)(param_1 + 0x1828) + 0x5c))();
    *(uint *)(iVar3 + 0x1d4) = *(uint *)(iVar3 + 0x1d4) & 0xfffffffe | uVar1 >> 2 & 1;
    uVar1 = *(uint *)(iVar2 + 8);
    iVar3 = (**(code **)(*(int *)(param_1 + 0x1af0) + 0x5c))();
    *(uint *)(iVar3 + 0x1d4) = *(uint *)(iVar3 + 0x1d4) & 0xfffffffe | uVar1 >> 3 & 1;
    uVar1 = *(uint *)(iVar2 + 8);
    iVar3 = (**(code **)(*(int *)(param_1 + 0x1db8) + 0x5c))();
    *(uint *)(iVar3 + 0x1d4) = *(uint *)(iVar3 + 0x1d4) & 0xfffffffe | uVar1 >> 4 & 1;
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = (**(code **)(*(int *)(param_1 + 0x2080) + 0x5c))();
    *(uint *)(iVar2 + 0x1d4) = *(uint *)(iVar2 + 0x1d4) & 0xfffffffe | uVar1 >> 5 & 1;
  }
  return;
}
```

### decoy: 0x008e1280 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e1280: 16-byte aligned: YES
  008e1280  55             PUSH EBP
  008e1281  8B EC          MOV EBP,ESP
  008e1283  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008e1280
string literals (0):
direct callees (4):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00416EA0  (unnamed)
struct offsets touched (top 5):
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0x2B74 [dec 11124]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x2  via EAX,EDX
  +0x130 [dec 304]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via EDX
```

## Card B16-1-05

GOG function: CSWGuiInGameGameplay::OnInvertCam  gog 0x005e57f0  callconv=(blank)  param_size_bytes=4  gog_size=120  gog_retn=4
Candidate Steam address: 0x008e0d40 (size 120, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0e40:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5770 CSWGuiInGameGameplay::OnAutoSave -> 0x008e0cc0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5870 CSWGuiInGameGameplay::OnReverse -> 0x008e0dc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x1c*,0xfffffffd*} | Steam {0x4*,0x1c*,0xfffffffd*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0d40: 16-byte aligned: YES
  008e0d40  55             PUSH EBP
  008e0d41  8B EC          MOV EBP,ESP
  008e0d43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0d40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0d40 FUN_008e0d40 size=120 ret=4 callers=0 callees=1

void __thiscall FUN_008e0d40(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar2 = FUN_0072fb00(param_1);
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = FUN_0072fb00();
    *(uint *)(iVar2 + 8) = *(uint *)(iVar2 + 8) & 0xfffffffd | (uint)((uVar1 >> 1 & 1) == 0) << 1;
  }
  return;
}
```

### decoy: 0x008e0e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0e40: 16-byte aligned: YES
  008e0e40  55             PUSH EBP
  008e0e41  8B EC          MOV EBP,ESP
  008e0e43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0e40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-06

GOG function: CSWGlobalVariableTable::AddIdentifierNumber  gog 0x00651350  callconv=(blank)  param_size_bytes=4  gog_size=88  gog_retn=4
Candidate Steam address: 0x00654490 (size 88, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::AddNumber | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00654430:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006512f0 CSWGlobalVariableTable::AddIdentifierBoolean -> 0x00654430  gate_class=REVIEW
- GOG-order neighbour after:  0x006513b0 CSWGlobalVariableTable::AddIdentifierLocation -> 0x006544f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00651190 CSWGlobalVariableTable::AddIdentifier -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xeba0*} | Steam {0xeba0*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x3e8*} | Steam {0x4*,0x8*,0x3e8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00654490: 16-byte aligned: YES
  00654490  55             PUSH EBP
  00654491  8B EC          MOV EBP,ESP
  00654493  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654490
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA0 [dec 60320]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00654490 FUN_00654490 size=88 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_00654490(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0xeba0) < 1000) {
    local_8 = FUN_006542d0(param_2,1,*(undefined4 *)(param_1 + 0xeba0));
    *(int *)(param_1 + 0xeba0) = *(int *)(param_1 + 0xeba0) + 1;
  }
  return local_8;
}
```

### decoy: 0x00654430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654430: 16-byte aligned: YES
  00654430  55             PUSH EBP
  00654431  8B EC          MOV EBP,ESP
  00654433  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00654430
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEB9C [dec 60316]  x4  via EAX,ECX
```

## Card B16-1-07

GOG function: CSWGlobalVariableTable::ReadTableWithoutCatalogue  gog 0x00652550  callconv=(blank)  param_size_bytes=8  gog_size=1110  gog_retn=8
Candidate Steam address: 0x00655010 (size 1644, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.61) | G8 mac_rotation: pass
- anchors: A-STR 5 | A-CALLEE 0 | A-SIG 0.76 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00655690:STR5/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651ed0 CSWGlobalVariableTable::WriteTable -> 0x00655010  gate_class=AUTO
- GOG-order neighbour after:  0x006529c0 CSWGlobalVariableTable::ReadTableWithCatalogue -> 0x00655b00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: no
  0x00624970 CResGFF::GetListCount -> Steam 0x00718810  in candidate callees: no
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: no
  0x006250f0 CResGFF::ReadFieldCExoString -> Steam 0x00718f90  in candidate callees: no
  0x006257a0 CResGFF::ReadFieldVOID -> Steam 0x00719640  in candidate callees: no
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): 'String'; 'ValString'; 'ValLocation'; 'ValNumber' ...
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*,0xeb9c,0xeba0}
imm sets (* = in both, 14 common): GOG {0x2*,0x3*,0x5*,0x8*,0xc*,0xe*,0x18*,0x64*,0x71*,0x3e8*,0x93d*,0x960*,0xe4c,0x3fff*,...(+1)} | Steam {0x2*,0x3*,0x4,0x5*,0x6,0x7,0x8*,0xc*,0xe*,0x18*,0x64*,0x71*,0xa0,0x3e8*,...(+5)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00655010: 16-byte aligned: YES
  00655010  55             PUSH EBP
  00655011  8B EC          MOV EBP,ESP
  00655013  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00655010
string literals (10):
  0x00998684  at 006551d8  'CatBoolean'
  0x00998678  at 006551f0  'CatNumber'
  0x0099866C  at 00655205  'CatLocation'
  0x00998660  at 0065521a  'CatString'
  0x00998654  at 006554c6  'ValBoolean'
  0x00998648  at 006554f7  'ValNumber'
  0x0099863C  at 0065551c  'ValLocation'
  0x00998630  at 00655539  'ValString'
  0x00998628  at 00655580  'String'
  0x00996EF4  at 006555fa  'REBOOTDATA:'
direct callees (22):
  0x0091D67E  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00734030  CExoString::Find
  0x00733780  CExoString::~CExoString
  0x00734500  CExoString::Left
  0x007337C0  CExoString::operator=
  0x00919723  operator new
  0x00716E60  CResGFF::Constructor
  0x0071A3D0  CResGFF::CreateGFFFile
  0x00719740  CResGFF::AddList
  0x0091C630  (unnamed)
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00719800  CResGFF::AddListElement
  0x00719E50  CResGFF::WriteFieldCExoString
  0x0071A1B0  CResGFF::WriteFieldVOID
  0x0071A5A0  CResGFF::WriteGFFFile
  0x00733A60  CExoString::operator==_2
  0x00716D80  CExoResMan::FreeResourceData_2
struct offsets touched (top 7):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via EAX,EDX
  +0xEB9C [dec 60316]  x1  via EDX
  +0xEBA0 [dec 60320]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00655010 FUN_00655010 size=1644 ret=8 callers=1 callees=22

void __thiscall FUN_00655010(int param_1,undefined4 param_2,int param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  undefined4 uVar3;
  int iVar4;
  undefined4 *local_ee0;
  undefined1 local_ec8 [8];
  undefined1 local_ec0 [8];
  undefined1 local_eb8 [8];
  int local_eb0;
  undefined4 *local_eac;
  undefined1 local_ea8 [8];
  undefined1 local_ea0 [8];
  undefined1 local_e98 [124];
  int local_e1c;
  undefined1 local_e18 [4];
  int local_e14;
  undefined4 local_e10[600 = byte +0x960];
  undefined1 local_4b0 [20];
  int local_49c;
  undefined1 local_498 [40];
  undefined1 local_470 [20];
  undefined1 *local_45c;
  undefined1 local_458 [1000];
  undefined1 local_70 [12];
  undefined4 *local_64;
  undefined1 local_60 [20];
  int local_4c;
  undefined1 local_48 [4];
  int local_44;
  int local_40;
  undefined1 local_3c [20];
  undefined1 local_28 [20];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958e99;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = uVar2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_e10,0x18,100,FUN_0053e060,FUN_00623320);
  local_8 = 0;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_498,8,5,FUN_00733540,FUN_00733780);
  local_8._0_1_ = 1;
  FUN_00733540(uVar2);
  local_8._0_1_ = 2;
  FUN_00733570(&DAT_009975d4);
  local_8._0_1_ = 3;
  local_e1c = FUN_00734030(local_ea0,0);
  local_8._0_1_ = 2;
  FUN_00733780();
  if (local_e1c != -1) {
    uVar3 = FUN_00734500(local_ea8,local_e1c + 1);
    local_8._0_1_ = 4;
    FUN_007337c0(uVar3);
    local_8._0_1_ = 2;
    FUN_00733780();
  }
  local_eb0 = FUN_00919723(0xa0);
  local_8._0_1_ = 5;
  if (local_eb0 == 0) {
    local_ee0 = (undefined4 *)0x0;
  }
  else {
    local_ee0 = (undefined4 *)FUN_00716e60();
  }
  local_eac = local_ee0;
  local_8._0_1_ = 2;
  local_64 = local_ee0;
  FUN_00733570(&DAT_009938e4);
  local_8._0_1_ = 6;
  FUN_00733570(&DAT_00998690);
  local_8._0_1_ = 7;
/* ... 95 more lines truncated by --max-lines */
```

### decoy: 0x00655690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00655690: 16-byte aligned: YES
  00655690  55             PUSH EBP
  00655691  8B EC          MOV EBP,ESP
  00655693  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00655690
string literals (5):
  0x00998654  at 00655736  'ValBoolean'
  0x00998648  at 00655758  'ValNumber'
  0x0099863C  at 0065577d  'ValLocation'
  0x00998630  at 006557a4  'ValString'
  0x00998628  at 00655853  'String'
direct callees (14):
  0x0091D67E  (unnamed)
  0x0091C630  (unnamed)
  0x00719640  CResGFF::ReadFieldVOID
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
```

## Card B16-1-08

GOG function: CExoMoviePlayerInternal::CancelMovie  gog 0x0091fe20  callconv=(blank)  param_size_bytes=8  gog_size=59  gog_retn=8
Candidate Steam address: 0x0040c3b0 (size 108, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040c3a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fdf0 CExoMoviePlayerInternal::RequestRestart -> 0x0040cc00  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fe60 CExoMoviePlayerInternal::StartMovie -> 0x0040cc70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x14,0x18,0x30,0x9c} | Steam {0x4,0xc,0x10}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x2,0x4,0x8*,0xd4d}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0040c3b0: 16-byte aligned: YES
  0040c3b0  55             PUSH EBP
  0040c3b1  8B EC          MOV EBP,ESP
  0040c3b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040c3b0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040c3b0 FUN_0040c3b0 size=108 ret=8 callers=1 callees=2

/* WARNING: Removing unreachable block (ram,0x0040c3e2) */

void __thiscall FUN_0040c3b0(int param_1,int param_2,int param_3)

{
  if ((param_2 != 0) && (param_3 != 0)) {
    if ((*(byte *)(param_1 + 4) & 1) != 0) {
      SteamAPI_UnregisterCallback(param_1);
    }
    *(int *)(param_1 + 0xc) = param_2;
    *(int *)(param_1 + 0x10) = param_3;
    SteamAPI_RegisterCallback(param_1,0xd4d);
  }
  return;
}
```

### decoy: 0x0040c3a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c3a0: 16-byte aligned: YES
  0040c3a0  55             PUSH EBP
  0040c3a1  8B EC          MOV EBP,ESP
  0040c3a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040c3a0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B16-1-09

GOG function: CSWGuiTextParams::SetDefaultColor_2  gog 0x00915550  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00416e10 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00416de0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915510 CSWGuiTextParams::SetDefaultColor -> 0x00417140  gate_class=AUTO
- GOG-order neighbour after:  0x00915570 CSWGuiText::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00918530 CSWGuiButton::SetActive -> Steam 0x0041a290  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2c} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00416e10: 16-byte aligned: YES
  00416e10  55             PUSH EBP
  00416e11  8B EC          MOV EBP,ESP
  00416e13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00416e10
string literals (0):
direct callees (1):
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00416e10 FUN_00416e10 size=19 ret=0 callers=9 callees=1
THUNK -> 0x00416D60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00416e10(undefined4 param_1)

{
  FUN_00416d60(param_1);
  return;
}
```

### decoy: 0x00416de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00416de0: 16-byte aligned: YES
  00416de0  55             PUSH EBP
  00416de1  8B EC          MOV EBP,ESP
  00416de3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00416de0
string literals (0):
direct callees (2):
  0x00710980  (unnamed)
  0x00416D60  CSWGuiTextParams::SetFontRename
struct offsets touched (top 0):
```

## Card B16-1-10

GOG function: CSWGuiInGameGameplay::Destructor  gog 0x005e5210  callconv=(blank)  param_size_bytes=4  gog_size=46  gog_retn=4
Candidate Steam address: 0x008e0780 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008df930:STR0/CALLEE0/SIG0.013/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e43c0 CSWGuiInGameGameplay::Constructor -> 0x008df930  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5240 CSWGuiInGameGameplay::Destructor_2 -> 0x008e07b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005e5240 CSWGuiInGameGameplay::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x20*} | Steam {0x4*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0780: 16-byte aligned: YES
  008e0780  55             PUSH EBP
  008e0781  8B EC          MOV EBP,ESP
  008e0783  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0780
string literals (0):
direct callees (2):
  0x008E07B0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008e0780 FUN_008e0780 size=46 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008e0780(undefined4 param_1,uint param_2)

{
  FUN_008e07b0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x008df930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008df930: 16-byte aligned: YES
  008df930  55             PUSH EBP
  008df931  8B EC          MOV EBP,ESP
  008df933  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008df930
string literals (21):
  0x009A8ECC  at 008dfbac  'optgameplay_p'
  0x009A8EBC  at 008dfbcc  'BTN_DIFFICULTY'
  0x009A8EB0  at 008dfc09  'CB_LEVELUP'
  0x009A8EA4  at 008dfc46  'CB_AUTOSAVE'
  0x009A8E98  at 008dfc82  'CB_REVERSE'
  0x009A8E84  at 008dfcbf  'CB_REVERSE_INGAME'
  0x009A8E74  at 008dfcfc  'CB_DISABLEMOVE'
  0x009A1818  at 008dfd38  'LBL_TITLE'
  0x009A1810  at 008dfd75  'LB_DESC'
  0x009A8E64  at 008dfdb2  'CB_INVERTCAM'
  0x009A8E54  at 008dfdee  'BTN_DIFFRIGHT'
  0x009A8E44  at 008dfe34  'BTN_DIFFLEFT'
direct callees (20):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00419740  (unnamed)
  0x0041CE90  (unnamed)
  0x0041A510  (unnamed)
  0x0041A330  (unnamed)
  0x0041AB60  (unnamed)
  0x0091D67E  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007D2360  (unnamed)
  0x00479AB0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00418AF0  CSWGuiControl::AddEvent
  0x00760EA0  (unnamed)
  0x0040EC00  CSWGuiPanel::SetActiveControl
  0x008E1620  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x12  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
  +0x5C9 [dec 1481]  x1  via ECX
  +0x5D4 [dec 1492]  x1  via EAX
  +0x9E4 [dec 2532]  x1  via EAX
  +0xBB4 [dec 2996]  x1  via EDX
```

## Card B16-1-11

GOG function: CSWGuiInGameGameplay::Constructor  gog 0x005e43c0  callconv=(blank)  param_size_bytes=4  gog_size=3659  gog_retn=4
Candidate Steam address: 0x008df930 (size 3659, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiOptionsGameplay | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 14 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'LBL_MOUSESEN'@009a1ecc; 'SLI_MOUSESEN'@009a1edc; 'BTN_MOUSE'@009a8e2c; CALLEE 1 (callees: SetActiveControl 0x40ec00); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008df5b0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e4360 CPazaakPlayer::GetNumCardsPlayed  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005e5210 CSWGuiInGameGameplay::Destructor -> 0x008e0780  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (20), mapped to Steam through trusted rows only:
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x005324a0 Global::AspyrAddButtonIcon -> Steam ?  in candidate callees: n/a
  0x00567cf0 Global::ExpandButtonToFitText -> Steam ?  in candidate callees: n/a
  0x005e60d0 CSWGuiInGameGameplay::SetUpOptions -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x008d03c0 Global::GetResolutionTextScalar -> Steam ?  in candidate callees: n/a
  ... 12 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (22): 'LBL_BAR4'; 'LBL_BAR3'; 'LBL_BAR2'; 'LBL_BAR1' ...
disp sets (* = in both, 16 common): GOG {0x4*,0x8*,0xc*,0x48*,0x68*,0x98*,0x5c9*,0x5d4*,0x9e4*,0xbb4*,0x12f4*,0x15bc*,0x1884*,0x1b4c*,...(+2)} | Steam {0x4*,0x8*,0xc*,0x48*,0x68*,0x98*,0x5c9*,0x5d4*,0x9e4*,0xbb4*,0x12f4*,0x15bc*,0x1884*,0x1b4c*,...(+2)}
imm sets (* = in both, 79 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+65)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+65)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008df930: 16-byte aligned: YES
  008df930  55             PUSH EBP
  008df931  8B EC          MOV EBP,ESP
  008df933  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008df930
string literals (21):
  0x009A8ECC  at 008dfbac  'optgameplay_p'
  0x009A8EBC  at 008dfbcc  'BTN_DIFFICULTY'
  0x009A8EB0  at 008dfc09  'CB_LEVELUP'
  0x009A8EA4  at 008dfc46  'CB_AUTOSAVE'
  0x009A8E98  at 008dfc82  'CB_REVERSE'
  0x009A8E84  at 008dfcbf  'CB_REVERSE_INGAME'
  0x009A8E74  at 008dfcfc  'CB_DISABLEMOVE'
  0x009A1818  at 008dfd38  'LBL_TITLE'
  0x009A1810  at 008dfd75  'LB_DESC'
  0x009A8E64  at 008dfdb2  'CB_INVERTCAM'
  0x009A8E54  at 008dfdee  'BTN_DIFFRIGHT'
  0x009A8E44  at 008dfe34  'BTN_DIFFLEFT'
direct callees (20):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00419740  (unnamed)
  0x0041CE90  (unnamed)
  0x0041A510  (unnamed)
  0x0041A330  (unnamed)
  0x0041AB60  (unnamed)
  0x0091D67E  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007D2360  (unnamed)
  0x00479AB0  (unnamed)
  0x0091C860  _ftol2_sse
  0x00418AF0  CSWGuiControl::AddEvent
  0x00760EA0  (unnamed)
  0x0040EC00  CSWGuiPanel::SetActiveControl
  0x008E1620  (unnamed)
struct offsets touched (top 10):
  +0x48 [dec 72]  x12  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
  +0x5C9 [dec 1481]  x1  via ECX
  +0x5D4 [dec 1492]  x1  via EAX
  +0x9E4 [dec 2532]  x1  via EAX
  +0xBB4 [dec 2996]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008df930 FUN_008df930 size=3659 ret=4 callers=1 callees=20

undefined4 * __thiscall FUN_008df930(undefined4 *param_1,undefined4 param_2)

{
  uint uVar1;
  float10 fVar2;
  undefined1 local_e0 [8];
  undefined1 local_d8 [8];
  undefined1 local_d0 [8];
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  undefined1 local_b8 [8];
  undefined1 local_b0 [8];
  undefined1 local_a8 [8];
  undefined1 local_a0 [8];
  undefined1 local_98 [8];
  undefined1 local_90 [8];
  undefined1 local_88 [8];
  undefined1 local_80 [8];
  undefined1 local_78 [8];
  undefined1 local_70 [8];
  undefined1 local_68 [8];
  undefined1 local_60 [8];
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [16];
  int local_28;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097d894;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040e5a0(param_2);
  local_8 = 0;
  *param_1 = CSWGuiInGameGameplay::vftable;
  FUN_00419740(uVar1);
  local_8._0_1_ = 1;
  FUN_00419740();
  local_8._0_1_ = 2;
  FUN_0041ce90();
  local_8._0_1_ = 3;
  FUN_0041a510();
  local_8._0_1_ = 4;
  FUN_0041a510();
  local_8._0_1_ = 5;
  FUN_0041a510();
  local_8._0_1_ = 6;
  FUN_0041a510();
  local_8._0_1_ = 7;
  FUN_0041a510();
  local_8._0_1_ = 8;
  FUN_0041a510();
  local_8._0_1_ = 9;
  FUN_0041a510();
  local_8._0_1_ = 10;
  FUN_0041a330();
  param_1[0x4a6 = byte +0x1298] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 0xb;
  FUN_0041a330();
  param_1[0x558 = byte +0x1560] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 0xc;
  FUN_0041a330();
  param_1[0x60a = byte +0x1828] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 0xd;
  FUN_0041a330();
  param_1[0x6bc = byte +0x1af0] = CSWGuiOptionsCheckbox::vftable;
  local_8._0_1_ = 0xe;
  FUN_0041a330();
  param_1[0x76e = byte +0x1db8] = CSWGuiOptionsCheckbox::vftable;
/* ... 182 more lines truncated by --max-lines */
```

### decoy: 0x008df5b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008df5b0: 16-byte aligned: YES
  008df5b0  55             PUSH EBP
  008df5b1  8B EC          MOV EBP,ESP
  008df5b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008df5b0
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x0073F750  ClientApp::GetGuiInGame
  0x007CDF10  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0x1C [dec 28]  x1  via ECX
  +0x4C [dec 76]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x58 [dec 88]  x1  via ECX
```

## Card B16-1-12

GOG function: CSWGuiInGameGameplay::OnDifficultyRight  gog 0x005e5c70  callconv=(blank)  param_size_bytes=4  gog_size=181  gog_retn=4
Candidate Steam address: 0x008e11c0 (size 181, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e1140:STR0/CALLEE0/SIG0.411/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5bf0 CSWGuiInGameGameplay::OnDifficultyLeft -> 0x008e1140  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> 0x008e1280  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x1c*,0x24*,0x50*,0x80*,0x130*,0x2b74*} | Steam {0x4*,0x1c*,0x24*,0x50*,0x80*,0x130*,0x2b74*}
imm sets (* = in both, 3 common): GOG {0x2*,0x4*,0x24*} | Steam {0x2*,0x4*,0x24*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e11c0: 16-byte aligned: YES
  008e11c0  55             PUSH EBP
  008e11c1  8B EC          MOV EBP,ESP
  008e11c3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e11c0
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 7):
  +0x2B74 [dec 11124]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x80 [dec 128]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e11c0 FUN_008e11c0 size=181 ret=4 callers=0 callees=3

void __thiscall FUN_008e11c0(int param_1,int param_2)

{
  undefined1 uVar1;
  int iVar2;
  
  if ((*(int *)(param_2 + 0x50) != 0) &&
     (*(int *)(param_1 + 0x2b74) !=
      *(int *)(*(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x80) + 0x24) + -2)) {
    *(int *)(param_1 + 0x2b74) = *(int *)(param_1 + 0x2b74) + 1;
    uVar1 = *(undefined1 *)(param_1 + 0x2b74);
    iVar2 = FUN_0072fb00();
    *(undefined1 *)(iVar2 + 4) = uVar1;
    FUN_008e1280();
    FUN_004122a0(1);
  }
  return;
}
```

### decoy: 0x008e1140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e1140: 16-byte aligned: YES
  008e1140  55             PUSH EBP
  008e1141  8B EC          MOV EBP,ESP
  008e1143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e1140
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 4):
  +0x2B74 [dec 11124]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-13

GOG function: CSWGuiInGameCharacter::SetModText  gog 0x00599070  callconv=(blank)  param_size_bytes=8  gog_size=360  gog_retn=8
Candidate Steam address: 0x0084f770 (size 360, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0084f900:STR0/CALLEE0/SIG0.038/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00597ff0 CSWGuiInGameCharacter::SetStats -> 0x0084e6f0  gate_class=AUTO
- GOG-order neighbour after:  0x005991e0 CSWGuiInGameCharacter::OnPanelRemoved -> 0x0084f8e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605810 CExoString::Constructor_5 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00915200 CSWGuiTextParams::SetText -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 10 common): GOG {0x2*,0x3*,0x4*,0x8*,0x48*,0xf0*,0xfffffff7*,0xfffffffb*,0xfffffffd*,0xfffffffe*} | Steam {0x2*,0x3*,0x4*,0x8*,0x48*,0xf0*,0xfffffff7*,0xfffffffb*,0xfffffffd*,0xfffffffe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0084f770: 16-byte aligned: YES
  0084f770  55             PUSH EBP
  0084f771  8B EC          MOV EBP,ESP
  0084f773  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0084f770
string literals (0):
direct callees (5):
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0084f770 FUN_0084f770 size=360 ret=8 callers=1 callees=5

void FUN_0084f770(undefined4 param_1,int param_2)

{
  bool bVar1;
  uint uVar2;
  undefined4 local_50;
  undefined1 local_2c [8];
  undefined1 local_24 [16];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00971e24;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  bVar1 = param_2 < 1;
  if (bVar1) {
    local_50 = FUN_00733700(param_2);
    local_8 = 3;
  }
  else {
    FUN_00733700(param_2);
    local_8 = 0;
    FUN_00733570(&DAT_00997644);
    local_8 = 1;
    local_50 = FUN_00733df0(local_2c,local_24);
    local_8 = 2;
  }
  local_14 = local_50;
  FUN_00416e30(local_50);
  local_8 = 2;
  if (bVar1) {
    FUN_00733780(uVar2);
  }
  local_8 = 1;
  if (!bVar1) {
    FUN_00733780(uVar2);
  }
  local_8 = 0;
  if (!bVar1) {
    FUN_00733780();
  }
  local_8 = 0xffffffff;
  if (!bVar1) {
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0084f900 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084f900: 16-byte aligned: YES
  0084f900  55             PUSH EBP
  0084f901  8B EC          MOV EBP,ESP
  0084f903  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084f900
string literals (0):
direct callees (13):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0073F750  ClientApp::GetGuiInGame
  0x00740FC0  CClientExoApp::GetCharacterChangeInProgress
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0084FCD0  CSWGuiInGameCharacter::ShowLevelUpGUI thiscall(panel, int) RET 4; callers 0x84F9BD, 0x7CB8E6
  0x0084FE20  CSWGuiInGameCharacter::StartAutoLevelup
  0x00740B80  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x007CA060  CGuiInGame::HideSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
  0x008500F0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 4):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x12C [dec 300]  x2  via ECX,EDX
  +0x84FAF4 [dec 8714996]  x1  via EAX
```

## Card B16-1-14

GOG function: CSWGuiInGameCharacter::RotateCharacter  gog 0x005999f0  callconv=(blank)  param_size_bytes=4  gog_size=566  gog_retn=4
Candidate Steam address: 0x008500f0 (size 566, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00850330:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599990 CSWGuiInGameCharacter::SetLabelBonusPenaltyColour -> 0x00850090  gate_class=REVIEW
- GOG-order neighbour after:  0x00599c30 CSWGuiInGameCharacter::RotateCharacterLeft -> 0x008500f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x88*,0x4b6c*} | Steam {0x4*,0x8*,0xc*,0x88*,0x4b6c*}
imm sets (* = in both, 5 common): GOG {0x4*,0x10*,0x30*,0x44*,0x60*} | Steam {0x4*,0x10*,0x30*,0x44*,0x60*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008500f0: 16-byte aligned: YES
  008500f0  55             PUSH EBP
  008500f1  8B EC          MOV EBP,ESP
  008500f3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008500f0
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008500f0 FUN_008500f0 size=566 ret=4 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_008500f0(int param_1,int param_2)

{
  int iVar1;
  float *pfVar2;
  float *pfVar3;
  undefined4 local_64;
  float *local_5c;
  float local_20;
  float local_1c;
  float local_18;
  undefined1 local_14 [16];
  
  if (param_2 == 0) {
    local_64 = _DAT_00987704;
  }
  else {
    local_64 = _DAT_0098bd88;
  }
  iVar1 = *(int *)(param_1 + 0x4b6c);
  pfVar3 = (float *)(iVar1 + 0x30);
  pfVar2 = (float *)FUN_00515620(local_14,local_64,0,0);
  if ((DAT_00a0f190 == 0) ||
     ((*pfVar2 != (float)_DAT_00986b50 && (*pfVar2 != (float)_DAT_00986d98)))) {
    local_20 = (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490) *
               *pfVar3 + (pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8] - pfVar2[3 = byte +0xc] * *pfVar2) * (float)_DAT_00987490 *
                         *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_1c = (pfVar2[3 = byte +0xc] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490 * *pfVar3 +
               (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc] - pfVar2[1 = byte +0x4] * *pfVar2) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_18 = (pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc] - pfVar2[2 = byte +0x8] * *pfVar2) * (float)_DAT_00987490 * *pfVar3 +
               (pfVar2[1 = byte +0x4] * *pfVar2 + pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x34) +
               (1.0 - (pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x38);
  }
  else {
    local_20 = *pfVar3;
    local_1c = *(float *)(iVar1 + 0x34);
    local_18 = *(float *)(iVar1 + 0x38);
  }
  local_5c = &local_20;
  (**(code **)(**(int **)(param_1 + 0x4b6c) + 0x88))(local_5c);
  return;
}
```

### decoy: 0x00850330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00850330: 16-byte aligned: YES
  00850330  55             PUSH EBP
  00850331  8B EC          MOV EBP,ESP
  00850333  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00850330
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

## Card B16-1-15

GOG function: CSWGuiInGameGameplay::Destructor_2  gog 0x005e5240  callconv=(blank)  param_size_bytes=0  gog_size=708  gog_retn=0
Candidate Steam address: 0x008e07b0 (size 708, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008e0a80:STR0/CALLEE0/SIG0.059/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5210 CSWGuiInGameGameplay::Destructor -> 0x008e0780  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5510 CSWGuiInGameGameplay::HandleInputEvent -> 0x008e0a80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x004ce500 CSWGuiControl::Destructor -> Steam 0x00418640  in candidate callees: yes
  0x004ce630 CSWGuiButton::Destructor -> Steam 0x0041a3d0  in candidate callees: yes
  0x0056ab00 CSWGuiSlider::Destructor -> Steam ?  in candidate callees: n/a
  0x0056ab90 CSWGuiOptionsGraphics::OnExitSlider -> Steam ?  in candidate callees: n/a
  0x005977a0 CSWGuiBlackenedLabel::Destructor -> Steam ?  in candidate callees: n/a
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
  0x00913030 CSWGuiBorder::Destructor_2 -> Steam 0x00414c60  in candidate callees: yes
  0x00915660 CSWGuiText::Destructor_2 -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 52 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+38)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xb*,0xc*,0xd*,0xe*,0xf*,...(+38)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008e07b0: 16-byte aligned: YES
  008e07b0  55             PUSH EBP
  008e07b1  8B EC          MOV EBP,ESP
  008e07b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e07b0
string literals (0):
direct callees (10):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0041AC10  (unnamed)
  0x007D5230  (unnamed)
  0x0041A3D0  CSWGuiButton::Destructor
  0x0041D190  (unnamed)
  0x0074AC30  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008e07b0 FUN_008e07b0 size=708 ret=0 callers=1 callees=10

void __fastcall FUN_008e07b0(int param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0097da29;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 0x12;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
            ((void *)(param_1 + 0x2654),0x148,4,
             CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton);
  local_8._1_3_ = (uint3)((uint)local_8 >> 8);
  *(undefined ***)(param_1 + 0x250c) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x14;
  FUN_00417290(uVar1);
  local_8._0_1_ = 0x13;
  FUN_00414c60();
  local_8._0_1_ = 0x11;
  FUN_00418640();
  local_8._0_1_ = 0x10;
  FUN_0041ac10();
  local_8._0_1_ = 0xf;
  FUN_007d5230();
  local_8._0_1_ = 0xe;
  FUN_007d5230();
  local_8._0_1_ = 0xd;
  FUN_007d5230();
  *(undefined ***)(param_1 + 0x1828) = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x16;
  FUN_00414c60();
  local_8._0_1_ = 0x15;
  FUN_00414c60();
  local_8._0_1_ = 0xc;
  FUN_0041a3d0();
  *(undefined ***)(param_1 + 0x1560) = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x18;
  FUN_00414c60();
  local_8._0_1_ = 0x17;
  FUN_00414c60();
  local_8._0_1_ = 0xb;
  FUN_0041a3d0();
  *(undefined ***)(param_1 + 0x1298) = CSWGuiButtonToggle::vftable;
  local_8._0_1_ = 0x1a;
  FUN_00414c60();
  local_8._0_1_ = 0x19;
  FUN_00414c60();
  local_8._0_1_ = 10;
  FUN_0041a3d0();
  local_8._0_1_ = 9;
  FUN_0041a3d0();
  local_8._0_1_ = 8;
  FUN_0041a3d0();
  local_8._0_1_ = 7;
  FUN_0041a3d0();
  local_8._0_1_ = 6;
  FUN_0041a3d0();
  local_8._0_1_ = 5;
  FUN_0041a3d0();
  local_8._0_1_ = 4;
  FUN_0041a3d0();
  local_8._0_1_ = 3;
  FUN_0041a3d0();
  local_8._0_1_ = 2;
  FUN_0041d190();
  *(undefined ***)(param_1 + 0x1b0) = CSWGuiLabel::vftable;
  local_8._0_1_ = 0x1c;
  FUN_00417290();
  local_8._0_1_ = 0x1b;
  FUN_00414c60();
  local_8._0_1_ = 1;
  FUN_00418640();
  local_8 = (uint)local_8._1_3_ << 8;
  CMFCToolBarButtonsListButton::~CMFCToolBarButtonsListButton
            ((CMFCToolBarButtonsListButton *)(param_1 + 0x68));
/* ... 5 more lines truncated by --max-lines */
```

### decoy: 0x008e0a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0a80: 16-byte aligned: YES
  008e0a80  55             PUSH EBP
  008e0a81  8B EC          MOV EBP,ESP
  008e0a83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0a80
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008E0F70  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 9):
  +0x20 [dec 32]  x7  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x5C [dec 92]  x3  via EAX,EDX
  +0x1D4 [dec 468]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x2F8 [dec 760]  x2  via EAX,EDX
  +0x808 [dec 2056]  x1  via EDX
  +0x8E0C38 [dec 9309240]  x1  via EDX
```

## Card B16-1-16

GOG function: CSWGuiInGameCharacter::RotateCharacterRight  gog 0x00599e50  callconv=(blank)  param_size_bytes=0  gog_size=543  gog_retn=4
Candidate Steam address: 0x00850330 (size 543, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00850550:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00599c30 CSWGuiInGameCharacter::RotateCharacterLeft -> 0x008500f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0059a070 CSWGuiInGameInventory::Constructor -> 0x008a6170  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x88*,0x4b6c*} | Steam {0x4*,0x8*,0xc*,0x88*,0x4b6c*}
imm sets (* = in both, 5 common): GOG {0x4*,0x10*,0x30*,0x44*,0x5c*} | Steam {0x4*,0x10*,0x30*,0x44*,0x5c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00850330: 16-byte aligned: YES
  00850330  55             PUSH EBP
  00850331  8B EC          MOV EBP,ESP
  00850333  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00850330
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00850330 FUN_00850330 size=543 ret=4 callers=0 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00850330(int param_1)

{
  int iVar1;
  float *pfVar2;
  float *pfVar3;
  float *local_5c;
  float local_20;
  float local_1c;
  float local_18;
  undefined1 local_14 [16];
  
  iVar1 = *(int *)(param_1 + 0x4b6c);
  pfVar3 = (float *)(iVar1 + 0x30);
  pfVar2 = (float *)FUN_00515620(local_14,_DAT_0098bd88,0,0);
  if ((DAT_00a0f190 == 0) ||
     ((*pfVar2 != (float)_DAT_00986b50 && (*pfVar2 != (float)_DAT_00986d98)))) {
    local_20 = (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490) *
               *pfVar3 + (pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8] - pfVar2[3 = byte +0xc] * *pfVar2) * (float)_DAT_00987490 *
                         *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_1c = (pfVar2[3 = byte +0xc] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490 * *pfVar3 +
               (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc] - pfVar2[1 = byte +0x4] * *pfVar2) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_18 = (pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc] - pfVar2[2 = byte +0x8] * *pfVar2) * (float)_DAT_00987490 * *pfVar3 +
               (pfVar2[1 = byte +0x4] * *pfVar2 + pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x34) +
               (1.0 - (pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x38);
  }
  else {
    local_20 = *pfVar3;
    local_1c = *(float *)(iVar1 + 0x34);
    local_18 = *(float *)(iVar1 + 0x38);
  }
  local_5c = &local_20;
  (**(code **)(**(int **)(param_1 + 0x4b6c) + 0x88))(local_5c);
  return;
}
```

### decoy: 0x00850550 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00850550: 16-byte aligned: YES
  00850550  55             PUSH EBP
  00850551  8B EC          MOV EBP,ESP
  00850553  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00850550
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

## Card B16-1-17

GOG function: CSWGlobalVariableTable::ClearValues  gog 0x00650c90  callconv=(blank)  param_size_bytes=0  gog_size=103  gog_retn=0
Candidate Steam address: 0x00653dd0 (size 103, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00653d40:STR0/CALLEE0/SIG0.111/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00650c00 CSWGlobalVariableTable::ClearIdentifiers -> 0x00653d40  gate_class=AUTO
- GOG-order neighbour after:  0x00650d00 CSWGlobalVariableTable::ReadCatalogue -> 0x00653e40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 9 common): GOG {0xc*,0x28*,0x71*,0x3e8*,0x960*,0xddb8*,0xde29*,0xe214*,0xeb74*} | Steam {0xc*,0x28*,0x71*,0x3e8*,0x960*,0xddb8*,0xde29*,0xe214*,0xeb74*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00653dd0: 16-byte aligned: YES
  00653dd0  55             PUSH EBP
  00653dd1  8B EC          MOV EBP,ESP
  00653dd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00653dd0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00653dd0 FUN_00653dd0 size=103 ret=0 callers=2 callees=1

void __fastcall FUN_00653dd0(int param_1)

{
  _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xddb8),0,0x71);
  _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xde29),0,1000);
  _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xe214),0,0x960);
  _memset [GHIDRA-AUTO-LABEL: unreliable]((void *)(param_1 + 0xeb74),0,0x28);
  return;
}
```

### decoy: 0x00653d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00653d40: 16-byte aligned: YES
  00653d40  55             PUSH EBP
  00653d41  8B EC          MOV EBP,ESP
  00653d43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00653d40
string literals (0):
direct callees (2):
  0x0091C630  (unnamed)
  0x00653DD0  (unnamed)
struct offsets touched (top 7):
  +0xEB9C [dec 60316]  x1  via ECX
  +0xEBA0 [dec 60320]  x1  via EDX
  +0xEBA4 [dec 60324]  x1  via EAX
  +0xEBA8 [dec 60328]  x1  via ECX
  +0xEFAC [dec 61356]  x1  via EDX
  +0xEFB0 [dec 61360]  x1  via EAX
  +0xEFB4 [dec 61364]  x1  via ECX
```

## Card B16-1-18

GOG function: CSWGuiInGameCharacter::OnPanelRemoved  gog 0x005991e0  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x0084f8e0 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: OnPanelRemoved 0x40f4c0)
- best decoy (neighbour/other candidate) scores: 0x0084f770:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599070 CSWGuiInGameCharacter::SetModText -> 0x0084f770  gate_class=REVIEW
- GOG-order neighbour after:  0x00599200 CSWGuiInGameCharacter::HandleInputEvent -> 0x0084f900  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4b70*} | Steam {0x4b70*}
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
prologue for 0x0084f8e0: 16-byte aligned: YES
  0084f8e0  55             PUSH EBP
  0084f8e1  8B EC          MOV EBP,ESP
  0084f8e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084f8e0
string literals (0):
direct callees (1):
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 1):
  +0x4B70 [dec 19312]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0084f8e0 FUN_0084f8e0 size=32 ret=0 callers=0 callees=1

void __fastcall FUN_0084f8e0(int param_1)

{
  *(undefined4 *)(param_1 + 0x4b70) = 0;
  FUN_0040f4c0();
  return;
}
```

### decoy: 0x0084f770 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084f770: 16-byte aligned: YES
  0084f770  55             PUSH EBP
  0084f771  8B EC          MOV EBP,ESP
  0084f773  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0084f770
string literals (0):
direct callees (5):
  0x00733700  CExoString::Constructor_5
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x00416E30  CSWGuiTextParams::SetText
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B16-1-19

GOG function: CSWGuiInGameGameplay::OnExitDiff  gog 0x005e6080  callconv=(blank)  param_size_bytes=0  gog_size=70  gog_retn=4
Candidate Steam address: 0x008e15d0 (size 70, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e1570:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005e6020 CSWGuiInGameGameplay::OnEnterDiff -> 0x008e1570  gate_class=REVIEW
- GOG-order neighbour after:  0x005e60d0 CSWGuiInGameGameplay::SetUpOptions -> 0x008e1620  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
imm sets (* = in both, 5 common): GOG {0x4*,0x6*,0x8*,0xea0*,0xffffffbf*} | Steam {0x4*,0x6*,0x8*,0xea0*,0xffffffbf*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008e15d0: 16-byte aligned: YES
  008e15d0  55             PUSH EBP
  008e15d1  8B EC          MOV EBP,ESP
  008e15d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e15d0
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e15d0 FUN_008e15d0 size=70 ret=4 callers=0 callees=1

void __fastcall FUN_008e15d0(int param_1)

{
  FUN_00417010(&DAT_00a7fe24);
  *(uint *)(param_1 + 0xed8) = *(uint *)(param_1 + 0xed8) & 0xffffffbf;
  return;
}
```

### decoy: 0x008e1570 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e1570: 16-byte aligned: YES
  008e1570  55             PUSH EBP
  008e1571  8B EC          MOV EBP,ESP
  008e1573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e1570
string literals (0):
direct callees (3):
  0x00417010  (unnamed)
  0x008E1280  (unnamed)
  0x008E1410  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

## Card B16-1-20

GOG function: CServerAIMaster::SaveEventQueue  gog 0x0064e840  callconv=(blank)  param_size_bytes=8  gog_size=168  gog_retn=8
Candidate Steam address: 0x0051ee20 (size 168, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x0064e8f0:0.82>0.52
- anchors: A-STR 1 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: SaveModuleIFOStart 0x55bd10); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0051eed0:STR1/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064e7a0 CServerAIMaster::GetPendingEvent -> 0x0051ed80  gate_class=AUTO
- GOG-order neighbour after:  0x0064e8f0 CServerAIMaster::LoadEventQueue -> 0x0051eed0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x006258a0 CResGFF::AddList -> Steam 0x00719740  in candidate callees: yes
  0x00625960 CResGFF::AddListElement -> Steam 0x00719800  in candidate callees: yes
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00639a30 unnamed -> Steam ?  in candidate callees: n/a
  0x0064fbb0 CServerAIEventNode::SaveNode -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0072e4c0 CSWSModule::SaveModuleIFOStart -> Steam 0x0055bd10  in candidate callers: yes
GOG strings (1): 'EventQueue'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x8*,0x28*,0x54*,0xabcd*} | Steam {0x8*,0x28*,0x54*,0xabcd*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0051ee20: 16-byte aligned: YES
  0051ee20  55             PUSH EBP
  0051ee21  8B EC          MOV EBP,ESP
  0051ee23  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051ee20
string literals (1):
  0x00992E20  at 0051ee33  'EventQueue'
direct callees (8):
  0x00719740  CResGFF::AddList
  0x005210F0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00719800  CResGFF::AddListElement
  0x00520190  (unnamed)
  0x0058C370  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051ee20 FUN_0051ee20 size=168 ret=8 callers=1 callees=8

void FUN_0051ee20(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  undefined4 extraout_var;
  int local_28;
  undefined1 local_24 [4];
  undefined4 local_20;
  undefined1 local_1c [20];
  uint local_8;
  
  local_8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  FUN_00719740(local_1c,param_2,"EventQueue");
  iVar1 = FUN_005210f0(extraout_var);
  if (iVar1 != 0) {
    local_28 = FUN_007a1720();
    while (local_28 != 0) {
      local_20 = FUN_00561430(local_28);
      FUN_00719800(local_24,local_1c,0xabcd);
      FUN_00520190(param_1,local_24);
      FUN_0058c370(&local_28);
    }
  }
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_8 ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0051eed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051eed0: 16-byte aligned: YES
  0051eed0  55             PUSH EBP
  0051eed1  8B EC          MOV EBP,ESP
  0051eed3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051eed0
string literals (1):
  0x00992E20  at 0051eee3  'EventQueue'
direct callees (9):
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x00718890  CResGFF::GetListElement
  0x005205F0  (unnamed)
  0x0054AA30  (unnamed)
  0x009196FD  operator delete
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

## Card B16-1-21

GOG function: CSWGlobalVariableTable::GetValueBoolean  gog 0x00651600  callconv=(blank)  param_size_bytes=8  gog_size=280  gog_retn=8
Candidate Steam address: 0x00654670 (size 78, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00654740:STR2/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651580 CSWGlobalVariableTable::SetBit -> 0x006546c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00651720 CSWGlobalVariableTable::GetValueNumber -> 0x00654670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: no
  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> Steam ?  in candidate callees: n/a
  0x00651530 CSWGlobalVariableTable::GetBit -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00793150 Global::ExecuteCommandGetGlobalNumber -> Steam 0x0067db70  in candidate callers: no
GOG strings (2): "Script var '%s' is not BOOLEAN!"; "Script var BOOLEAN '%s' not in catalogue"
disp sets (* = in both, 0 common): GOG {0x16} | Steam {}
imm sets (* = in both, 2 common): GOG {0x3*,0x8*,0xc,0xe,0x18,0x3fff,0xddb8} | Steam {0x3*,0x7,0x8*,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654670: 16-byte aligned: YES
  00654670  55             PUSH EBP
  00654671  8B EC          MOV EBP,ESP
  00654673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654670
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654670 FUN_00654670 size=78 ret=8 callers=4 callees=0

bool FUN_00654670(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = (int)(param_2 + (param_2 >> 0x1f & 7U)) >> 3;
  return ((int)(uint)*(byte *)(param_1 + iVar1) >> (7U - ((char)param_2 + (char)iVar1 * -8) & 0x1f)
         & 1U) != 0;
}
```

### decoy: 0x00654740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654740: 16-byte aligned: YES
  00654740  55             PUSH EBP
  00654741  8B EC          MOV EBP,ESP
  00654743  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00654740
string literals (2):
  0x009984F8  at 0065479e  "Script var BOOLEAN '%s' not in catalogue!"
  0x009984D8  at 006547f8  "Script var '%s' is not BOOLEAN!"
direct callees (6):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x00654670  (unnamed)
struct offsets touched (top 0):
```

## Card B16-1-22

GOG function: CSWGuiInGameGameplay::OnKeyMap  gog 0x005e5a70  callconv=(blank)  param_size_bytes=4  gog_size=178  gog_retn=4
Candidate Steam address: 0x008e0fc0 (size 178, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: rotation:0x005e5bf0:0.83>0.65
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e1080:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5a20 CSWGuiInGameGameplay::OnDefault -> 0x008e0f70  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5b30 CSWGuiInGameGameplay::OnMouse -> 0x008e1080  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005eecd0 CSWGuiInGameOptKeyMappings::Constructor -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x50*} | Steam {0x1c*,0x50*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x20*,0x1370*} | Steam {0x3*,0x4*,0x20*,0x1370*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0fc0: 16-byte aligned: YES
  008e0fc0  55             PUSH EBP
  008e0fc1  8B EC          MOV EBP,ESP
  008e0fc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e0fc0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x008FFF20  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 2):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0fc0 FUN_008e0fc0 size=178 ret=4 callers=0 callees=3

void __thiscall FUN_008e0fc0(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fa7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar1 = FUN_00919723(0x1370,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_008fff20(*(undefined4 *)(param_1 + 0x1c));
    }
    local_8 = 0xffffffff;
    FUN_00410530(local_30,3,1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008e1080 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e1080: 16-byte aligned: YES
  008e1080  55             PUSH EBP
  008e1081  8B EC          MOV EBP,ESP
  008e1083  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008e1080
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x007DC410  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 2):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-23

GOG function: CSWGlobalVariableTable::SetValueBoolean  gog 0x00651a70  callconv=(blank)  param_size_bytes=8  gog_size=270  gog_retn=8
Candidate Steam address: 0x00654aa0 (size 268, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.68 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00654bb0:STR2/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651960 CSWGlobalVariableTable::GetValueString -> 0x00654aa0  gate_class=AUTO
- GOG-order neighbour after:  0x00651b80 CSWGlobalVariableTable::SetValueNumber -> 0x00654cc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> Steam ?  in candidate callees: n/a
  0x00651580 CSWGlobalVariableTable::SetBit -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x007a6da0 Global::ExecuteCommandSetGlobalNumber -> Steam 0x00691750  in candidate callers: no
GOG strings (2): "Script var BOOLEAN '%s' not in catalogue"; "Script var '%s' not a BOOLEAN!"
disp sets (* = in both, 1 common): GOG {0x16*} | Steam {0x16*,0xeb74}
imm sets (* = in both, 6 common): GOG {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*,0xddb8} | Steam {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654aa0: 16-byte aligned: YES
  00654aa0  55             PUSH EBP
  00654aa1  8B EC          MOV EBP,ESP
  00654aa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00654aa0
string literals (2):
  0x009985DC  at 00654af5  "Script var STRING '%s' not in catalogue!"
  0x009985BC  at 00654b50  "Script var '%s' not a STRING!"
direct callees (6):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x007337C0  CExoString::operator=
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654aa0 FUN_00654aa0 size=268 ret=8 callers=1 callees=6

void __thiscall FUN_00654aa0(int param_1,undefined4 param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958e00;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_006545b0(param_2);
  if (local_14 < 0) {
    FUN_00733540(uVar1);
    local_8 = 0;
    uVar2 = FUN_00734010();
    FUN_00734270(local_1c,"Script var STRING \'%s\' not in catalogue!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else if (*(ushort *)(param_1 + 0x16 + local_14 * 0x18) >> 0xe == 3) {
    FUN_007337c0(param_1 + 0xeb74 +
                 (uint)(*(ushort *)(param_1 + 0x16 + local_14 * 0x18) & 0x3fff) * 8);
  }
  else {
    FUN_00733540(uVar1);
    local_8 = 1;
    uVar2 = FUN_00734010();
    FUN_00734270(local_24,"Script var \'%s\' not a STRING!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00654bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654bb0: 16-byte aligned: YES
  00654bb0  55             PUSH EBP
  00654bb1  8B EC          MOV EBP,ESP
  00654bb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00654bb0
string literals (2):
  0x009984F8  at 00654c05  "Script var BOOLEAN '%s' not in catalogue!"
  0x00998608  at 00654c5f  "Script var '%s' not a BOOLEAN!"
direct callees (6):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x006546C0  (unnamed)
struct offsets touched (top 0):
```

## Card B16-1-24

GOG function: CSWGlobalVariableTable::AddIdentifierBoolean  gog 0x006512f0  callconv=(blank)  param_size_bytes=4  gog_size=88  gog_retn=4
Candidate Steam address: 0x00654430 (size 88, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::AddBoolean | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00654490:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651190 CSWGlobalVariableTable::AddIdentifier -> 0x006542d0  gate_class=AUTO
- GOG-order neighbour after:  0x00651350 CSWGlobalVariableTable::AddIdentifierNumber -> 0x00654490  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00651190 CSWGlobalVariableTable::AddIdentifier -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xeb9c*} | Steam {0xeb9c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x384*} | Steam {0x4*,0x8*,0x384*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00654430: 16-byte aligned: YES
  00654430  55             PUSH EBP
  00654431  8B EC          MOV EBP,ESP
  00654433  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654430
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEB9C [dec 60316]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00654430 FUN_00654430 size=88 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_00654430(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0xeb9c) < 900) {
    local_8 = FUN_006542d0(param_2,0,*(undefined4 *)(param_1 + 0xeb9c));
    *(int *)(param_1 + 0xeb9c) = *(int *)(param_1 + 0xeb9c) + 1;
  }
  return local_8;
}
```

### decoy: 0x00654490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654490: 16-byte aligned: YES
  00654490  55             PUSH EBP
  00654491  8B EC          MOV EBP,ESP
  00654493  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00654490
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA0 [dec 60320]  x4  via EAX,ECX
```

## Card B16-1-25

GOG function: CSWGuiInGameGameplay::OnAutoLevelUp  gog 0x005e5700  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x008e0c50 (size 101, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiOptionsGameplay | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0d40:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e56e0 CSWGuiInGameGameplay::OnPanelAdded -> 0x008e0cc0  gate_class=REJECT
- GOG-order neighbour after:  0x005e5770 CSWGuiInGameGameplay::OnAutoSave -> 0x008e0cc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x00462c60 CClientOptions::SetAutoLevelUpNPCs -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 2 common): GOG {0x4*,0x18*} | Steam {0x4*,0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0c50: 16-byte aligned: YES
  008e0c50  55             PUSH EBP
  008e0c51  8B EC          MOV EBP,ESP
  008e0c53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0c50
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5EF0  ClientOptions::SetAutoLevelUpNPCs (+8 bit0; calls AutoLevelUpNPCs when set)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0c50 FUN_008e0c50 size=101 ret=4 callers=0 callees=2

void __thiscall FUN_008e0c50(undefined4 param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  undefined4 uVar4;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 4);
    iVar1 = FUN_0072fb00(param_1);
    uVar2 = *(uint *)(iVar1 + 8) & 1;
    uVar3 = (uint)(uVar2 == 0);
    FUN_0072fb00(uVar3,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4),uVar2,iVar1,uVar4);
    FUN_007b5ef0(uVar3);
  }
  return;
}
```

### decoy: 0x008e0d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0d40: 16-byte aligned: YES
  008e0d40  55             PUSH EBP
  008e0d41  8B EC          MOV EBP,ESP
  008e0d43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0d40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-26

GOG function: CSWGuiInGameGameplay::OnDefault  gog 0x005e5a20  callconv=(blank)  param_size_bytes=4  gog_size=65  gog_retn=4
Candidate Steam address: 0x008e0f70 (size 65, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0c50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e59f0 CSWGuiInGameGameplay::OnEnterSlot -> 0x008e0f40  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5a70 CSWGuiInGameGameplay::OnKeyMap -> 0x008e0fc0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x00468830 CClientOptions::SetDefaultGameplayOptions -> Steam ?  in candidate callees: n/a
  0x005e60d0 CSWGuiInGameGameplay::SetUpOptions -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x50*} | Steam {0x4*,0x50*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0f70: 16-byte aligned: YES
  008e0f70  55             PUSH EBP
  008e0f71  8B EC          MOV EBP,ESP
  008e0f73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0f70
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BBB60  (unnamed)
  0x008E1620  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0f70 FUN_008e0f70 size=65 ret=4 callers=1 callees=3

void __thiscall FUN_008e0f70(undefined4 param_1,int param_2)

{
  if (*(int *)(param_2 + 0x50) != 0) {
    FUN_0072fb00(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
    FUN_007bbb60();
    FUN_008e1620();
  }
  return;
}
```

### decoy: 0x008e0c50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0c50: 16-byte aligned: YES
  008e0c50  55             PUSH EBP
  008e0c51  8B EC          MOV EBP,ESP
  008e0c53  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0c50
string literals (0):
direct callees (2):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007B5EF0  ClientOptions::SetAutoLevelUpNPCs (+8 bit0; calls AutoLevelUpNPCs when set)
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-27

GOG function: CSWGlobalVariableTable::SetValueNumber  gog 0x00651b80  callconv=(blank)  param_size_bytes=5  gog_size=259  gog_retn=8
Candidate Steam address: 0x00654cc0 (size 259, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::SetNumber | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Script var '%s' not a NUMBER!'@00998524; 'Script var NUMBER '%s' not in catalogue!'@00998544; CALLEE 7 (callers: ModifyAlignment 0x6ad400, SetExperience 0x6b6bd0, OnRemoveFactionModifier 0x5953e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00654bb0:STR0/CALLEE1/SIG0.679/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651a70 CSWGlobalVariableTable::SetValueBoolean -> 0x00654aa0  gate_class=REVIEW
- GOG-order neighbour after:  0x00651c90 CSWGlobalVariableTable::SetValueLocation -> 0x00654dd0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 15): the card's A-CALLEE credit may come from these, not from callees
  0x006e9880 CSWSCreatureStats::ModifyAlignment -> Steam 0x006ad400  in candidate callers: yes
  0x006f3050 CSWSCreatureStats::SetExperience -> Steam 0x006b6bd0  in candidate callers: yes
  0x00737cd0 CSWSEffectListHandler::OnRemoveFactionModifier -> Steam 0x005953e0  in candidate callers: yes
  0x00782130 Global::ExecuteCommandAdjustAlignment -> Steam 0x0066cb50  in candidate callers: yes
GOG strings (2): "Script var '%s' not a NUMBER!"; "Script var NUMBER '%s' not in catalogue!"
disp sets (* = in both, 2 common): GOG {0x16*,0xde29*} | Steam {0x16*,0xde29*}
imm sets (* = in both, 6 common): GOG {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*} | Steam {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654cc0: 16-byte aligned: YES
  00654cc0  55             PUSH EBP
  00654cc1  8B EC          MOV EBP,ESP
  00654cc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00654cc0
string literals (2):
  0x00998544  at 00654d15  "Script var NUMBER '%s' not in catalogue!"
  0x00998524  at 00654d6d  "Script var '%s' not a NUMBER!"
direct callees (5):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654cc0 FUN_00654cc0 size=259 ret=8 callers=15 callees=5

void __thiscall FUN_00654cc0(int param_1,undefined4 param_2,undefined1 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958e00;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_006545b0(param_2);
  if (local_14 < 0) {
    FUN_00733540(uVar1);
    local_8 = 0;
    uVar2 = FUN_00734010();
    FUN_00734270(local_1c,"Script var NUMBER \'%s\' not in catalogue!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else if (*(ushort *)(param_1 + 0x16 + local_14 * 0x18) >> 0xe == 1) {
    *(undefined1 *)
     (param_1 + 0xde29 + (uint)(*(ushort *)(param_1 + 0x16 + local_14 * 0x18) & 0x3fff)) = param_3;
  }
  else {
    FUN_00733540(uVar1);
    local_8 = 1;
    uVar2 = FUN_00734010();
    FUN_00734270(local_24,"Script var \'%s\' not a NUMBER!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00654bb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654bb0: 16-byte aligned: YES
  00654bb0  55             PUSH EBP
  00654bb1  8B EC          MOV EBP,ESP
  00654bb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00654bb0
string literals (2):
  0x009984F8  at 00654c05  "Script var BOOLEAN '%s' not in catalogue!"
  0x00998608  at 00654c5f  "Script var '%s' not a BOOLEAN!"
direct callees (6):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x006546C0  (unnamed)
struct offsets touched (top 0):
```

## Card B16-1-28

GOG function: CSWGlobalVariableTable::GetValueNumber  gog 0x00651720  callconv=(blank)  param_size_bytes=8  gog_size=267  gog_retn=8
Candidate Steam address: 0x00654670 (size 78, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00654860:STR2/CALLEE6/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651600 CSWGlobalVariableTable::GetValueBoolean -> 0x00654670  gate_class=REVIEW
- GOG-order neighbour after:  0x00651830 CSWGlobalVariableTable::GetValueLocation -> 0x00654970  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: no
  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 19): the card's A-CALLEE credit may come from these, not from callees
  0x006e1280 CSWSCreatureStats::GetLevel -> Steam 0x006a4e00  in candidate callers: no
  0x006f3050 CSWSCreatureStats::SetExperience -> Steam 0x006b6bd0  in candidate callers: no
  0x00762e30 CSWSDoor::AIUpdate -> Steam 0x00617980  in candidate callers: no
  0x0076efd0 CSWSPlaceable::AIUpdate -> Steam 0x005f4c00  in candidate callers: no
GOG strings (2): "Script var '%s' not a NUMBER!"; "Script var NUMBER '%s' not in catalogue!"
disp sets (* = in both, 0 common): GOG {0x16,0xde29} | Steam {}
imm sets (* = in both, 2 common): GOG {0x3*,0x8*,0xc,0xe,0x18,0x3fff} | Steam {0x3*,0x7,0x8*,0x10}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654670: 16-byte aligned: YES
  00654670  55             PUSH EBP
  00654671  8B EC          MOV EBP,ESP
  00654673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654670
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654670 FUN_00654670 size=78 ret=8 callers=4 callees=0

bool FUN_00654670(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = (int)(param_2 + (param_2 >> 0x1f & 7U)) >> 3;
  return ((int)(uint)*(byte *)(param_1 + iVar1) >> (7U - ((char)param_2 + (char)iVar1 * -8) & 0x1f)
         & 1U) != 0;
}
```

### decoy: 0x00654860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654860: 16-byte aligned: YES
  00654860  55             PUSH EBP
  00654861  8B EC          MOV EBP,ESP
  00654863  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00654860
string literals (2):
  0x00998544  at 006548bb  "Script var NUMBER '%s' not in catalogue!"
  0x00998524  at 00654913  "Script var '%s' not a NUMBER!"
direct callees (5):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B16-1-29

GOG function: CExoMoviePlayerInternal::Destructor  gog 0x0091fbe0  callconv=(blank)  param_size_bytes=0  gog_size=291  gog_retn=0
Candidate Steam address: 0x0040c880 (size 360, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x0091fe60:0.71>0.51
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.43 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040c9f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fa70 CExoMoviePlayerInternal::Constructor -> 0x0040c880  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> 0x0040cb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00610cf0 CExoCriticalSection::Destructor -> Steam ?  in candidate callees: n/a
  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x38*,0x44*} | Steam {0x4*,0x14,0x18,0x1c,0x20,0x24,0x30,0x34,0x38*,0x44*,0x48,0x4c,0x50,0x64,...(+1)}
imm sets (* = in both, 8 common): GOG {0x2*,0x3*,0x4*,0x8*,0x14,0x28*,0x40*,0x68*,0x90*,0x103} | Steam {0x2*,0x3*,0x4*,0x8*,0x28*,0x3c,0x40*,0x68*,0x90*,0x8000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040c880: 16-byte aligned: YES
  0040c880  55             PUSH EBP
  0040c881  8B EC          MOV EBP,ESP
  0040c883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0040c880
string literals (0):
direct callees (5):
  0x0040DEF0  (unnamed)
  0x00733540  CExoString::Constructor
  0x00739C10  (unnamed)
  0x0084A640  (unnamed)
  0x007338D0  CExoString::operator=_2
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x18 [dec 24]  x1  via EDX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via ECX
  +0x24 [dec 36]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
  +0x34 [dec 52]  x1  via EAX
  +0x38 [dec 56]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0040c880 FUN_0040c880 size=360 ret=0 callers=1 callees=7

undefined4 * __fastcall FUN_0040c880(undefined4 *param_1)

{
  uint uVar1;
  HANDLE pvVar2;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094813a;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_0040def0(0);
  local_8 = 0;
  FUN_00733540(uVar1);
  local_8._0_1_ = 1;
  FUN_00739c10();
  local_8._0_1_ = 2;
  FUN_00733540();
  local_8._0_1_ = 3;
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,4);
  FUN_007338d0(&DAT_00986925);
  param_1[0xc = byte +0x30] = 0;
  *param_1 = 0;
  pvVar2 = CreateEventA((LPSECURITY_ATTRIBUTES)0x0,0,0,(LPCSTR)0x0);
  param_1[1 = byte +0x4] = pvVar2;
  pvVar2 = CreateThread((LPSECURITY_ATTRIBUTES)0x0,0x8000,FUN_0040d430,param_1,0,param_1 + 0xf);
  param_1[0xe = byte +0x38] = pvVar2;
  param_1[0xd = byte +0x34] = 0;
  param_1[5 = byte +0x14] = 0;
  param_1[6 = byte +0x18] = 0;
  param_1[7 = byte +0x1c] = 0;
  param_1[8 = byte +0x20] = 0;
  param_1[9 = byte +0x24] = 0;
  param_1[0x11 = byte +0x44] = 0;
  param_1[0x13 = byte +0x4c] = 0;
  param_1[0x12 = byte +0x48] = 0;
  param_1[0x1c = byte +0x70] = 0;
  param_1[0x14 = byte +0x50] = 0;
  param_1[0x19 = byte +0x64] = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0040c9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c9f0: 16-byte aligned: YES
  0040c9f0  55             PUSH EBP
  0040c9f1  8B EC          MOV EBP,ESP
  0040c9f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040c9f0
string literals (0):
direct callees (5):
  0x0040CB20  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00739C90  (unnamed)
  0x0040DF30  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x38 [dec 56]  x2  via EAX,ECX
```

## Card B16-1-30

GOG function: CSWGuiInGameGameplay::OnEnterSlot  gog 0x005e59f0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x008e0f40 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0f70:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5970 CSWGuiInGameGameplay::OnCombatMovement -> 0x008e0ec0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5a20 CSWGuiInGameGameplay::OnDefault -> 0x008e0f70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x005e5ec0 CSWGuiInGameGameplay::SetDescription -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x5c*} | Steam {0x5c*}
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
prologue for 0x008e0f40: 16-byte aligned: YES
  008e0f40  55             PUSH EBP
  008e0f41  8B EC          MOV EBP,ESP
  008e0f43  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0f40
string literals (0):
direct callees (1):
  0x008E1410  (unnamed)
struct offsets touched (top 1):
  +0x5C [dec 92]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0f40 FUN_008e0f40 size=36 ret=4 callers=0 callees=1
THUNK -> 0x008E1410 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_008e0f40(int param_1)

{
  FUN_008e1410(*(undefined4 *)(param_1 + 0x5c));
  return;
}
```

### decoy: 0x008e0f70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0f70: 16-byte aligned: YES
  008e0f70  55             PUSH EBP
  008e0f71  8B EC          MOV EBP,ESP
  008e0f73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0f70
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BBB60  (unnamed)
  0x008E1620  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-31

GOG function: CSWGuiTextParams::SetDefaultColor_2  gog 0x00915550  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x004171a0 (size 188, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00417180:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00915510 CSWGuiTextParams::SetDefaultColor -> 0x00417140  gate_class=AUTO
- GOG-order neighbour after:  0x00915570 CSWGuiText::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x00918530 CSWGuiButton::SetActive -> Steam 0x0041a290  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x14,0x6c}
imm sets (* = in both, 0 common): GOG {0x2c} | Steam {0x4,0x8,0x18,0x190}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004171a0: 16-byte aligned: YES
  004171a0  55             PUSH EBP
  004171a1  8B EC          MOV EBP,ESP
  004171a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004171a0
string literals (0):
direct callees (2):
  0x00416960  (unnamed)
  0x00417070  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x6C [dec 108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 004171a0 FUN_004171a0 size=188 ret=0 callers=6 callees=2

undefined4 * __fastcall FUN_004171a0(undefined4 *param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00948743;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWGuiDrawable::vftable;
  param_1[4 = byte +0x10] = 0;
  param_1[3 = byte +0xc] = 0;
  param_1[2 = byte +0x8] = 0;
  param_1[1 = byte +0x4] = 0;
  local_8 = 0;
  *param_1 = CSWGuiText::vftable;
  FUN_00416960(uVar1);
  local_8 = CONCAT31(local_8._1_3_,1);
  param_1[5 = byte +0x14] = 0;
  FUN_00417070(param_1);
  param_1[0x1b = byte +0x6c] = 400;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x00417180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00417180: 16-byte aligned: YES
  00417180  55             PUSH EBP
  00417181  8B EC          MOV EBP,ESP
  00417183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00417180
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 0):
```

## Card B16-1-32

GOG function: CSWGlobalVariableTable::SetValueString  gog 0x00651dc0  callconv=(blank)  param_size_bytes=8  gog_size=268  gog_retn=8
Candidate Steam address: 0x00654f00 (size 268, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGlobalVariableTable::SetString | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'Script var '%s' not a STRING!'@009985bc; 'Script var STRING '%s' not in catalogue!'@009985dc; CALLEE 1 (callers: ExecuteCommandSetGlobalString 0x691890); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00654cc0:STR0/CALLEE0/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651c90 CSWGlobalVariableTable::SetValueLocation -> 0x00654dd0  gate_class=AUTO
- GOG-order neighbour after:  0x00651ed0 CSWGlobalVariableTable::WriteTable -> 0x00655010  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007a6ee0 Global::ExecuteCommandSetGlobalString -> Steam 0x00691890  in candidate callers: yes
GOG strings (2): "Script var '%s' not a STRING!"; "Script var STRING '%s' not in catalogue!"
disp sets (* = in both, 2 common): GOG {0x16*,0xeb74*} | Steam {0x16*,0xeb74*}
imm sets (* = in both, 6 common): GOG {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*} | Steam {0x3*,0x8*,0xc*,0xe*,0x18*,0x3fff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654f00: 16-byte aligned: YES
  00654f00  55             PUSH EBP
  00654f01  8B EC          MOV EBP,ESP
  00654f03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00654f00
string literals (2):
  0x009985DC  at 00654f55  "Script var STRING '%s' not in catalogue!"
  0x009985BC  at 00654fb0  "Script var '%s' not a STRING!"
direct callees (6):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
  0x007337C0  CExoString::operator=
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654f00 FUN_00654f00 size=268 ret=8 callers=1 callees=6

void __thiscall FUN_00654f00(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_24 [8];
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00958e00;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_006545b0(param_2);
  if (local_14 < 0) {
    FUN_00733540(uVar1);
    local_8 = 0;
    uVar2 = FUN_00734010();
    FUN_00734270(local_1c,"Script var STRING \'%s\' not in catalogue!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else if (*(ushort *)(param_1 + 0x16 + local_14 * 0x18) >> 0xe == 3) {
    FUN_007337c0(param_3);
  }
  else {
    FUN_00733540(uVar1);
    local_8 = 1;
    uVar2 = FUN_00734010();
    FUN_00734270(local_24,"Script var \'%s\' not a STRING!",uVar2);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00654cc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654cc0: 16-byte aligned: YES
  00654cc0  55             PUSH EBP
  00654cc1  8B EC          MOV EBP,ESP
  00654cc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00654cc0
string literals (2):
  0x00998544  at 00654d15  "Script var NUMBER '%s' not in catalogue!"
  0x00998524  at 00654d6d  "Script var '%s' not a NUMBER!"
direct callees (5):
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00733540  CExoString::Constructor
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

## Card B16-1-33

GOG function: CSWGuiInGameGameplay::HandleInputEvent  gog 0x005e5510  callconv=(blank)  param_size_bytes=8  gog_size=413  gog_retn=8
Candidate Steam address: 0x008e0a80 (size 413, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.73) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e07b0:STR0/CALLEE0/SIG0.059/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5240 CSWGuiInGameGameplay::Destructor_2 -> 0x008e07b0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e56e0 CSWGuiInGameGameplay::OnPanelAdded -> 0x008e0cc0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x005e5a20 CSWGuiInGameGameplay::OnDefault -> Steam ?  in candidate callees: n/a
  0x0090cf20 CSWGuiPanel::HandleInputEvent -> Steam 0x0040eaa0  in candidate callees: yes
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: yes
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 8 common): GOG {0x1c*,0x20*,0x3c*,0x48*,0x5c*,0x1d4*,0x2f8*,0x808*} | Steam {0x1c*,0x20*,0x3c*,0x48*,0x5c*,0x1d4*,0x2f8*,0x808*}
imm sets (* = in both, 14 common): GOG {0x3*,0x7*,0x8*,0x12*,0x14*,0x28*,0x31*,0x32*,0x200*,0x2f8*,0x7b8*,0xfffffcff*,0xffffff7f*,0xfffffffe*} | Steam {0x3*,0x7*,0x8*,0x12*,0x14*,0x28*,0x31*,0x32*,0x200*,0x2f8*,0x7b8*,0xfffffcff*,0xffffff7f*,0xfffffffe*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x008e0a80: 16-byte aligned: YES
  008e0a80  55             PUSH EBP
  008e0a81  8B EC          MOV EBP,ESP
  008e0a83  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0a80
string literals (0):
direct callees (4):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x008E0F70  (unnamed)
  0x004109C0  CSWGuiManager::PopModalPanel
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 9):
  +0x20 [dec 32]  x7  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via ECX,EDX
  +0x48 [dec 72]  x4  via EAX,ECX
  +0x5C [dec 92]  x3  via EAX,EDX
  +0x1D4 [dec 468]  x3  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,EDX
  +0x2F8 [dec 760]  x2  via EAX,EDX
  +0x808 [dec 2056]  x1  via EDX
  +0x8E0C38 [dec 9309240]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0a80 FUN_008e0a80 size=413 ret=8 callers=0 callees=4

void __thiscall FUN_008e0a80(int param_1,undefined4 param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  
  if (param_3 != 0) {
    switch(param_2) {
    case 0x28:
    case 0x2e:
      FUN_004122a0(0);
      FUN_004109c0();
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xffffff7f;
      *(uint *)(param_1 + 0x48) = *(uint *)(param_1 + 0x48) & 0xfffffcff | 0x200;
      break;
    case 0x2a:
      FUN_004122a0(0);
      *(undefined4 *)(param_1 + 0x808) = 1;
      FUN_008e0f70(param_1 + 0x7b8);
      break;
    case 0x2d:
      if ((*(int *)(param_1 + 0x20) != 0) &&
         (iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))(), iVar2 != 0)) {
        FUN_004122a0(3);
        iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))();
        uVar1 = *(uint *)(iVar2 + 0x1d4);
        iVar2 = (**(code **)(**(int **)(param_1 + 0x20) + 0x5c))();
        *(uint *)(iVar2 + 0x1d4) = *(uint *)(iVar2 + 0x1d4) & 0xfffffffe | (uint)((uVar1 & 1) == 0);
      }
      break;
    case 0x39:
      (**(code **)(*(int *)(param_1 + 0x2f8) + 0x3c))(0x31,1);
      break;
    case 0x3a:
      (**(code **)(*(int *)(param_1 + 0x2f8) + 0x3c))(0x32,1);
    }
  }
  FUN_0040eaa0(param_2,param_3);
  return;
}
```

### decoy: 0x008e07b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e07b0: 16-byte aligned: YES
  008e07b0  55             PUSH EBP
  008e07b1  8B EC          MOV EBP,ESP
  008e07b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008e07b0
string literals (0):
direct callees (10):
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0041AC10  (unnamed)
  0x007D5230  (unnamed)
  0x0041A3D0  CSWGuiButton::Destructor
  0x0041D190  (unnamed)
  0x0074AC30  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

## Card B16-1-34

GOG function: CSWGuiInGameGameplay::OnMouse  gog 0x005e5b30  callconv=(blank)  param_size_bytes=4  gog_size=178  gog_retn=4
Candidate Steam address: 0x008e1080 (size 178, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: rotation:0x005e5bf0:0.83>0.65
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0fc0:STR0/CALLEE0/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5a70 CSWGuiInGameGameplay::OnKeyMap -> 0x008e0fc0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5bf0 CSWGuiInGameGameplay::OnDifficultyLeft -> 0x008e1140  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00571d20 CSWGuiOptionsMouse::Constructor -> Steam ?  in candidate callees: n/a
  0x0090e9b0 CSWGuiManager::AddPanel -> Steam 0x00410530  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1c*,0x50*} | Steam {0x1c*,0x50*}
imm sets (* = in both, 4 common): GOG {0x3*,0x4*,0x20*,0x147c*} | Steam {0x3*,0x4*,0x20*,0x147c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e1080: 16-byte aligned: YES
  008e1080  55             PUSH EBP
  008e1081  8B EC          MOV EBP,ESP
  008e1083  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e1080
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x007DC410  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 2):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1080 FUN_008e1080 size=178 ret=4 callers=0 callees=3

void __thiscall FUN_008e1080(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_30;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094fa7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar1 = FUN_00919723(0x147c,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_30 = 0;
    }
    else {
      local_30 = FUN_007dc410(*(undefined4 *)(param_1 + 0x1c));
    }
    local_8 = 0xffffffff;
    FUN_00410530(local_30,3,1);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008e0fc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0fc0: 16-byte aligned: YES
  008e0fc0  55             PUSH EBP
  008e0fc1  8B EC          MOV EBP,ESP
  008e0fc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x008e0fc0
string literals (0):
direct callees (3):
  0x00919723  operator new
  0x008FFF20  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
struct offsets touched (top 2):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-35

GOG function: CSWGuiInGameCharacter::SetLabelBonusPenaltyColour  gog 0x00599990  callconv=(blank)  param_size_bytes=8  gog_size=86  gog_retn=8
Candidate Steam address: 0x00850090 (size 86, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00850010:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599910 CSWGuiInGameCharacter::DoAutoLevelup -> 0x00850010  gate_class=AUTO
- GOG-order neighbour after:  0x005999f0 CSWGuiInGameCharacter::RotateCharacter -> 0x008500f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xf0*} | Steam {0x8*,0xf0*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00850090: 16-byte aligned: YES
  00850090  55             PUSH EBP
  00850091  8B EC          MOV EBP,ESP
  00850093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00850090
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00850090 FUN_00850090 size=86 ret=8 callers=1 callees=1

void FUN_00850090(undefined4 param_1,int param_2)

{
  if (param_2 < 1) {
    if (param_2 < 0) {
      FUN_00417010(&DAT_00a7fdd0);
    }
    else {
      FUN_00417010(&DAT_00a7fe24);
    }
  }
  else {
    FUN_00417010(&DAT_00a7fe48);
  }
  return;
}
```

### decoy: 0x00850010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00850010: 16-byte aligned: YES
  00850010  55             PUSH EBP
  00850011  8B EC          MOV EBP,ESP
  00850013  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00850010
string literals (0):
direct callees (5):
  0x0073F750  ClientApp::GetGuiInGame
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006BEDF0  CSWSCreatureStats::AutoLevelup
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX
  +0x68 [dec 104]  x1  via ECX
  +0xA0 [dec 160]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EDX
```

## Card B16-1-36

GOG function: CServerAIMaster::ClearEventQueue  gog 0x0064da00  callconv=(blank)  param_size_bytes=0  gog_size=91  gog_retn=0
Candidate Steam address: 0x0051dfe0 (size 91, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Events::ClearAll | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: UnloadModule 0x5315e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051e220:STR0/CALLEE0/SIG0.367/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064d1b0 CServerAIMaster::UpdateState -> 0x0051d790  gate_class=AUTO
- GOG-order neighbour after:  0x0064da60 CServerAIMaster::AddObject -> 0x0051e040  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x00650700 CServerAIMaster::DeleteEventData -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007b4960 CServerExoAppInternal::UnloadModule -> Steam 0x005315e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x10*,0x14*} | Steam {0x10*,0x14*}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x54*} | Steam {0x4*,0xc*,0x54*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051dfe0: 16-byte aligned: YES
  0051dfe0  55             PUSH EBP
  0051dfe1  8B EC          MOV EBP,ESP
  0051dfe3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051dfe0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00520CE0  Events::FreePayload(type,ptr)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051dfe0 FUN_0051dfe0 size=91 ret=0 callers=2 callees=4

void __fastcall FUN_0051dfe0(undefined4 param_1)

{
  int iVar1;
  
  while( true ) {
    iVar1 = FUN_007a1780(param_1);
    if (iVar1 != 0) break;
    iVar1 = FUN_007f2a90();
    FUN_00520ce0(*(undefined4 *)(iVar1 + 0x10),*(undefined4 *)(iVar1 + 0x14));
    FUN_009196fd(iVar1);
  }
  return;
}
```

### decoy: 0x0051e220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051e220: 16-byte aligned: YES
  0051e220  55             PUSH EBP
  0051e221  8B EC          MOV EBP,ESP
  0051e223  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051e220
string literals (0):
direct callees (8):
  0x0051E350  (unnamed)
  0x00919723  operator new
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0051AFE0  CWorldTimer::CompareWorldTimes
  0x005210D0  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

## Card B16-1-37

GOG function: CSWGlobalVariableTable::ReadTableWithoutCatalogue  gog 0x00652550  callconv=(blank)  param_size_bytes=8  gog_size=1110  gog_retn=8
Candidate Steam address: 0x00655b00 (size 2481, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGlobalVariableTable::ReadTableWithCat | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 5 | A-CALLEE 1 | A-SIG 0.76 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00655690:STR5/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651ed0 CSWGlobalVariableTable::WriteTable -> 0x00655010  gate_class=AUTO
- GOG-order neighbour after:  0x006529c0 CSWGlobalVariableTable::ReadTableWithCatalogue -> 0x00655b00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (14), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006248c0 CResGFF::GetList -> Steam 0x00718760  in candidate callees: yes
  0x00624970 CResGFF::GetListCount -> Steam 0x00718810  in candidate callees: yes
  0x006249f0 CResGFF::GetListElement -> Steam 0x00718890  in candidate callees: yes
  0x006250f0 CResGFF::ReadFieldCExoString -> Steam 0x00718f90  in candidate callees: yes
  0x006257a0 CResGFF::ReadFieldVOID -> Steam 0x00719640  in candidate callees: yes
  ... 6 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): 'String'; 'ValString'; 'ValLocation'; 'ValNumber' ...
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x16*,0xde29*,0xe214*,0xeb74*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3*,0x5*,0x8*,0xc*,0xe*,0x18*,0x64*,0x71*,0x3e8*,0x93d,0x960*,0xe4c,0x3fff*,...(+1)} | Steam {0x2*,0x3*,0x4,0x5*,0x6,0x7,0x8*,0x9,0xa,0xb,0xc*,0xd,0xe*,0xf,...(+9)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00655b00: 16-byte aligned: YES
  00655b00  55             PUSH EBP
  00655b01  8B EC          MOV EBP,ESP
  00655b03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00655b00
string literals (13):
  0x00998654  at 00655bc7  'ValBoolean'
  0x00998684  at 00655beb  'CatBoolean'
  0x00998720  at 00655d37  "ReadTableWithCat(): BOOLEAN '%s' won't fit!"
  0x00998648  at 00655dbf  'ValNumber'
  0x00998678  at 00655de6  'CatNumber'
  0x009986F4  at 00655f32  "ReadTableWithCat(): NUMBER '%s' won't fit!"
  0x0099863C  at 00655f95  'ValLocation'
  0x0099866C  at 00655fbc  'CatLocation'
  0x009986C4  at 00656108  "ReadTableWithCat(): LOCATION '%s' won't fit!"
  0x00998630  at 0065618b  'ValString'
  0x00998628  at 0065623a  'String'
  0x00998660  at 006562a4  'CatString'
direct callees (24):
  0x0091D67E  (unnamed)
  0x00733540  CExoString::Constructor
  0x00653DD0  (unnamed)
  0x0091C630  (unnamed)
  0x00719640  CResGFF::ReadFieldVOID
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00733B60  (unnamed)
  0x006545B0  GlobalVars Find thiscall(table, CExoString* name) RET 4 (AddBoolean 0x654430 has no dup check)
  0x00654430  CSWGlobalVariableTable::AddBoolean(name)
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00654490  CSWGlobalVariableTable::AddNumber(name)
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00655b00 FUN_00655b00 size=2481 ret=8 callers=1 callees=24

void __thiscall FUN_00655b00(int param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int local_ee0;
  undefined1 local_ec0 [8];
  undefined1 local_eb8 [8];
  undefined1 local_eb0 [8];
  undefined1 local_ea8 [8];
  undefined1 local_ea0 [8];
  undefined1 local_e98 [8];
  undefined1 local_e90 [8];
  undefined1 local_e88 [8];
  undefined1 local_e80 [8];
  undefined1 local_e78 [8];
  undefined1 local_e70 [8];
  int local_e68;
  undefined1 local_e64 [8];
  undefined1 local_e5c [8];
  undefined1 local_e54 [8];
  undefined4 local_e4c;
  undefined1 local_e48 [120];
  int local_dd0;
  undefined1 local_dcc [4];
  undefined4 local_dc8[600 = byte +0x960];
  undefined1 local_468 [8];
  undefined1 local_460 [40];
  undefined1 local_438 [1000];
  undefined1 local_50 [4];
  undefined1 local_4c [20];
  int local_38;
  int local_34;
  undefined1 local_2d;
  undefined4 local_2c;
  undefined1 local_28 [20];
  uint local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00959011;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = uVar2;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_dc8,0x18,100,FUN_0053e060,FUN_00623320);
  local_8 = 0;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_460,8,5,FUN_00733540,FUN_00733780);
  local_8._0_1_ = 1;
  FUN_00733540(uVar2);
  local_8 = CONCAT31(local_8._1_3_,2);
  local_2c = 0;
  local_2d = 0;
  FUN_00653dd0();
  _memset [GHIDRA-AUTO-LABEL: unreliable](local_e48,0,0x71);
  _memset [GHIDRA-AUTO-LABEL: unreliable](local_438,0,1000);
  FUN_00719640(param_3,local_e48,0x71,"ValBoolean",&local_38,&local_2d);
  if (local_38 != 0) {
    FUN_00718760(local_28,param_3,"CatBoolean");
    local_34 = 0;
    while( true ) {
      iVar3 = FUN_00718810(local_28);
      if (iVar3 <= local_34) break;
      FUN_00718890(local_50,local_28,local_34);
      FUN_00733570(&DAT_00986925);
      local_8._0_1_ = 3;
      uVar4 = FUN_00718f90(local_e80,local_50,&DAT_0099353c,&local_38,local_e78);
      local_8._0_1_ = 4;
      FUN_007337c0(uVar4);
      local_8._0_1_ = 3;
      FUN_00733780();
      local_8 = CONCAT31(local_8._1_3_,2);
      FUN_00733780();
      if (local_38 != 0) {
        iVar3 = FUN_00733b60(&DAT_00986925);
/* ... 175 more lines truncated by --max-lines */
```

### decoy: 0x00655690 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00655690: 16-byte aligned: YES
  00655690  55             PUSH EBP
  00655691  8B EC          MOV EBP,ESP
  00655693  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00655690
string literals (5):
  0x00998654  at 00655736  'ValBoolean'
  0x00998648  at 00655758  'ValNumber'
  0x0099863C  at 0065577d  'ValLocation'
  0x00998630  at 006557a4  'ValString'
  0x00998628  at 00655853  'String'
direct callees (14):
  0x0091D67E  (unnamed)
  0x0091C630  (unnamed)
  0x00719640  CResGFF::ReadFieldVOID
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00654670  (unnamed)
  0x006546C0  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x009196EE  __security_check_cookie
struct offsets touched (top 5):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0xC [dec 12]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via ECX,EDX
  +0x14 [dec 20]  x2  via ECX,EDX
```

## Card B16-1-38

GOG function: CSWGuiInGameGameplay::SetDifficultyText  gog 0x005e5d30  callconv=(blank)  param_size_bytes=0  gog_size=391  gog_retn=0
Candidate Steam address: 0x008e1280 (size 391, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x005e5c70:0.63>0.39
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e11c0:STR0/CALLEE0/SIG0.312/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5c70 CSWGuiInGameGameplay::OnDifficultyRight -> 0x008e11c0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5ec0 CSWGuiInGameGameplay::SetDescription -> 0x008e1410  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
  0x00915270 CSWGuiTextParams::SetStrRef -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x24*,0x48*,0x80*,0x130*,0x2b74*} | Steam {0x24*,0x48*,0x80*,0x130*,0x2b74*}
imm sets (* = in both, 6 common): GOG {0x2*,0x3c*,0xea0*,0xef8*,0x10c8*,0xfffffffd*} | Steam {0x2*,0x3c*,0xea0*,0xef8*,0x10c8*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008e1280: 16-byte aligned: YES
  008e1280  55             PUSH EBP
  008e1281  8B EC          MOV EBP,ESP
  008e1283  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e1280
string literals (0):
direct callees (4):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00416EA0  (unnamed)
struct offsets touched (top 5):
  +0x48 [dec 72]  x8  via EAX,ECX,EDX
  +0x2B74 [dec 11124]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x2  via EAX,EDX
  +0x130 [dec 304]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1280 FUN_008e1280 size=391 ret=0 callers=4 callees=4

void __fastcall FUN_008e1280(int param_1)

{
  uint uVar1;
  undefined1 local_24 [8];
  int local_1c;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0097da58;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570(&DAT_00997610);
  local_8 = 0;
  local_1c = FUN_0071bcf0(*(undefined4 *)(param_1 + 0x2b74),local_24,&local_14);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_1c != 0) {
    FUN_00416ea0(local_14);
  }
  *(uint *)(param_1 + 0x1110) = *(uint *)(param_1 + 0x1110) & 0xfffffffd | 2;
  *(uint *)(param_1 + 0xf40) = *(uint *)(param_1 + 0xf40) & 0xfffffffd | 2;
  if (*(int *)(param_1 + 0x2b74) == 0) {
    *(uint *)(param_1 + 0x1110) = *(uint *)(param_1 + 0x1110) & 0xfffffffd;
  }
  else if (*(int *)(param_1 + 0x2b74) ==
           *(int *)(*(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x80) + 0x24) + -2) {
    *(uint *)(param_1 + 0xf40) = *(uint *)(param_1 + 0xf40) & 0xfffffffd;
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008e11c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e11c0: 16-byte aligned: YES
  008e11c0  55             PUSH EBP
  008e11c1  8B EC          MOV EBP,ESP
  008e11c3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e11c0
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 7):
  +0x2B74 [dec 11124]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x80 [dec 128]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B16-1-39

GOG function: CSWGlobalVariableTable::FindIdentifierIndex  gog 0x00651470  callconv=(blank)  param_size_bytes=4  gog_size=190  gog_retn=4
Candidate Steam address: 0x006544f0 (size 85, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x006545b0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651410 CSWGlobalVariableTable::AddIdentifierString -> 0x00654550  gate_class=REVIEW
- GOG-order neighbour after:  0x00651530 CSWGlobalVariableTable::GetBit -> 0x00654670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00607070 CExoString::CompareNoCase_2 -> Steam ?  in candidate callees: n/a
  0x006510d0 CSWGlobalVariableTable::Hash -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 10): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0xeba4}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x18,0x93d} | Steam {0x2,0x4*,0x8,0x64}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x006544f0: 16-byte aligned: YES
  006544f0  55             PUSH EBP
  006544f1  8B EC          MOV EBP,ESP
  006544f3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006544f0
string literals (0):
direct callees (1):
  0x006542D0  (unnamed)
struct offsets touched (top 1):
  +0xEBA4 [dec 60324]  x4  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 006544f0 FUN_006544f0 size=85 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_006544f0(int param_1,undefined4 param_2)

{
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  if (*(int *)(param_1 + 0xeba4) < 100) {
    local_8 = FUN_006542d0(param_2,2,*(undefined4 *)(param_1 + 0xeba4));
    *(int *)(param_1 + 0xeba4) = *(int *)(param_1 + 0xeba4) + 1;
  }
  return local_8;
}
```

### decoy: 0x006545b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006545b0: 16-byte aligned: YES
  006545b0  55             PUSH EBP
  006545b1  8B EC          MOV EBP,ESP
  006545b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006545b0
string literals (0):
direct callees (2):
  0x00654210  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
struct offsets touched (top 0):
```

## Card B16-1-40

GOG function: CAurTextureBasic::GetController  gog 0x00815aa0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x004233f0 (size 138, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CAurTexture | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00423480:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00815700 CAurTextureBasic::Constructor -> 0x00423000  gate_class=AUTO
- GOG-order neighbour after:  0x00815ac0 Global::CAurTextureBasic_scalar_deleting_destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x34} | Steam {0x10}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004233f0: 16-byte aligned: YES
  004233f0  55             PUSH EBP
  004233f1  8B EC          MOV EBP,ESP
  004233f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x004233f0
string literals (0):
direct callees (4):
  0x004741E0  SafePointer_T_::operator=
  0x004276A0  ReleaseTextureProxy (delete if back-ref count 0)
  0x00473E80  (unnamed)
  0x00473EB0  (unnamed)
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 004233f0 FUN_004233f0 size=138 ret=0 callers=4 callees=4

void __fastcall FUN_004233f0(undefined4 *param_1)

{
  undefined4 uVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_00948f06;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CAurTexture::vftable;
  local_8 = 1;
  uVar1 = param_1[4 = byte +0x10];
  FUN_004741e0(0);
  FUN_004276a0(uVar1,uVar2);
  local_8 = local_8 & 0xffffff00;
  FUN_00473e80();
  local_8 = 0xffffffff;
  FUN_00473eb0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00423480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00423480: 16-byte aligned: YES
  00423480  55             PUSH EBP
  00423481  8B EC          MOV EBP,ESP
  00423483  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00423480
string literals (0):
direct callees (9):
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x009196FD  operator delete
  0x0091C6AA  operator delete[]
  0x00425DB0  CAurTextureBasic::GetMemoryUsage
  0x00425160  CAurTextureBasic::FreeImage
  0x00423930  TextureLists Remove
  0x00476620  (unnamed)
  0x00449170  (unnamed)
  0x004233F0  CAurTexture (proxy) dtor
struct offsets touched (top 9):
  +0x40 [dec 64]  x4  via EAX,ECX
  +0x48 [dec 72]  x3  via EAX,EDX
  +0x34 [dec 52]  x1  via ECX
  +0x38 [dec 56]  x1  via ECX
  +0x3C [dec 60]  x1  via ECX
  +0x58 [dec 88]  x1  via ECX
  +0xC6 [dec 198]  x1  via EDX
  +0xC8 [dec 200]  x1  via EAX
  +0xD6 [dec 214]  x1  via ECX
```

## Card B16-1-41

GOG function: CSWGlobalVariableTable::SetBit  gog 0x00651580  callconv=(blank)  param_size_bytes=12  gog_size=118  gog_retn=12
Candidate Steam address: 0x006546c0 (size 118, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00654670:STR0/CALLEE0/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651530 CSWGlobalVariableTable::GetBit -> 0x00654670  gate_class=REVIEW
- GOG-order neighbour after:  0x00651600 CSWGlobalVariableTable::GetValueBoolean -> 0x00654670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x3*,0x7*,0xc*,0x10*} | Steam {0x3*,0x7*,0xc*,0x10*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x006546c0: 16-byte aligned: YES
  006546c0  55             PUSH EBP
  006546c1  8B EC          MOV EBP,ESP
  006546c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x006546c0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 006546c0 FUN_006546c0 size=118 ret=12 callers=4 callees=0

void FUN_006546c0(int param_1,int param_2,int param_3)

{
  int iVar1;
  byte bVar2;
  
  iVar1 = (int)(param_2 + (param_2 >> 0x1f & 7U)) >> 3;
  bVar2 = (byte)(1 << (7U - ((char)param_2 + (char)iVar1 * -8) & 0x1f));
  if (param_3 == 0) {
    *(byte *)(param_1 + iVar1) = *(byte *)(param_1 + iVar1) & ~bVar2;
  }
  else {
    *(byte *)(param_1 + iVar1) = *(byte *)(param_1 + iVar1) | bVar2;
  }
  return;
}
```

### decoy: 0x00654670 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00654670: 16-byte aligned: YES
  00654670  55             PUSH EBP
  00654671  8B EC          MOV EBP,ESP
  00654673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00654670
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B16-1-42

GOG function: CServerAIMaster::AddEventAbsoluteTime  gog 0x0064dc40  callconv=(blank)  param_size_bytes=24  gog_size=290  gog_retn=24
Candidate Steam address: 0x0051e220 (size 290, retn 24)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=EventQueue::AddEvent | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: CompareWorldTimes 0x51afe0; callers: ValidateLevelUp 0x6b9640); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051dfe0:STR0/CALLEE0/SIG0.367/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064db90 CServerAIMaster::AddEventDeltaTime -> 0x0051e170  gate_class=AUTO
- GOG-order neighbour after:  0x0064dd70 Global::LogEventAdd  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x0064dd70 Global::LogEventAdd -> Steam ?  in candidate callees: n/a
  0x00733b00 CExoLinkedListInternal::AddTail_2 -> Steam ?  in candidate callees: n/a
  0x00777c00 CExoLinkedList_CAuroraModel_::AddBefore -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
  0x0080b4c0 CWorldTimer::CompareWorldTimes -> Steam 0x0051afe0  in candidate callees: yes
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x006f5ac0 CSWSCreatureStats::ValidateLevelUp -> Steam 0x006b9640  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*}
imm sets (* = in both, 5 common): GOG {0x4*,0x10*,0x18*,0x1c*,0x54*} | Steam {0x4*,0x10*,0x18*,0x1c*,0x54*}

### abi
```
GOG param bytes: 24; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(24/4)*4 = 24 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=24 ; asm RET = 24
Stack Slot Match: TRUE (GOG 24 bytes -> x86 slots 24; ASM RET 24)
```

### prologue
```
prologue for 0x0051e220: 16-byte aligned: YES
  0051e220  55             PUSH EBP
  0051e221  8B EC          MOV EBP,ESP
  0051e223  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051e220
string literals (0):
direct callees (8):
  0x0051E350  (unnamed)
  0x00919723  operator new
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0051AFE0  CWorldTimer::CompareWorldTimes
  0x005210D0  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x10048 [dec 65608]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051e220 FUN_0051e220 size=290 ret=24 callers=5 callees=8

undefined4
FUN_0051e220(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6)

{
  undefined4 *puVar1;
  int iVar2;
  int local_10;
  int local_c;
  undefined4 *local_8;
  
  if (DAT_00a7d884 != 0) {
    FUN_0051e350(param_3,param_4,param_5,param_6);
  }
  puVar1 = (undefined4 *)FUN_00919723(0x18);
  *puVar1 = param_1;
  puVar1[1 = byte +0x4] = param_2;
  puVar1[2 = byte +0x8] = param_3;
  puVar1[3 = byte +0xc] = param_4;
  puVar1[4 = byte +0x10] = param_5;
  puVar1[5 = byte +0x14] = param_6;
  local_c = 0;
  local_10 = FUN_007a1720();
  while ((local_10 != 0 && (local_c == 0))) {
    local_8 = (undefined4 *)FUN_00561430(local_10);
    iVar2 = FUN_0051afe0(*local_8,local_8[1 = byte +0x4],param_1,param_2);
    if (iVar2 == 1) {
      FUN_005210d0(puVar1,local_10);
      local_c = 1;
    }
    else {
      FUN_0058c370(&local_10);
    }
  }
  if (local_c == 0) {
    FUN_0054aa30(puVar1);
  }
  return 1;
}
```

### decoy: 0x0051dfe0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051dfe0: 16-byte aligned: YES
  0051dfe0  55             PUSH EBP
  0051dfe1  8B EC          MOV EBP,ESP
  0051dfe3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051dfe0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x00520CE0  Events::FreePayload(type,ptr)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card B16-1-43

GOG function: CServerAIMaster::DeleteEventData  gog 0x00650700  callconv=(blank)  param_size_bytes=8  gog_size=491  gog_retn=8
Candidate Steam address: 0x00520ce0 (size 491, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Events::FreePayload | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: DeleteScriptSituation 0x6fdbb0; callers: UpdateState 0x51d790); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00521060:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00650010 CServerAIEventNode::LoadNode  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00650960 CServerAIBodyBagInfo::SaveBodyBag  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549cb0 CSWCCMessageData::Destructor -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00640dd0 CVirtualMachine::DeleteScriptSituation -> Steam 0x006fdbb0  in candidate callees: yes
  0x00771380 CSWSCombatAttackData::Destructor -> Steam ?  in candidate callees: n/a
  0x0077aa40 CSWCCMessageData::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0064d1b0 CServerAIMaster::UpdateState -> Steam 0x0051d790  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x19*,0x1c*,0x50*} | Steam {0x4*,0x8*,0x19*,0x1c*,0x50*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00520ce0: 16-byte aligned: YES
  00520ce0  55             PUSH EBP
  00520ce1  8B EC          MOV EBP,ESP
  00520ce3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00520ce0
string literals (0):
direct callees (6):
  0x0058CCD0  (unnamed)
  0x009196FD  operator delete
  0x005E25D0  (unnamed)
  0x005E1AF0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x006FDBB0  CVirtualMachine::DeleteScriptSituation
struct offsets touched (top 1):
  +0x520F20 [dec 5377824]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00520ce0 FUN_00520ce0 size=491 ret=8 callers=2 callees=6

void FUN_00520ce0(undefined4 param_1,int param_2)

{
  if (param_2 != 0) {
    switch(param_1) {
    case 1:
      FUN_006fdbb0(param_2);
      break;
    case 8:
    case 0x13:
      if (param_2 != 0) {
        FUN_00733780();
        FUN_009196fd(param_2);
      }
      break;
    case 10:
      if (param_2 != 0) {
        FUN_005e1af0();
        FUN_009196fd(param_2);
      }
      break;
    case 0x15:
      if (param_2 != 0) {
        FUN_0058ccd0();
        FUN_009196fd(param_2);
      }
      break;
    case 0x16:
      if (param_2 != 0) {
        FUN_005e25d0();
        FUN_009196fd(param_2);
      }
      break;
    case 0x18:
      if (param_2 != 0) {
        FUN_005e1af0();
        FUN_009196fd(param_2);
      }
      break;
    case 0x1a:
      if (param_2 != 0) {
        FUN_005e1af0();
        FUN_009196fd(param_2);
      }
    }
  }
  return;
}
```

### decoy: 0x00521060 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00521060: 16-byte aligned: YES
  00521060  55             PUSH EBP
  00521061  8B EC          MOV EBP,ESP
  00521063  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00521060
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via ECX
```

## Card B16-1-44

GOG function: CSWGuiInGameCharacter::Destructor  gog 0x00597770  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0084de70 (size 120, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0084def0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00595eb0 CSWGuiInGameCharacter::Constructor -> 0x0073b4d0  gate_class=REVIEW
- GOG-order neighbour after:  0x005977a0 CSWGuiBlackenedLabel::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00597870 CSWGuiInGameCharacter::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x60}

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

### decoy: 0x0084def0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084def0: 16-byte aligned: YES
  0084def0  55             PUSH EBP
  0084def1  8B EC          MOV EBP,ESP
  0084def3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084def0
string literals (0):
direct callees (2):
  0x0084DF70  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B16-1-45

GOG function: CSWGuiInGameGameplay::OnEnterDiff  gog 0x005e6020  callconv=(blank)  param_size_bytes=0  gog_size=94  gog_retn=4
Candidate Steam address: 0x008e1570 (size 94, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e15d0:STR0/CALLEE0/SIG0.917/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005e5ec0 CSWGuiInGameGameplay::SetDescription -> 0x008e1410  gate_class=REVIEW
- GOG-order neighbour after:  0x005e6080 CSWGuiInGameGameplay::OnExitDiff -> 0x008e15d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> Steam ?  in candidate callees: n/a
  0x005e5ec0 CSWGuiInGameGameplay::SetDescription -> Steam ?  in candidate callees: n/a
  0x009153e0 CSWGuiTextParams::SetColor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
imm sets (* = in both, 6 common): GOG {0x4*,0x6*,0x8*,0xea0*,0xa519*,0xffffffbf*} | Steam {0x4*,0x6*,0x8*,0xea0*,0xa519*,0xffffffbf*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008e1570: 16-byte aligned: YES
  008e1570  55             PUSH EBP
  008e1571  8B EC          MOV EBP,ESP
  008e1573  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e1570
string literals (0):
direct callees (3):
  0x00417010  (unnamed)
  0x008E1280  (unnamed)
  0x008E1410  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1570 FUN_008e1570 size=94 ret=4 callers=0 callees=3

void __fastcall FUN_008e1570(int param_1)

{
  FUN_00417010(&DAT_00a7fdac);
  *(uint *)(param_1 + 0xed8) = *(uint *)(param_1 + 0xed8) & 0xffffffbf | 0x40;
  FUN_008e1280();
  FUN_008e1410(0xa519);
  return;
}
```

### decoy: 0x008e15d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e15d0: 16-byte aligned: YES
  008e15d0  55             PUSH EBP
  008e15d1  8B EC          MOV EBP,ESP
  008e15d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e15d0
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

## Card B16-1-46

GOG function: CSWGuiInGameGameplay::OnReverseIngame  gog 0x005e58f0  callconv=(blank)  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x008e0e40 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0d40:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5870 CSWGuiInGameGameplay::OnReverse -> 0x008e0dc0  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5970 CSWGuiInGameGameplay::OnCombatMovement -> 0x008e0ec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 3 common): GOG {0x4*,0x1c*,0xffffffef*} | Steam {0x4*,0x1c*,0xffffffef*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0e40: 16-byte aligned: YES
  008e0e40  55             PUSH EBP
  008e0e41  8B EC          MOV EBP,ESP
  008e0e43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0e40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0e40 FUN_008e0e40 size=122 ret=4 callers=0 callees=1

void __thiscall FUN_008e0e40(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar2 = FUN_0072fb00(param_1);
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = FUN_0072fb00();
    *(uint *)(iVar2 + 8) = *(uint *)(iVar2 + 8) & 0xffffffef | (uint)((uVar1 >> 4 & 1) == 0) << 4;
  }
  return;
}
```

### decoy: 0x008e0d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0d40: 16-byte aligned: YES
  008e0d40  55             PUSH EBP
  008e0d41  8B EC          MOV EBP,ESP
  008e0d43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0d40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-47

GOG function: CSWGlobalVariableTable::GetBit  gog 0x00651530  callconv=(blank)  param_size_bytes=8  gog_size=78  gog_retn=8
Candidate Steam address: 0x00654670 (size 78, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x006546c0:STR0/CALLEE0/SIG0.3/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00651470 CSWGlobalVariableTable::FindIdentifierIndex -> 0x006544f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00651580 CSWGlobalVariableTable::SetBit -> 0x006546c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x3*,0x7*,0x8*,0x10*} | Steam {0x3*,0x7*,0x8*,0x10*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00654670: 16-byte aligned: YES
  00654670  55             PUSH EBP
  00654671  8B EC          MOV EBP,ESP
  00654673  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00654670
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00654670 FUN_00654670 size=78 ret=8 callers=4 callees=0

bool FUN_00654670(int param_1,int param_2)

{
  int iVar1;
  
  iVar1 = (int)(param_2 + (param_2 >> 0x1f & 7U)) >> 3;
  return ((int)(uint)*(byte *)(param_1 + iVar1) >> (7U - ((char)param_2 + (char)iVar1 * -8) & 0x1f)
         & 1U) != 0;
}
```

### decoy: 0x006546c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x006546c0: 16-byte aligned: YES
  006546c0  55             PUSH EBP
  006546c1  8B EC          MOV EBP,ESP
  006546c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x006546c0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B16-1-48

GOG function: CSWGuiInGameGameplay::SetDescription  gog 0x005e5ec0  callconv=(blank)  param_size_bytes=4  gog_size=341  gog_retn=4
Candidate Steam address: 0x008e1410 (size 341, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e15d0:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> 0x008e1280  gate_class=REVIEW
- GOG-order neighbour after:  0x005e6020 CSWGuiInGameGameplay::OnEnterDiff -> 0x008e1570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x008d03c0 Global::GetResolutionTextScalar -> Steam ?  in candidate callees: n/a
  0x00915270 CSWGuiTextParams::SetStrRef -> Steam ?  in candidate callees: n/a
  0x00915c80 CSWGuiText::GetFontHeight -> Steam ?  in candidate callees: n/a
  0x0091b2a0 CSWGuiListBox::ClearItems -> Steam ?  in candidate callees: n/a
  0x0091c940 CSWGuiListBox::SetActiveControl -> Steam ?  in candidate callees: n/a
  0x0091dd30 CSWGuiListBox::AddControls_2 -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0xc*,0x68*} | Steam {0x4*,0x8*,0xc*,0x68*}
imm sets (* = in both, 7 common): GOG {0x4*,0x2c*,0x68*,0x6c*,0x140*,0x158*,0x2f8*} | Steam {0x4*,0x2c*,0x68*,0x6c*,0x140*,0x158*,0x2f8*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e1410: 16-byte aligned: YES
  008e1410  55             PUSH EBP
  008e1411  8B EC          MOV EBP,ESP
  008e1413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008e1410
string literals (0):
direct callees (10):
  0x00416EA0  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x0041D290  (unnamed)
  0x00919F10  _strlen
  0x004178B0  (unnamed)
  0x00479AB0  (unnamed)
  0x0091C860  _ftol2_sse
  0x0041FD90  (unnamed)
  0x0041E9A0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 4):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x68 [dec 104]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1410 FUN_008e1410 size=341 ret=4 callers=2 callees=10

void __thiscall FUN_008e1410(int param_1,undefined4 param_2)

{
  uint uVar1;
  float10 fVar2;
  size_t local_34;
  int local_30;
  char *local_2c;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009610d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00416ea0(param_2);
  FUN_007335f0(param_1 + 0x158);
  local_8 = 0;
  FUN_0041d290(uVar1);
  local_20 = *(undefined4 *)(param_1 + 0x6c);
  local_1c = *(undefined4 *)(param_1 + 0x70);
  local_18 = *(undefined4 *)(param_1 + 0x74);
  local_14 = *(undefined4 *)(param_1 + 0x78);
  if (local_2c == (char *)0x0) {
    local_34 = 0;
  }
  else {
    local_34 = _strlen [GHIDRA-AUTO-LABEL: unreliable](local_2c);
  }
  if ((int)local_34 < 1) {
    local_14 = FUN_004178b0();
  }
  else {
    local_14 = (**(code **)(*(int *)(param_1 + 0x140) + 8))();
  }
  fVar2 = (float10)FUN_00479ab0();
  local_24 = (float)fVar2;
  local_14 = FUN_0091c860();
  (**(code **)(*(int *)(param_1 + 0x68) + 4))(&local_20);
  local_30 = param_1 + 0x68;
  FUN_0041fd90(&local_30,1,0,0,0);
  FUN_0041e9a0(param_1 + 0x68,0);
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x008e15d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e15d0: 16-byte aligned: YES
  008e15d0  55             PUSH EBP
  008e15d1  8B EC          MOV EBP,ESP
  008e15d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e15d0
string literals (0):
direct callees (1):
  0x00417010  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via ECX,EDX
```

## Card B16-1-49

GOG function: CServerAIMaster::AppendToScriptsRun  gog 0x0077e650  callconv=(blank)  param_size_bytes=4  gog_size=192  gog_retn=4
Candidate Steam address: 0x0064e710 (size 508, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0064e910:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0077e600 CSWVirtualMachineCommands::RunScriptCallback -> 0x00669020  trusted
- GOG-order neighbour after:  0x0077e710 CSWVirtualMachineCommands::CopyGameDefinedStructure -> 0x00669130  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: no
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: no
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0077e600 CSWVirtualMachineCommands::RunScriptCallback -> Steam 0x00669020  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x30,0x34,0x64,0x68,0x6c,0x74,0x78,0x7c,0x80,0x84,0x88,...(+1)}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x2c,0x60} | Steam {0x4*,0x30,0x8c,0x9c,0xac}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0064e710: 16-byte aligned: YES
  0064e710  55             PUSH EBP
  0064e711  8B EC          MOV EBP,ESP
  0064e713  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0064e710
string literals (0):
direct callees (1):
  0x005EA190  Global::AurPartTriggerDestroy
struct offsets touched (top 10):
  +0x34 [dec 52]  x16  via EAX,ECX,EDX
  +0x4 [dec 4]  x6  via EAX
  +0x30 [dec 48]  x4  via EAX,ECX
  +0x8 [dec 8]  x3  via EDX
  +0xC [dec 12]  x3  via ECX
  +0x64 [dec 100]  x1  via ECX
  +0x68 [dec 104]  x1  via EDX
  +0x6C [dec 108]  x1  via ECX
  +0x74 [dec 116]  x1  via EAX
  +0x78 [dec 120]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0064e710 FUN_0064e710 size=508 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_0064e710(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_10;
  undefined4 local_c;
  
  iVar1 = FUN_005ea190(param_2);
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else {
    if (*(int *)(param_2 + 0x34) == 0) {
      local_30 = 0;
    }
    else {
      local_30 = *(int *)(param_2 + 0x30) + *(int *)(*(int *)(param_2 + 0x34) + 0x74);
    }
    *(int *)(param_1 + 0x88) = local_30;
    if (*(int *)(param_2 + 0x34) == 0) {
      local_34 = 0;
    }
    else {
      local_34 = *(undefined4 *)(*(int *)(param_2 + 0x34) + 0x6c);
    }
    *(undefined4 *)(param_1 + 0xd4) = local_34;
    if (*(int *)(param_2 + 0x34) == 0) {
      local_10 = 0;
    }
    else {
      local_10 = *(undefined4 *)(*(int *)(param_2 + 0x34) + 0x78);
    }
    if (*(int *)(param_2 + 0x34) == 0) {
      local_c = 0;
    }
    else {
      local_c = *(int *)(param_2 + 0x30) + *(int *)(*(int *)(param_2 + 0x34) + 0x7c);
    }
    if (*(int *)(param_1 + 0x90) == 0) {
      *(int *)(param_1 + 0x8c) = local_c;
      *(undefined4 *)(param_1 + 0x90) = local_10;
      *(undefined4 *)(param_1 + 0x94) = local_10;
      *(undefined4 *)(param_1 + 0x98) = 1;
    }
    if (*(int *)(param_2 + 0x34) == 0) {
      local_1c = 0;
    }
    else {
      local_1c = *(undefined4 *)(*(int *)(param_2 + 0x34) + 0x64 [dec 100]);
    }
    if (*(int *)(param_2 + 0x34) == 0) {
      local_18 = 0;
    }
    else {
      local_18 = *(int *)(param_2 + 0x30) + *(int *)(*(int *)(param_2 + 0x34) + 0x68);
    }
    if (*(int *)(param_1 + 0xb0) == 0) {
      *(int *)(param_1 + 0xac) = local_18;
      *(undefined4 *)(param_1 + 0xb0) = local_1c;
      *(undefined4 *)(param_1 + 0xb4) = local_1c;
      *(undefined4 *)(param_1 + 0xb8) = 1;
    }
    if (*(int *)(param_2 + 0x34) == 0) {
      local_28 = 0;
    }
    else {
      local_28 = *(undefined4 *)(*(int *)(param_2 + 0x34) + 0x80);
    }
    if (*(int *)(param_2 + 0x34) == 0) {
      local_24 = 0;
    }
    else {
/* ... 12 more lines truncated by --max-lines */
```

### decoy: 0x0064e910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0064e910: 16-byte aligned: YES
  0064e910  55             PUSH EBP
  0064e911  8B EC          MOV EBP,ESP
  0064e913  81 EC BC 00 00 00 SUB ESP,0xbc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0064e910
string literals (0):
direct callees (10):
  0x005EA490  (unnamed)
  0x00734010  CExoString::CStr
  0x0091CAEB  (unnamed)
  0x005EA7B0  (unnamed)
  0x0091F3EF  (unnamed)
  0x0091A825  (unnamed)
  0x009205FC  (unnamed)
  0x0091A338  (unnamed)
  0x0091C5B3  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 8):
  +0x88 [dec 136]  x1  via EDX
  +0x8C [dec 140]  x1  via ECX
  +0x90 [dec 144]  x1  via ECX
  +0x9C [dec 156]  x1  via EAX
  +0xA0 [dec 160]  x1  via EAX
  +0xAC [dec 172]  x1  via EAX
  +0xB0 [dec 176]  x1  via ECX
  +0xD4 [dec 212]  x1  via EDX
```

## Card B16-1-50

GOG function: CSWGuiDialogLetterbox::`scalar  gog 0x005af1f0  callconv=(blank)  param_size_bytes=4  gog_size=114  gog_retn=4
Candidate Steam address: 0x008baaa0 (size 114, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008ba940:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005af0d0 CSWGuiDialogLetterbox::Constructor -> 0x008ba980  gate_class=AUTO
- GOG-order neighbour after:  0x005af270 CSWGuiDialogLetterbox::SetExtentSpecial -> 0x008bab20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x90*} | Steam {0x4*,0x90*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008baaa0: 16-byte aligned: YES
  008baaa0  55             PUSH EBP
  008baaa1  8B EC          MOV EBP,ESP
  008baaa3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008baaa0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x0040E860  CSWGuiPanel::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008baaa0 FUN_008baaa0 size=114 ret=4 callers=0 callees=3

undefined4 __thiscall FUN_008baaa0(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00979d58;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0xffffffff;
  FUN_0040e860();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x008ba940 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008ba940: 16-byte aligned: YES
  008ba940  55             PUSH EBP
  008ba941  8B EC          MOV EBP,ESP
  008ba943  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008ba940
string literals (0):
direct callees (1):
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 3):
  +0x1C [dec 28]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x2A36 [dec 10806]  x1  via ECX
```

## Card B16-1-51

GOG function: CSWGuiInGameCharacter::Constructor  gog 0x00595eb0  callconv=(blank)  param_size_bytes=4  gog_size=6331  gog_retn=4
Candidate Steam address: 0x0073b4d0 (size 149, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.01 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0073b470:STR0/CALLEE0/SIG0.008/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00595cf0 CSWGuiBlackenedLabel::Draw  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00597770 CSWGuiInGameCharacter::Destructor -> 0x0084de70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (18), mapped to Steam through trusted rows only:
  0x004102b0 CSWCCreature::Constructor -> Steam ?  in candidate callees: n/a
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  0x005324a0 Global::AspyrAddButtonIcon -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x0090cb30 CSWGuiPanel::Constructor -> Steam 0x0040e5a0  in candidate callees: no
  0x0090d2c0 CSWGuiPanel::StartLoadFromLayout -> Steam 0x0040ee40  in candidate callees: no
  ... 10 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (61): 'Acamerahook'; 'LBL_BAR6'; 'LBL_BAR5'; 'LBL_BAR4' ...
disp sets (* = in both, 0 common): GOG {0x38,0x44,0x48,0x70,0x74,0x1d0,0x1d4,0x3c98,0x3c9c,0x3e88,0x3e8c,0x4b6c,0x4b70,0x4b74} | Steam {0x4}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+165)} | Steam {0x4*,0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0073b4d0: 16-byte aligned: YES
  0073b4d0  55             PUSH EBP
  0073b4d1  8B EC          MOV EBP,ESP
  0073b4d3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073b4d0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x00919F10  _strlen
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0073b4d0 FUN_0073b4d0 size=149 ret=4 callers=1 callees=4

size_t __thiscall FUN_0073b4d0(int param_1,uint param_2)

{
  size_t sVar1;
  undefined4 local_10;
  int local_c;
  uint local_8;
  
  if (param_2 < *(uint *)(param_1 + 4)) {
    local_10 = FUN_007a1720();
    local_c = FUN_00521060();
    local_8 = 0;
    while (local_c != 0) {
      if (local_8 == param_2) {
        if (*(int *)(local_c + 4) == 0) {
          return 0;
        }
        sVar1 = _strlen [GHIDRA-AUTO-LABEL: unreliable](*(char **)(local_c + 4));
        return sVar1;
      }
      local_c = FUN_0058c370(&local_10);
      local_8 = local_8 + 1;
    }
  }
  return 0;
}
```

### decoy: 0x0073b470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073b470: 16-byte aligned: YES
  0073b470  55             PUSH EBP
  0073b471  8B EC          MOV EBP,ESP
  0073b473  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073b470
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x007337C0  CExoString::operator=
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B16-1-52

GOG function: CSWGuiInGameCharacter::OnPanelAdded  gog 0x00597fc0  callconv=(blank)  param_size_bytes=0  gog_size=35  gog_retn=0
Candidate Steam address: 0x0084e6c0 (size 35, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: OnPanelAdded 0x40f3c0)
- best decoy (neighbour/other candidate) scores: 0x0084df70:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00597870 CSWGuiInGameCharacter::Destructor_2 -> 0x0084df70  gate_class=AUTO
- GOG-order neighbour after:  0x00597ff0 CSWGuiInGameCharacter::SetStats -> 0x0084e6f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00597ff0 CSWGuiInGameCharacter::SetStats -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x0090d840 CSWGuiPanel::OnPanelAdded -> Steam 0x0040f3c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0084e6c0: 16-byte aligned: YES
  0084e6c0  55             PUSH EBP
  0084e6c1  8B EC          MOV EBP,ESP
  0084e6c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084e6c0
string literals (0):
direct callees (3):
  0x0084E6F0  CSWGuiInGameCharacter::SetStats (CanLevelUp at 0x84F221 -> button visibility)
  0x00623320  CSWSEncounter::AIUpdate
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0084e6c0 FUN_0084e6c0 size=35 ret=0 callers=0 callees=3

void __fastcall FUN_0084e6c0(undefined4 param_1)

{
  FUN_0084e6f0(param_1);
  FUN_00623320();
  FUN_0040f3c0();
  return;
}
```

### decoy: 0x0084df70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084df70: 16-byte aligned: YES
  0084df70  55             PUSH EBP
  0084df71  8B EC          MOV EBP,ESP
  0084df73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0084df70
string literals (0):
direct callees (9):
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0041A3D0  CSWGuiButton::Destructor
  0x00418130  (unnamed)
  0x0041AC10  (unnamed)
  0x0074AC30  (unnamed)
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 1):
  +0x4B6C [dec 19308]  x2  via ECX,EDX
```

## Card B16-1-53

GOG function: CSWGuiInGameCharacter::Draw  gog 0x005994b0  callconv=(blank)  param_size_bytes=4  gog_size=68  gog_retn=4
Candidate Steam address: 0x0084fbb0 (size 68, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: Draw 0x40ec90); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0084f900:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00599200 CSWGuiInGameCharacter::HandleInputEvent -> 0x0084f900  gate_class=AUTO
- GOG-order neighbour after:  0x00599500 CSWGuiInGameCharacter::AttachModel -> 0x0084fc00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00597ff0 CSWGuiInGameCharacter::SetStats -> Steam ?  in candidate callees: n/a
  0x0090d110 CSWGuiPanel::Draw -> Steam 0x0040ec90  in candidate callees: yes
  0x00919270 CSWGuiSlider::SetCurValue -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3ad8*} | Steam {0x3ad8*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x3a60*} | Steam {0x4*,0x8*,0x3a60*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0084fbb0: 16-byte aligned: YES
  0084fbb0  55             PUSH EBP
  0084fbb1  8B EC          MOV EBP,ESP
  0084fbb3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0084fbb0
string literals (0):
direct callees (3):
  0x0084E6F0  CSWGuiInGameCharacter::SetStats (CanLevelUp at 0x84F221 -> button visibility)
  0x0041B1D0  (unnamed)
  0x0040EC90  CSWGuiPanel::Draw
struct offsets touched (top 1):
  +0x3AD8 [dec 15064]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0084fbb0 FUN_0084fbb0 size=68 ret=4 callers=0 callees=3

void __thiscall FUN_0084fbb0(int param_1,undefined4 param_2)

{
  FUN_0084e6f0();
  FUN_0041b1d0(*(undefined4 *)(param_1 + 0x3ad8));
  FUN_0040ec90(param_2);
  return;
}
```

### decoy: 0x0084f900 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0084f900: 16-byte aligned: YES
  0084f900  55             PUSH EBP
  0084f901  8B EC          MOV EBP,ESP
  0084f903  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0084f900
string literals (0):
direct callees (13):
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0073F750  ClientApp::GetGuiInGame
  0x00740FC0  CClientExoApp::GetCharacterChangeInProgress
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0084FCD0  CSWGuiInGameCharacter::ShowLevelUpGUI thiscall(panel, int) RET 4; callers 0x84F9BD, 0x7CB8E6
  0x0084FE20  CSWGuiInGameCharacter::StartAutoLevelup
  0x00740B80  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
  0x007CA060  CGuiInGame::HideSWInGameGui
  0x0073FEE0  CClientExoApp::SetInputClass
  0x008500F0  (unnamed)
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 4):
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x12C [dec 300]  x2  via ECX,EDX
  +0x84FAF4 [dec 8714996]  x1  via EAX
```

## Card B16-1-54

GOG function: CServerAIMaster::AdjustTargetAndWitnessReputations  gog 0x0064eac0  callconv=(blank)  param_size_bytes=0  gog_size=68  gog_retn=12
Candidate Steam address: 0x0051f0a0 (size 68, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: OnApplyDeath 0x597d90)
- best decoy (neighbour/other candidate) scores: 0x0051f920:STR0/CALLEE0/SIG0.033/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0064ea90 CServerAIMaster::OnItemPropertyRemoved -> 0x0051f070  gate_class=AUTO
- GOG-order neighbour after:  0x0064f340 CServerAIMaster::LoadReputationAdjustments -> 0x0051f920  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0073a680 CSWSEffectListHandler::OnApplyDeath -> Steam 0x00597d90  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x118*} | Steam {0xc*,0x118*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x0051f0a0: 16-byte aligned: YES
  0051f0a0  55             PUSH EBP
  0051f0a1  8B EC          MOV EBP,ESP
  0051f0a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0051f0a0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051f0a0 FUN_0051f0a0 size=68 ret=12 callers=2 callees=0

void FUN_0051f0a0(void)

{
  return;
}
```

### decoy: 0x0051f920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051f920: 16-byte aligned: YES
  0051f920  55             PUSH EBP
  0051f921  8B EC          MOV EBP,ESP
  0051f923  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0051f920
string literals (11):
  0x00992E80  at 0051f995  'PERSONALREP'
  0x00992E74  at 0051f9cc  'FACTIONREP'
  0x00992E6C  at 0051fa03  'WITFRIA'
  0x00992E64  at 0051fa3a  'WITFRIB'
  0x00992E5C  at 0051fa71  'WITFRIC'
  0x00992E54  at 0051faa8  'WITNEUA'
  0x00992E4C  at 0051fadf  'WITNEUB'
  0x00992E44  at 0051fb16  'WITNEUC'
  0x00992E3C  at 0051fb4d  'WITENEA'
  0x00992E34  at 0051fb8d  'WITENEB'
  0x00992E2C  at 0051fbcd  'WITENEC'
direct callees (6):
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x0084A640  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 6):
  +0x80 [dec 128]  x12  via EAX,ECX,EDX
  +0x84 [dec 132]  x12  via EAX,ECX,EDX
  +0x88 [dec 136]  x12  via EAX,ECX,EDX
  +0x8C [dec 140]  x12  via EAX,ECX,EDX
  +0x94 [dec 148]  x1  via EDX
  +0x130 [dec 304]  x1  via EAX
```

## Card B16-1-55

GOG function: CSWGuiInGameCharacter::RotateCharacterLeft  gog 0x00599c30  callconv=(blank)  param_size_bytes=0  gog_size=543  gog_retn=4
Candidate Steam address: 0x008500f0 (size 566, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00850330:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005999f0 CSWGuiInGameCharacter::RotateCharacter -> 0x008500f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00599e50 CSWGuiInGameCharacter::RotateCharacterRight -> 0x00850330  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0080d4b0 Global::YawPitchRoll -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x88*,0x4b6c*} | Steam {0x4*,0x8*,0xc*,0x88*,0x4b6c*}
imm sets (* = in both, 4 common): GOG {0x4*,0x10*,0x30*,0x44*,0x5c} | Steam {0x4*,0x10*,0x30*,0x44*,0x60}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x008500f0: 16-byte aligned: YES
  008500f0  55             PUSH EBP
  008500f1  8B EC          MOV EBP,ESP
  008500f3  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008500f0
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008500f0 FUN_008500f0 size=566 ret=4 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_008500f0(int param_1,int param_2)

{
  int iVar1;
  float *pfVar2;
  float *pfVar3;
  undefined4 local_64;
  float *local_5c;
  float local_20;
  float local_1c;
  float local_18;
  undefined1 local_14 [16];
  
  if (param_2 == 0) {
    local_64 = _DAT_00987704;
  }
  else {
    local_64 = _DAT_0098bd88;
  }
  iVar1 = *(int *)(param_1 + 0x4b6c);
  pfVar3 = (float *)(iVar1 + 0x30);
  pfVar2 = (float *)FUN_00515620(local_14,local_64,0,0);
  if ((DAT_00a0f190 == 0) ||
     ((*pfVar2 != (float)_DAT_00986b50 && (*pfVar2 != (float)_DAT_00986d98)))) {
    local_20 = (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490) *
               *pfVar3 + (pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8] - pfVar2[3 = byte +0xc] * *pfVar2) * (float)_DAT_00987490 *
                         *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_1c = (pfVar2[3 = byte +0xc] * *pfVar2 + pfVar2[1 = byte +0x4] * pfVar2[2 = byte +0x8]) * (float)_DAT_00987490 * *pfVar3 +
               (1.0 - (pfVar2[3 = byte +0xc] * pfVar2[3 = byte +0xc] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x34) +
               (pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc] - pfVar2[1 = byte +0x4] * *pfVar2) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x38);
    local_18 = (pfVar2[1 = byte +0x4] * pfVar2[3 = byte +0xc] - pfVar2[2 = byte +0x8] * *pfVar2) * (float)_DAT_00987490 * *pfVar3 +
               (pfVar2[1 = byte +0x4] * *pfVar2 + pfVar2[2 = byte +0x8] * pfVar2[3 = byte +0xc]) * (float)_DAT_00987490 *
               *(float *)(iVar1 + 0x34) +
               (1.0 - (pfVar2[2 = byte +0x8] * pfVar2[2 = byte +0x8] + pfVar2[1 = byte +0x4] * pfVar2[1 = byte +0x4]) * (float)_DAT_00987490) *
               *(float *)(iVar1 + 0x38);
  }
  else {
    local_20 = *pfVar3;
    local_1c = *(float *)(iVar1 + 0x34);
    local_18 = *(float *)(iVar1 + 0x38);
  }
  local_5c = &local_20;
  (**(code **)(**(int **)(param_1 + 0x4b6c) + 0x88))(local_5c);
  return;
}
```

### decoy: 0x00850330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00850330: 16-byte aligned: YES
  00850330  55             PUSH EBP
  00850331  8B EC          MOV EBP,ESP
  00850333  83 EC 5C       SUB ESP,0x5c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00850330
string literals (0):
direct callees (1):
  0x00515620  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
  +0x4B6C [dec 19308]  x3  via EAX,EDX
  +0x88 [dec 136]  x1  via EDX
```

## Card B16-1-56

GOG function: CSWGuiDialogLetterbox::SetExtentSpecial  gog 0x005af270  callconv=(blank)  param_size_bytes=16  gog_size=263  gog_retn=16
Candidate Steam address: 0x008bab20 (size 263, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiDialogLetterbox::ResizeToText | G7 mac: pass(0.78) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008bac30:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005af1f0 CSWGuiDialogLetterbox::`scalar -> 0x008baaa0  gate_class=REVIEW
- GOG-order neighbour after:  0x005af380 CSWGuiDialogLetterbox::SetExtent -> 0x008bac30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x6c*,0x6e*,0x74*,0x88*,0x8c*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x6c*,0x6e*,0x74*,0x88*,0x8c*}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x10*,0x14*,0x68*} | Steam {0x2*,0x4*,0x10*,0x14*,0x68*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x008bab20: 16-byte aligned: YES
  008bab20  55             PUSH EBP
  008bab21  8B EC          MOV EBP,ESP
  008bab23  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008bab20
string literals (0):
direct callees (0):
struct offsets touched (top 9):
  +0x6C [dec 108]  x4  via EAX,ECX,EDX
  +0x74 [dec 116]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x8 [dec 8]  x2  via EAX
  +0xC [dec 12]  x2  via EAX
  +0x88 [dec 136]  x2  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x6E [dec 110]  x1  via EDX
  +0x8C [dec 140]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 008bab20 FUN_008bab20 size=263 ret=16 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_008bab20(int param_1,undefined4 param_2,int param_3,undefined4 param_4,int param_5)

{
  if (*(int *)(param_1 + 0x88) == 1) {
    param_3 = 0;
    _DAT_00a309b4 = param_5;
  }
  else if (*(int *)(param_1 + 0x88) == 2) {
    param_5 = *(short *)(*(int *)(param_1 + 0x1c) + 0x6e) - param_3;
    _DAT_00a309b8 = param_3;
  }
  if (*(int *)(param_1 + 0x8c) == 0) {
    if (*(int *)(param_1 + 0x88) == 1) {
      if (*(int *)(param_1 + 0x74) < param_5) {
        *(int *)(param_1 + 0x74) = param_5;
      }
    }
    else if ((*(int *)(param_1 + 0x88) == 2) && (param_3 < *(int *)(param_1 + 0x6c))) {
      param_3 = *(int *)(param_1 + 0x6c) - param_3;
      *(int *)(param_1 + 0x6c) = *(int *)(param_1 + 0x6c) - param_3;
      *(int *)(param_1 + 0x74) = *(int *)(param_1 + 0x74) + param_3;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x68) = param_2;
    *(int *)(param_1 + 0x6c) = param_3;
    *(undefined4 *)(param_1 + 0x70) = param_4;
    *(int *)(param_1 + 0x74) = param_5;
    *(undefined4 *)(param_1 + 4) = param_2;
    *(int *)(param_1 + 8) = param_3;
    *(undefined4 *)(param_1 + 0xc) = param_4;
    *(int *)(param_1 + 0x10) = param_5;
  }
  return;
}
```

### decoy: 0x008bac30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008bac30: 16-byte aligned: YES
  008bac30  55             PUSH EBP
  008bac31  8B EC          MOV EBP,ESP
  008bac33  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008bac30
string literals (0):
direct callees (1):
  0x0040E980  CSWGuiPanel::SetExtent
struct offsets touched (top 6):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x1C [dec 28]  x1  via EDX
  +0x6E [dec 110]  x1  via ECX
  +0x88 [dec 136]  x1  via EAX
```

## Card B16-1-57

GOG function: CSWGuiInGameGameplay::OnDifficultyLeft  gog 0x005e5bf0  callconv=(blank)  param_size_bytes=4  gog_size=127  gog_retn=4
Candidate Steam address: 0x008e1140 (size 127, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e11c0:STR0/CALLEE0/SIG0.411/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5b30 CSWGuiInGameGameplay::OnMouse -> 0x008e1080  gate_class=REVIEW
- GOG-order neighbour after:  0x005e5c70 CSWGuiInGameGameplay::OnDifficultyRight -> 0x008e11c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x005e5d30 CSWGuiInGameGameplay::SetDifficultyText -> Steam ?  in candidate callees: n/a
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x1c*,0x50*,0x2b74*} | Steam {0x4*,0x1c*,0x50*,0x2b74*}
imm sets (* = in both, 2 common): GOG {0x4*,0x14*} | Steam {0x4*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e1140: 16-byte aligned: YES
  008e1140  55             PUSH EBP
  008e1141  8B EC          MOV EBP,ESP
  008e1143  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e1140
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 4):
  +0x2B74 [dec 11124]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e1140 FUN_008e1140 size=127 ret=4 callers=0 callees=3

void __thiscall FUN_008e1140(int param_1,int param_2)

{
  undefined1 uVar1;
  int iVar2;
  
  if ((*(int *)(param_2 + 0x50) != 0) && (0 < *(int *)(param_1 + 0x2b74))) {
    *(int *)(param_1 + 0x2b74) = *(int *)(param_1 + 0x2b74) + -1;
    uVar1 = *(undefined1 *)(param_1 + 0x2b74);
    iVar2 = FUN_0072fb00();
    *(undefined1 *)(iVar2 + 4) = uVar1;
    FUN_008e1280();
    FUN_004122a0(1);
  }
  return;
}
```

### decoy: 0x008e11c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e11c0: 16-byte aligned: YES
  008e11c0  55             PUSH EBP
  008e11c1  8B EC          MOV EBP,ESP
  008e11c3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e11c0
string literals (0):
direct callees (3):
  0x0072FB00  CClientExoApp::GetClientOptions
  0x008E1280  (unnamed)
  0x004122A0  CSWGuiManager::PlayGuiSound
struct offsets touched (top 7):
  +0x2B74 [dec 11124]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x50 [dec 80]  x1  via EAX
  +0x80 [dec 128]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
```

## Card B16-1-58

GOG function: CExoMoviePlayerInternal::Destructor  gog 0x0091fbe0  callconv=(blank)  param_size_bytes=0  gog_size=291  gog_retn=0
Candidate Steam address: 0x0040cb20 (size 220, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x0091fe60:0.71>0.51
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040c9f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0091fa70 CExoMoviePlayerInternal::Constructor -> 0x0040c880  gate_class=REVIEW
- GOG-order neighbour after:  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> 0x0040cb20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044dda0 CExoArrayList_CExoString_::Destructor -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x00610cf0 CExoCriticalSection::Destructor -> Steam ?  in candidate callees: n/a
  0x0091fd10 CExoMoviePlayerInternal::ShutDown -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x38,0x44} | Steam {0x24,0x48,0x4c,0x50,0x64}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x8,0x14,0x28,0x40,0x68,0x90,0x103} | Steam {0x84}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0040cb20: 16-byte aligned: YES
  0040cb20  55             PUSH EBP
  0040cb21  8B EC          MOV EBP,ESP
  0040cb23  81 EC 84 00 00 00 SUB ESP,0x84
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040cb20
string literals (0):
direct callees (0):
struct offsets touched (top 5):
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0x4C [dec 76]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EAX
  +0x50 [dec 80]  x2  via EAX,EDX
  +0x64 [dec 100]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040cb20 FUN_0040cb20 size=220 ret=0 callers=3 callees=5

void __fastcall FUN_0040cb20(int param_1)

{
  undefined1 local_84 [128];
  
  *(undefined4 *)(param_1 + 0x24) = 1;
  if (*(int *)(param_1 + 0x48) != 0) {
    _BinkGetSummary_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),local_84);
    _BinkPause_8 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48),1);
    if (*(int *)(param_1 + 0x64 [dec 100]) == 0) {
      _BinkClose_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x48));
      *(undefined4 *)(param_1 + 0x48) = 0;
    }
    if (*(int *)(param_1 + 0x4c) != 0) {
      _BinkBufferClose_4 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(param_1 + 0x4c));
      *(undefined4 *)(param_1 + 0x4c) = 0;
    }
    DestroyWindow(*(HWND *)(param_1 + 0x50));
    *(undefined4 *)(param_1 + 0x50) = 0;
    DAT_00a1b9e8 = 0;
  }
  *(undefined4 *)(param_1 + 0x24) = 0;
  return;
}
```

### decoy: 0x0040c9f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040c9f0: 16-byte aligned: YES
  0040c9f0  55             PUSH EBP
  0040c9f1  8B EC          MOV EBP,ESP
  0040c9f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040c9f0
string literals (0):
direct callees (5):
  0x0040CB20  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00739C90  (unnamed)
  0x0040DF30  (unnamed)
struct offsets touched (top 3):
  +0x44 [dec 68]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x38 [dec 56]  x2  via EAX,ECX
```

## Card B16-1-59

GOG function: CSWGuiInGameGameplay::OnCombatMovement  gog 0x005e5970  callconv=(blank)  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x008e0ec0 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0e40:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e58f0 CSWGuiInGameGameplay::OnReverseIngame -> 0x008e0e40  gate_class=REVIEW
- GOG-order neighbour after:  0x005e59f0 CSWGuiInGameGameplay::OnEnterSlot -> 0x008e0f40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 4 common): GOG {0x4*,0x5*,0x1c*,0xffffffdf*} | Steam {0x4*,0x5*,0x1c*,0xffffffdf*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0ec0: 16-byte aligned: YES
  008e0ec0  55             PUSH EBP
  008e0ec1  8B EC          MOV EBP,ESP
  008e0ec3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0ec0
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0ec0 FUN_008e0ec0 size=122 ret=4 callers=0 callees=1

void __thiscall FUN_008e0ec0(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar2 = FUN_0072fb00(param_1);
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = FUN_0072fb00();
    *(uint *)(iVar2 + 8) = *(uint *)(iVar2 + 8) & 0xffffffdf | (uint)((uVar1 >> 5 & 1) == 0) << 5;
  }
  return;
}
```

### decoy: 0x008e0e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0e40: 16-byte aligned: YES
  008e0e40  55             PUSH EBP
  008e0e41  8B EC          MOV EBP,ESP
  008e0e43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0e40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

## Card B16-1-60

GOG function: CSWGuiInGameGameplay::OnAutoSave  gog 0x005e5770  callconv=(blank)  param_size_bytes=4  gog_size=122  gog_retn=4
Candidate Steam address: 0x008e0cc0 (size 122, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x008e0d40:STR0/CALLEE0/SIG0.7/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005e5700 CSWGuiInGameGameplay::OnAutoLevelUp -> 0x008e0c50  gate_class=REVIEW
- GOG-order neighbour after:  0x005e57f0 CSWGuiInGameGameplay::OnInvertCam -> 0x008e0d40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0x50*} | Steam {0x4*,0x8*,0x50*}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x1c*,0xfffffffb*} | Steam {0x2*,0x4*,0x1c*,0xfffffffb*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008e0cc0: 16-byte aligned: YES
  008e0cc0  55             PUSH EBP
  008e0cc1  8B EC          MOV EBP,ESP
  008e0cc3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008e0cc0
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008e0cc0 FUN_008e0cc0 size=122 ret=4 callers=0 callees=1

void __thiscall FUN_008e0cc0(undefined4 param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  
  if (*(int *)(param_2 + 0x50) != 0) {
    iVar2 = FUN_0072fb00(param_1);
    uVar1 = *(uint *)(iVar2 + 8);
    iVar2 = FUN_0072fb00();
    *(uint *)(iVar2 + 8) = *(uint *)(iVar2 + 8) & 0xfffffffb | (uint)((uVar1 >> 2 & 1) == 0) << 2;
  }
  return;
}
```

### decoy: 0x008e0d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008e0d40: 16-byte aligned: YES
  008e0d40  55             PUSH EBP
  008e0d41  8B EC          MOV EBP,ESP
  008e0d43  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008e0d40
string literals (0):
direct callees (1):
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 3):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EAX
```
