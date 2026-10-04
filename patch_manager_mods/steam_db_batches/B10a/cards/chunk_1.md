# Review cards B10a chunk 1

Answer each card with one TSV row in `steam_db_batches/B10a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B10a-1-01

GOG function: CSWSObject::SaveObjectState  gog 0x006a69c0  callconv=(blank)  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0053fd50 (size 1683, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::LoadActionQueue | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053fce0:STR1/CALLEE11/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6950 CSWSObject::LoadObjectState -> 0x0053fc70  gate_class=AUTO
- GOG-order neighbour after:  0x006a6a30 CSWSObject::LoadActionQueue -> 0x0053fd50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00625b30 CResGFF::WriteFieldBYTE -> Steam 0x007199d0  in candidate callees: no
  0x006a70f0 CSWSObject::SaveActionQueue -> Steam ?  in candidate callees: n/a
  0x006a7380 unnamed -> Steam ?  in candidate callees: n/a
  0x006a7540 CSWSObject::SaveEffectList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x0068d030 CSWSCreature::SaveCreature -> Steam 0x00574500  in candidate callers: no
  0x0071f8e0 CSWSArea::SaveItems -> Steam 0x0052a730  in candidate callers: no
  0x0071f9f0 CSWSArea::SaveDoors -> Steam 0x0052a840  in candidate callers: no
  0x0071fb00 CSWSArea::SaveTriggers -> Steam 0x0052a950  in candidate callers: no
GOG strings (1): 'Commandable'
disp sets (* = in both, 0 common): GOG {0xec} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x2,0x3,0x4,0x5,0x8*,0xd,0x154}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0053fd50: 16-byte aligned: YES
  0053fd50  55             PUSH EBP
  0053fd51  8B EC          MOV EBP,ESP
  0053fd53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053fd50
string literals (10):
  0x0099462C  at 0053fd81  'ActionList'
  0x00994620  at 0053fe2f  'ActionId'
  0x00994610  at 0053fe49  'GroupActionId'
  0x00994604  at 0053fe64  'NumParams'
  0x009945F8  at 0053fe85  'Paramaters'
  0x00992E8C  at 0053ff5b  'Value'
  0x00992E8C  at 0053ffbd  'Value'
  0x00992E8C  at 0054007a  'Value'
  0x00992E8C  at 0054011d  'Value'
  0x00992E8C  at 00540156  'Value'
direct callees (18):
  0x00718760  CResGFF::GetList
  0x00718810  CResGFF::GetListCount
  0x00718890  CResGFF::GetListElement
  0x00718C00  CResGFF::ReadFieldDWORD
  0x00718B00  CResGFF::ReadFieldWORD
  0x00919723  operator new
  0x00718C80  CResGFF::ReadFieldINT
  0x00718D00  CResGFF::ReadFieldFLOAT
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00718F90  CResGFF::ReadFieldCExoString
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00718960  CResGFF::GetStructFromStruct
  0x006FDB80  CVirtualMachine::LoadScriptSituation
  0x0053F7F0  CSWSObject::AddAction
  0x009196FD  operator delete
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053fd50 FUN_0053fd50 size=1683 ret=8 callers=1 callees=18

void FUN_0053fd50(uint *param_1,undefined4 param_2)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  float10 fVar4;
  uint local_154;
  undefined1 local_130 [8];
  undefined1 local_128 [8];
  int local_120;
  uint local_11c;
  float *local_118;
  undefined4 *local_114;
  undefined4 auStack_110[13 = byte +0x34];
  uint local_dc;
  uint local_d8;
  float *local_d4;
  uint local_d0[13 = byte +0x34];
  undefined4 local_9c[13 = byte +0x34];
  undefined4 local_68;
  uint local_64;
  undefined2 local_60;
  undefined1 local_5c [4];
  uint local_58;
  undefined1 local_54 [4];
  uint local_50;
  undefined1 local_4c [20];
  undefined1 local_38 [20];
  uint local_24;
  ushort local_20;
  undefined4 *local_1c;
  undefined1 local_18 [4];
  undefined4 *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0094f734;
  local_10 = ExceptionList;
  local_24 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  iVar1 = FUN_00718760(local_38,param_2,"ActionList");
  if (iVar1 != 0) {
    local_50 = 0;
    while (uVar2 = FUN_00718810(local_38), local_50 < uVar2) {
      iVar1 = FUN_00718890(local_5c,local_38,local_50);
      if (iVar1 != 0) {
        for (local_d8 = 0; local_d8 < 0xd; local_d8 = local_d8 + 1) {
          local_9c[local_d8] = 0;
          local_d0[local_d8] = 0;
        }
        local_68 = FUN_00718c00(local_5c,"ActionId",local_54,0);
        local_60 = FUN_00718b00(local_5c,"GroupActionId",local_54,0);
        local_20 = FUN_00718b00(local_5c,"NumParams",local_54,0);
        if (local_20 != 0) {
          FUN_00718760(local_4c,local_5c,"Paramaters");
          for (local_d8 = 0; local_d8 < local_20; local_d8 = local_d8 + 1) {
            FUN_00718890(local_18,local_4c,local_d8);
            uVar3 = FUN_00718c00(local_18,&DAT_00994500,local_54,0);
            local_9c[local_d8] = uVar3;
            switch(local_9c[local_d8]) {
            case 1:
              local_114 = (undefined4 *)FUN_00919723(4);
              local_1c = local_114;
              uVar3 = FUN_00718c80(local_18,"Value",local_54,0);
              *local_1c = uVar3;
              local_d0[local_d8] = (uint)local_1c;
              auStack_110[local_d8] = 1;
              break;
            case 2:
              local_118 = (float *)FUN_00919723(4);
              local_d4 = local_118;
              fVar4 = (float10)FUN_00718d00(local_18,"Value",local_54,0);
              *local_d4 = (float)fVar4;
              local_d0[local_d8] = (uint)local_d4;
              auStack_110[local_d8] = 2;
              break;
/* ... 76 more lines truncated by --max-lines */
```

### decoy: 0x0053fce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053fce0: 16-byte aligned: YES
  0053fce0  55             PUSH EBP
  0053fce1  8B EC          MOV EBP,ESP
  0053fce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053fce0
string literals (1):
  0x009945EC  at 0053fd25  'Commandable'
direct callees (4):
  0x00540860  CSWSObject::SaveEffectList
  0x005406A0  (unnamed)
  0x00540410  CSWSObject::SaveActionQueue
  0x007199D0  CResGFF::WriteFieldBYTE
struct offsets touched (top 1):
  +0xEC [dec 236]  x1  via EAX
```

## Card B10a-1-02

GOG function: CSWCObject::AnimationWalking  gog 0x004745d0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef3e0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474540 CSWCObject::AnimationHideEquippedItems -> 0x007ef910  gate_class=REVIEW
- GOG-order neighbour after:  0x00474660 CSWCObject::AnimationLooping -> 0x007ef500  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ef3e0: 16-byte aligned: YES
  007ef3e0  55             PUSH EBP
  007ef3e1  8B EC          MOV EBP,ESP
  007ef3e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef3e0
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
// FN 007ef3e0 FUN_007ef3e0 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef3e0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe30,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B10a-1-03

GOG function: CSWCObject::GetAnimationBase  gog 0x00472990  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x007ed830 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 10 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 10 (callers: MainLoop 0x781be0, GetClientMeleeAnimation 0x79c9c0, GetClientRangedAnimation 0x79ca90)
- best decoy (neighbour/other candidate) scores: 0x007ed7f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472950 CSWCObject::AttachToScene -> 0x007ed7f0  gate_class=AUTO
- GOG-order neighbour after:  0x004729b0 CSWCObject::GetModel -> 0x007ed850  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (10 of 52): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
  0x00448fc0 CClientExoAppInternal::GetClientMeleeAnimation -> Steam 0x0079c9c0  in candidate callers: yes
  0x00449090 CClientExoAppInternal::GetClientRangedAnimation -> Steam 0x0079ca90  in candidate callers: yes
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam 0x0079f430  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
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
prologue for 0x007ed830: 16-byte aligned: YES
  007ed830  55             PUSH EBP
  007ed831  8B EC          MOV EBP,ESP
  007ed833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed830
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x68 [dec 104]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed830 FUN_007ed830 size=17 ret=0 callers=52 callees=0

undefined4 __fastcall FUN_007ed830(int param_1)

{
  return *(undefined4 *)(param_1 + 0x68);
}
```

### decoy: 0x007ed7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed7f0: 16-byte aligned: YES
  007ed7f0  55             PUSH EBP
  007ed7f1  8B EC          MOV EBP,ESP
  007ed7f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed7f0
string literals (0):
direct callees (1):
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via EDX
```

## Card B10a-1-04

GOG function: CSWCObject::GetName  gog 0x00472580  callconv=(blank)  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x007ed420 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed590:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004721b0 CSWCObject::Constructor -> 0x007ed050  gate_class=AUTO
- GOG-order neighbour after:  0x004725b0 CSWCObject::GetCameraFollowPart -> 0x007ed450  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007ed420: 16-byte aligned: YES
  007ed420  55             PUSH EBP
  007ed421  8B EC          MOV EBP,ESP
  007ed423  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed420
string literals (0):
direct callees (1):
  0x00733570  CExoString::CExoString_char
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed420 FUN_007ed420 size=47 ret=4 callers=0 callees=1

undefined4 FUN_007ed420(undefined4 param_1)

{
  FUN_00733570(&DAT_00986925);
  return param_1;
}
```

### decoy: 0x007ed590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed590: 16-byte aligned: YES
  007ed590  55             PUSH EBP
  007ed591  8B EC          MOV EBP,ESP
  007ed593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed590
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x88 [dec 136]  x1  via EDX
```

## Card B10a-1-05

GOG function: CSWCObject::DisableBumpedOutShell  gog 0x00477630  callconv=__thiscall  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007f2490 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2450:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:GetFirstDocTemplatePosition
- GOG-order neighbour before: 0x004775f0 CSWCObject::EnableBumpedOutShell -> 0x007f2450  gate_class=AUTO
- GOG-order neighbour after:  0x00477660 CSWCObject::ClearAllActions -> 0x007f24c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x2c*,0x68*} | Steam {0x2c*,0x68*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f2490: 16-byte aligned: YES
  007f2490  55             PUSH EBP
  007f2491  8B EC          MOV EBP,ESP
  007f2493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2490
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x2C [dec 44]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2490 CWinApp::GetFirstDocTemplatePosition size=43 ret=0 callers=1 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: struct __POSITION * __thiscall CWinApp::GetFirstDocTemplatePosition(void)const 
   
   Libraries: Visual Studio 2005 Debug, Visual Studio 2008 Debug, Visual Studio 2010 Debug */

__POSITION [GHIDRA-AUTO-LABEL: unreliable] * __thiscall CWinApp::GetFirstDocTemplatePosition(CWinApp *this)

{
  __POSITION [GHIDRA-AUTO-LABEL: unreliable] *p_Var1;
  
  if (*(int *)(this + 0x68) == 0) {
    p_Var1 = (__POSITION [GHIDRA-AUTO-LABEL: unreliable] *)0x0;
  }
  else {
    p_Var1 = (__POSITION [GHIDRA-AUTO-LABEL: unreliable] *)(**(code **)(**(int **)(this + 0x68) + 0x2c))();
  }
  return p_Var1;
}
```

### decoy: 0x007f2450 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2450: 16-byte aligned: YES
  007f2450  55             PUSH EBP
  007f2451  8B EC          MOV EBP,ESP
  007f2453  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2450
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x28 [dec 40]  x1  via EDX
```

## Card B10a-1-06

GOG function: CSWCObject::RestoreEnvMap  gog 0x00475850  callconv=(blank)  param_size_bytes=0  gog_size=46  gog_retn=0
Candidate Steam address: 0x007f06f0 (size 46, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f0720:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004756d0 CSWCObject::SetEnvMap_2 -> 0x007f0570  gate_class=AUTO
- GOG-order neighbour after:  0x00475880 CSWCObject::RestoreTexture -> 0x007f0720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xa0*} | Steam {0x68*,0xa0*}
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
prologue for 0x007f06f0: 16-byte aligned: YES
  007f06f0  55             PUSH EBP
  007f06f1  8B EC          MOV EBP,ESP
  007f06f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f06f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xA0 [dec 160]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f06f0 FUN_007f06f0 size=46 ret=0 callers=1 callees=0

undefined4 __fastcall FUN_007f06f0(int param_1)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x68) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = (**(code **)(**(int **)(param_1 + 0x68) + 0xa0))();
  }
  return uVar1;
}
```

### decoy: 0x007f0720 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f0720: 16-byte aligned: YES
  007f0720  55             PUSH EBP
  007f0721  8B EC          MOV EBP,ESP
  007f0723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f0720
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x24 [dec 36]  x1  via EDX
```

## Card B10a-1-07

GOG function: CSWSObject::GetNumActionGroups  gog 0x006a62c0  callconv=(blank)  param_size_bytes=0  gog_size=177  gog_retn=0
Candidate Steam address: 0x0053f7a0 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::GetNewActionGroupId | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f6a0:STR0/CALLEE1/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6240 CSWSObject::GetNodeById -> 0x0053f560  gate_class=AUTO
- GOG-order neighbour after:  0x006a6380 CSWSObject::RemoveGroup -> 0x0053f6a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0069f2c0 CSWSCreature::ClearHostileActionsVersus -> Steam 0x005867e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x6c} | Steam {0x14,0x16}
imm sets (* = in both, 0 common): GOG {0x14,0x100} | Steam {0x8,0xffff}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053f7a0: 16-byte aligned: YES
  0053f7a0  55             PUSH EBP
  0053f7a1  8B EC          MOV EBP,ESP
  0053f7a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f7a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x14 [dec 20]  x5  via EAX,ECX,EDX
  +0x16 [dec 22]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f7a0 FUN_0053f7a0 size=80 ret=0 callers=2 callees=0

undefined2 __fastcall FUN_0053f7a0(int param_1)

{
  undefined2 uVar1;
  
  uVar1 = *(undefined2 *)(param_1 + 0x14);
  *(short *)(param_1 + 0x14) = *(short *)(param_1 + 0x14) + 1;
  if (*(short *)(param_1 + 0x14) == -1) {
    *(undefined2 *)(param_1 + 0x14) = 0;
  }
  *(undefined2 *)(param_1 + 0x16) = uVar1;
  return uVar1;
}
```

### decoy: 0x0053f6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f6a0: 16-byte aligned: YES
  0053f6a0  55             PUSH EBP
  0053f6a1  8B EC          MOV EBP,ESP
  0053f6a3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f6a0
string literals (0):
direct callees (7):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x0064DDE0  (unnamed)
  0x0053E740  Action-list free params
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via ECX
```

## Card B10a-1-08

GOG function: CSWSObject::GetPortraitId  gog 0x006638a0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x0053f000 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053efd0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663870 CSWSObject::SetPortrait -> 0x0053efd0  gate_class=REVIEW
- GOG-order neighbour after:  0x006638c0 CSWSObject::GetFirstName -> 0x004598b0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
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
prologue for 0x0053f000: 16-byte aligned: YES
  0053f000  55             PUSH EBP
  0053f001  8B EC          MOV EBP,ESP
  0053f003  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f000
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f000 FUN_0053f000 size=18 ret=0 callers=0 callees=0

undefined2 __fastcall FUN_0053f000(int param_1)

{
  return *(undefined2 *)(param_1 + 0x30);
}
```

### decoy: 0x0053efd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

## Card B10a-1-09

GOG function: CSWCObject::HideShadowBlob  gog 0x00410c80  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x00764fb0 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764f80:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW
- GOG-order neighbour after:  0x00410cb0 CSWCObject::GetCanSeeHeight -> 0x00764fe0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xb0*} | Steam {0x68*,0xb0*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00764fb0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xb0))();
  }
  return;
}
```

### decoy: 0x00764f80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764f80: 16-byte aligned: YES
  00764f80  55             PUSH EBP
  00764f81  8B EC          MOV EBP,ESP
  00764f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xAC [dec 172]  x1  via EDX
```

## Card B10a-1-10

GOG function: CSWSObject::GetDialogResref  gog 0x006637e0  callconv=(blank)  param_size_bytes=4  gog_size=29  gog_retn=4
Candidate Steam address: 0x0053ef30 (size 29, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004597d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006634d0 CSWSItem::Constructor -> 0x005ffd60  trusted
- GOG-order neighbour after:  0x00663800 CSWSObject::GetMaxHitPoints -> 0x004597f0  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-1-11

GOG function: CSWCObject::Destructor_2  gog 0x00472750  callconv=(blank)  param_size_bytes=0  gog_size=96  gog_retn=None
Candidate Steam address: 0x007ed5f0 (size 475, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 4 | A-SIG 0.23 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 4 (callees: AddObjectToHitcheckIgnoreList 0x73f8d0; callers: Destructor_2 0x87b1a0, Destructor_2 0x841200, Destructor_2 0x8ee820)
- best decoy (neighbour/other candidate) scores: 0x007ed5c0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472720 CSWCObject::Destructor -> 0x007ed5c0  gate_class=REVIEW
- GOG-order neighbour after:  0x004727b0 CSWCObject::Destructor_3 -> 0x007ed5f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040cdb0 CClientExoApp::RemoveObjectFromHitcheckIgnoreList -> Steam 0x0073f8d0  in candidate callees: yes
  0x00475bb0 CSWCObject::KillAllVisualEffects -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (3 of 30): the card's A-CALLEE credit may come from these, not from callees
  0x00506a80 CSWCDoor::Destructor_2 -> Steam 0x0087b1a0  in candidate callers: yes
  0x00534a80 CSWCPlaceable::Destructor_2 -> Steam 0x00841200  in candidate callers: yes
  0x00582200 CSWCAreaOfEffectObject::Destructor_2 -> Steam 0x008ee820  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x5c,0xc8,0xcc,0xf8}
imm sets (* = in both, 2 common): GOG {0x4*,0x30*} | Steam {0x2,0x3,0x4*,0x30*,0x6c,0x70,0xa8,0xe8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ed5f0
string literals (0):
direct callees (11):
  0x0073F8D0  CClientExoApp::AddObjectToHitcheckIgnoreList
  0x007F0A50  CSWCObject::KillAllVisualEffects
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A2540  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x0054A900  CSWSObject::DetachFromClientObject
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0072F5F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0xC8 [dec 200]  x3  via EAX,ECX
  +0xCC [dec 204]  x3  via EAX,EDX
  +0xF8 [dec 248]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed5f0 FUN_007ed5f0 size=475 ret=0 callers=30 callees=11

void __fastcall FUN_007ed5f0(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0096b9ca;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCObject::vftable;
  local_8 = 4;
  FUN_0073f8d0(param_1[1 = byte +0x4]);
  FUN_007f0a50(uVar2);
  FUN_0073f790();
  FUN_007a2540(param_1);
  if (param_1[0x32 = byte +0xc8] != 0) {
    if ((undefined4 *)param_1[0x32 = byte +0xc8] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x32 = byte +0xc8])(1);
    }
    param_1[0x32 = byte +0xc8] = 0;
  }
  if (param_1[0x33 = byte +0xcc] != 0) {
    iVar1 = param_1[0x33 = byte +0xcc];
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    param_1[0x33 = byte +0xcc] = 0;
  }
  if (param_1[0x17 = byte +0x5c] != 0) {
    FUN_009196fd(param_1[0x17 = byte +0x5c]);
    param_1[0x17 = byte +0x5c] = 0;
  }
  if (param_1[0x3e = byte +0xf8] != 0) {
    FUN_0054a900();
    param_1[0x3e = byte +0xf8] = 0;
  }
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  FUN_00623320();
  local_8._0_1_ = 1;
  Free();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  *param_1 = CGameObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007ed5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5c0: 16-byte aligned: YES
  007ed5c0  55             PUSH EBP
  007ed5c1  8B EC          MOV EBP,ESP
  007ed5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed5c0
string literals (0):
direct callees (2):
  0x007ED5F0  CSWCObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B10a-1-12

GOG function: CSWCObject::AnimationAttack  gog 0x004749e0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef880 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef7f0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474950 CSWCObject::AnimationParry -> 0x007ef7f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474a70 CSWCObject::AnimationRunning -> 0x007ef470  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ef880: 16-byte aligned: YES
  007ef880  55             PUSH EBP
  007ef881  8B EC          MOV EBP,ESP
  007ef883  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007ef880 FUN_007ef880 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef880(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe4c,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef7f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef7f0: 16-byte aligned: YES
  007ef7f0  55             PUSH EBP
  007ef7f1  8B EC          MOV EBP,ESP
  007ef7f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef7f0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B10a-1-13

GOG function: CSWCObject::ClearAllActions  gog 0x00477660  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x007f24c0 (size 54, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 8 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 8 (callees: ClearAllActions 0x51cb10; callers: Update 0x7ac470, HandleInputEvent 0x7b12c0, DoorSaber 0x87bad0)
- best decoy (neighbour/other candidate) scores: 0x007f2500:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00477630 CSWCObject::DisableBumpedOutShell -> 0x007f2490  gate_class=REVIEW
- GOG-order neighbour after:  0x004776a0 CSWCObject::ClearAllQueuedCombatActions -> 0x007f2500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x0064c530 CServerExoApp::ClearAllActions -> Steam 0x0051cb10  in candidate callees: yes
GOG callers mapped through trusted rows (7 of 11): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
  0x0045ded0 CClientExoAppInternal::HandleInputEvent -> Steam 0x007b12c0  in candidate callers: yes
  0x005073b0 CSWCDoor::DoorSaber -> Steam 0x0087bad0  in candidate callers: yes
  0x00507400 CSWCDoor::ToggleDoorState -> Steam 0x0087bb20  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007f24c0: 16-byte aligned: YES
  007f24c0  55             PUSH EBP
  007f24c1  8B EC          MOV EBP,ESP
  007f24c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f24c0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CB10  CServerExoApp::ClearAllActions
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f24c0 FUN_007f24c0 size=54 ret=0 callers=11 callees=2

void __fastcall FUN_007f24c0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  FUN_0051cb10(uVar1);
  return;
}
```

### decoy: 0x007f2500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2500: 16-byte aligned: YES
  007f2500  55             PUSH EBP
  007f2501  8B EC          MOV EBP,ESP
  007f2503  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2500
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBA0  CServerExoApp::ClearAllQueuedCombatActions
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

## Card B10a-1-14

GOG function: CSWSObject::Destructor  gog 0x006a5d00  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0053efd0 (size 34, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWSObject::SetPortrait | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053efa0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a5ce0 CSWSObject::GetLastName -> 0x0053ef50  gate_class=REVIEW
- GOG-order neighbour after:  0x006a5d30 CSWSObject::Destructor_2 -> 0x0053f050  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a5d30 CSWSObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x14,0x20}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053efd0 FUN_0053efd0 size=34 ret=4 callers=0 callees=1
THUNK -> 0x00710CA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0053efd0(undefined4 param_1)

{
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,param_1);
  return;
}
```

### decoy: 0x0053efa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efa0: 16-byte aligned: YES
  0053efa0  55             PUSH EBP
  0053efa1  8B EC          MOV EBP,ESP
  0053efa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efa0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

## Card B10a-1-15

GOG function: CSWCObject::AnimationStationary  gog 0x00474420  callconv=(blank)  param_size_bytes=2  gog_size=132  gog_retn=4
Candidate Steam address: 0x007ef2c0 (size 132, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef350:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004743a0 CSWCObject::SetOrientation -> 0x007ef240  gate_class=AUTO
- GOG-order neighbour after:  0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> 0x007ef730  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ef2c0: 16-byte aligned: YES
  007ef2c0  55             PUSH EBP
  007ef2c1  8B EC          MOV EBP,ESP
  007ef2c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007ef2c0 FUN_007ef2c0 size=132 ret=4 callers=6 callees=1

bool __thiscall FUN_007ef2c0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe28,&local_8);
  return iVar1 == 0 || local_8 != 0;
}
```

### decoy: 0x007ef350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef350: 16-byte aligned: YES
  007ef350  55             PUSH EBP
  007ef351  8B EC          MOV EBP,ESP
  007ef353  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B10a-1-16

GOG function: CSWSObject::GetNumActionGroups  gog 0x006a62c0  callconv=(blank)  param_size_bytes=0  gog_size=177  gog_retn=0
Candidate Steam address: 0x0053f050 (size 868, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::Destructor_2 | G7 mac: pass(0.13) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f3c0:STR0/CALLEE1/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6240 CSWSObject::GetNodeById -> 0x0053f560  gate_class=AUTO
- GOG-order neighbour after:  0x006a6380 CSWSObject::RemoveGroup -> 0x0053f6a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0069f2c0 CSWSCreature::ClearHostileActionsVersus -> Steam 0x005867e0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x6c} | Steam {0x4,0x8,0x34,0x18c,0x1b0,0x1b4,0x1d0,0x264}
imm sets (* = in both, 1 common): GOG {0x14,0x100*} | Steam {0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xc,0x18,0x58,0x100*,0x104,...(+6)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053f050: 16-byte aligned: YES
  0053f050  55             PUSH EBP
  0053f051  8B EC          MOV EBP,ESP
  0053f053  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0053f050
string literals (0):
direct callees (16):
  0x006A2710  (unnamed)
  0x009196FD  operator delete
  0x00547040  CSWSObject::ClearMatchedExpressionStrings
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DEA0  (unnamed)
  0x005E4CA0  CGameEffect::Destructor
  0x00840EF0  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x0053E740  Action-list free params
  0x007F2630  CSWCObject::DetachFromServerObject
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x005E6DE0  CSWSScriptVarTable::Destructor
  0x0072F5F0  (unnamed)
  0x00735360  CExoLocString::Destructor
struct offsets touched (top 8):
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x18C [dec 396]  x3  via EAX,EDX
  +0x1D0 [dec 464]  x3  via EAX,ECX
  +0x264 [dec 612]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EDX
  +0x1B0 [dec 432]  x2  via ECX
  +0x8 [dec 8]  x1  via EAX
  +0x1B4 [dec 436]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f050 FUN_0053f050 size=868 ret=0 callers=30 callees=16

void __fastcall FUN_0053f050(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0094f6ee;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWSObject::vftable;
  local_8 = 10;
  if (param_1[0xd = byte +0x34] != 0) {
    if ((undefined4 *)param_1[0xd = byte +0x34] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0xd = byte +0x34])(1,uVar1);
    }
    param_1[0xd = byte +0x34] = 0;
  }
  for (local_1c = 0; local_1c < (int)param_1[0x6d = byte +0x1b4]; local_1c = local_1c + 1) {
    if ((*(int *)(param_1[0x6c = byte +0x1b0] + local_1c * 4) != 0) &&
       (iVar2 = *(int *)(param_1[0x6c = byte +0x1b0] + local_1c * 4), iVar2 != 0)) {
      FUN_006a2710();
      FUN_009196fd(iVar2);
    }
  }
  FUN_00547040();
  uVar3 = param_1[1 = byte +0x4];
  FUN_0051c080(uVar3);
  FUN_0053dea0(uVar3);
  local_18 = param_1[0x53 = byte +0x14c];
  while (local_18 = local_18 + -1, -1 < local_18) {
    iVar2 = *(int *)(param_1[0x52 = byte +0x148] + local_18 * 4);
    if (iVar2 != 0) {
      FUN_005e4ca0();
      FUN_009196fd(iVar2);
    }
  }
  FUN_00840ef0(0);
  if (param_1[0x74 = byte +0x1d0] != 0) {
    FUN_009196fd(param_1[0x74 = byte +0x1d0]);
    param_1[0x74 = byte +0x1d0] = 0;
  }
  if (param_1[99 = byte +0x18c] != 0) {
    FUN_009196fd(param_1[99 = byte +0x18c]);
    param_1[99 = byte +0x18c] = 0;
  }
  while (iVar2 = FUN_007a1780(), iVar2 == 0) {
    iVar2 = FUN_007f2a90();
    if (iVar2 != 0) {
      FUN_0053e740();
      FUN_009196fd(iVar2);
    }
  }
  if (param_1[0x99 = byte +0x264] != 0) {
    FUN_007f2630();
    param_1[0x99 = byte +0x264] = 0;
  }
  local_8._0_1_ = 9;
  Free();
  local_8._0_1_ = 8;
  Free();
  local_8._0_1_ = 7;
  FUN_00733780();
  local_8._0_1_ = 6;
  Free();
  local_8._0_1_ = 5;
  Free();
  local_8._0_1_ = 4;
  Free();
  local_8._0_1_ = 3;
  FUN_005e6de0();
  local_8._0_1_ = 2;
  FUN_0072f5f0();
/* ... 8 more lines truncated by --max-lines */
```

### decoy: 0x0053f3c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f3c0: 16-byte aligned: YES
  0053f3c0  55             PUSH EBP
  0053f3c1  8B EC          MOV EBP,ESP
  0053f3c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f3c0
string literals (0):
direct callees (5):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x005D7190  CSWSObject::GetAcceptableAction
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via EAX
```

## Card B10a-1-17

GOG function: CSWCObject::ClientServerSyncHandler_Orientation  gog 0x004726f0  callconv=(blank)  param_size_bytes=4  gog_size=33  gog_retn=4
Candidate Steam address: 0x007ed590 (size 33, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed5c0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004726c0 CSWCObject::ClientServerSyncHandler_Position -> 0x00405970  gate_class=REJECT
- GOG-order neighbour after:  0x00472720 CSWCObject::Destructor -> 0x007ed5c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x88*} | Steam {0x88*}
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
prologue for 0x007ed590: 16-byte aligned: YES
  007ed590  55             PUSH EBP
  007ed591  8B EC          MOV EBP,ESP
  007ed593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed590
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x88 [dec 136]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed590 FUN_007ed590 size=33 ret=4 callers=0 callees=0

void __thiscall FUN_007ed590(int *param_1,undefined4 param_2)

{
  (**(code **)(*param_1 + 0x88))(param_2);
  return;
}
```

### decoy: 0x007ed5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5c0: 16-byte aligned: YES
  007ed5c0  55             PUSH EBP
  007ed5c1  8B EC          MOV EBP,ESP
  007ed5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed5c0
string literals (0):
direct callees (2):
  0x007ED5F0  CSWCObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B10a-1-18

GOG function: CSWSObject::Destructor  gog 0x006a5d00  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0053f4a0 (size 187, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::GetPositionByGroupIndex | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f560:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a5ce0 CSWSObject::GetLastName -> 0x0053ef50  gate_class=REVIEW
- GOG-order neighbour after:  0x006a5d30 CSWSObject::Destructor_2 -> 0x0053f050  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x006a5d30 CSWSObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x6c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x14,0x100}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053f4a0: 16-byte aligned: YES
  0053f4a0  55             PUSH EBP
  0053f4a1  8B EC          MOV EBP,ESP
  0053f4a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f4a0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x6C [dec 108]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f4a0 FUN_0053f4a0 size=187 ret=4 callers=2 callees=4

undefined4 __thiscall FUN_0053f4a0(undefined4 param_1,int param_2)

{
  int iVar1;
  int local_14;
  int local_10;
  undefined4 local_c;
  short local_8;
  
  local_14 = 0;
  iVar1 = FUN_007a1780(param_1);
  if (iVar1 == 1) {
    local_c = 0;
  }
  else {
    local_c = FUN_007a1720();
    local_10 = FUN_00561430(local_c);
    local_8 = *(short *)(local_10 + 0x6c);
    if (param_2 != 0) {
      while (local_10 != 0) {
        if (*(short *)(local_10 + 0x6c) != local_8) {
          local_8 = *(short *)(local_10 + 0x6c);
          local_14 = local_14 + 1;
        }
        if (local_14 == param_2) {
          return local_c;
        }
        local_10 = FUN_0058c370(&local_c);
      }
      local_c = 0;
    }
  }
  return local_c;
}
```

### decoy: 0x0053f560 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f560: 16-byte aligned: YES
  0053f560  55             PUSH EBP
  0053f561  8B EC          MOV EBP,ESP
  0053f563  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f560
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B10a-1-19

GOG function: CSWCObject::AnimationRunning  gog 0x00474a70  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef470 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef3e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW
- GOG-order neighbour after:  0x00474b00 CSWCObject::AddFireForgetAnimation -> 0x007ef9a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ef470: 16-byte aligned: YES
  007ef470  55             PUSH EBP
  007ef471  8B EC          MOV EBP,ESP
  007ef473  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef470
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
// FN 007ef470 FUN_007ef470 size=129 ret=4 callers=1 callees=1

bool __thiscall FUN_007ef470(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe34,&local_8);
  return iVar1 != 0 && local_8 != 0;
}
```

### decoy: 0x007ef3e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef3e0: 16-byte aligned: YES
  007ef3e0  55             PUSH EBP
  007ef3e1  8B EC          MOV EBP,ESP
  007ef3e3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef3e0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B10a-1-20

GOG function: CSWCObject::SetId  gog 0x00410c30  callconv=(blank)  param_size_bytes=4  gog_size=22  gog_retn=4
Candidate Steam address: 0x00764f60 (size 22, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bb30:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c50 CSWCObject::ShowShadowBlob -> 0x00764f80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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

### decoy: 0x0078bb30 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B10a-1-21

GOG function: CSWCObject::RemoveAllFireForgetAnimations  gog 0x00475420  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x007f02c0 (size 53, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f00b0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00475210 CSWCObject::RemoveTopFireForgetAnimation -> 0x007f00b0  gate_class=AUTO
- GOG-order neighbour after:  0x00475460 CSWCObject::AbortFireForgetAnimation -> 0x007f0300  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00475210 CSWCObject::RemoveTopFireForgetAnimation -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x5a*} | Steam {0x5a*}
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
prologue for 0x007f02c0: 16-byte aligned: YES
  007f02c0  55             PUSH EBP
  007f02c1  8B EC          MOV EBP,ESP
  007f02c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f02c0
string literals (0):
direct callees (1):
  0x007F00B0  CSWCObject::RemoveTopFireForgetAnimation
struct offsets touched (top 1):
  +0x5A [dec 90]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f02c0 FUN_007f02c0 size=53 ret=0 callers=1 callees=1

void __fastcall FUN_007f02c0(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < (int)(uint)*(ushort *)(param_1 + 0x5a); local_8 = local_8 + 1) {
    FUN_007f00b0();
  }
  return;
}
```

### decoy: 0x007f00b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f00b0: 16-byte aligned: YES
  007f00b0  55             PUSH EBP
  007f00b1  8B EC          MOV EBP,ESP
  007f00b3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f00b0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x5C [dec 92]  x21  via EAX,ECX,EDX,ESI
  +0x60 [dec 96]  x11  via EAX,ECX,EDX
```

## Card B10a-1-22

GOG function: CSWCObject::DisableDistortion  gog 0x00477820  callconv=__thiscall  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007f2680 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2650:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:OnClose
- GOG-order neighbour before: 0x004777f0 CSWCObject::EnableDistortion -> 0x007f2650  gate_class=REVIEW
- GOG-order neighbour after:  0x00477850 CSWCObject::GetReticlePoint -> 0x007f26b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xe8*} | Steam {0x68*,0xe8*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f2680: 16-byte aligned: YES
  007f2680  55             PUSH EBP
  007f2681  8B EC          MOV EBP,ESP
  007f2683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2680
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2680 CMDITabProxyWnd::OnClose size=44 ret=0 callers=2 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    protected: void __thiscall CMDITabProxyWnd::OnClose(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CMDITabProxyWnd::OnClose(CMDITabProxyWnd *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xe8))();
  }
  return;
}
```

### decoy: 0x007f2650 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2650: 16-byte aligned: YES
  007f2650  55             PUSH EBP
  007f2651  8B EC          MOV EBP,ESP
  007f2653  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2650
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xE4 [dec 228]  x1  via EDX
```

## Card B10a-1-23

GOG function: CSWCObject::AnimationParry  gog 0x00474950  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef7f0 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004748e0 CSWCObject::AnimationDialog -> 0x007ef780  gate_class=AUTO
- GOG-order neighbour after:  0x004749e0 CSWCObject::AnimationAttack -> 0x007ef880  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ef7f0: 16-byte aligned: YES
  007ef7f0  55             PUSH EBP
  007ef7f1  8B EC          MOV EBP,ESP
  007ef7f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef7f0
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
// FN 007ef7f0 FUN_007ef7f0 size=129 ret=4 callers=2 callees=1

bool __thiscall FUN_007ef7f0(int param_1,undefined2 param_2)

{
  int iVar1;
  int local_8;
  
  local_8 = 0;
  if (*(int *)(param_1 + 0x68) != 0) {
    param_2 = (**(code **)(**(int **)(param_1 + 0x68) + 0xe0))(param_2);
  }
  iVar1 = FUN_0071b8b0(param_2,DAT_00a0fe3c,&local_8);
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

## Card B10a-1-24

GOG function: CSWCObject::AnimationHideEquippedItems  gog 0x00474540  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef910 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef880:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004744b0 CSWCObject::AnimationPlayOutOfPlace -> 0x007ef730  gate_class=REVIEW
- GOG-order neighbour after:  0x004745d0 CSWCObject::AnimationWalking -> 0x007ef3e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-1-25

GOG function: CSWCObject::SetLoopingAnimation  gog 0x00472ab0  callconv=(blank)  param_size_bytes=2  gog_size=42  gog_retn=4
Candidate Steam address: 0x007ed950 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: AnimationLooping 0x7ef500; callers: SetAnimationWithoutTrigger 0x87b9e0)
- best decoy (neighbour/other candidate) scores: 0x007ed9a0:STR0/CALLEE1/SIG0.021/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472a70 CSWCObject::GetAnimationLength -> 0x007ed910  gate_class=AUTO
- GOG-order neighbour after:  0x00472ae0 CSWCObject::GetLoopingAnimation -> 0x007ed980  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00474660 CSWCObject::AnimationLooping -> Steam 0x007ef500  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x005072c0 CSWCDoor::SetAnimationWithoutTrigger -> Steam 0x0087b9e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x58*} | Steam {0x58*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ed950: 16-byte aligned: YES
  007ed950  55             PUSH EBP
  007ed951  8B EC          MOV EBP,ESP
  007ed953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed950
string literals (0):
direct callees (1):
  0x007EF500  CSWCObject::AnimationLooping
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed950 FUN_007ed950 size=42 ret=4 callers=6 callees=1

void __thiscall FUN_007ed950(int param_1,undefined2 param_2)

{
  int iVar1;
  
  iVar1 = FUN_007ef500(param_2);
  if (iVar1 == 1) {
    *(undefined2 *)(param_1 + 0x58) = param_2;
  }
  return;
}
```

### decoy: 0x007ed9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed9a0: 16-byte aligned: YES
  007ed9a0  55             PUSH EBP
  007ed9a1  8B EC          MOV EBP,ESP
  007ed9a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ed9a0
string literals (3):
  0x009A2384  at 007ee5b8  'DeathVFX'
  0x009A2374  at 007ee68f  'DeathVFXNode'
  0x009914B8  at 007ee736  'impact'
direct callees (27):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0073F750  ClientApp::GetGuiInGame
  0x007EF730  CSWCObject::AnimationPlayOutOfPlace
  0x00774B40  CSWCCreature::StopAmbientPauseAnim
  0x007EF910  CSWCObject::AnimationHideEquippedItems
  0x007107F0  (unnamed)
  0x00773A10  CSWCCreature::HideEquippedItems
  0x007EEEE0  (unnamed)
  0x007759B0  CSWCCreature::GetCameraFollowPart
  0x007EF500  CSWCObject::AnimationLooping
  0x007ED950  CSWCObject::SetLoopingAnimation
  0x007F00B0  CSWCObject::RemoveTopFireForgetAnimation
  0x007F0300  CSWCObject::AbortFireForgetAnimation
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00733540  CExoString::Constructor
  0x0071B1D0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00734010  CExoString::CStr
struct offsets touched (top 10):
  +0x68 [dec 104]  x47  via EAX,ECX,EDX
  +0x58 [dec 88]  x37  via EAX,ECX,EDX
  +0x48 [dec 72]  x17  via EAX,ECX,EDX
  +0x2C [dec 44]  x13  via EAX,EDX
  +0x44 [dec 68]  x9  via EAX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x50 [dec 80]  x4  via EAX,ECX,EDX
  +0x54 [dec 84]  x4  via ECX,EDX
  +0xC6 [dec 198]  x4  via EAX
```

## Card B10a-1-26

GOG function: CSWSObject::GetPortrait  gog 0x00663840  callconv=__thiscall  param_size_bytes=4  gog_size=47  gog_retn=4
Candidate Steam address: 0x0053efa0 (size 47, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0053efd0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: autolabel:GetRect
- GOG-order neighbour before: 0x00663830 CSWSObject::GetLockOrientationToObject -> -  gate_class=NOCAND
- GOG-order neighbour after:  0x00663870 CSWSObject::SetPortrait -> 0x0053efd0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 2 common): GOG {0x4*,0x20*} | Steam {0x4*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053efa0: 16-byte aligned: YES
  0053efa0  55             PUSH EBP
  0053efa1  8B EC          MOV EBP,ESP
  0053efa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053efa0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053efa0 CMFCRibbonCategory::GetRect size=47 ret=4 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: class CRect __thiscall CMFCRibbonCategory::GetRect(void)const 
   
   Library: Visual Studio 2010 Debug */

undefined4 * __thiscall CMFCRibbonCategory::GetRect(CMFCRibbonCategory *this)

{
  undefined4 *in_stack_00000004;
  
  *in_stack_00000004 = *(undefined4 *)(this + 0x20);
  in_stack_00000004[1 = byte +0x4] = *(undefined4 *)(this + 0x24);
  in_stack_00000004[2 = byte +0x8] = *(undefined4 *)(this + 0x28);
  in_stack_00000004[3 = byte +0xc] = *(undefined4 *)(this + 0x2c);
  return in_stack_00000004;
}
```

### decoy: 0x0053efd0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

## Card B10a-1-27

GOG function: CSWCObject::Destructor_3  gog 0x004727b0  callconv=(blank)  param_size_bytes=0  gog_size=379  gog_retn=0
Candidate Steam address: 0x007ed5f0 (size 475, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCObject::Destructor_2 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.94 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: GetClientAIMaster 0x73f790); SIG 0.94
- best decoy (neighbour/other candidate) scores: 0x007ed5c0:STR0/CALLEE0/SIG0.071/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472750 CSWCObject::Destructor_2 -> 0x007ed5f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472930 CSWCObject::GetCurrentAnimation -> 0x007ed7d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040cc50 CClientExoApp::GetClientAIMaster -> Steam 0x0073f790  in candidate callees: yes
  0x0044eee0 CClientAIMaster::RemoveObject -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x0063a020 CExoStreamingSoundSource::Destructor -> Steam ?  in candidate callees: n/a
  0x006b15e0 CSWSObject::DetachFromClientObject -> Steam ?  in candidate callees: n/a
  0x00758fa0 CExoLinkedListInternal::Destructor -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x5c*,0xc8*,0xcc*,0xf8*} | Steam {0x4*,0x5c*,0xc8*,0xcc*,0xf8*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0x6c*,0x70*,0xa8*,0xe8*} | Steam {0x2*,0x3*,0x4*,0x30,0x6c*,0x70*,0xa8*,0xe8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007ed5f0
string literals (0):
direct callees (11):
  0x0073F8D0  CClientExoApp::AddObjectToHitcheckIgnoreList
  0x007F0A50  CSWCObject::KillAllVisualEffects
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A2540  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x0054A900  CSWSObject::DetachFromClientObject
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0072F5F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0xC8 [dec 200]  x3  via EAX,ECX
  +0xCC [dec 204]  x3  via EAX,EDX
  +0xF8 [dec 248]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed5f0 FUN_007ed5f0 size=475 ret=0 callers=30 callees=11

void __fastcall FUN_007ed5f0(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_0096b9ca;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = CSWCObject::vftable;
  local_8 = 4;
  FUN_0073f8d0(param_1[1 = byte +0x4]);
  FUN_007f0a50(uVar2);
  FUN_0073f790();
  FUN_007a2540(param_1);
  if (param_1[0x32 = byte +0xc8] != 0) {
    if ((undefined4 *)param_1[0x32 = byte +0xc8] != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)param_1[0x32 = byte +0xc8])(1);
    }
    param_1[0x32 = byte +0xc8] = 0;
  }
  if (param_1[0x33 = byte +0xcc] != 0) {
    iVar1 = param_1[0x33 = byte +0xcc];
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    param_1[0x33 = byte +0xcc] = 0;
  }
  if (param_1[0x17 = byte +0x5c] != 0) {
    FUN_009196fd(param_1[0x17 = byte +0x5c]);
    param_1[0x17 = byte +0x5c] = 0;
  }
  if (param_1[0x3e = byte +0xf8] != 0) {
    FUN_0054a900();
    param_1[0x3e = byte +0xf8] = 0;
  }
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  FUN_00623320();
  local_8._0_1_ = 1;
  Free();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_0072f5f0();
  *param_1 = CGameObject::vftable;
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007ed5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5c0: 16-byte aligned: YES
  007ed5c0  55             PUSH EBP
  007ed5c1  8B EC          MOV EBP,ESP
  007ed5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed5c0
string literals (0):
direct callees (2):
  0x007ED5F0  CSWCObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B10a-1-28

GOG function: CSWCObject::GetCanSeeHeight  gog 0x00410cb0  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00764fe0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764fb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c80 CSWCObject::HideShadowBlob -> 0x00764fb0  gate_class=REVIEW
- GOG-order neighbour after:  0x00410cd0 CSWCCreature::GetPortrait  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
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
prologue for 0x00764fe0: 16-byte aligned: YES
  00764fe0  55             PUSH EBP
  00764fe1  8B EC          MOV EBP,ESP
  00764fe3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764fe0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00764fe0 FUN_00764fe0 size=17 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00764fe0(void)

{
  return (float10)_DAT_0098b5c0;
}
```

### decoy: 0x00764fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B10a-1-29

GOG function: CSWCObject::GetPortraitId  gog 0x00472610  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x007ed4b0 (size 21, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004725e0 CSWCObject::GetPortrait -> 0x007ed480  gate_class=REVIEW
- GOG-order neighbour after:  0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xb8*} | Steam {0xb8*}
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
prologue for 0x007ed4b0: 16-byte aligned: YES
  007ed4b0  55             PUSH EBP
  007ed4b1  8B EC          MOV EBP,ESP
  007ed4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed4b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed4b0 FUN_007ed4b0 size=21 ret=0 callers=0 callees=0

undefined2 __fastcall FUN_007ed4b0(int param_1)

{
  return *(undefined2 *)(param_1 + 0xb8);
}
```

### decoy: 0x007ed4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B10a-1-30

GOG function: CSWCObject::ClearAllQueuedCombatActions  gog 0x004776a0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x007f2500 (size 54, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: ClearAllQueuedCombatActions 0x51cba0)
- best decoy (neighbour/other candidate) scores: 0x007f24c0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00477660 CSWCObject::ClearAllActions -> 0x007f24c0  gate_class=REVIEW
- GOG-order neighbour after:  0x004776e0 CSWCObject::GetServerObject -> 0x007f2540  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x0064c5c0 CServerExoApp::ClearAllQueuedCombatActions -> Steam 0x0051cba0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007f2500: 16-byte aligned: YES
  007f2500  55             PUSH EBP
  007f2501  8B EC          MOV EBP,ESP
  007f2503  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2500
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBA0  CServerExoApp::ClearAllQueuedCombatActions
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2500 FUN_007f2500 size=54 ret=0 callers=2 callees=2

void __fastcall FUN_007f2500(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  FUN_0051cba0(uVar1);
  return;
}
```

### decoy: 0x007f24c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f24c0: 16-byte aligned: YES
  007f24c0  55             PUSH EBP
  007f24c1  8B EC          MOV EBP,ESP
  007f24c3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f24c0
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CB10  CServerExoApp::ClearAllActions
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
```

## Card B10a-1-31

GOG function: CSWCObject::GetStickyMouseThreshhold  gog 0x00477ba0  callconv=(blank)  param_size_bytes=0  gog_size=73  gog_retn=0
Candidate Steam address: 0x007f2a00 (size 73, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2910:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00477ab0 CSWCObject::AcquireAnimationBase -> 0x007f2910  gate_class=AUTO
- GOG-order neighbour after:  0x00477bf0 CExoLinkedList_CSWCVisualEffectOnObject_::GetAtHead  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f2a00: 16-byte aligned: YES
  007f2a00  55             PUSH EBP
  007f2a01  8B EC          MOV EBP,ESP
  007f2a03  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2a00
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007f2a00 FUN_007f2a00 size=73 ret=0 callers=0 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_007f2a00(void)

{
  return (float10)(((float)DAT_009f42a8 / (float)_DAT_00989ee0) *
                   ((float)DAT_009f42a8 / (float)_DAT_00989ee0) +
                  ((float)DAT_009f42a4 / (float)_DAT_00989ee0) *
                  ((float)DAT_009f42a4 / (float)_DAT_00989ee0));
}
```

### decoy: 0x007f2910 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2910: 16-byte aligned: YES
  007f2910  55             PUSH EBP
  007f2911  8B EC          MOV EBP,ESP
  007f2913  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B10a-1-32

GOG function: CSWSObject::GetLastName  gog 0x006a5ce0  callconv=__thiscall  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x0053e740 (size 239, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Action-list | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053e830:STR0/CALLEE0/SIG0.015/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a55e0 CSWSObject::Constructor -> 0x0053e830  gate_class=REVIEW
- GOG-order neighbour after:  0x006a5d00 CSWSObject::Destructor -> 0x0053f020  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x38,0x6e}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {0x4,0x5,0x18}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053e740: 16-byte aligned: YES
  0053e740  55             PUSH EBP
  0053e741  8B EC          MOV EBP,ESP
  0053e743  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053e740
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
  0x006FDBB0  CVirtualMachine::DeleteScriptSituation
struct offsets touched (top 1):
  +0x6E [dec 110]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0053e740 FUN_0053e740 size=239 ret=0 callers=11 callees=3

void __fastcall FUN_0053e740(int param_1)

{
  int iVar1;
  int local_c;
  
  for (local_c = 0; local_c < (int)(uint)*(ushort *)(param_1 + 0x6e); local_c = local_c + 1) {
    if ((*(int *)(param_1 + 0x38 + local_c * 4) != 0) && (*(int *)(param_1 + 4 + local_c * 4) == 4))
    {
      iVar1 = *(int *)(param_1 + 0x38 + local_c * 4);
      if (iVar1 != 0) {
        FUN_00733780();
        FUN_009196fd(iVar1);
      }
      *(undefined4 *)(param_1 + 0x38 + local_c * 4) = 0;
    }
    if (((*(int *)(param_1 + 0x38 + local_c * 4) != 0) && (*(int *)(param_1 + 4 + local_c * 4) == 5)
        ) && (DAT_00a1b4a8 != 0)) {
      FUN_006fdbb0(*(undefined4 *)(param_1 + 0x38 + local_c * 4));
      *(undefined4 *)(param_1 + 0x38 + local_c * 4) = 0;
    }
  }
  return;
}
```

### decoy: 0x0053e830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053e830: 16-byte aligned: YES
  0053e830  55             PUSH EBP
  0053e831  8B EC          MOV EBP,ESP
  0053e833  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0053e830
string literals (0):
direct callees (18):
  0x0053D660  (unnamed)
  0x00735240  (unnamed)
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x00722870  (unnamed)
  0x0052E4A0  (unnamed)
  0x005E91B0  CSWVarTable::Constructor
  0x0084A640  (unnamed)
  0x00543F50  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DD20  (unnamed)
  0x0053D890  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x00919723  operator new
  0x0091C630  (unnamed)
  0x0051C370  CServerExoApp::GetWorldTimer
  0x0051AD20  CWorldTimer::GetWorldTime
struct offsets touched (top 10):
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x54 [dec 84]  x2  via EAX,EDX
  +0x18C [dec 396]  x2  via ECX
  +0x1D0 [dec 464]  x2  via ECX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via ECX
  +0x16 [dec 22]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
```

## Card B10a-1-33

GOG function: CSWCObject::GetLoopingAnimation  gog 0x00472ae0  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x007ed980 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: AnimationUpdate 0x87b3f0, AnimationUpdate 0x841440)
- best decoy (neighbour/other candidate) scores: 0x007ed950:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472ab0 CSWCObject::SetLoopingAnimation -> 0x007ed950  gate_class=REVIEW
- GOG-order neighbour after:  0x00472b00 CSWCObject::SetAnimation -> 0x007ed9a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x00506cd0 CSWCDoor::AnimationUpdate -> Steam 0x0087b3f0  in candidate callers: yes
  0x00534cc0 CSWCPlaceable::AnimationUpdate -> Steam 0x00841440  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x58*} | Steam {0x58*}
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
prologue for 0x007ed980: 16-byte aligned: YES
  007ed980  55             PUSH EBP
  007ed981  8B EC          MOV EBP,ESP
  007ed983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed980
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed980 FUN_007ed980 size=18 ret=0 callers=9 callees=0

undefined2 __fastcall FUN_007ed980(int param_1)

{
  return *(undefined2 *)(param_1 + 0x58);
}
```

### decoy: 0x007ed950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed950: 16-byte aligned: YES
  007ed950  55             PUSH EBP
  007ed951  8B EC          MOV EBP,ESP
  007ed953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed950
string literals (0):
direct callees (1):
  0x007EF500  CSWCObject::AnimationLooping
struct offsets touched (top 1):
  +0x58 [dec 88]  x1  via ECX
```

## Card B10a-1-34

GOG function: CSWCObject::TestAndReEquipweapons  gog 0x00474040  callconv=(blank)  param_size_bytes=0  gog_size=163  gog_retn=8
Candidate Steam address: 0x007eeee0 (size 163, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007ef240:STR0/CALLEE0/SIG0.156/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00472b00 CSWCObject::SetAnimation -> 0x007ed9a0  gate_class=AUTO
- GOG-order neighbour after:  0x004740f0 CSWCObject::SetPosition -> 0x007eef90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0041f9c0 CSWCCreature::ShowEquippedItems -> Steam ?  in candidate callees: n/a
  0x00474a70 CSWCObject::AnimationRunning -> Steam 0x007ef470  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x2c*,0xcc*,0x3e4*} | Steam {0x4*,0x2c*,0xcc*,0x3e4*}
imm sets (* = in both, 2 common): GOG {0x8*,0x1c*} | Steam {0x8*,0x1c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x007eeee0: 16-byte aligned: YES
  007eeee0  55             PUSH EBP
  007eeee1  8B EC          MOV EBP,ESP
  007eeee3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007eeee0
string literals (0):
direct callees (3):
  0x007EF910  CSWCObject::AnimationHideEquippedItems
  0x0073F750  ClientApp::GetGuiInGame
  0x00773AF0  CSWCCreature::ShowEquippedItems
struct offsets touched (top 4):
  +0x2C [dec 44]  x3  via EDX
  +0x4 [dec 4]  x1  via EAX
  +0xCC [dec 204]  x1  via EDX
  +0x3E4 [dec 996]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007eeee0 FUN_007eeee0 size=163 ret=8 callers=2 callees=3

void __thiscall FUN_007eeee0(int *param_1,undefined4 param_2,undefined2 param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_007ef910(param_3);
  iVar2 = (**(code **)(*param_1 + 0x2c))();
  if ((((iVar2 != 0) && (iVar2 = (**(code **)(*param_1 + 0x2c))(), *(int *)(iVar2 + 0x3e4) != 0)) &&
      (iVar1 == 0)) && (iVar1 = FUN_0073f750(), *(int *)(iVar1 + 0xcc) == 0)) {
    (**(code **)(*param_1 + 0x2c))();
    FUN_00773af0();
  }
  return;
}
```

### decoy: 0x007ef240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef240: 16-byte aligned: YES
  007ef240  55             PUSH EBP
  007ef241  8B EC          MOV EBP,ESP
  007ef243  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef240
string literals (0):
direct callees (1):
  0x005143C0  (unnamed)
struct offsets touched (top 6):
  +0x68 [dec 104]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x50 [dec 80]  x1  via EDX
  +0x64 [dec 100]  x1  via EDX
  +0x90 [dec 144]  x1  via EDX
```

## Card B10a-1-35

GOG function: CSWCObject::TurnOffShadows  gog 0x00472660  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed500 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4d0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472630 CSWCObject::TurnOnShadows -> 0x007ed4d0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472690 CSWCObject::BarkString -> 0x007ed530  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xbc*} | Steam {0x68*,0xbc*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed500: 16-byte aligned: YES
  007ed500  55             PUSH EBP
  007ed501  8B EC          MOV EBP,ESP
  007ed503  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed500
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xBC [dec 188]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed500 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xbc))();
  }
  return;
}
```

### decoy: 0x007ed4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B10a-1-36

GOG function: CSWCObject::DetachVisualEffects  gog 0x00476f40  callconv=(blank)  param_size_bytes=0  gog_size=111  gog_retn=0
Candidate Steam address: 0x007f1de0 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f1e50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00476ec0 CSWCObject::CalculateProjectileTimeToTarget -> 0x007f1d60  gate_class=REVIEW
- GOG-order neighbour after:  0x00476fb0 CSWCObject::ReAttachVisualEffects -> 0x007f1e50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x00503930 CSWCVisualEffectOnObject::DetachVisualEffect -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xa8*,0xc0*} | Steam {0xa8*,0xc0*}
imm sets (* = in both, 2 common): GOG {0x14*,0x6c*} | Steam {0x14*,0x6c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f1de0: 16-byte aligned: YES
  007f1de0  55             PUSH EBP
  007f1de1  8B EC          MOV EBP,ESP
  007f1de3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f1de0
string literals (0):
direct callees (4):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00878050  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f1de0 FUN_007f1de0 size=111 ret=0 callers=0 callees=4

void __fastcall FUN_007f1de0(undefined4 param_1)

{
  int local_10;
  int local_c;
  int local_8;
  
  local_c = FUN_00521060(param_1);
  local_10 = FUN_007a1720();
  while (local_10 != 0) {
    if ((*(int *)(local_c + 0xc0) == 0) && (local_8 = *(int *)(local_c + 0xa8), local_8 != 0)) {
      FUN_00878050();
    }
    local_c = FUN_0058c370(&local_10);
  }
  return;
}
```

### decoy: 0x007f1e50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f1e50: 16-byte aligned: YES
  007f1e50  55             PUSH EBP
  007f1e51  8B EC          MOV EBP,ESP
  007f1e53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f1e50
string literals (0):
direct callees (4):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00878190  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

## Card B10a-1-37

GOG function: CSWCObject::ReAttachVisualEffects  gog 0x00476fb0  callconv=(blank)  param_size_bytes=0  gog_size=111  gog_retn=0
Candidate Steam address: 0x007f1e50 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f1de0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00476f40 CSWCObject::DetachVisualEffects -> 0x007f1de0  gate_class=REVIEW
- GOG-order neighbour after:  0x00477020 CSWCObject::RemoveShadowBlob -> 0x007f1ec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x00503a70 CSWCVisualEffectOnObject::StartVisualEffect -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xa8*,0xc0*} | Steam {0xa8*,0xc0*}
imm sets (* = in both, 2 common): GOG {0x14*,0x6c*} | Steam {0x14*,0x6c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f1e50: 16-byte aligned: YES
  007f1e50  55             PUSH EBP
  007f1e51  8B EC          MOV EBP,ESP
  007f1e53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f1e50
string literals (0):
direct callees (4):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00878190  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f1e50 FUN_007f1e50 size=111 ret=0 callers=0 callees=4

void __fastcall FUN_007f1e50(undefined4 param_1)

{
  int local_10;
  int local_c;
  int local_8;
  
  local_c = FUN_00521060(param_1);
  local_10 = FUN_007a1720();
  while (local_10 != 0) {
    if ((*(int *)(local_c + 0xc0) == 0) && (local_8 = *(int *)(local_c + 0xa8), local_8 != 0)) {
      FUN_00878190();
    }
    local_c = FUN_0058c370(&local_10);
  }
  return;
}
```

### decoy: 0x007f1de0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f1de0: 16-byte aligned: YES
  007f1de0  55             PUSH EBP
  007f1de1  8B EC          MOV EBP,ESP
  007f1de3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f1de0
string literals (0):
direct callees (4):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00878050  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

## Card B10a-1-38

GOG function: CSWSObject::SetDirtyFlag  gog 0x006b1110  callconv=(blank)  param_size_bytes=4  gog_size=74  gog_retn=4
Candidate Steam address: 0x0054a3e0 (size 69, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0054a430:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b10c0 CSWSObject::AnimationWalking -> 0x0054a3e0  gate_class=AUTO
- GOG-order neighbour after:  0x006b1160 CSWSObject::IsDirtyFlagSet -> 0x0054a480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00685f80 CSWSCreature::SetAnimation -> Steam 0x0056d350  in candidate callers: no
  0x006a8070 CSWSObject::SetAnimation -> Steam 0x00541390  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x224} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x5,0xc} | Steam {0x4*,0x8,0x2712,0x276d,0x2795}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a3e0: 16-byte aligned: YES
  0054a3e0  55             PUSH EBP
  0054a3e1  8B EC          MOV EBP,ESP
  0054a3e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a3e0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054a3e0 FUN_0054a3e0 size=69 ret=4 callers=1 callees=0

undefined4 FUN_0054a3e0(short param_1)

{
  undefined4 local_c;
  
  if (((param_1 == 0x2712) || (param_1 == 0x276d)) || (param_1 == 0x2795)) {
    local_c = 1;
  }
  else {
    local_c = 0;
  }
  return local_c;
}
```

### decoy: 0x0054a430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a430: 16-byte aligned: YES
  0054a430  55             PUSH EBP
  0054a431  8B EC          MOV EBP,ESP
  0054a433  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a430
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B10a-1-39

GOG function: CSWCObject::KillAllVisualEffects  gog 0x00475bb0  callconv=(blank)  param_size_bytes=0  gog_size=121  gog_retn=0
Candidate Steam address: 0x007f0a50 (size 121, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f0ad0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00475b50 CSWCObject::StopAllVisualEffects -> 0x007f09f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00475c30 CSWCObject::RefreshVisualEffects -> 0x007f0ad0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x004fad60 CSWCVisualEffectOnObject::Destructor -> Steam ?  in candidate callees: n/a
  0x004fe0d0 CSWCVisualEffectOnObject::KillVisualEffect -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x6c*} | Steam {0x4*,0x14*,0x6c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f0a50: 16-byte aligned: YES
  007f0a50  55             PUSH EBP
  007f0a51  8B EC          MOV EBP,ESP
  007f0a53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f0a50
string literals (0):
direct callees (5):
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x008727F0  (unnamed)
  0x0086F480  CSWCVisualEffectOnObject::dtor (mapper vote)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007f0a50 FUN_007f0a50 size=121 ret=0 callers=1 callees=5

void FUN_007f0a50(void)

{
  int iVar1;
  
  while (iVar1 = FUN_007a1780(), iVar1 == 0) {
    iVar1 = FUN_007f2a90();
    if ((iVar1 != 0) && (FUN_008727f0(), iVar1 != 0)) {
      FUN_0086f480();
      FUN_009196fd(iVar1);
    }
  }
  return;
}
```

### decoy: 0x007f0ad0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f0ad0: 16-byte aligned: YES
  007f0ad0  55             PUSH EBP
  007f0ad1  8B EC          MOV EBP,ESP
  007f0ad3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f0ad0
string literals (0):
direct callees (5):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00871820  (unnamed)
  0x008710D0  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

## Card B10a-1-40

GOG function: CSWCObject::RemoveShadowBlob  gog 0x00477020  callconv=(blank)  param_size_bytes=0  gog_size=78  gog_retn=0
Candidate Steam address: 0x007f1ec0 (size 78, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007f1e50:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00476fb0 CSWCObject::ReAttachVisualEffects -> 0x007f1e50  gate_class=REVIEW
- GOG-order neighbour after:  0x00477070 CSWCObject::SetGroundNormal -> 0x007f1f10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x68*,0xa8*,0xb8*,0xd4*} | Steam {0x68*,0xa8*,0xb8*,0xd4*}
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
prologue for 0x007f1ec0: 16-byte aligned: YES
  007f1ec0  55             PUSH EBP
  007f1ec1  8B EC          MOV EBP,ESP
  007f1ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f1ec0
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0xA8 [dec 168]  x1  via EDX
  +0xB8 [dec 184]  x1  via EDX
  +0xD4 [dec 212]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f1ec0 FUN_007f1ec0 size=78 ret=0 callers=1 callees=0

void __fastcall FUN_007f1ec0(int param_1)

{
  if ((*(int *)(param_1 + 0x68) != 0) &&
     ((**(code **)(**(int **)(param_1 + 0x68) + 0xa8))(), *(int *)(param_1 + 0xd4) != 0)) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xb8))();
  }
  return;
}
```

### decoy: 0x007f1e50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f1e50: 16-byte aligned: YES
  007f1e50  55             PUSH EBP
  007f1e51  8B EC          MOV EBP,ESP
  007f1e53  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f1e50
string literals (0):
direct callees (4):
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00878190  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 2):
  +0xA8 [dec 168]  x1  via EDX
  +0xC0 [dec 192]  x1  via EAX
```

## Card B10a-1-41

GOG function: CSWCObject::GetPortrait  gog 0x004725e0  callconv=(blank)  param_size_bytes=5  gog_size=39  gog_retn=8
Candidate Steam address: 0x007ed480 (size 39, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed850:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004725b0 CSWCObject::GetCameraFollowPart -> 0x007ed450  gate_class=AUTO
- GOG-order neighbour after:  0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00495600 CSWPortrait::GetPortrait -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0xa8*} | Steam {0x8*,0xa8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007ed480: 16-byte aligned: YES
  007ed480  55             PUSH EBP
  007ed481  8B EC          MOV EBP,ESP
  007ed483  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed480
string literals (0):
direct callees (1):
  0x0085E8C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed480 FUN_007ed480 size=39 ret=8 callers=0 callees=1

undefined4 FUN_007ed480(undefined4 param_1,undefined1 param_2)

{
  FUN_0085e8c0(param_1,param_2);
  return param_1;
}
```

### decoy: 0x007ed850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed850: 16-byte aligned: YES
  007ed850  55             PUSH EBP
  007ed851  8B EC          MOV EBP,ESP
  007ed853  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed850
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EDX
```

## Card B10a-1-42

GOG function: CSWCObject::DetachFromServerObject  gog 0x004777d0  callconv=(blank)  param_size_bytes=0  gog_size=24  gog_retn=0
Candidate Steam address: 0x007f2630 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2610:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004777b0 CSWCObject::AttachmentFromServerObject -> 0x007f2610  gate_class=REVIEW
- GOG-order neighbour after:  0x004777f0 CSWCObject::EnableDistortion -> 0x007f2650  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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

## Card B10a-1-43

GOG function: CSWCObject::UnHilite  gog 0x004754e0  callconv=(blank)  param_size_bytes=1  gog_size=42  gog_retn=4
Candidate Steam address: 0x007f0380 (size 42, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f0750:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004754a0 CSWCObject::Hilite -> 0x007f0340  gate_class=REVIEW
- GOG-order neighbour after:  0x00475510 CSWCObject::SetAlpha -> 0x007f03b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c9350 CSWCAnimBase::UnHilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
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
prologue for 0x007f0380: 16-byte aligned: YES
  007f0380  55             PUSH EBP
  007f0381  8B EC          MOV EBP,ESP
  007f0383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f0380
string literals (0):
direct callees (1):
  0x00861480  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f0380 FUN_007f0380 size=42 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_007f0380(int param_1,undefined1 param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x68) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00861480(param_2);
  }
  return uVar1;
}
```

### decoy: 0x007f0750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f0750: 16-byte aligned: YES
  007f0750  55             PUSH EBP
  007f0751  8B EC          MOV EBP,ESP
  007f0753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f0750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
```

## Card B10a-1-44

GOG function: CSWCObject::ShowShadowBlob  gog 0x00410c50  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x00764f80 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764fb0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410c30 CSWCObject::SetId -> 0x00764f60  gate_class=REVIEW
- GOG-order neighbour after:  0x00410c80 CSWCObject::HideShadowBlob -> 0x00764fb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xac*} | Steam {0x68*,0xac*}
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
prologue for 0x00764f80: 16-byte aligned: YES
  00764f80  55             PUSH EBP
  00764f81  8B EC          MOV EBP,ESP
  00764f83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00764f80
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xAC [dec 172]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00764f80 FUN_00764f80 size=44 ret=0 callers=0 callees=0

void __fastcall FUN_00764f80(int param_1)

{
  if (*(int *)(param_1 + 0x68) != 0) {
    (**(code **)(**(int **)(param_1 + 0x68) + 0xac))(0);
  }
  return;
}
```

### decoy: 0x00764fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764fb0: 16-byte aligned: YES
  00764fb0  55             PUSH EBP
  00764fb1  8B EC          MOV EBP,ESP
  00764fb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764fb0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB0 [dec 176]  x1  via EDX
```

## Card B10a-1-45

GOG function: CSWCObject::BarkString  gog 0x00472690  callconv=(blank)  param_size_bytes=16  gog_size=45  gog_retn=16
Candidate Steam address: 0x007ed530 (size 45, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed890:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:OnDragEnter
- GOG-order neighbour before: 0x00472660 CSWCObject::TurnOffShadows -> 0x007ed500  gate_class=REVIEW
- GOG-order neighbour after:  0x004726c0 CSWCObject::ClientServerSyncHandler_Position -> 0x00405970  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x124*} | Steam {0x124*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x007ed530: 16-byte aligned: YES
  007ed530  55             PUSH EBP
  007ed531  8B EC          MOV EBP,ESP
  007ed533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed530
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x124 [dec 292]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed530 OnDragEnter size=45 ret=16 callers=0 callees=0

/* Library Function - Multiple Matches With Same Base Name [GHIDRA-AUTO-LABEL: unreliable]
    protected: virtual unsigned long __thiscall CMFCOutlookBarTabCtrl::OnDragEnter(class
   COleDataObject *,unsigned long,class CPoint)
    public: virtual unsigned long __thiscall CMFCTabCtrl::OnDragEnter(class COleDataObject
   *,unsigned long,class CPoint)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
OnDragEnter(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
           )

{
  (**(code **)(*param_1 + 0x124))(param_2,param_3,param_4,param_5);
  return;
}
```

### decoy: 0x007ed890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed890: 16-byte aligned: YES
  007ed890  55             PUSH EBP
  007ed891  8B EC          MOV EBP,ESP
  007ed893  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed890
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EDX
```

## Card B10a-1-46

GOG function: CSWCObject::Hilite  gog 0x004754a0  callconv=(blank)  param_size_bytes=5  gog_size=50  gog_retn=16
Candidate Steam address: 0x007f0340 (size 50, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f0380:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00475460 CSWCObject::AbortFireForgetAnimation -> 0x007f0300  gate_class=AUTO
- GOG-order neighbour after:  0x004754e0 CSWCObject::UnHilite -> 0x007f0380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004c92d0 CSWCAnimBase::Hilite -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x68*} | Steam {0x68*}
imm sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 5 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x007f0340: 16-byte aligned: YES
  007f0340  55             PUSH EBP
  007f0341  8B EC          MOV EBP,ESP
  007f0343  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f0340
string literals (0):
direct callees (1):
  0x00861400  CSWCAnimBase::Hilite
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007f0340 FUN_007f0340 size=50 ret=16 callers=0 callees=1

undefined4 __thiscall
FUN_007f0340(int param_1,undefined4 param_2,undefined1 param_3,undefined4 param_4,undefined4 param_5
            )

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x68) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_00861400(param_2,param_3,param_5);
  }
  return uVar1;
}
```

### decoy: 0x007f0380 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f0380: 16-byte aligned: YES
  007f0380  55             PUSH EBP
  007f0381  8B EC          MOV EBP,ESP
  007f0383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f0380
string literals (0):
direct callees (1):
  0x00861480  (unnamed)
struct offsets touched (top 1):
  +0x68 [dec 104]  x2  via EAX,EDX
```

## Card B10a-1-47

GOG function: CSWSObject::SetDirtyFlag  gog 0x006b1110  callconv=(blank)  param_size_bytes=4  gog_size=74  gog_retn=4
Candidate Steam address: 0x0054a480 (size 67, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::IsDirtyFlagSet | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0054a430:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006b10c0 CSWSObject::AnimationWalking -> 0x0054a3e0  gate_class=AUTO
- GOG-order neighbour after:  0x006b1160 CSWSObject::IsDirtyFlagSet -> 0x0054a480  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00685f80 CSWSCreature::SetAnimation -> Steam 0x0056d350  in candidate callers: no
  0x006a8070 CSWSObject::SetAnimation -> Steam 0x00541390  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
imm sets (* = in both, 3 common): GOG {0x4*,0x5*,0xc*} | Steam {0x4*,0x5*,0xc*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0054a480: 16-byte aligned: YES
  0054a480  55             PUSH EBP
  0054a481  8B EC          MOV EBP,ESP
  0054a483  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0054a480
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0054a480 FUN_0054a480 size=67 ret=4 callers=1 callees=0

bool __thiscall FUN_0054a480(int param_1,uint param_2)

{
  return (1 << ((byte)param_2 & 0x1f) & *(uint *)(param_1 + 0x224 + (param_2 >> 5) * 4)) != 0;
}
```

### decoy: 0x0054a430 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0054a430: 16-byte aligned: YES
  0054a430  55             PUSH EBP
  0054a431  8B EC          MOV EBP,ESP
  0054a433  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0054a430
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

## Card B10a-1-48

GOG function: CSWCObject::AnimationPlayOutOfPlace  gog 0x004744b0  callconv=(blank)  param_size_bytes=2  gog_size=129  gog_retn=4
Candidate Steam address: 0x007ef730 (size 68, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ef6a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474420 CSWCObject::AnimationStationary -> 0x007ef2c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00474540 CSWCObject::AnimationHideEquippedItems -> 0x007ef910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00621350 Global::GetINTEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x68,0xcc,0xe0,0x130} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x10} | Steam {0x4*,0x4b0,0x52f,0x640,0x6bf}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007ef730: 16-byte aligned: YES
  007ef730  55             PUSH EBP
  007ef731  8B EC          MOV EBP,ESP
  007ef733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ef730
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ef730 FUN_007ef730 size=68 ret=4 callers=3 callees=0

undefined4 FUN_007ef730(ushort param_1)

{
  undefined4 uVar1;
  
  if (((param_1 < 0x4b0) || (0x52f < param_1)) && ((param_1 < 0x640 || (0x6bf < param_1)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x007ef6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ef6a0: 16-byte aligned: YES
  007ef6a0  55             PUSH EBP
  007ef6a1  8B EC          MOV EBP,ESP
  007ef6a3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ef6a0
string literals (0):
direct callees (1):
  0x0071B8B0  (unnamed)
struct offsets touched (top 4):
  +0x68 [dec 104]  x3  via EAX,ECX,EDX
  +0xCC [dec 204]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
  +0x130 [dec 304]  x1  via ECX
```

## Card B10a-1-49

GOG function: CSWCObject::TurnOnShadows  gog 0x00472630  callconv=__thiscall  param_size_bytes=0  gog_size=42  gog_retn=0
Candidate Steam address: 0x007ed4d0 (size 42, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed4b0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00472610 CSWCObject::GetPortraitId -> 0x007ed4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472660 CSWCObject::TurnOffShadows -> 0x007ed500  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xb8*} | Steam {0x68*,0xb8*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007ed4d0: 16-byte aligned: YES
  007ed4d0  55             PUSH EBP
  007ed4d1  8B EC          MOV EBP,ESP
  007ed4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed4d0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed4d0 CPaneContainerManager::NotifyPaneDivider size=42 ret=0 callers=0 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: void __thiscall CPaneContainerManager::NotifyPaneDivider(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CPaneContainerManager::NotifyPaneDivider(CPaneContainerManager *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xb8))();
  }
  return;
}
```

### decoy: 0x007ed4b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed4b0: 16-byte aligned: YES
  007ed4b0  55             PUSH EBP
  007ed4b1  8B EC          MOV EBP,ESP
  007ed4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed4b0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xB8 [dec 184]  x1  via EAX
```

## Card B10a-1-50

GOG function: CSWCObject::StopAllVisualEffects  gog 0x00475b50  callconv=(blank)  param_size_bytes=0  gog_size=83  gog_retn=0
Candidate Steam address: 0x007f09f0 (size 83, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f0920:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00475a80 CSWCObject::DeleteVisualEffect -> 0x007f0920  gate_class=AUTO
- GOG-order neighbour after:  0x00475bb0 CSWCObject::KillAllVisualEffects -> 0x007f0a50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x004fdbe0 CSWCVisualEffectOnObject::StopVisualEffect -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x6c*} | Steam {0xc*,0x6c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f09f0: 16-byte aligned: YES
  007f09f0  55             PUSH EBP
  007f09f1  8B EC          MOV EBP,ESP
  007f09f3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f09f0
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x00872300  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007f09f0 FUN_007f09f0 size=83 ret=0 callers=0 callees=4

void __fastcall FUN_007f09f0(undefined4 param_1)

{
  undefined4 local_c;
  int local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = FUN_00521060();
  while (local_8 != 0) {
    if (local_8 != 0) {
      FUN_00872300(1);
    }
    local_8 = FUN_0058c370(&local_c);
  }
  return;
}
```

### decoy: 0x007f0920 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f0920: 16-byte aligned: YES
  007f0920  55             PUSH EBP
  007f0921  8B EC          MOV EBP,ESP
  007f0923  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f0920
string literals (0):
direct callees (6):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x007F0F70  CSWCObject::DetachVisualEffectsMatchingHardCodedClass
  0x00872300  (unnamed)
  0x00877EB0  (unnamed)
struct offsets touched (top 3):
  +0xC0 [dec 192]  x1  via ECX
  +0xC4 [dec 196]  x1  via EAX
  +0xDC [dec 220]  x1  via EDX
```

## Card B10a-1-51

GOG function: CSWCObject::Destructor  gog 0x00472720  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x007ed5c0 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007ed590:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004726f0 CSWCObject::ClientServerSyncHandler_Orientation -> 0x007ed590  gate_class=REVIEW
- GOG-order neighbour after:  0x00472750 CSWCObject::Destructor_2 -> 0x007ed5f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00472750 CSWCObject::Destructor_2 -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007ed5c0: 16-byte aligned: YES
  007ed5c0  55             PUSH EBP
  007ed5c1  8B EC          MOV EBP,ESP
  007ed5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed5c0
string literals (0):
direct callees (2):
  0x007ED5F0  CSWCObject::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007ed5c0 FUN_007ed5c0 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_007ed5c0(undefined4 param_1,uint param_2)

{
  FUN_007ed5f0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007ed590 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed590: 16-byte aligned: YES
  007ed590  55             PUSH EBP
  007ed591  8B EC          MOV EBP,ESP
  007ed593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007ed590
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x88 [dec 136]  x1  via EDX
```

## Card B10a-1-52

GOG function: CSWCObject::GetCurrentAnimation  gog 0x00472930  callconv=(blank)  param_size_bytes=0  gog_size=18  gog_retn=0
Candidate Steam address: 0x007ed7d0 (size 18, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 6 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 6 (callers: OnObjectSelected 0x798d70, Update 0x7ac470, BuildHoverActions 0x7b40f0)
- best decoy (neighbour/other candidate) scores: 0x007ed5f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004727b0 CSWCObject::Destructor_3 -> 0x007ed5f0  gate_class=REVIEW
- GOG-order neighbour after:  0x00472950 CSWCObject::AttachToScene -> 0x007ed7f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (6 of 22): the card's A-CALLEE credit may come from these, not from callees
  0x00445390 CClientExoAppInternal::ShowObject -> Steam 0x00798d70  in candidate callers: yes
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
  0x00460d20 CClientExoAppInternal::GetDefaultActions -> Steam 0x007b40f0  in candidate callers: yes
  0x004d9ad0 CGuiInGame::ResetDialogAnimations -> Steam 0x007c93e0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x48*} | Steam {0x48*}
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
prologue for 0x007ed7d0: 16-byte aligned: YES
  007ed7d0  55             PUSH EBP
  007ed7d1  8B EC          MOV EBP,ESP
  007ed7d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007ed7d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x48 [dec 72]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007ed7d0 FUN_007ed7d0 size=18 ret=0 callers=22 callees=0

undefined2 __fastcall FUN_007ed7d0(int param_1)

{
  return *(undefined2 *)(param_1 + 0x48);
}
```

### decoy: 0x007ed5f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007ed5f0: 16-byte aligned: YES
  007ed5f0  55             PUSH EBP
  007ed5f1  8B EC          MOV EBP,ESP
  007ed5f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007ed5f0
string literals (0):
direct callees (11):
  0x0073F8D0  CClientExoApp::AddObjectToHitcheckIgnoreList
  0x007F0A50  CSWCObject::KillAllVisualEffects
  0x0073F790  CClientExoApp::GetClientAIMaster
  0x007A2540  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x009196FD  operator delete
  0x0054A900  CSWSObject::DetachFromClientObject
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x0072F5F0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x5C [dec 92]  x3  via ECX,EDX
  +0xC8 [dec 200]  x3  via EAX,ECX
  +0xCC [dec 204]  x3  via EAX,EDX
  +0xF8 [dec 248]  x3  via EAX,ECX,EDX
```

## Card B10a-1-53

GOG function: CSWCObject::AttachmentFromServerObject  gog 0x004777b0  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x007f2610 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetClientObject 0x54a840)
- best decoy (neighbour/other candidate) scores: 0x007f2630:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004776e0 CSWCObject::GetServerObject -> 0x007f2540  gate_class=AUTO
- GOG-order neighbour after:  0x004777d0 CSWCObject::DetachFromServerObject -> 0x007f2630  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xf8*} | Steam {0xf8*}
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
prologue for 0x007f2610: 16-byte aligned: YES
  007f2610  55             PUSH EBP
  007f2611  8B EC          MOV EBP,ESP
  007f2613  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2610
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2610 FUN_007f2610 size=25 ret=4 callers=1 callees=0

void __thiscall FUN_007f2610(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0xf8) = param_2;
  return;
}
```

### decoy: 0x007f2630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2630: 16-byte aligned: YES
  007f2630  55             PUSH EBP
  007f2631  8B EC          MOV EBP,ESP
  007f2633  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2630
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xF8 [dec 248]  x1  via EAX
```

## Card B10a-1-54

GOG function: CSWSObject::TestActionList  gog 0x006a68f0  callconv=(blank)  param_size_bytes=0  gog_size=92  gog_retn=0
Candidate Steam address: 0x0053f7a0 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::GetNewActionGroupId | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053f6a0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6880 CSWSObject::SetGroupInterruptable -> 0x0053fba0  gate_class=AUTO
- GOG-order neighbour after:  0x006a6950 CSWSObject::LoadObjectState -> 0x0053fc70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x006a7980 CSWSObject::AddActionToFront -> Steam 0x00540ca0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x14,0x16}
imm sets (* = in both, 0 common): GOG {0x10,0x100} | Steam {0x8,0xffff}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0053f7a0: 16-byte aligned: YES
  0053f7a0  55             PUSH EBP
  0053f7a1  8B EC          MOV EBP,ESP
  0053f7a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053f7a0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x14 [dec 20]  x5  via EAX,ECX,EDX
  +0x16 [dec 22]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053f7a0 FUN_0053f7a0 size=80 ret=0 callers=2 callees=0

undefined2 __fastcall FUN_0053f7a0(int param_1)

{
  undefined2 uVar1;
  
  uVar1 = *(undefined2 *)(param_1 + 0x14);
  *(short *)(param_1 + 0x14) = *(short *)(param_1 + 0x14) + 1;
  if (*(short *)(param_1 + 0x14) == -1) {
    *(undefined2 *)(param_1 + 0x14) = 0;
  }
  *(undefined2 *)(param_1 + 0x16) = uVar1;
  return uVar1;
}
```

### decoy: 0x0053f6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053f6a0: 16-byte aligned: YES
  0053f6a0  55             PUSH EBP
  0053f6a1  8B EC          MOV EBP,ESP
  0053f6a3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053f6a0
string literals (0):
direct callees (7):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x0064DDE0  (unnamed)
  0x0053E740  Action-list free params
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x6C [dec 108]  x1  via ECX
```

## Card B10a-1-55

GOG function: CSWCObject::RestoreTexture  gog 0x00475880  callconv=__thiscall  param_size_bytes=0  gog_size=43  gog_retn=0
Candidate Steam address: 0x007f0720 (size 43, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f06f0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:GetFirstDocTemplatePosition
- GOG-order neighbour before: 0x00475850 CSWCObject::RestoreEnvMap -> 0x007f06f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004758b0 CSWCObject::RestoreTexture_2 -> 0x007f0750  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x24*,0x68*} | Steam {0x24*,0x68*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f0720: 16-byte aligned: YES
  007f0720  55             PUSH EBP
  007f0721  8B EC          MOV EBP,ESP
  007f0723  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f0720
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0x24 [dec 36]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f0720 CWinApp::GetFirstDocTemplatePosition size=43 ret=0 callers=2 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: struct __POSITION * __thiscall CWinApp::GetFirstDocTemplatePosition(void)const 
   
   Libraries: Visual Studio 2005 Debug, Visual Studio 2008 Debug, Visual Studio 2010 Debug */

__POSITION [GHIDRA-AUTO-LABEL: unreliable] * __thiscall CWinApp::GetFirstDocTemplatePosition(CWinApp *this)

{
  __POSITION [GHIDRA-AUTO-LABEL: unreliable] *p_Var1;
  
  if (*(int *)(this + 0x68) == 0) {
    p_Var1 = (__POSITION [GHIDRA-AUTO-LABEL: unreliable] *)0x0;
  }
  else {
    p_Var1 = (__POSITION [GHIDRA-AUTO-LABEL: unreliable] *)(**(code **)(**(int **)(this + 0x68) + 0x24))();
  }
  return p_Var1;
}
```

### decoy: 0x007f06f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f06f0: 16-byte aligned: YES
  007f06f0  55             PUSH EBP
  007f06f1  8B EC          MOV EBP,ESP
  007f06f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f06f0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xA0 [dec 160]  x1  via EDX
```

## Card B10a-1-56

GOG function: CSWSObject::SaveObjectState  gog 0x006a69c0  callconv=(blank)  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0053fc70 (size 104, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSObject::LoadObjectState | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0053fce0:STR1/CALLEE11/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a6950 CSWSObject::LoadObjectState -> 0x0053fc70  gate_class=AUTO
- GOG-order neighbour after:  0x006a6a30 CSWSObject::LoadActionQueue -> 0x0053fd50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00625b30 CResGFF::WriteFieldBYTE -> Steam 0x007199d0  in candidate callees: no
  0x006a70f0 CSWSObject::SaveActionQueue -> Steam ?  in candidate callees: n/a
  0x006a7380 unnamed -> Steam ?  in candidate callees: n/a
  0x006a7540 CSWSObject::SaveEffectList -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 10): the card's A-CALLEE credit may come from these, not from callees
  0x0068d030 CSWSCreature::SaveCreature -> Steam 0x00574500  in candidate callers: no
  0x0071f8e0 CSWSArea::SaveItems -> Steam 0x0052a730  in candidate callers: no
  0x0071f9f0 CSWSArea::SaveDoors -> Steam 0x0052a840  in candidate callers: no
  0x0071fb00 CSWSArea::SaveTriggers -> Steam 0x0052a950  in candidate callers: no
GOG strings (1): 'Commandable'
disp sets (* = in both, 1 common): GOG {0xec*} | Steam {0xec*}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*,0xc}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0053fc70: 16-byte aligned: YES
  0053fc70  55             PUSH EBP
  0053fc71  8B EC          MOV EBP,ESP
  0053fc73  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053fc70
string literals (1):
  0x009945EC  at 0053fcaf  'Commandable'
direct callees (4):
  0x005406E0  CSWSObject::LoadEffectList
  0x00540660  (unnamed)
  0x0053FD50  CSWSObject::LoadActionQueue
  0x00718A00  CResGFF::ReadFieldBYTE
struct offsets touched (top 1):
  +0xEC [dec 236]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0053fc70 FUN_0053fc70 size=104 ret=8 callers=11 callees=4

void __thiscall FUN_0053fc70(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  undefined1 local_c [8];
  
  FUN_005406e0(param_2,param_3);
  FUN_00540660(param_2,param_3);
  FUN_0053fd50(param_2,param_3);
  uVar1 = FUN_00718a00(param_3,"Commandable",local_c,1);
  *(uint *)(param_1 + 0xec) = uVar1 & 0xff;
  return;
}
```

### decoy: 0x0053fce0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053fce0: 16-byte aligned: YES
  0053fce0  55             PUSH EBP
  0053fce1  8B EC          MOV EBP,ESP
  0053fce3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053fce0
string literals (1):
  0x009945EC  at 0053fd25  'Commandable'
direct callees (4):
  0x00540860  CSWSObject::SaveEffectList
  0x005406A0  (unnamed)
  0x00540410  CSWSObject::SaveActionQueue
  0x007199D0  CResGFF::WriteFieldBYTE
struct offsets touched (top 1):
  +0xEC [dec 236]  x1  via EAX
```

## Card B10a-1-57

GOG function: CSWCObject::CalculateProjectileTimeToTarget  gog 0x00476ec0  callconv=(blank)  param_size_bytes=8  gog_size=126  gog_retn=8
Candidate Steam address: 0x007f1d60 (size 126, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x007f1c00:STR0/CALLEE0/SIG0.214/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00476d60 CSWCObject::SetObjectVisibilityOnSetArea -> 0x007f1c00  gate_class=AUTO
- GOG-order neighbour after:  0x00476f40 CSWCObject::DetachVisualEffects -> 0x007f1de0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x0080c0a0 Global::float_magnitude_Vector_const_ref -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0xc*,0x24*,0x38*,0xc00*} | Steam {0x4*,0x8*,0xc*,0x24*,0x38*,0xc00*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007f1d60: 16-byte aligned: YES
  007f1d60  55             PUSH EBP
  007f1d61  8B EC          MOV EBP,ESP
  007f1d63  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f1d60
string literals (0):
direct callees (2):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x005142F0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007f1d60 FUN_007f1d60 size=126 ret=8 callers=1 callees=2

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_007f1d60(int param_1,undefined4 param_2,int param_3)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined4 local_3c;
  undefined1 local_1c [16];
  float local_c;
  
  uVar1 = FUN_004452b0(local_1c,param_2,param_1 + 0x24);
  fVar2 = (float10)FUN_005142f0(uVar1);
  if (param_3 == 0) {
    local_c = _DAT_0099a9b0;
  }
  else {
    local_c = _DAT_00994f50;
  }
  local_3c = (undefined4)(longlong)ROUND(((float)fVar2 / local_c) * (float)_DAT_00986b48);
  return local_3c;
}
```

### decoy: 0x007f1c00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f1c00: 16-byte aligned: YES
  007f1c00  55             PUSH EBP
  007f1c01  8B EC          MOV EBP,ESP
  007f1c03  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f1c00
string literals (0):
direct callees (5):
  0x00842D00  CSWCPlaceable::GetServerPlaceable
  0x00517980  Vec3::Equal
  0x0078EB80  (unnamed)
  0x007F18C0  (unnamed)
  0x007F1650  CSWCObject::SetDesiredFadeState
struct offsets touched (top 5):
  +0x8 [dec 8]  x5  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
  +0x4C [dec 76]  x1  via EDX
  +0x498 [dec 1176]  x1  via ECX
  +0x49C [dec 1180]  x1  via EAX
```

## Card B10a-1-58

GOG function: CSWCObject::EnableDistortion  gog 0x004777f0  callconv=__thiscall  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x007f2650 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007f2680:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: autolabel:OnClose
- GOG-order neighbour before: 0x004777d0 CSWCObject::DetachFromServerObject -> 0x007f2630  gate_class=REVIEW
- GOG-order neighbour after:  0x00477820 CSWCObject::DisableDistortion -> 0x007f2680  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x68*,0xe4*} | Steam {0x68*,0xe4*}
imm sets (* = in both, 0 common): GOG {} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007f2650: 16-byte aligned: YES
  007f2650  55             PUSH EBP
  007f2651  8B EC          MOV EBP,ESP
  007f2653  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007f2650
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xE4 [dec 228]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007f2650 CMDITabProxyWnd::OnClose size=44 ret=0 callers=2 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    protected: void __thiscall CMDITabProxyWnd::OnClose(void)
   
   Library: Visual Studio 2010 Debug */

void __thiscall CMDITabProxyWnd::OnClose(CMDITabProxyWnd *this)

{
  if (*(int *)(this + 0x68) != 0) {
    (**(code **)(**(int **)(this + 0x68) + 0xe4))();
  }
  return;
}
```

### decoy: 0x007f2680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007f2680: 16-byte aligned: YES
  007f2680  55             PUSH EBP
  007f2681  8B EC          MOV EBP,ESP
  007f2683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007f2680
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x3  via EAX,ECX
  +0xE8 [dec 232]  x1  via EDX
```

## Card B10a-1-59

GOG function: CSWSObject::SetPortrait  gog 0x00663870  callconv=(blank)  param_size_bytes=4  gog_size=34  gog_retn=4
Candidate Steam address: 0x0053efd0 (size 34, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0053efa0:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00663840 CSWSObject::GetPortrait -> 0x0053efa0  gate_class=REVIEW
- GOG-order neighbour after:  0x006638a0 CSWSObject::GetPortraitId -> 0x0053f000  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x20*} | Steam {0x4*,0x14*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0053efd0: 16-byte aligned: YES
  0053efd0  55             PUSH EBP
  0053efd1  8B EC          MOV EBP,ESP
  0053efd3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0053efd0
string literals (0):
direct callees (1):
  0x00710CA0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0053efd0 FUN_0053efd0 size=34 ret=4 callers=0 callees=1
THUNK -> 0x00710CA0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0053efd0(undefined4 param_1)

{
  undefined1 local_14 [16];
  
  FUN_00710ca0(local_14,param_1);
  return;
}
```

### decoy: 0x0053efa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0053efa0: 16-byte aligned: YES
  0053efa0  55             PUSH EBP
  0053efa1  8B EC          MOV EBP,ESP
  0053efa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0053efa0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

## Card B10a-1-60

GOG function: CSWCObject::AnimationPause  gog 0x00474890  callconv=(blank)  param_size_bytes=2  gog_size=68  gog_retn=4
Candidate Steam address: 0x007ef350 (size 129, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x004745d0:0.99>0.14
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007efac0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00474800 CSWCObject::AnimationOverlay -> 0x007ef6a0  gate_class=AUTO
- GOG-order neighbour after:  0x004748e0 CSWCObject::AnimationDialog -> 0x007ef780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00534cc0 CSWCPlaceable::AnimationUpdate -> Steam 0x00841440  in candidate callers: no
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

### decoy: 0x007efac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007efac0: 16-byte aligned: YES
  007efac0  55             PUSH EBP
  007efac1  8B EC          MOV EBP,ESP
  007efac3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007efac0
string literals (0):
direct callees (4):
  0x0073F3B0  CClientExoApp::GetWorldTimer
  0x0051ACD0  (unnamed)
  0x0051AD20  CWorldTimer::GetWorldTime
  0x0051AFE0  CWorldTimer::CompareWorldTimes
struct offsets touched (top 7):
  +0x5C [dec 92]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
```
