# Review cards B9 chunk 4

Answer each card with one TSV row in `steam_db_batches/B9/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B9-4-01

GOG function: CServerExoAppInternal::Uninitialize  gog 0x007bda10  callconv=(blank)  param_size_bytes=0  gog_size=32  gog_retn=0
Candidate Steam address: 0x0053a7c0 (size 32, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: RunDeathSequence 0x7403a0)
- best decoy (neighbour/other candidate) scores: 0x0053a7e0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bd310 CServerExoAppInternal::TempDeathUpdate -> 0x0053a0c0  gate_class=AUTO
- GOG-order neighbour after:  0x007bda30 CServerExoAppInternal::TestSafeLocation -> 0x0053a7e0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040d860 CClientExoApp::RunDeathSequence -> Steam 0x007403a0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0053a7c0: 16-byte aligned: YES
  0053a7c0  55             PUSH EBP
  0053a7c1  8B EC          MOV EBP,ESP
  0053a7c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053a7c0
string literals (0):
direct callees (1):
  0x007403A0  CClientExoApp::RunDeathSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053a7c0 FUN_0053a7c0 size=32 ret=0 callers=1 callees=1
THUNK -> 0x007403A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_0053a7c0(undefined4 param_1)

{
  FUN_007403a0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
  return;
}
```

### decoy: 0x0053a7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053a7e0: 16-byte aligned: YES
  0053a7e0  55             PUSH EBP
  0053a7e1  8B EC          MOV EBP,ESP
  0053a7e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053a7e0
string literals (0):
direct callees (3):
  0x00537B70  (unnamed)
  0x00537BE0  (unnamed)
  0x0054B650  CSWSArea::TestSafeLocationPoint
struct offsets touched (top 2):
  +0x90 [dec 144]  x1  via EAX
  +0x380 [dec 896]  x1  via ECX
```

## Card B9-4-02

GOG function: CServerExoAppInternal::GetFirstPCObject  gog 0x007bce00  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x00539bb0 (size 100, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00539a90:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bcce0 Global::UpdateLoadScreen  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x007bce70 CServerExoAppInternal::UpdateAutoSaveTimer -> 0x00539c20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00539bb0: 16-byte aligned: YES
  00539bb0  55             PUSH EBP
  00539bb1  8B EC          MOV EBP,ESP
  00539bb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539bb0
string literals (0):
direct callees (2):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
struct offsets touched (top 3):
  +0x100F8 [dec 65784]  x3  via EAX,ECX,EDX
  +0x10064 [dec 65636]  x2  via EAX,EDX
  +0x38 [dec 56]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00539bb0 FUN_00539bb0 size=100 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_00539bb0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_007a1720();
  *(undefined4 *)(param_1 + 0x100f8) = uVar1;
  uVar1 = DAT_00994484;
  if ((*(int *)(param_1 + 0x100f8) != 0) &&
     (iVar2 = FUN_00561430(*(undefined4 *)(param_1 + 0x100f8)), uVar1 = DAT_00994484, iVar2 != 0)) {
    uVar1 = *(undefined4 *)(iVar2 + 0x38);
  }
  return uVar1;
}
```

### decoy: 0x00539a90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539a90: 16-byte aligned: YES
  00539a90  55             PUSH EBP
  00539a91  8B EC          MOV EBP,ESP
  00539a93  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539a90
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x100E4 [dec 65764]  x6  via ECX,EDX
  +0x100E8 [dec 65768]  x6  via ECX,EDX
  +0x100DC [dec 65756]  x5  via ECX,EDX
  +0x100E0 [dec 65760]  x5  via EAX,ECX,EDX
```

## Card B9-4-03

GOG function: CServerExoAppInternal::DoModuleEnterSaveCleanup  gog 0x007bfb10  callconv=(blank)  param_size_bytes=8  gog_size=413  gog_retn=8
Candidate Steam address: 0x0053cd70 (size 413, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 3 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'ModuleName'@00994440; 'SaveGroup'@0099444c; 'DeleteSaveGroupOnEnter'@00994458; CALLEE 2 (callees: RemoveFile 0x7158e0; callers: StartNewModule 0x52fda0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053c890:STR0/CALLEE0/SIG0.277/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bfa90 CServerExoAppInternal::GetMoveToWorldTime -> 0x0053cc80  gate_class=REVIEW
- GOG-order neighbour after:  0x007bfcb0 CServerExoAppInternal::GetModulePreviouslyVisited -> 0x0053cf10  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x0061f240 CExoResMan::RemoveFile -> Steam 0x007158e0  in candidate callees: yes
  0x00620c70 C2DA::GetCExoStringEntry_3 -> Steam ?  in candidate callees: n/a
  0x00621570 C2DA::GetINTEntry -> Steam ?  in candidate callees: n/a
  0x00621790 C2DA::GetINTEntry_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
GOG strings (4): 'GAMEINPROGRESS:'; 'ModuleName'; 'SaveGroup'; 'DeleteSaveGroupOnEnter'
disp sets (* = in both, 1 common): GOG {0x24*} | Steam {0x24*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0x58*,0x809*} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0x58*,0x809*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0053cd70: 16-byte aligned: YES
  0053cd70  55             PUSH EBP
  0053cd71  8B EC          MOV EBP,ESP
  0053cd73  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053cd70
string literals (4):
  0x00994458  at 0053cd98  'DeleteSaveGroupOnEnter'
  0x00993E84  at 0053cddc  'GAMEINPROGRESS:'
  0x0099444C  at 0053ce23  'SaveGroup'
  0x00994440  at 0053ce65  'ModuleName'
direct callees (8):
  0x00733570  CExoString::CExoString_char
  0x0071BAD0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x0071BCF0  C2DA::GetINTEntry_2
  0x0071B1D0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x007158E0  CExoResMan::RemoveFile
struct offsets touched (top 1):
  +0x24 [dec 36]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053cd70 FUN_0053cd70 size=413 ret=8 callers=1 callees=8

void FUN_0053cd70(undefined4 param_1,int param_2)

{
  uint uVar1;
  undefined4 uVar2;
  undefined1 local_58 [8];
  undefined1 local_50 [8];
  int local_48;
  undefined1 local_44 [8];
  int local_3c;
  undefined1 local_38 [8];
  int local_30;
  int local_24;
  undefined1 local_20 [8];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f4f0;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733570("DeleteSaveGroupOnEnter");
  local_8 = 0;
  local_30 = FUN_0071bad0(param_1,local_38,&local_14);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  if (local_30 != 0) {
    FUN_00733570("GAMEINPROGRESS:");
    local_8 = 1;
    FUN_00733540();
    local_8._0_1_ = 2;
    for (local_24 = 0; local_24 < *(int *)(param_2 + 0x24); local_24 = local_24 + 1) {
      FUN_00733570("SaveGroup");
      local_8._0_1_ = 3;
      local_3c = FUN_0071bcf0(local_24,local_44,&local_18);
      local_8._0_1_ = 2;
      FUN_00733780();
      if ((local_3c != 0) && (local_18 == local_14)) {
        FUN_00733570("ModuleName");
        local_8._0_1_ = 4;
        local_48 = FUN_0071b1d0(local_24,local_50,local_20);
        local_8._0_1_ = 2;
        FUN_00733780();
        if (local_48 != 0) {
          uVar2 = FUN_00733df0(local_58,local_20);
          local_8._0_1_ = 5;
          FUN_007158e0(uVar2,0x809);
          local_8._0_1_ = 2;
          FUN_00733780();
        }
      }
    }
    local_8 = CONCAT31(local_8._1_3_,1);
    FUN_00733780();
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0053c890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053c890: 16-byte aligned: YES
  0053c890  55             PUSH EBP
  0053c891  8B EC          MOV EBP,ESP
  0053c893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053c890
string literals (20):
  0x00994430  at 0053c897  'DISPLAYSPENDING'
  0x00994420  at 0053c8b1  'ITEMRECEIVED'
  0x00994414  at 0053c8ce  'ITEMLOST'
  0x0099440C  at 0053c8eb  'JOURNAL'
  0x00994400  at 0053c908  'LIGHTSHIFT'
  0x009943F4  at 0053c921  'DARKSHIFT'
  0x009943EC  at 0053c93a  'CREDITS'
  0x009943E0  at 0053c952  'CREDITSNET'
  0x009943D0  at 0053c987  'STEALTHXP'
  0x009943C0  at 0053c99f  'SOUNDPENDING'
  0x009943B0  at 0053c9bb  'LEVELUPSOUND'
  0x009943A0  at 0053c9d8  'NEWQUESTSOUND'
direct callees (2):
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719BF0  CResGFF::WriteFieldINT
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EAX
  +0x5 [dec 5]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
```

## Card B9-4-04

GOG function: CServerExoAppInternal::IsOnActiveExclusionList  gog 0x007bc7f0  callconv=(blank)  param_size_bytes=4  gog_size=96  gog_retn=4
Candidate Steam address: 0x005395a0 (size 96, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005397c0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bc710 CServerExoAppInternal::RemoveFromExclusionList -> 0x005393b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007bc850 CServerExoAppInternal::GetActiveExclusionList -> 0x00539600  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007bc850 CServerExoAppInternal::GetActiveExclusionList -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x005395a0: 16-byte aligned: YES
  005395a0  55             PUSH EBP
  005395a1  8B EC          MOV EBP,ESP
  005395a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005395a0
string literals (0):
direct callees (4):
  0x00539600  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005395a0 FUN_005395a0 size=96 ret=4 callers=1 callees=4

undefined4 __thiscall FUN_005395a0(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int local_c;
  int local_8;
  
  local_8 = FUN_00539600(param_1);
  if (local_8 == 0) {
    uVar1 = 1;
  }
  else {
    local_c = FUN_007a1720();
    while (local_c != 0) {
      iVar2 = FUN_00561430(local_c);
      if (iVar2 == param_2) {
        return 1;
      }
      FUN_0058c370(&local_c);
    }
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x005397c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005397c0: 16-byte aligned: YES
  005397c0  55             PUSH EBP
  005397c1  8B EC          MOV EBP,ESP
  005397c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005397c0
string literals (0):
direct callees (1):
  0x006A2140  (unnamed)
struct offsets touched (top 2):
  +0x110 [dec 272]  x1  via ECX
  +0x10004 [dec 65540]  x1  via EAX
```

## Card B9-4-05

GOG function: CServerExoAppInternal::AddToExclusionList  gog 0x007bc600  callconv=(blank)  param_size_bytes=5  gog_size=260  gog_retn=8
Candidate Steam address: 0x005393b0 (size 260, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005394c0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bc120 CServerExoAppInternal::SetPauseState -> 0x00538ed0  gate_class=AUTO
- GOG-order neighbour after:  0x007bc710 CServerExoAppInternal::RemoveFromExclusionList -> 0x005393b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00733b00 CExoLinkedListInternal::AddTail_2 -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0xc*} | Steam {0x2*,0x8*,0xc*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005393b0: 16-byte aligned: YES
  005393b0  55             PUSH EBP
  005393b1  8B EC          MOV EBP,ESP
  005393b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005393b0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005393b0 FUN_005393b0 size=260 ret=8 callers=1 callees=4

void __thiscall FUN_005393b0(undefined4 param_1,int param_2,byte param_3)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (param_3 != 0) {
    if ((param_3 & 2) == 0) {
      if ((param_3 & 1) != 0) {
        local_8 = 0;
        local_c = FUN_007a1720(param_1);
        while (local_c != 0) {
          iVar1 = FUN_00561430(local_c);
          if (iVar1 == param_2) {
            local_8 = 1;
            break;
          }
          FUN_0058c370(&local_c);
        }
        if (local_8 == 0) {
          FUN_0054aa30(param_2);
        }
      }
    }
    else {
      local_8 = 0;
      local_c = FUN_007a1720(param_1);
      while (local_c != 0) {
        iVar1 = FUN_00561430(local_c);
        if (iVar1 == param_2) {
          local_8 = 1;
          break;
        }
        FUN_0058c370(&local_c);
      }
      if (local_8 == 0) {
        FUN_0054aa30(param_2);
      }
    }
  }
  return;
}
```

### decoy: 0x005394c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005394c0: 16-byte aligned: YES
  005394c0  55             PUSH EBP
  005394c1  8B EC          MOV EBP,ESP
  005394c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005394c0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

## Card B9-4-06

GOG function: CServerExoAppInternal::HasEnoughDiskSpaceForSaveGame  gog 0x007c0210  callconv=(blank)  param_size_bytes=0  gog_size=199  gog_retn=0
Candidate Steam address: 0x0053d470 (size 199, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: GetFreeDiskSpace 0x714de0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0053cad0:STR0/CALLEE0/SIG0.045/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bfcb0 CServerExoAppInternal::GetModulePreviouslyVisited -> 0x0053cf10  gate_class=REVIEW
- GOG-order neighbour after:  0x007c02e0 CExoLinkedList_CSWSPlayerLUOInventoryItem_::AddHead  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0061e740 CExoResMan::GetFreeDiskSpace -> Steam 0x00714de0  in candidate callees: yes
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'SAVES:'
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x20*,0x640*,0x4000*} | Steam {0x20*,0x640*,0x4000*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053d470: 16-byte aligned: YES
  0053d470  55             PUSH EBP
  0053d471  8B EC          MOV EBP,ESP
  0053d473  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053d470
string literals (1):
  0x00993E30  at 0053d4b4  'SAVES:'
direct callees (4):
  0x00733570  CExoString::CExoString_char
  0x00714DE0  CExoResMan::GetFreeDiskSpace
  0x00733780  CExoString::~CExoString
  0x0091D220  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053d470 FUN_0053d470 size=199 ret=0 callers=1 callees=4

/* WARNING: Removing unreachable block (ram,0x0053d50c) */

bool FUN_0053d470(void)

{
  uint uVar1;
  longlong lVar2;
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f588;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_18 = 0;
  local_14 = 0;
  local_20 = 0;
  local_1c = 0;
  FUN_00733570("SAVES:");
  local_8 = 0;
  FUN_00714de0(local_28,&local_18);
  local_8 = 0xffffffff;
  FUN_00733780(uVar1);
  lVar2 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](local_18,local_14,0x4000,0);
  ExceptionList = local_10;
  return 0x640 < lVar2;
}
```

### decoy: 0x0053cad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053cad0: 16-byte aligned: YES
  0053cad0  55             PUSH EBP
  0053cad1  8B EC          MOV EBP,ESP
  0053cad3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053cad0
string literals (0):
direct callees (2):
  0x007338D0  CExoString::operator=_2
  0x0091C630  (unnamed)
struct offsets touched (top 9):
  +0x40 [dec 64]  x1  via EAX
  +0x44 [dec 68]  x1  via ECX
  +0x45 [dec 69]  x1  via EDX
  +0x46 [dec 70]  x1  via EAX
  +0x48 [dec 72]  x1  via EDX
  +0x4A [dec 74]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
  +0x50 [dec 80]  x1  via ECX
  +0x54 [dec 84]  x1  via EDX
```

## Card B9-4-07

GOG function: CServerExoAppInternal::ClientToServerObjectId  gog 0x007bcf80  callconv=(blank)  param_size_bytes=4  gog_size=37  gog_retn=4
Candidate Steam address: 0x00539d30 (size 37, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00539d60:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bce70 CServerExoAppInternal::UpdateAutoSaveTimer -> 0x00539c20  gate_class=AUTO
- GOG-order neighbour after:  0x007bcfb0 CServerExoAppInternal::GetPlayerCreatureId -> 0x00539d60  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x7f000000*} | Steam {0x4*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00539d30: 16-byte aligned: YES
  00539d30  55             PUSH EBP
  00539d31  8B EC          MOV EBP,ESP
  00539d33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539d30
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00539d30 FUN_00539d30 size=37 ret=4 callers=3 callees=0

uint FUN_00539d30(uint param_1)

{
  if (param_1 == 0x7f000000) {
    param_1 = 0x7f000000;
  }
  else {
    param_1 = ~DAT_0099449c & param_1;
  }
  return param_1;
}
```

### decoy: 0x00539d60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539d60: 16-byte aligned: YES
  00539d60  55             PUSH EBP
  00539d61  8B EC          MOV EBP,ESP
  00539d63  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539d60
string literals (0):
direct callees (2):
  0x0051C620  (unnamed)
  0x00632A20  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

## Card B9-4-08

GOG function: CServerExoApp::InitiateModuleForPlayer  gog 0x0064c070  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c670 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c650:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c040 CServerExoApp::GetClientObjectByPlayerId -> 0x0051c620  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c090 CServerExoApp::MovePlayerToArea -> 0x0051c670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b8a10 CServerExoAppInternal::InitiateModuleForPlayer -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x0051c670: 16-byte aligned: YES
  0051c670  55             PUSH EBP
  0051c671  8B EC          MOV EBP,ESP
  0051c673  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c670
string literals (0):
direct callees (1):
  0x00535A10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c670 FUN_0051c670 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00535A10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c670(undefined4 param_1)

{
  FUN_00535a10(param_1);
  return;
}
```

### decoy: 0x0051c650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051c650: 16-byte aligned: YES
  0051c650  55             PUSH EBP
  0051c651  8B EC          MOV EBP,ESP
  0051c653  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051c650
string literals (0):
direct callees (1):
  0x005356D0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-4-09

GOG function: CServerExoAppInternal::AddCharListRequest  gog 0x007bca50  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x00539800 (size 31, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005397c0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca10 CServerExoAppInternal::GetDifficultyOption -> 0x005397c0  gate_class=AUTO
- GOG-order neighbour after:  0x007bcab0 CServerExoAppInternal::UpdateShutdownTimer -> 0x00539860  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007e44a0 CExoArrayList_int_::AddUnique -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x100ec*} | Steam {0x4*,0x100ec*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00539800: 16-byte aligned: YES
  00539800  55             PUSH EBP
  00539801  8B EC          MOV EBP,ESP
  00539803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539800
string literals (0):
direct callees (1):
  0x005210A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00539800 FUN_00539800 size=31 ret=4 callers=1 callees=1
THUNK -> 0x005210A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00539800(undefined4 param_1)

{
  FUN_005210a0(param_1);
  return;
}
```

### decoy: 0x005397c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005397c0: 16-byte aligned: YES
  005397c0  55             PUSH EBP
  005397c1  8B EC          MOV EBP,ESP
  005397c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005397c0
string literals (0):
direct callees (1):
  0x006A2140  (unnamed)
struct offsets touched (top 2):
  +0x110 [dec 272]  x1  via ECX
  +0x10004 [dec 65540]  x1  via EAX
```

## Card B9-4-10

GOG function: CServerExoAppInternal::GetModulePreviouslyVisited  gog 0x007bfcb0  callconv=(blank)  param_size_bytes=4  gog_size=1367  gog_retn=4
Candidate Steam address: 0x0053cf10 (size 1367, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR ':MODULES'@00993f2c; CALLEE 5 (callees: AddResourceDirectory 0x711690, RemoveResourceDirectory 0x711710, Exists 0x711ed0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053c890:STR0/CALLEE0/SIG0.306/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bfb10 CServerExoAppInternal::DoModuleEnterSaveCleanup -> 0x0053cd70  gate_class=REVIEW
- GOG-order neighbour after:  0x007c0210 CServerExoAppInternal::HasEnoughDiskSpaceForSaveGame -> 0x0053d470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (19), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  0x0060b820 CExoAliasList::GetAliasPath -> Steam ?  in candidate callees: n/a
  0x006167b0 CExoInput::UnpauseRumble -> Steam ?  in candidate callees: n/a
  ... 11 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (4): 'LIVE%d'; 'MODULES:'; ':MODULES'; 'NWMFILES:'
disp sets (* = in both, 2 common): GOG {0x8*,0xc*} | Steam {0x8*,0xc*}
imm sets (* = in both, 15 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0xec*,0x7db*,0x809*,0x80e*,0xbba*,0xbc1*,...(+1)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0xc*,0xec*,0x7db*,0x809*,0x80e*,0xbba*,0xbc1*,...(+1)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053cf10: 16-byte aligned: YES
  0053cf10  55             PUSH EBP
  0053cf11  8B EC          MOV EBP,ESP
  0053cf13  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053cf10
string literals (6):
  0x00993F40  at 0053d087  'NWMFILES:'
  0x00993F40  at 0053d0e9  'NWMFILES:'
  0x00993E50  at 0053d125  'MODULES:'
  0x00993E50  at 0053d1d9  'MODULES:'
  0x00993F38  at 0053d255  'LIVE%d'
  0x00993F2C  at 0053d2be  ':MODULES'
direct callees (19):
  0x00919F10  _strlen
  0x00733540  CExoString::Constructor
  0x0053AB60  (unnamed)
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FDCA0  CSWPartyTable::AddGameInProgress
  0x00710810  (unnamed)
  0x00711ED0  CExoResMan::Exists
  0x005FDD30  CSWPartyTable::RemoveGameInProgress
  0x00733570  CExoString::CExoString_char
  0x00711690  CExoResMan::AddResourceDirectory
  0x00733780  CExoString::~CExoString
  0x00711710  CExoResMan::RemoveResourceDirectory
  0x005564B0  (unnamed)
  0x00556590  (unnamed)
  0x00734270  CExoString::Format
  0x00739790  (unnamed)
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x0072FBB0  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x2  via EDX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0053cf10 FUN_0053cf10 size=1367 ret=4 callers=1 callees=19

undefined4 FUN_0053cf10(int *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  int *piVar4;
  size_t local_f0;
  size_t local_e0;
  undefined1 local_d4 [16];
  undefined1 local_c4 [16];
  undefined1 local_b4 [8];
  undefined1 local_ac [8];
  undefined1 local_a4 [8];
  undefined1 local_9c [16];
  undefined1 local_8c [16];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [16];
  undefined1 local_5c [8];
  undefined1 local_54 [16];
  undefined1 local_44 [16];
  int local_34;
  undefined1 local_30 [8];
  undefined4 local_28;
  short local_1c;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f561;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*param_1 == 0) {
    local_e0 = 0;
  }
  else {
    local_e0 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_1);
  }
  if ((int)local_e0 < 1) {
    uVar2 = 1;
  }
  else {
    FUN_00733540(uVar1);
    local_8 = 0;
    FUN_00733540();
    local_8 = CONCAT31(local_8._1_3_,1);
    local_1c = -1;
    local_28 = 0;
    iVar3 = FUN_0053ab60(param_1);
    if (iVar3 != 0) {
      FUN_0051c8d0();
      FUN_005fdca0();
      FUN_00710810(param_1);
      iVar3 = FUN_00711ed0(local_44,0xbc1,0);
      if (iVar3 != 0) {
        local_1c = 0xbc1;
        local_28 = 1;
      }
      if (local_1c == -1) {
        FUN_00710810(param_1);
        iVar3 = FUN_00711ed0(local_54,0x809,0);
        if (iVar3 != 0) {
          local_1c = 0x809;
          local_28 = 1;
        }
      }
      FUN_0051c8d0();
      FUN_005fdd30();
    }
    if (local_1c == -1) {
      FUN_00733570("NWMFILES:");
      local_8._0_1_ = 2;
      FUN_00711690(local_5c);
      local_8._0_1_ = 1;
      FUN_00733780();
      FUN_00710810(param_1);
/* ... 102 more lines truncated by --max-lines */
```

### decoy: 0x0053c890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053c890: 16-byte aligned: YES
  0053c890  55             PUSH EBP
  0053c891  8B EC          MOV EBP,ESP
  0053c893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053c890
string literals (20):
  0x00994430  at 0053c897  'DISPLAYSPENDING'
  0x00994420  at 0053c8b1  'ITEMRECEIVED'
  0x00994414  at 0053c8ce  'ITEMLOST'
  0x0099440C  at 0053c8eb  'JOURNAL'
  0x00994400  at 0053c908  'LIGHTSHIFT'
  0x009943F4  at 0053c921  'DARKSHIFT'
  0x009943EC  at 0053c93a  'CREDITS'
  0x009943E0  at 0053c952  'CREDITSNET'
  0x009943D0  at 0053c987  'STEALTHXP'
  0x009943C0  at 0053c99f  'SOUNDPENDING'
  0x009943B0  at 0053c9bb  'LEVELUPSOUND'
  0x009943A0  at 0053c9d8  'NEWQUESTSOUND'
direct callees (2):
  0x007199D0  CResGFF::WriteFieldBYTE
  0x00719BF0  CResGFF::WriteFieldINT
struct offsets touched (top 10):
  +0x4 [dec 4]  x1  via EAX
  +0x5 [dec 5]  x1  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x14 [dec 20]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
  +0x24 [dec 36]  x1  via EAX
```

## Card B9-4-11

GOG function: CServerExoApp::GetDoorByGameObjectID  gog 0x0064bbc0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c1c0 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c1a0:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bba0 CServerExoApp::GetStoreByGameObjectID -> 0x0051c180  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbe0 CServerExoApp::GetAreaOfEffectByGameObjectID -> 0x0051c1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baff0 CServerExoAppInternal::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: no
  0x00720b70 CSWSArea::HandleTransparentDoors -> Steam 0x0052b9c0  in candidate callers: no
  0x00721750 CSWSArea::GetDoorCrossed -> Steam 0x0052c670  in candidate callers: no
  0x00721af0 CSWSArea::BudgeCreatures -> Steam 0x0052ca10  in candidate callers: no
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
prologue for 0x0051c1c0: 16-byte aligned: YES
  0051c1c0  55             PUSH EBP
  0051c1c1  8B EC          MOV EBP,ESP
  0051c1c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c1c0
string literals (0):
direct callees (1):
  0x00537E10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c1c0 FUN_0051c1c0 size=28 ret=4 callers=1 callees=1
THUNK -> 0x00537E10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c1c0(undefined4 param_1)

{
  FUN_00537e10(param_1);
  return;
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

## Card B9-4-12

GOG function: CServerExoAppInternal::GetMoveToWorldTime  gog 0x007bfa90  callconv=(blank)  param_size_bytes=28  gog_size=117  gog_retn=28
Candidate Steam address: 0x0053cc80 (size 103, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053ccf0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bfa20 CServerExoAppInternal::SetMoveToWorldTime -> 0x0053c5c0  gate_class=REJECT
- GOG-order neighbour after:  0x007bfb10 CServerExoAppInternal::DoModuleEnterSaveCleanup -> 0x0053cd70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x1c*} | Steam {0x1c*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x0053cc80: 16-byte aligned: YES
  0053cc80  55             PUSH EBP
  0053cc81  8B EC          MOV EBP,ESP
  0053cc83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053cc80
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x1009C [dec 65692]  x1  via EAX
  +0x100A0 [dec 65696]  x1  via EDX
  +0x100A1 [dec 65697]  x1  via ECX
  +0x100A2 [dec 65698]  x1  via EAX
  +0x100A4 [dec 65700]  x1  via EDX
  +0x100A6 [dec 65702]  x1  via ECX
  +0x100A8 [dec 65704]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053cc80 FUN_0053cc80 size=103 ret=28 callers=1 callees=0

void __thiscall
FUN_0053cc80(int param_1,undefined4 param_2,undefined1 param_3,undefined1 param_4,undefined1 param_5
            ,undefined2 param_6,undefined2 param_7,undefined2 param_8)

{
  *(undefined4 *)(param_1 + 0x1009c) = param_2;
  *(undefined1 *)(param_1 + 0x100a0) = param_3;
  *(undefined1 *)(param_1 + 0x100a1) = param_4;
  *(undefined1 *)(param_1 + 0x100a2) = param_5;
  *(undefined2 *)(param_1 + 0x100a4) = param_6;
  *(undefined2 *)(param_1 + 0x100a6) = param_7;
  *(undefined2 *)(param_1 + 0x100a8) = param_8;
  return;
}
```

### decoy: 0x0053ccf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053ccf0: 16-byte aligned: YES
  0053ccf0  55             PUSH EBP
  0053ccf1  8B EC          MOV EBP,ESP
  0053ccf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053ccf0
string literals (0):
direct callees (0):
struct offsets touched (top 7):
  +0x1009C [dec 65692]  x1  via ECX
  +0x100A0 [dec 65696]  x1  via ECX
  +0x100A1 [dec 65697]  x1  via ECX
  +0x100A2 [dec 65698]  x1  via ECX
  +0x100A4 [dec 65700]  x1  via ECX
  +0x100A6 [dec 65702]  x1  via ECX
  +0x100A8 [dec 65704]  x1  via ECX
```

## Card B9-4-13

GOG function: CServerExoApp::GetModule  gog 0x0064b7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051be80 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051be50:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b780 CServerExoApp::GetModuleDescription -> 0x0051bd60  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b7d0 CServerExoApp::GetModuleName -> 0x0051bdb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (68 of 123): the card's A-CALLEE credit may come from these, not from callees
  0x004543f0 CSWCArea::GetSurfaceMaterial -> Steam 0x007a79f0  in candidate callers: no
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: no
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051be80: 16-byte aligned: YES
  0051be80  55             PUSH EBP
  0051be81  8B EC          MOV EBP,ESP
  0051be83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051be80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051be80 FUN_0051be80 size=31 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_0051be80(int param_1)

{
  return *(undefined4 *)(*(int *)(param_1 + 4) + 0x10080);
}
```

### decoy: 0x0051be50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051be50: 16-byte aligned: YES
  0051be50  55             PUSH EBP
  0051be51  8B EC          MOV EBP,ESP
  0051be53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051be50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x10080 [dec 65664]  x1  via EDX
```

## Card B9-4-14

GOG function: CSWSModule::RemoveObjectFromLimbo  gog 0x00731f50  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x0055f930 (size 90, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0055f8c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731ee0 CSWSModule::AddObjectToLimbo -> 0x0055f850  gate_class=REVIEW
- GOG-order neighbour after:  0x00731fc0 CSWSModule::IsObjectInLimbo -> 0x0055f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1fc*,0x200*} | Steam {0x1fc*,0x200*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*,0x1fc} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f930: 16-byte aligned: YES
  0055f930  55             PUSH EBP
  0055f931  8B EC          MOV EBP,ESP
  0055f933  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f930
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f930 FUN_0055f930 size=90 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_0055f930(int param_1,int param_2)

{
  int local_8;
  
  local_8 = *(int *)(param_1 + 0x200);
  do {
    local_8 = local_8 + -1;
    if (local_8 < 0) {
      return 0;
    }
  } while (param_2 != *(int *)(*(int *)(param_1 + 0x1fc) + local_8 * 4));
  return 1;
}
```

### decoy: 0x0055f8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f8c0: 16-byte aligned: YES
  0055f8c0  55             PUSH EBP
  0055f8c1  8B EC          MOV EBP,ESP
  0055f8c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f8c0
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

## Card B9-4-15

GOG function: CServerExoAppInternal::GetActiveExclusionList  gog 0x007bc850  callconv=(blank)  param_size_bytes=0  gog_size=67  gog_retn=0
Candidate Steam address: 0x00539600 (size 67, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005394c0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bc7f0 CServerExoAppInternal::IsOnActiveExclusionList -> 0x005395a0  gate_class=REVIEW
- GOG-order neighbour after:  0x007bc8a0 CServerExoAppInternal::ResolvePlayerByFirstName -> 0x00539650  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007bc010 CServerExoAppInternal::GetActivePauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x2*} | Steam {0x2*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00539600: 16-byte aligned: YES
  00539600  55             PUSH EBP
  00539601  8B EC          MOV EBP,ESP
  00539603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539600
string literals (0):
direct callees (1):
  0x00538DC0  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x1  via ECX
  +0x10074 [dec 65652]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00539600 FUN_00539600 size=67 ret=0 callers=2 callees=1

undefined4 __fastcall FUN_00539600(int param_1)

{
  char cVar1;
  undefined4 uVar2;
  
  cVar1 = FUN_00538dc0();
  if (cVar1 == '\x02') {
    uVar2 = *(undefined4 *)(param_1 + 0x10070);
  }
  else {
    cVar1 = FUN_00538dc0();
    if (cVar1 == '\x01') {
      uVar2 = *(undefined4 *)(param_1 + 0x10074);
    }
    else {
      uVar2 = 0;
    }
  }
  return uVar2;
}
```

### decoy: 0x005394c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005394c0: 16-byte aligned: YES
  005394c0  55             PUSH EBP
  005394c1  8B EC          MOV EBP,ESP
  005394c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005394c0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

## Card B9-4-16

GOG function: CServerExoAppInternal::ShrinkLineByCreatures  gog 0x007be0f0  callconv=(blank)  param_size_bytes=12  gog_size=47  gog_retn=12
Candidate Steam address: 0x0053aea0 (size 47, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: ShrinkLineByCreatures 0x54e070)
- best decoy (neighbour/other candidate) scores: 0x0053ad00:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bdf50 CServerExoAppInternal::AutoSaveOnEnteringModule -> 0x0053ad00  gate_class=AUTO
- GOG-order neighbour after:  0x007be120 CServerExoAppInternal::GetSystemTimeAsDWORD64 -> 0x0053aed0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00708410 CSWSArea::ShrinkLineByCreatures -> Steam 0x0054e070  in candidate callees: yes
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0053aea0: 16-byte aligned: YES
  0053aea0  55             PUSH EBP
  0053aea1  8B EC          MOV EBP,ESP
  0053aea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053aea0
string literals (0):
direct callees (3):
  0x0052F4C0  CServerExoAppInternal::GetModule (cached lookup by id `+0x10060`)
  0x00557EF0  (unnamed)
  0x0054E070  CSWSArea::ShrinkLineByCreatures
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053aea0 FUN_0053aea0 size=47 ret=12 callers=1 callees=3

void __thiscall
FUN_0053aea0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0052f4c0(param_2,param_3,param_4,param_1);
  FUN_00557ef0();
  FUN_0054e070(param_2,param_3,param_4);
  return;
}
```

### decoy: 0x0053ad00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053ad00: 16-byte aligned: YES
  0053ad00  55             PUSH EBP
  0053ad01  8B EC          MOV EBP,ESP
  0053ad03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0053ad00
string literals (2):
  0x009941F8  at 0053ad37  'AutoSaveOnEnter'
  0x009941F0  at 0053adb2  'force'
direct callees (6):
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x0071B0B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00734F60  CExoString::CompareNoCase(char*)
  0x0072FB00  CClientExoApp::GetClientOptions
struct offsets touched (top 4):
  +0x1F344 [dec 127812]  x3  via EAX,ECX,EDX
  +0x1F33C [dec 127804]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via ECX
```

## Card B9-4-17

GOG function: CSWSModule::RemoveObjectFromLimbo  gog 0x00731f50  callconv=(blank)  param_size_bytes=4  gog_size=101  gog_retn=4
Candidate Steam address: 0x0055f850 (size 101, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0055f8c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00731ee0 CSWSModule::AddObjectToLimbo -> 0x0055f850  gate_class=REVIEW
- GOG-order neighbour after:  0x00731fc0 CSWSModule::IsObjectInLimbo -> 0x0055f930  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00903df0 CExoArrayList::DelIndex -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1fc*,0x200*} | Steam {0x1fc*,0x200*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x1fc*} | Steam {0x4*,0x10*,0x1fc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0055f850: 16-byte aligned: YES
  0055f850  55             PUSH EBP
  0055f851  8B EC          MOV EBP,ESP
  0055f853  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0055f850
string literals (0):
direct callees (1):
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0055f850 FUN_0055f850 size=101 ret=4 callers=1 callees=1

void __thiscall FUN_0055f850(int param_1,int param_2)

{
  int local_c;
  
  local_c = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x200) <= local_c) {
      FUN_0083ea60(param_2);
      return;
    }
    if (param_2 == *(int *)(*(int *)(param_1 + 0x1fc) + local_c * 4)) break;
    local_c = local_c + 1;
  }
  return;
}
```

### decoy: 0x0055f8c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0055f8c0: 16-byte aligned: YES
  0055f8c0  55             PUSH EBP
  0055f8c1  8B EC          MOV EBP,ESP
  0055f8c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0055f8c0
string literals (0):
direct callees (1):
  0x0049B840  (unnamed)
struct offsets touched (top 2):
  +0x1FC [dec 508]  x1  via ECX
  +0x200 [dec 512]  x1  via EAX
```

## Card B9-4-18

GOG function: CServerExoApp::GetModule  gog 0x0064b7b0  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0051bd60 (size 46, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051bd90:STR0/CALLEE68/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064b780 CServerExoApp::GetModuleDescription -> 0x0051bd60  gate_class=REVIEW
- GOG-order neighbour after:  0x0064b7d0 CServerExoApp::GetModuleName -> 0x0051bdb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (68 of 123): the card's A-CALLEE credit may come from these, not from callees
  0x004543f0 CSWCArea::GetSurfaceMaterial -> Steam 0x007a79f0  in candidate callers: no
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: no
  0x00564c30 CSWGuiMapHider::Draw -> Steam 0x00895cc0  in candidate callers: no
  0x00663910 CSWSItem::Destructor_2 -> Steam 0x006000e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x4c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0051bd60: 16-byte aligned: YES
  0051bd60  55             PUSH EBP
  0051bd61  8B EC          MOV EBP,ESP
  0051bd63  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051bd60
string literals (0):
direct callees (1):
  0x0051BD90  GetModule (fastcall wrapper of 0x52F4C0)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0051bd60 FUN_0051bd60 size=46 ret=0 callers=0 callees=1

int __fastcall FUN_0051bd60(undefined4 param_1)

{
  int iVar1;
  
  iVar1 = FUN_0051bd90(param_1);
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    iVar1 = FUN_0051bd90(param_1);
    iVar1 = iVar1 + 0x4c;
  }
  return iVar1;
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

## Card B9-4-19

GOG function: CServerExoApp::SetMoveToPausedTimeOfDay  gog 0x0064c9c0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051d070 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051d090:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c9a0 CServerExoApp::GetMoveToPausedCalendarDay -> 0x0051cf80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c9f0 CServerExoApp::GetMoveToPausedTimeOfDay -> 0x0051cfd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x0051d070: 16-byte aligned: YES
  0051d070  55             PUSH EBP
  0051d071  8B EC          MOV EBP,ESP
  0051d073  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051d070
string literals (0):
direct callees (1):
  0x0053CF10  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051d070 FUN_0051d070 size=28 ret=4 callers=1 callees=1
THUNK -> 0x0053CF10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051d070(undefined4 param_1)

{
  FUN_0053cf10(param_1);
  return;
}
```

### decoy: 0x0051d090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051d090: 16-byte aligned: YES
  0051d090  55             PUSH EBP
  0051d091  8B EC          MOV EBP,ESP
  0051d093  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051d090
string literals (0):
direct callees (1):
  0x00530170  CServerExoAppInternal::ExecuteLoadModule
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B9-4-20

GOG function: CServerExoAppInternal::IsActionsPending  gog 0x007bd180  callconv=(blank)  param_size_bytes=4  gog_size=76  gog_retn=4
Candidate Steam address: 0x00539f30 (size 76, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00539e30:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bd080 CServerExoAppInternal::UpdateSubareasOnMoveTo -> 0x00539e30  gate_class=AUTO
- GOG-order neighbour after:  0x007bd1d0 CServerExoAppInternal::CreatureAcquireItem -> 0x00539f80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x007badc0 CServerExoAppInternal::GetCreatureByGameObjectID -> Steam ?  in candidate callees: n/a
  0x007bcf80 CServerExoAppInternal::ClientToServerObjectId -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x100*} | Steam {0x4*,0xc*,0x100*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00539f30: 16-byte aligned: YES
  00539f30  55             PUSH EBP
  00539f31  8B EC          MOV EBP,ESP
  00539f33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539f30
string literals (0):
direct callees (3):
  0x00539D30  (unnamed)
  0x00537B70  (unnamed)
  0x007A1780  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00539f30 FUN_00539f30 size=76 ret=4 callers=1 callees=3

bool FUN_00539f30(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  bool bVar3;
  undefined4 extraout_var;
  undefined4 uVar4;
  
  uVar1 = FUN_00539d30(param_1);
  uVar4 = extraout_var;
  iVar2 = FUN_00537b70(uVar1);
  if (iVar2 == 0) {
    bVar3 = false;
  }
  else {
    iVar2 = FUN_007a1780(uVar4);
    bVar3 = iVar2 == 0;
  }
  return bVar3;
}
```

### decoy: 0x00539e30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539e30: 16-byte aligned: YES
  00539e30  55             PUSH EBP
  00539e31  8B EC          MOV EBP,ESP
  00539e33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00539e30
string literals (0):
direct callees (7):
  0x00539D30  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x0084A640  (unnamed)
  0x0054CFB0  CSWSArea::IntersectingLineSegment
  0x005C25E0  CSWSCreature::UpdateSubareasOnMoveTo
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 3):
  +0x10 [dec 16]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x1005C [dec 65628]  x1  via EAX
```

## Card B9-4-21

GOG function: CServerExoAppInternal::UpdateShutdownTimer  gog 0x007bcab0  callconv=(blank)  param_size_bytes=8  gog_size=541  gog_retn=8
Candidate Steam address: 0x00539860 (size 541, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00539a90:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bca50 CServerExoAppInternal::AddCharListRequest -> 0x00539800  gate_class=REVIEW
- GOG-order neighbour after:  0x007bcce0 Global::UpdateLoadScreen  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x007e3eb0 CSWSMessage::SendServerToPlayerShutDownServer -> Steam ?  in candidate callees: n/a
  0x00924e50 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x8*,0x30*,0xf4240*,0x1c9c380*,0x3938700*} | Steam {0x8*,0x30*,0xf4240*,0x1c9c380*,0x3938700*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00539860: 16-byte aligned: YES
  00539860  55             PUSH EBP
  00539861  8B EC          MOV EBP,ESP
  00539863  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00539860
string literals (0):
direct callees (2):
  0x0091D220  (unnamed)
  0x0064D790  (unnamed)
struct offsets touched (top 7):
  +0x10028 [dec 65576]  x10  via EAX,ECX,EDX
  +0x1002C [dec 65580]  x10  via EAX,ECX,EDX
  +0x10030 [dec 65584]  x6  via EAX,ECX,EDX
  +0x10034 [dec 65588]  x6  via EAX,ECX,EDX
  +0x10010 [dec 65552]  x1  via EDX
  +0x1001C [dec 65564]  x1  via ECX
  +0x10020 [dec 65568]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00539860 FUN_00539860 size=541 ret=8 callers=1 callees=2

undefined4 __thiscall FUN_00539860(int param_1,uint param_2,int param_3)

{
  undefined4 uVar1;
  uint uVar2;
  uint uVar3;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x10028) == 0 && *(int *)(param_1 + 0x1002c) == 0) {
    uVar1 = 0;
  }
  else {
    uVar2 = (param_3 - *(int *)(param_1 + 0x10034)) - (uint)(param_2 < *(uint *)(param_1 + 0x10030))
    ;
    if ((uVar2 < *(uint *)(param_1 + 0x1002c)) ||
       ((uVar2 == *(uint *)(param_1 + 0x1002c) &&
        (param_2 - *(uint *)(param_1 + 0x10030) <= *(uint *)(param_1 + 0x10028))))) {
      if (((((*(int *)(param_1 + 0x1002c) != 0) || (60000000 < *(uint *)(param_1 + 0x10028))) &&
           (uVar2 = param_2 - *(uint *)(param_1 + 0x10030),
           *(int *)(param_1 + 0x1002c) -
           ((param_3 - *(int *)(param_1 + 0x10034)) - (uint)(param_2 < *(uint *)(param_1 + 0x10030))
           ) == (uint)(*(uint *)(param_1 + 0x10028) < uVar2))) &&
          (*(uint *)(param_1 + 0x10028) - uVar2 < 60000000)) ||
         (((*(int *)(param_1 + 0x1002c) != 0 || (30000000 < *(uint *)(param_1 + 0x10028))) &&
          ((uVar2 = param_2 - *(uint *)(param_1 + 0x10030),
           *(int *)(param_1 + 0x1002c) -
           ((param_3 - *(int *)(param_1 + 0x10034)) - (uint)(param_2 < *(uint *)(param_1 + 0x10030))
           ) == (uint)(*(uint *)(param_1 + 0x10028) < uVar2) &&
           (*(uint *)(param_1 + 0x10028) - uVar2 < 30000000)))))) {
        for (local_8 = 0; local_8 < *(int *)(param_1 + 0x10020); local_8 = local_8 + 1) {
          uVar2 = param_2 - *(uint *)(param_1 + 0x10030);
          uVar1 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](*(uint *)(param_1 + 0x10028) - uVar2,
                            (*(int *)(param_1 + 0x1002c) -
                            ((param_3 - *(int *)(param_1 + 0x10034)) -
                            (uint)(param_2 < *(uint *)(param_1 + 0x10030)))) -
                            (uint)(*(uint *)(param_1 + 0x10028) < uVar2),1000000,0);
          FUN_0064d790(*(undefined4 *)(*(int *)(param_1 + 0x1001c) + local_8 * 4),uVar1);
        }
      }
      uVar3 = param_2 - *(uint *)(param_1 + 0x10030);
      uVar2 = *(uint *)(param_1 + 0x10028);
      *(uint *)(param_1 + 0x10028) = uVar2 - uVar3;
      *(uint *)(param_1 + 0x1002c) =
           (*(int *)(param_1 + 0x1002c) -
           ((param_3 - *(int *)(param_1 + 0x10034)) - (uint)(param_2 < *(uint *)(param_1 + 0x10030))
           )) - (uint)(uVar2 < uVar3);
      *(uint *)(param_1 + 0x10030) = param_2;
      *(int *)(param_1 + 0x10034) = param_3;
      uVar1 = 0;
    }
    else {
      *(undefined4 *)(param_1 + 0x10028) = 0;
      *(undefined4 *)(param_1 + 0x1002c) = 0;
      uVar1 = 1;
    }
  }
  return uVar1;
}
```

### decoy: 0x00539a90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00539a90: 16-byte aligned: YES
  00539a90  55             PUSH EBP
  00539a91  8B EC          MOV EBP,ESP
  00539a93  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00539a90
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x100E4 [dec 65764]  x6  via ECX,EDX
  +0x100E8 [dec 65768]  x6  via ECX,EDX
  +0x100DC [dec 65756]  x5  via ECX,EDX
  +0x100E0 [dec 65760]  x5  via EAX,ECX,EDX
```

## Card B9-4-22

GOG function: CServerExoApp::GetDoorByGameObjectID  gog 0x0064bbc0  callconv=(blank)  param_size_bytes=4  gog_size=28  gog_retn=4
Candidate Steam address: 0x0051c180 (size 28, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051c1a0:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064bba0 CServerExoApp::GetStoreByGameObjectID -> 0x0051c180  gate_class=REVIEW
- GOG-order neighbour after:  0x0064bbe0 CServerExoApp::GetAreaOfEffectByGameObjectID -> 0x0051c1c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x007baff0 CServerExoAppInternal::GetDoorByGameObjectID -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: no
  0x00720b70 CSWSArea::HandleTransparentDoors -> Steam 0x0052b9c0  in candidate callers: no
  0x00721750 CSWSArea::GetDoorCrossed -> Steam 0x0052c670  in candidate callers: no
  0x00721af0 CSWSArea::BudgeCreatures -> Steam 0x0052ca10  in candidate callers: no
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
prologue for 0x0051c180: 16-byte aligned: YES
  0051c180  55             PUSH EBP
  0051c181  8B EC          MOV EBP,ESP
  0051c183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051c180
string literals (0):
direct callees (1):
  0x00537FD0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0051c180 FUN_0051c180 size=28 ret=4 callers=10 callees=1
THUNK -> 0x00537FD0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0051c180(undefined4 param_1)

{
  FUN_00537fd0(param_1);
  return;
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

## Card B9-4-23

GOG function: CServerExoAppInternal::RemoveFromExclusionList  gog 0x007bc710  callconv=(blank)  param_size_bytes=5  gog_size=220  gog_retn=8
Candidate Steam address: 0x005393b0 (size 260, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005394c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007bc600 CServerExoAppInternal::AddToExclusionList -> 0x005393b0  gate_class=REVIEW
- GOG-order neighbour after:  0x007bc7f0 CServerExoAppInternal::IsOnActiveExclusionList -> 0x005395a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x006207c0 CExoLinkedListInternal::Remove_2 -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x2*,0x8*} | Steam {0x2*,0x8*,0xc}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005393b0: 16-byte aligned: YES
  005393b0  55             PUSH EBP
  005393b1  8B EC          MOV EBP,ESP
  005393b3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005393b0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x0054AA30  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005393b0 FUN_005393b0 size=260 ret=8 callers=1 callees=4

void __thiscall FUN_005393b0(undefined4 param_1,int param_2,byte param_3)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (param_3 != 0) {
    if ((param_3 & 2) == 0) {
      if ((param_3 & 1) != 0) {
        local_8 = 0;
        local_c = FUN_007a1720(param_1);
        while (local_c != 0) {
          iVar1 = FUN_00561430(local_c);
          if (iVar1 == param_2) {
            local_8 = 1;
            break;
          }
          FUN_0058c370(&local_c);
        }
        if (local_8 == 0) {
          FUN_0054aa30(param_2);
        }
      }
    }
    else {
      local_8 = 0;
      local_c = FUN_007a1720(param_1);
      while (local_c != 0) {
        iVar1 = FUN_00561430(local_c);
        if (iVar1 == param_2) {
          local_8 = 1;
          break;
        }
        FUN_0058c370(&local_c);
      }
      if (local_8 == 0) {
        FUN_0054aa30(param_2);
      }
    }
  }
  return;
}
```

### decoy: 0x005394c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005394c0: 16-byte aligned: YES
  005394c0  55             PUSH EBP
  005394c1  8B EC          MOV EBP,ESP
  005394c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005394c0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0x10070 [dec 65648]  x4  via EAX,ECX,EDX
  +0x10074 [dec 65652]  x4  via EAX,ECX,EDX
```

## Card B9-4-24

GOG function: CServerExoApp::SetMoveToPausedTimeOfDay  gog 0x0064c9c0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x0051cf50 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: LoadPCAutoSave 0x8529e0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0051cf80:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0064c9a0 CServerExoApp::GetMoveToPausedCalendarDay -> 0x0051cf80  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c9f0 CServerExoApp::GetMoveToPausedTimeOfDay -> 0x0051cfd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x005d1d20 CSWGuiSaveLoad::LoadPCAutoSave -> Steam 0x008529e0  in candidate callers: yes
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
prologue for 0x0051cf50: 16-byte aligned: YES
  0051cf50  55             PUSH EBP
  0051cf51  8B EC          MOV EBP,ESP
  0051cf53  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cf50 FUN_0051cf50 size=36 ret=4 callers=2 callees=0

void __thiscall FUN_0051cf50(int param_1,undefined4 param_2)

{
  *(undefined4 *)(*(int *)(param_1 + 4) + 0x100b0) = param_2;
  return;
}
```

### decoy: 0x0051cf80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051cf80: 16-byte aligned: YES
  0051cf80  55             PUSH EBP
  0051cf81  8B EC          MOV EBP,ESP
  0051cf83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051cf80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x100B0 [dec 65712]  x1  via ECX
```

## Card B9-4-25

GOG function: CServerExoAppInternal::GetSystemTimeAsDWORD64  gog 0x007be120  callconv=(blank)  param_size_bytes=0  gog_size=72  gog_retn=0
Candidate Steam address: 0x0053aed0 (size 72, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: DoPCAutosave 0x53af20)
- best decoy (neighbour/other candidate) scores: 0x0053af20:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007be0f0 CServerExoAppInternal::ShrinkLineByCreatures -> 0x0053aea0  gate_class=REVIEW
- GOG-order neighbour after:  0x007be170 CServerExoAppInternal::DoPCAutosave -> 0x0053af20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00927aa0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x007be170 CServerExoAppInternal::DoPCAutosave -> Steam 0x0053af20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x20*,0x24*} | Steam {0x20*,0x24*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053aed0: 16-byte aligned: YES
  0053aed0  55             PUSH EBP
  0053aed1  8B EC          MOV EBP,ESP
  0053aed3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053aed0
string literals (0):
direct callees (1):
  0x00920060  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053aed0 FUN_0053aed0 size=72 ret=0 callers=3 callees=3

uint FUN_0053aed0(void)

{
  uint uVar1;
  _FILETIME [GHIDRA-AUTO-LABEL: unreliable] local_24 [2];
  _SYSTEMTIME [GHIDRA-AUTO-LABEL: unreliable] local_14;
  
  GetLocalTime(&local_14);
  SystemTimeToFileTime(&local_14,local_24);
  uVar1 = __allshl [GHIDRA-AUTO-LABEL: unreliable]();
  return uVar1 | local_24[0].dwLowDateTime;
}
```

### decoy: 0x0053af20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053af20: 16-byte aligned: YES
  0053af20  55             PUSH EBP
  0053af21  8B EC          MOV EBP,ESP
  0053af23  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0053af20
string literals (35):
  0x00993E30  at 0053b069  'SAVES:'
  0x00993F50  at 0053b0b3  'AUTOSAVE'
  0x00993E20  at 0053b0ba  'SAVES:%06d - %s'
  0x009940B8  at 0053b2fa  'savenfo'
  0x00994258  at 0053b3f8  'AreaName'
  0x0099424C  at 0053b4a3  'BMPResRef'
  0x00994244  at 0053b52a  'load_%s'
  0x0099424C  at 0053b5a1  'BMPResRef'
  0x0099423C  at 0053b5b5  'DEFAULT'
  0x009940A4  at 0053b635  'AREANAME'
  0x00994098  at 0053b64d  'LASTMODULE'
  0x0099408C  at 0053b662  'TIMEPLAYED'
direct callees (72):
  0x00740540  CClientExoApp::SetLoadScreenByModuleName
  0x00740DC0  (unnamed)
  0x00740980  CClientExoApp::SetLoadScreenHint
  0x0073FF20  CClientExoApp::ShowLoadScreen
  0x007405D0  CClientExoApp::SetLoadBarProgress
  0x0079D550  CClientExoAppInternal::UpdateRumble
  0x0072FB90  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x004113A0  CSWGuiManager::Draw
  0x00409ED0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00714F30  CExoResMan::CreateDirectory2
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x007156E0  CExoResMan::CleanDirectory
  0x00740C60  CClientExoApp::GetLoadStep
  0x005FB1A0  CSWPartyTable::SaveTableInfo
  0x00656640  CSWGlobalVariableTable::SaveToSave (thiscall table, CExoString*) ret 4
  0x00733DF0  CExoString::operator+
struct offsets touched (top 8):
  +0x4 [dec 4]  x38  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x130 [dec 304]  x2  via ECX,EDX
  +0xC8 [dec 200]  x1  via EAX
  +0x148 [dec 328]  x1  via EAX
  +0x1F2F0 [dec 127728]  x1  via EAX
  +0x1F2FC [dec 127740]  x1  via ECX
```

## Card B9-4-26

GOG function: CServerExoApp::GetRoomIndex  gog 0x0064c620  callconv=(blank)  param_size_bytes=0  gog_size=86  gog_retn=12
Candidate Steam address: 0x0051cae0 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0051caa0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: autolabel:Statistics;gog_db_param_bad
- GOG-order neighbour before: 0x0064c600 CServerExoApp::IsActionsPending -> 0x0051cbe0  gate_class=REVIEW
- GOG-order neighbour after:  0x0064c680 CServerExoApp::CreatureAcquireItem -> 0x0051cc60  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00705750 CSWSArea::GetRoomIndex -> Steam 0x0054b3b0  in candidate callees: no
  0x0072a6a0 CSWSModule::GetArea -> Steam ?  in candidate callees: n/a
  0x007b2840 CServerExoAppInternal::GetModule -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x004576e0 CSWCArea::GetRoomIndex -> Steam 0x007aace0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 1 common): GOG {0xc*} | Steam {0xc*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 12)
```

### prologue
```
prologue for 0x0051cae0: 16-byte aligned: YES
  0051cae0  55             PUSH EBP
  0051cae1  8B EC          MOV EBP,ESP
  0051cae3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0051cae0
string literals (0):
direct callees (1):
  0x00539E30  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0051cae0 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=2 callees=1
THUNK -> 0x00539E30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_00539e30(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0051caa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0051caa0: 16-byte aligned: YES
  0051caa0  55             PUSH EBP
  0051caa1  8B EC          MOV EBP,ESP
  0051caa3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0051caa0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x00538C50  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```
