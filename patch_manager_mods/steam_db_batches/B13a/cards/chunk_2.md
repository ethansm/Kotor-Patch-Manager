# Review cards B13a chunk 2

Answer each card with one TSV row in `steam_db_batches/B13a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B13a-2-01

GOG function: CSWCCreature::CreateHostileItemEntry  gog 0x00428130  callconv=(blank)  param_size_bytes=12  gog_size=398  gog_retn=12
Candidate Steam address: 0x0077c1b0 (size 398, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.62) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077c340:STR0/CALLEE2/SIG0.812/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00427c70 CSWCCreature::CreateUsableItemEntry -> 0x0077bcf0  gate_class=AUTO
- GOG-order neighbour after:  0x004282c0 CSWCCreature::CreateHostileItemEntry_2 -> 0x0077c340  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam 0x0073f350  in candidate callees: yes
  0x00427b10 CSWCCreature::IsHostileUsableItem -> Steam ?  in candidate callees: n/a
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x006077c0 CExoLocString::GetString_3 -> Steam ?  in candidate callees: n/a
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x34*,0x2cc*} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x34*,0x2cc*}
imm sets (* = in both, 6 common): GOG {0xc*,0x20*,0x3c*,0x58*,0x2c0*,0x40000000*} | Steam {0xc*,0x20*,0x3c*,0x58*,0x2c0*,0x40000000*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0077c1b0: 16-byte aligned: YES
  0077c1b0  55             PUSH EBP
  0077c1b1  8B EC          MOV EBP,ESP
  0077c1b3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077c1b0
string literals (0):
direct callees (9):
  0x0077BB90  CSWCCreature::IsHostileUsableItem
  0x00733540  CExoString::Constructor
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

### annotate (candidate, first 80 lines)
```
// FN 0077c1b0 FUN_0077c1b0 size=398 ret=12 callers=1 callees=9

void FUN_0077c1b0(int *param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  undefined4 extraout_var;
  undefined1 local_4c [16];
  undefined1 local_3c [16];
  code *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965368;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = param_1[1 = byte +0x4];
  iVar3 = FUN_0077bb90(param_2);
  if (iVar3 != 0) {
    FUN_00733540(uVar2,extraout_var);
    local_8 = 0;
    uVar6 = 0;
    puVar5 = local_1c;
    uVar4 = FUN_0073f350(puVar5,0);
    FUN_007356b0(uVar4,puVar5,uVar6);
    FUN_0058c3b0(param_1[1 = byte +0x4] + 1);
    FUN_007337c0(local_1c);
    *(uint *)(*param_1 + 0x8 [dec 8] {int*: byte +0x20} + local_14 * 0x3c) = *(uint *)(param_2 + 0x14) | 0x40000000;
    local_2c = FUN_0077cd00;
    local_28 = 0;
    local_24 = 0;
    local_20 = 0;
    puVar1 = (undefined4 *)(*param_1 + 0xc + local_14 * 0x3c);
    *puVar1 = FUN_0077cd00;
    puVar1[1 = byte +0x4] = 0;
    puVar1[2 = byte +0x8] = 0;
    puVar1[3 = byte +0xc] = 0;
    *(undefined4 *)(*param_1 + 0x1c + local_14 * 0x3c) = *(undefined4 *)(param_3 + 4);
    uVar4 = FUN_00615cc0(local_3c);
    FUN_00710ca0(local_4c,uVar4);
    *(undefined4 *)(*param_1 + 0x34 + local_14 * 0x3c) = *(undefined4 *)(param_2 + 0x2cc);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0077c340 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077c340: 16-byte aligned: YES
  0077c340  55             PUSH EBP
  0077c341  8B EC          MOV EBP,ESP
  0077c343  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0077c340
string literals (1):
  0x0099D78C  at 0077c3e7  '%s (%d)'
direct callees (11):
  0x0077BB90  CSWCCreature::IsHostileUsableItem
  0x00733540  CExoString::Constructor
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007337C0  CExoString::operator=
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x2CC [dec 716]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card B13a-2-02

GOG function: CSWCCreature::UpdateRollingFootstepSound  gog 0x00413e70  callconv=(blank)  param_size_bytes=4  gog_size=516  gog_retn=8
Candidate Steam address: 0x00769620 (size 179, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::ClientServerSyncHandler_Animation | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007696e0:STR0/CALLEE0/SIG0.103/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00413e30 CSWCCreature::ForceAppearanceUpdate -> 0x00768060  gate_class=REVIEW
- GOG-order neighbour after:  0x00414080 CSWCCreature::SetCombatState -> 0x007682b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x00472990 CSWCObject::GetAnimationBase -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam 0x0079f430  in candidate callers: no
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x4c*,0xc5,0x138,0x2e8,0x3b0} | Steam {0x4c*,0x9c,0xa0}
imm sets (* = in both, 1 common): GOG {0x8*,0x10,0x20,0x40,0x272e,0x2736,0x2737,0x2738,0x2739,0x273a,0x27aa,0x27ab} | Steam {0x8*,0x2716,0x2718,0x2719,0x271b,0x271c,0x271e,0x277d,0x27ac}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x00769620: 16-byte aligned: YES
  00769620  55             PUSH EBP
  00769621  8B EC          MOV EBP,ESP
  00769623  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00769620
string literals (0):
direct callees (2):
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x007EF5D0  CSWCObject::AnimationFireAndForget
struct offsets touched (top 3):
  +0x4C [dec 76]  x1  via EAX
  +0x9C [dec 156]  x1  via EAX
  +0xA0 [dec 160]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00769620 FUN_00769620 size=179 ret=8 callers=0 callees=2

void __thiscall FUN_00769620(int *param_1,uint param_2,int param_3)

{
  uint uVar1;
  int iVar2;
  
  param_1[0x13 = byte +0x4c] = param_3;
  if ((((((param_2 != 0x2719) && (param_2 != 0x277d)) && (param_2 != 0x271c)) &&
       ((param_2 != 0x271b && (param_2 != 0x271e)))) &&
      ((param_2 != 0x2716 && ((param_2 != 0x2718 && (param_2 != 0x27ac)))))) &&
     (uVar1 = FUN_007ed7d0(), param_2 != (uVar1 & 0xffff))) {
    iVar2 = FUN_007ef5d0(param_2 & 0xffff);
    if (iVar2 == 0) {
      (**(code **)(*param_1 + 0x9c))(param_2 & 0xffff,0,0);
    }
    else {
      (**(code **)(*param_1 + 0xa0))(param_2 & 0xffff,0);
    }
  }
  return;
}
```

### decoy: 0x007696e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007696e0: 16-byte aligned: YES
  007696e0  55             PUSH EBP
  007696e1  8B EC          MOV EBP,ESP
  007696e3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007696e0
string literals (0):
direct callees (4):
  0x005143C0  (unnamed)
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x007EF2C0  CSWCObject::AnimationStationary
  0x00768610  CSWCCreature::RotateCreatureToOrientation
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

## Card B13a-2-03

GOG function: CSWCCreature::GetInterfaceTargetType  gog 0x0042a040  callconv=(blank)  param_size_bytes=4  gog_size=302  gog_retn=4
Candidate Steam address: 0x0077e0c0 (size 302, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callees: GetTargetChanging 0x740e80, GetServerPlaceable 0x842d00; callers: BuildHoverActions 0x7b40f0)
- best decoy (neighbour/other candidate) scores: 0x0077df90:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00429f10 CSWCCreature::GetTargetInterfaceTargetType -> 0x0077df90  gate_class=REVIEW
- GOG-order neighbour after:  0x0042a170 CSWCCreature::GetHostileInterfaceActions -> 0x0077e1f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040e340 CClientExoApp::GetTargetChanging -> Steam 0x00740e80  in candidate callees: yes
  0x00536580 CSWCPlaceable::GetServerPlaceable -> Steam 0x00842d00  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00460d20 CClientExoAppInternal::GetDefaultActions -> Steam 0x007b40f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x14*,0x2c*,0x3c*,0x4c*,0x138*,0x37c*} | Steam {0x4*,0x14*,0x2c*,0x3c*,0x4c*,0x138*,0x37c*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3*,0x4*,0x20*} | Steam {0x2*,0x3*,0x4*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0077e0c0: 16-byte aligned: YES
  0077e0c0  55             PUSH EBP
  0077e0c1  8B EC          MOV EBP,ESP
  0077e0c3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077e0c0
string literals (0):
direct callees (2):
  0x00740E80  CClientExoApp::GetTargetChanging
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 7):
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x138 [dec 312]  x1  via EDX
  +0x37C [dec 892]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077e0c0 FUN_0077e0c0 size=302 ret=4 callers=2 callees=2

undefined4 FUN_0077e0c0(int *param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 local_24;
  undefined4 local_8;
  
  local_8 = 0;
  if (param_1 == (int *)0x0) {
    local_8 = 0;
  }
  else {
    iVar1 = (**(code **)(*param_1 + 0x2c))();
    if (iVar1 == 0) {
      iVar1 = (**(code **)(*param_1 + 0x14))();
      if (iVar1 == 0) {
        iVar1 = (**(code **)(*param_1 + 0x4c))();
        if (iVar1 == 0) {
          iVar1 = (**(code **)(*param_1 + 0x3c))();
          if (iVar1 != 0) {
            local_8 = 2;
          }
        }
        else {
          iVar1 = (**(code **)(*param_1 + 0x4c))();
          if (iVar1 == 0) {
            local_24 = 0;
          }
          else {
            local_24 = FUN_00842d00();
          }
          if ((local_24 == 0) || (*(int *)(local_24 + 0x37c) == 0)) {
            local_8 = 3;
          }
          else {
            local_8 = 1;
          }
        }
      }
      else {
        local_8 = 1;
      }
    }
    else {
      piVar2 = (int *)(**(code **)(*param_1 + 0x2c))();
      iVar1 = (**(code **)(*piVar2 + 0x138))();
      if ((iVar1 == 0) && (iVar1 = FUN_00740e80(), iVar1 == 0)) {
        local_8 = 3;
      }
      else {
        local_8 = 4;
      }
    }
  }
  return local_8;
}
```

### decoy: 0x0077df90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077df90: 16-byte aligned: YES
  0077df90  55             PUSH EBP
  0077df91  8B EC          MOV EBP,ESP
  0077df93  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077df90
string literals (0):
direct callees (2):
  0x00740E80  CClientExoApp::GetTargetChanging
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 7):
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x138 [dec 312]  x1  via EDX
  +0x37C [dec 892]  x1  via EDX
```

## Card B13a-2-04

GOG function: CSWCCreature::BlurWeapon  gog 0x00413850  callconv=(blank)  param_size_bytes=4  gog_size=152  gog_retn=4
Candidate Steam address: 0x00767800 (size 237, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007678f0:STR0/CALLEE0/SIG0.062/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004137c0 CSWCCreature::ClearVisibilityList -> 0x007679f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0xc,0x34,0x198,0x2e4} | Steam {0x2ea}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*,0x1c} | Steam {0x2,0x3,0x4*,0xa,0xb,0xc,0xd,0xe,0xf,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00767800: 16-byte aligned: YES
  00767800  55             PUSH EBP
  00767801  8B EC          MOV EBP,ESP
  00767803  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00767800
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x2EA [dec 746]  x11  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00767800 FUN_00767800 size=237 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_00767800(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if ((param_2 == 0) &&
     ((*(char *)(param_1 + 0x2ea) == '\x02' || (*(char *)(param_1 + 0x2ea) == '\x03')))) {
    uVar1 = 1;
  }
  else if ((param_2 == 1) &&
          ((*(char *)(param_1 + 0x2ea) == '\0' || (*(char *)(param_1 + 0x2ea) == '\x01')))) {
    uVar1 = 1;
  }
  else if ((param_2 == 3) && (*(char *)(param_1 + 0x2ea) == '\x0e')) {
    uVar1 = 1;
  }
  else if ((param_2 == 2) &&
          ((((*(char *)(param_1 + 0x2ea) == '\n' || (*(char *)(param_1 + 0x2ea) == '\v')) ||
            (*(char *)(param_1 + 0x2ea) == '\f')) ||
           (((*(char *)(param_1 + 0x2ea) == '\r' || (*(char *)(param_1 + 0x2ea) == '\x0f')) ||
            (*(char *)(param_1 + 0x2ea) == '\x10')))))) {
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x007678f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007678f0: 16-byte aligned: YES
  007678f0  55             PUSH EBP
  007678f1  8B EC          MOV EBP,ESP
  007678f3  83 EC 4C       SUB ESP,0x4c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007678f0
string literals (0):
direct callees (5):
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0x88 [dec 136]  x1  via EAX
  +0x30C [dec 780]  x1  via EDX
```

## Card B13a-2-05

GOG function: CSWCCreature::ActionMenuSwapImplantRegen  gog 0x00426430  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a490 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a470:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426240 CSWCCreature::ActionSwapImplant -> 0x0077a280  gate_class=AUTO
- GOG-order neighbour after:  0x00426450 CSWCCreature::ActionMenuSwapImplantStr -> 0x0077a4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x408} | Steam {0x8*,0x409}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a490: 16-byte aligned: YES
  0077a490  55             PUSH EBP
  0077a491  8B EC          MOV EBP,ESP
  0077a493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a490
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a490 FUN_0077a490 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a490(void)

{
  FUN_0077a280(0x409);
  return;
}
```

### decoy: 0x0077a470 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a470: 16-byte aligned: YES
  0077a470  55             PUSH EBP
  0077a471  8B EC          MOV EBP,ESP
  0077a473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a470
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

## Card B13a-2-06

GOG function: CSWCCreature::ActionMenuSelectBehavior  gog 0x00429810  callconv=(blank)  param_size_bytes=8  gog_size=417  gog_retn=8
Candidate Steam address: 0x0077d890 (size 417, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=StanceActionClicked | G7 mac: pass(0.61) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callees: ShowTutorialWindow 0x740a20); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077d840:STR0/CALLEE0/SIG0.121/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004297c0 CSWCCreature::AttachToScene -> 0x0077d840  gate_class=AUTO
- GOG-order neighbour after:  0x004299d0 CSWCCreature::ActionMenuSwitchWeapons -> 0x0077da50  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040dee0 CClientExoApp::ShowTutorialWindow -> Steam 0x00740a20  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0048f9b0 CSWCLevelUpStats::GetClass -> Steam ?  in candidate callees: n/a
  0x005110e0 Global::IsJedi -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x32*,0x98*,0x146*,0x310*,0x1198*} | Steam {0x4*,0x32*,0x98*,0x146*,0x310*,0x1198*}
imm sets (* = in both, 20 common): GOG {0x4*,0x8*,0x9*,0xb*,0xc*,0xd*,0x34*,0x3d*,0x3f*,0x40*,0x41*,0x42*,0xff*,0x410*,...(+6)} | Steam {0x4*,0x8*,0x9*,0xb*,0xc*,0xd*,0x34*,0x3d*,0x3f*,0x40*,0x41*,0x42*,0xff*,0x410*,...(+6)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077d890: 16-byte aligned: YES
  0077d890  55             PUSH EBP
  0077d891  8B EC          MOV EBP,ESP
  0077d893  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077d890
string literals (0):
direct callees (5):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F2A30  IsForceUserClass(classId) {3,4,5,11..16}
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x0073F750  ClientApp::GetGuiInGame
struct offsets touched (top 8):
  +0x146 [dec 326]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x310 [dec 784]  x2  via EAX,EDX
  +0x32 [dec 50]  x1  via ECX
  +0x98 [dec 152]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
  +0x1540C [dec 87052]  x1  via EAX
  +0x15410 [dec 87056]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077d890 FUN_0077d890 size=417 ret=8 callers=0 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0077d890(int param_1,undefined4 param_2)

{
  undefined1 uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar2 = FUN_0077d800();
  if ((iVar2 != 0) && (iVar2 = *(int *)(iVar2 + 0x1198), iVar2 != 0)) {
    local_c = 0xffffffff;
    local_8 = 0xff;
    switch(param_2) {
    case 0x410:
      local_c = 0x1e33c;
      *(undefined2 *)(iVar2 + 0x146) = 9;
      local_8 = 0x3d;
      break;
    case 0x412:
      local_c = 0x1e33e;
      *(undefined2 *)(iVar2 + 0x146) = 0xb;
      local_8 = 0x3f;
      break;
    case 0x413:
      local_c = 0x1e33f;
      *(undefined2 *)(iVar2 + 0x146) = 0xc;
      local_8 = 0x40;
      break;
    case 0x414:
      uVar1 = FUN_00844e40(*(byte *)(*(int *)(param_1 + 0x310) + 0x32) - 1);
      iVar3 = FUN_006f2a30(uVar1);
      if (iVar3 == 0) {
        local_c = 0x1ec66;
        local_8 = 0x42;
      }
      else {
        local_c = 0x1ec3e;
        local_8 = 0x41;
      }
      *(undefined2 *)(iVar2 + 0x146) = 0xd;
    }
    FUN_00740a20(local_8,0x7f000000,0x7f000000,0);
    iVar2 = FUN_0073f750();
    iVar2 = *(int *)(iVar2 + 0x98);
    *(undefined4 *)(iVar2 + 0x15410) = local_c;
    *(undefined4 *)(iVar2 + 0x1540c) = _DAT_00986b78;
  }
  return;
}
```

### decoy: 0x0077d840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077d840: 16-byte aligned: YES
  0077d840  55             PUSH EBP
  0077d841  8B EC          MOV EBP,ESP
  0077d843  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077d840
string literals (0):
direct callees (2):
  0x007400A0  CClientExoApp::GetMiniGame
  0x007ED7F0  CSWCObject::AttachToScene
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x3BC [dec 956]  x1  via EAX
```

## Card B13a-2-07

GOG function: CSWCCreature::GetHostileItems  gog 0x00428620  callconv=(blank)  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x0077c6a0 (size 107, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077c710:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428490 CSWCCreature::GetUsableItems -> 0x0077c510  gate_class=AUTO
- GOG-order neighbour after:  0x00428690 CSWCCreature::GetHostileItems_2 -> 0x0077c710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00428130 CSWCCreature::CreateHostileItemEntry -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00669930 CItemRepository::ItemListGetItem -> Steam 0x00606100  in candidate callees: yes
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam 0x0058a2c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
imm sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077c6a0: 16-byte aligned: YES
  0077c6a0  55             PUSH EBP
  0077c6a1  8B EC          MOV EBP,ESP
  0077c6a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077c6a0
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C1B0  CSWCCreature::CreateHostileItemEntry
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c6a0 FUN_0077c6a0 size=107 ret=8 callers=1 callees=4

void __thiscall FUN_0077c6a0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  uVar2 = 1;
  FUN_0077d800(1,param_1);
  iVar1 = FUN_0058a2c0(uVar2);
  for (local_8 = 0; local_8 < *(uint *)(iVar1 + 0x10); local_8 = local_8 + 1) {
    uVar2 = FUN_00606100(local_8);
    FUN_0077c1b0(param_2,uVar2,param_3);
  }
  return;
}
```

### decoy: 0x0077c710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077c710: 16-byte aligned: YES
  0077c710  55             PUSH EBP
  0077c711  8B EC          MOV EBP,ESP
  0077c713  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077c710
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C340  CSWCCreature::CreateHostileItemEntry_2
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

## Card B13a-2-08

GOG function: CSWCCreature::GetHostileItems_2  gog 0x00428690  callconv=(blank)  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x0077c710 (size 107, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077c6a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428620 CSWCCreature::GetHostileItems -> 0x0077c6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00428700 CSWCCreature::PartyMenuUseItemSelect -> 0x0077c780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004282c0 CSWCCreature::CreateHostileItemEntry_2 -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00669930 CItemRepository::ItemListGetItem -> Steam 0x00606100  in candidate callees: yes
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam 0x0058a2c0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
imm sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077c710: 16-byte aligned: YES
  0077c710  55             PUSH EBP
  0077c711  8B EC          MOV EBP,ESP
  0077c713  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077c710
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C340  CSWCCreature::CreateHostileItemEntry_2
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c710 FUN_0077c710 size=107 ret=8 callers=1 callees=4

void __thiscall FUN_0077c710(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  uVar2 = 1;
  FUN_0077d800(1,param_1);
  iVar1 = FUN_0058a2c0(uVar2);
  for (local_8 = 0; local_8 < *(uint *)(iVar1 + 0x10); local_8 = local_8 + 1) {
    uVar2 = FUN_00606100(local_8);
    FUN_0077c340(param_2,uVar2,param_3);
  }
  return;
}
```

### decoy: 0x0077c6a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077c6a0: 16-byte aligned: YES
  0077c6a0  55             PUSH EBP
  0077c6a1  8B EC          MOV EBP,ESP
  0077c6a3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077c6a0
string literals (0):
direct callees (4):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00606100  CItemRepository::ItemListGetItem
  0x0077C1B0  CSWCCreature::CreateHostileItemEntry
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via ECX
```

## Card B13a-2-09

GOG function: CSWCCreature::SetCombatState  gog 0x00414080  callconv=(blank)  param_size_bytes=4  gog_size=397  gog_retn=4
Candidate Steam address: 0x00768440 (size 429, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007682b0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> 0x007680a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00414210 CSWCCreature::MoveCreatureToPosition -> 0x00768440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040c990 CClientExoApp::GetGameObject -> Steam 0x0073f4d0  in candidate callees: no
  0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> Steam ?  in candidate callees: n/a
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x0041fa70 Global::GetCreatureRadius -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0047f970 CSWCItem::PowerItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00693090 CSWSCreature::SetCombatState -> Steam 0x0057a560  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x34,0x94,0x9c,0x138,0x11ac} | Steam {0x44,0x8c,0x1f0}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x20,0x7f000000} | Steam {0x4*,0x5,0x8,0xc,0x24,0x8c,0x1e4}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00768440: 16-byte aligned: YES
  00768440  55             PUSH EBP
  00768441  8B EC          MOV EBP,ESP
  00768443  81 EC 8C 00 00 00 SUB ESP,0x8c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00768440
string literals (0):
direct callees (4):
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0045CFE0  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 3):
  +0x1F0 [dec 496]  x8  via EAX,ECX,EDX
  +0x8C [dec 140]  x4  via EAX
  +0x44 [dec 68]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00768440 FUN_00768440 size=429 ret=4 callers=3 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00768440(int *param_1,undefined4 param_2)

{
  undefined4 uVar1;
  float10 fVar2;
  undefined1 local_4c [12];
  undefined1 local_40 [12];
  undefined1 local_34 [12];
  float local_28;
  float local_24;
  float local_20;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined8 local_c;
  
  local_c = _DAT_00996378;
  local_18 = 0;
  local_14 = 0;
  local_10 = 0;
  if ((uint)param_1[0x11 = byte +0x44] < (uint)param_1[0x7c = byte +0x1f0]) {
    param_1[0x7c = byte +0x1f0] = param_1[0x7c = byte +0x1f0] - param_1[0x11 = byte +0x44];
    FUN_004452b0(&local_24,param_1 + 0x79,param_1 + 0x9 [dec 9] {int*: byte +0x24});
    fVar2 = (float10)FUN_0044b8a0(&local_24,&local_24);
    if ((float10)_DAT_00993768 <= fVar2) {
      if (local_20 * local_20 + local_24 * local_24 < (float)_DAT_0099efa0) {
        local_28 = (float)((float10)(uint)param_1[0x7c = byte +0x1f0] /
                          (float10)(uint)(param_1[0x7c = byte +0x1f0] + param_1[0x11 = byte +0x44]));
        uVar1 = FUN_004452b0(local_34,param_1 + 0x9 [dec 9] {int*: byte +0x24},param_2);
        uVar1 = FUN_0045cfe0(local_40,local_28,uVar1);
        uVar1 = FUN_0044c8a0(local_4c,param_2,uVar1);
        (**(code **)(*param_1 + 0x8c))(uVar1);
      }
      else {
        param_1[0x7c = byte +0x1f0] = 0;
        (**(code **)(*param_1 + 0x8c))(param_2);
      }
    }
    else {
      param_1[0x7c = byte +0x1f0] = 0;
      (**(code **)(*param_1 + 0x8c))(param_2);
    }
  }
  else {
    param_1[0x7c = byte +0x1f0] = 0;
    (**(code **)(*param_1 + 0x8c))(param_2);
  }
  return;
}
```

### decoy: 0x007682b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007682b0: 16-byte aligned: YES
  007682b0  55             PUSH EBP
  007682b1  8B EC          MOV EBP,ESP
  007682b3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007682b0
string literals (0):
direct callees (6):
  0x007680A0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x00773BA0  (unnamed)
struct offsets touched (top 6):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x138 [dec 312]  x2  via EAX,EDX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

## Card B13a-2-10

GOG function: CSWCCreature::ActionMenuSwapImplantStr  gog 0x00426450  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a4b0 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a490:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426430 CSWCCreature::ActionMenuSwapImplantRegen -> 0x0077a490  gate_class=REVIEW
- GOG-order neighbour after:  0x00426470 CSWCCreature::ActionMenuSwapImplantAgi -> 0x0077a4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x409} | Steam {0x8*,0x40b}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a4b0: 16-byte aligned: YES
  0077a4b0  55             PUSH EBP
  0077a4b1  8B EC          MOV EBP,ESP
  0077a4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a4b0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a4b0 FUN_0077a4b0 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a4b0(void)

{
  FUN_0077a280(0x40b);
  return;
}
```

### decoy: 0x0077a490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a490: 16-byte aligned: YES
  0077a490  55             PUSH EBP
  0077a491  8B EC          MOV EBP,ESP
  0077a493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a490
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

## Card B13a-2-11

GOG function: CSWCCreature::SetEquippedItemID  gog 0x0041a710  callconv=(blank)  param_size_bytes=8  gog_size=473  gog_retn=8
Candidate Steam address: 0x0076d9e0 (size 1013, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0076d890:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041a480 CSWCCreature::GetEquippedItemID -> 0x0076e610  gate_class=AUTO
- GOG-order neighbour after:  0x0041a990 CSWCCreature::SetLastInstant -> 0x0076eb20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x228,0x22c,0x230,0x234,0x238,0x23c,0x240,0x244,0x248,0x24c,0x250,0x254,0x258,0x25c,...(+3)} | Steam {0x4,0x8,0x38,0xb4,0xb8,0x19c,0x11f8}
imm sets (* = in both, 1 common): GOG {0x8*,0x7f,0x100,0x200,0x400,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000} | Steam {0x8*,0x15,0x9c,0x1f40,0x1f41,0x1f42,0x7f000000}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0076d9e0: 16-byte aligned: YES
  0076d9e0  55             PUSH EBP
  0076d9e1  8B EC          MOV EBP,ESP
  0076d9e3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076d9e0
string literals (0):
direct callees (8):
  0x00765AA0  CSWCCreature::GetSoundSetStrres
  0x0072FB00  CClientExoApp::GetClientOptions
  0x00484C80  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005FAD70  CSWPartyTable::GetPUPObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0058AE90  CSWSCreature::GetClientCreature
struct offsets touched (top 7):
  +0xB4 [dec 180]  x6  via EAX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x38 [dec 56]  x4  via EAX,EDX
  +0xB8 [dec 184]  x4  via EAX,EDX
  +0x19C [dec 412]  x3  via EAX
  +0x8 [dec 8]  x2  via ECX
  +0x11F8 [dec 4600]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0076d9e0 FUN_0076d9e0 size=1013 ret=8 callers=1 callees=8

void __thiscall FUN_0076d9e0(int *param_1,int param_2)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int *local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_8 = 0;
  if ((param_2 == 1) && (param_1[0x67 = byte +0x19c] == 0)) {
    local_8 = 1;
    FUN_00765aa0(0x15,1);
    iVar2 = FUN_0072fb00();
    if ((*(int *)(iVar2 + 0x38) == 0) || (cVar1 = FUN_00484c80(), cVar1 == '\0')) {
      local_34 = 0;
      local_30 = 0;
      local_2c = 0;
      (**(code **)(*param_1 + 0xb4))(0x1f42,0,0x7f000000,0x7f000000,0,0,&local_34);
    }
    else {
      local_28 = 0;
      local_24 = 0;
      local_20 = 0;
      (**(code **)(*param_1 + 0xb4))(8000,0,0x7f000000,0x7f000000,0,0,&local_28);
    }
  }
  else if ((param_2 == 0) && (param_1[0x67 = byte +0x19c] == 1)) {
    local_8 = 1;
    iVar2 = FUN_0072fb00();
    if ((*(int *)(iVar2 + 0x38) == 0) || (cVar1 = FUN_00484c80(), cVar1 == '\0')) {
      (**(code **)(*param_1 + 0xb8))(0x1f42);
    }
    else {
      (**(code **)(*param_1 + 0xb8))(8000);
    }
    local_40 = 0;
    local_3c = 0;
    local_38 = 0;
    (**(code **)(*param_1 + 0xb4))(0x1f41,1,0x7f000000,0x7f000000,0,0,&local_40);
  }
  param_1[0x67 = byte +0x19c] = param_2;
  local_10 = FUN_0077d800();
  if (((local_10 != 0) && (iVar2 = *(int *)(local_10 + 0x11f8), local_8 != 0)) && (iVar2 != -1)) {
    uVar4 = 1;
    uVar3 = 0;
    local_c = iVar2;
    FUN_0051c8d0(iVar2,0,1);
    local_1c = FUN_005fad70(iVar2,uVar3,uVar4);
    local_14 = FUN_0051c0a0(local_1c);
    if ((local_14 != 0) && (local_18 = (int *)FUN_0058ae90(), local_18 != (int *)0x0)) {
      if (param_2 == 0) {
        iVar2 = FUN_0072fb00();
        if ((*(int *)(iVar2 + 0x38) == 0) || (cVar1 = FUN_00484c80(), cVar1 == '\0')) {
          (**(code **)(*local_18 + 0xb8))(0x1f42);
        }
/* ... 28 more lines truncated by --max-lines */
```

### decoy: 0x0076d890 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076d890: 16-byte aligned: YES
  0076d890  55             PUSH EBP
  0076d891  8B EC          MOV EBP,ESP
  0076d893  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076d890
string literals (0):
direct callees (6):
  0x007ED830  CSWCObject::GetAnimationBase
  0x007680A0  (unnamed)
  0x00774B40  CSWCCreature::StopAmbientPauseAnim
  0x0077FD90  (unnamed)
  0x007FE6C0  CSWCItem::PowerItem
  0x007637E0  (unnamed)
struct offsets touched (top 6):
  +0x50 [dec 80]  x1  via EDX
  +0x70 [dec 112]  x1  via EDX
  +0x78 [dec 120]  x1  via EDX
  +0x90 [dec 144]  x1  via EDX
  +0x98 [dec 152]  x1  via EDX
  +0x154 [dec 340]  x1  via ECX
```

## Card B13a-2-12

GOG function: CSWCCreature::UpdateRelativePerception_2  gog 0x00419dc0  callconv=(blank)  param_size_bytes=4  gog_size=470  gog_retn=4
Candidate Steam address: 0x0076dde0 (size 363, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRelativePerception | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.79 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0076df50:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00419c50 CSWCCreature::UpdateRelativePerception -> 0x0076dde0  gate_class=REVIEW
- GOG-order neighbour after:  0x00419fa0 CSWCCreature::EquipToInventorySlot -> 0x0076e130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: yes
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x20*,0xb4*,0xb8*,0x1b8*} | Steam {0x4*,0x20*,0xb4*,0xb8*,0x1b8*}
imm sets (* = in both, 4 common): GOG {0x4*,0x24*,0x44,0x378,0x7d1*,0x7f000000*} | Steam {0x4*,0x24*,0x30,0x7d1*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076dde0: 16-byte aligned: YES
  0076dde0  55             PUSH EBP
  0076dde1  8B EC          MOV EBP,ESP
  0076dde3  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076dde0
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0xB4 [dec 180]  x1  via EAX
  +0xB8 [dec 184]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0076dde0 FUN_0076dde0 size=363 ret=4 callers=2 callees=2

void FUN_0076dde0(undefined4 *param_1)

{
  int iVar1;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  int *local_c;
  int local_8;
  
  local_8 = 0;
  local_10 = 0;
  local_c = (int *)FUN_0073f550(*param_1);
  if (local_c != (int *)0x0) {
    if ((((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 1) == 0) && ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 1 & 1) == 1)) ||
       ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 4 & 1) == 1)) {
      local_8 = 1;
    }
    else if (((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 1) == 1) && ((*(byte *)(param_1 + 0x1 [dec 1] {undefined4*: byte +0x4}) >> 4 & 1) == 0)) {
      local_10 = 1;
    }
    iVar1 = FUN_0078eb80(local_c + 0x9 [dec 9] {int*: byte +0x24});
    if (iVar1 == 1) {
      if ((local_8 == 1) && (local_c[0x6e = byte +0x1b8] == 0)) {
        local_1c = 0;
        local_18 = 0;
        local_14 = 0;
        (**(code **)(*local_c + 0xb4))(0x7d1,0,0x7f000000,0x7f000000,0,0,&local_1c);
        local_c[0x6e = byte +0x1b8] = 1;
      }
      else if ((local_10 == 1) && (local_c[0x6e = byte +0x1b8] == 1)) {
        (**(code **)(*local_c + 0xb8))(0x7d1);
        local_c[0x6e = byte +0x1b8] = 0;
      }
    }
  }
  return;
}
```

### decoy: 0x0076df50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076df50: 16-byte aligned: YES
  0076df50  55             PUSH EBP
  0076df51  8B EC          MOV EBP,ESP
  0076df53  83 EC 44       SUB ESP,0x44
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076df50
string literals (0):
direct callees (2):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
  0x0078EB80  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x7  via EAX,EDX
  +0x1B8 [dec 440]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x1  via EAX
  +0xB4 [dec 180]  x1  via EDX
  +0xB8 [dec 184]  x1  via EDX
```

## Card B13a-2-13

GOG function: CSWCCreature::GetEquipmentForcePowers  gog 0x004248c0  callconv=(blank)  param_size_bytes=16  gog_size=55  gog_retn=16
Candidate Steam address: 0x00778900 (size 55, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00778ba0:STR0/CALLEE0/SIG0.028/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004247a0 CSWCCreature::CheckFeat -> 0x007787e0  gate_class=AUTO
- GOG-order neighbour after:  0x00424900 CSWCCreature::EnableFeatForMenu -> 0x00778940  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x10*} | Steam {0x8*,0x10*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x00778900: 16-byte aligned: YES
  00778900  55             PUSH EBP
  00778901  8B EC          MOV EBP,ESP
  00778903  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00778900
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00778900 FUN_00778900 size=55 ret=16 callers=2 callees=0

int FUN_00778900(int param_1,int param_2,int param_3,int param_4)

{
  undefined4 local_8;
  
  local_8 = 0;
  if (((param_1 == param_2) || (param_1 == param_3)) || (param_1 == param_4)) {
    local_8 = param_4;
  }
  return local_8;
}
```

### decoy: 0x00778ba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00778ba0: 16-byte aligned: YES
  00778ba0  55             PUSH EBP
  00778ba1  8B EC          MOV EBP,ESP
  00778ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00778ba0
string literals (0):
direct callees (20):
  0x0084A640  (unnamed)
  0x005933B0  (unnamed)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x007FDB50  CSWCItem::GetActivePropertySpellID
  0x006C1450  (unnamed)
  0x006C11C0  (unnamed)
  0x005934C0  (unnamed)
  0x00414280  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00593450  CExoArrayList<uint16>::Add
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x006C12D0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710C20  (unnamed)
  0x00710CA0  (unnamed)
  0x007FEE70  CSWCItem::GetServerItem
  0x00604260  CSWSItem::GetActiveProperty
  0x00603B50  CSWSItem::GetUsedActivePropertyUsesLeft
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x6 [dec 6]  x2  via EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0x19C [dec 412]  x2  via EAX,ECX
  +0x14 [dec 20]  x1  via EAX
  +0x13C [dec 316]  x1  via EAX
  +0x194 [dec 404]  x1  via EAX
  +0x198 [dec 408]  x1  via EAX
```

## Card B13a-2-14

GOG function: CSWCCreature::SetSWAnimationSpeed  gog 0x0042bd00  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x0077fce0 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077fc10:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042bc30 CSWCCreature::SetLockOrientationToObject -> 0x0077fc10  gate_class=AUTO
- GOG-order neighbour after:  0x0042bd40 CSWCCreature::SetCombatMode -> 0x0077fd20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x11dc*} | Steam {0x11dc*}
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
prologue for 0x0077fce0: 16-byte aligned: YES
  0077fce0  55             PUSH EBP
  0077fce1  8B EC          MOV EBP,ESP
  0077fce3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fce0
string literals (0):
direct callees (1):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
struct offsets touched (top 1):
  +0x11DC [dec 4572]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fce0 FUN_0077fce0 size=59 ret=0 callers=1 callees=1

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 __fastcall FUN_0077fce0(undefined4 param_1)

{
  float fVar1;
  int iVar2;
  
  iVar2 = FUN_0077d800(param_1);
  fVar1 = _DAT_00994f50;
  if (iVar2 != 0) {
    iVar2 = FUN_0077d800(param_1);
    fVar1 = *(float *)(iVar2 + 0x11dc);
  }
  return (float10)fVar1;
}
```

### decoy: 0x0077fc10 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fc10: 16-byte aligned: YES
  0077fc10  55             PUSH EBP
  0077fc11  8B EC          MOV EBP,ESP
  0077fc13  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fc10
string literals (0):
direct callees (2):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0x30C [dec 780]  x1  via EDX
  +0x11AC [dec 4524]  x1  via ECX
```

## Card B13a-2-15

GOG function: CSWCCreature::CreateHostileItemEntry_2  gog 0x004282c0  callconv=(blank)  param_size_bytes=12  gog_size=458  gog_retn=12
Candidate Steam address: 0x0077c340 (size 458, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077c1b0:STR0/CALLEE2/SIG0.812/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428130 CSWCCreature::CreateHostileItemEntry -> 0x0077c1b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00428490 CSWCCreature::GetUsableItems -> 0x0077c510  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam 0x0073f350  in candidate callees: yes
  0x00427b10 CSWCCreature::IsHostileUsableItem -> Steam ?  in candidate callees: n/a
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00606380 CExoString::Format -> Steam 0x00734270  in candidate callees: yes
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '%s (%d)'
disp sets (* = in both, 7 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x34*,0x2cc*} | Steam {0x4*,0x8*,0xc*,0x14*,0x1c*,0x34*,0x2cc*}
imm sets (* = in both, 7 common): GOG {0xc*,0x10*,0x20*,0x3c*,0x60*,0x2c0*,0x40000000*} | Steam {0xc*,0x10*,0x20*,0x3c*,0x60*,0x2c0*,0x40000000*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0077c340: 16-byte aligned: YES
  0077c340  55             PUSH EBP
  0077c341  8B EC          MOV EBP,ESP
  0077c343  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077c340
string literals (1):
  0x0099D78C  at 0077c3e7  '%s (%d)'
direct callees (11):
  0x0077BB90  CSWCCreature::IsHostileUsableItem
  0x00733540  CExoString::Constructor
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x00734010  CExoString::CStr
  0x00734270  CExoString::Format
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007337C0  CExoString::operator=
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x2CC [dec 716]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c340 FUN_0077c340 size=458 ret=12 callers=1 callees=11

void FUN_0077c340(int *param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  undefined4 extraout_var;
  undefined1 local_4c [16];
  undefined1 local_3c [16];
  code *local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00973f58;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = param_1[1 = byte +0x4];
  iVar3 = FUN_0077bb90(param_2);
  if (iVar3 != 0) {
    FUN_00733540(uVar2,extraout_var);
    local_8 = 0;
    uVar6 = 0;
    puVar5 = local_1c;
    uVar4 = FUN_0073f350(puVar5,0);
    FUN_007356b0(uVar4,puVar5,uVar6);
    if (1 < *(int *)(param_2 + 0x2cc)) {
      uVar4 = FUN_00734010(*(undefined4 *)(param_2 + 0x2cc));
      FUN_00734270(local_1c,"%s (%d)",uVar4);
    }
    FUN_0058c3b0(param_1[1 = byte +0x4] + 1);
    FUN_007337c0(local_1c);
    *(uint *)(*param_1 + 0x8 [dec 8] {int*: byte +0x20} + local_14 * 0x3c) = *(uint *)(param_2 + 0x14) | 0x40000000;
    local_2c = FUN_0077cd00;
    local_28 = 0;
    local_24 = 0;
    local_20 = 0;
    puVar1 = (undefined4 *)(*param_1 + 0xc + local_14 * 0x3c);
    *puVar1 = FUN_0077cd00;
    puVar1[1 = byte +0x4] = 0;
    puVar1[2 = byte +0x8] = 0;
    puVar1[3 = byte +0xc] = 0;
    *(undefined4 *)(*param_1 + 0x1c + local_14 * 0x3c) = *(undefined4 *)(param_3 + 4);
    uVar4 = FUN_00615cc0(local_3c);
    FUN_00710ca0(local_4c,uVar4);
    *(undefined4 *)(*param_1 + 0x34 + local_14 * 0x3c) = *(undefined4 *)(param_2 + 0x2cc);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
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
  0x00733540  CExoString::Constructor
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

## Card B13a-2-16

GOG function: CSWCCreature::GetCameraOffset  gog 0x00421880  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775950 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775930:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421860 CSWCCreature::GetHeadHeightOffset -> 0x00775990  gate_class=REVIEW
- GOG-order neighbour after:  0x004218a0 CSWCCreature::GetPauseReadyAnimation -> 0x007759b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d460 CSWCCreatureAppearance::GetCameraOffset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
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
prologue for 0x00775950: 16-byte aligned: YES
  00775950  55             PUSH EBP
  00775951  8B EC          MOV EBP,ESP
  00775953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775950
string literals (0):
direct callees (1):
  0x0085D2D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775950 FUN_00775950 size=25 ret=0 callers=0 callees=1
THUNK -> 0x0085D2D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775950(undefined4 param_1)

{
  FUN_0085d2d0(param_1);
  return;
}
```

### decoy: 0x00775930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775930: 16-byte aligned: YES
  00775930  55             PUSH EBP
  00775931  8B EC          MOV EBP,ESP
  00775933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775930
string literals (0):
direct callees (1):
  0x0085D260  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

## Card B13a-2-17

GOG function: CSWCCreature::GetTargetInterfaceTargetType  gog 0x00429f10  callconv=(blank)  param_size_bytes=4  gog_size=302  gog_retn=4
Candidate Steam address: 0x0077df90 (size 302, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.86) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077e0c0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00429b20 CSWCCreature::GetCombatFeatsSkills -> 0x0077dba0  gate_class=AUTO
- GOG-order neighbour after:  0x0042a040 CSWCCreature::GetInterfaceTargetType -> 0x0077e0c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040e340 CClientExoApp::GetTargetChanging -> Steam 0x00740e80  in candidate callees: yes
  0x00536580 CSWCPlaceable::GetServerPlaceable -> Steam 0x00842d00  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 7 common): GOG {0x4*,0x14*,0x2c*,0x3c*,0x4c*,0x138*,0x37c*} | Steam {0x4*,0x14*,0x2c*,0x3c*,0x4c*,0x138*,0x37c*}
imm sets (* = in both, 4 common): GOG {0x2*,0x3*,0x4*,0x20*} | Steam {0x2*,0x3*,0x4*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0077df90: 16-byte aligned: YES
  0077df90  55             PUSH EBP
  0077df91  8B EC          MOV EBP,ESP
  0077df93  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077df90
string literals (0):
direct callees (2):
  0x00740E80  CClientExoApp::GetTargetChanging
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 7):
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x138 [dec 312]  x1  via EDX
  +0x37C [dec 892]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077df90 FUN_0077df90 size=302 ret=4 callers=1 callees=2

undefined1 FUN_0077df90(int *param_1)

{
  undefined1 uVar1;
  int iVar2;
  int *piVar3;
  undefined4 local_24;
  
  uVar1 = 0xff;
  if (param_1 == (int *)0x0) {
    uVar1 = 0xff;
  }
  else {
    iVar2 = (**(code **)(*param_1 + 0x2c))();
    if (iVar2 == 0) {
      iVar2 = (**(code **)(*param_1 + 0x14))();
      if (iVar2 == 0) {
        iVar2 = (**(code **)(*param_1 + 0x4c))();
        if (iVar2 == 0) {
          iVar2 = (**(code **)(*param_1 + 0x3c))();
          if (iVar2 != 0) {
            uVar1 = 1;
          }
        }
        else {
          iVar2 = (**(code **)(*param_1 + 0x4c))();
          if (iVar2 == 0) {
            local_24 = 0;
          }
          else {
            local_24 = FUN_00842d00();
          }
          if ((local_24 == 0) || (*(int *)(local_24 + 0x37c) == 0)) {
            uVar1 = 2;
          }
          else {
            uVar1 = 0;
          }
        }
      }
      else {
        uVar1 = 0;
      }
    }
    else {
      piVar3 = (int *)(**(code **)(*param_1 + 0x2c))();
      iVar2 = (**(code **)(*piVar3 + 0x138))();
      if ((iVar2 == 0) && (iVar2 = FUN_00740e80(), iVar2 == 0)) {
        uVar1 = 2;
      }
      else {
        uVar1 = 3;
      }
    }
  }
  return uVar1;
}
```

### decoy: 0x0077e0c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077e0c0: 16-byte aligned: YES
  0077e0c0  55             PUSH EBP
  0077e0c1  8B EC          MOV EBP,ESP
  0077e0c3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077e0c0
string literals (0):
direct callees (2):
  0x00740E80  CClientExoApp::GetTargetChanging
  0x00842D00  CSWCPlaceable::GetServerPlaceable
struct offsets touched (top 7):
  +0x2C [dec 44]  x2  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x3C [dec 60]  x1  via EDX
  +0x138 [dec 312]  x1  via EDX
  +0x37C [dec 892]  x1  via EDX
```

## Card B13a-2-18

GOG function: CSWCCreature::InCreatureCombat  gog 0x00422e00  callconv=(blank)  param_size_bytes=0  gog_size=126  gog_retn=0
Candidate Steam address: 0x00776e40 (size 126, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: AnimationDialog 0x7ef780, IsActionsPending 0x51cbe0; callers: Update 0x7ac470); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00776ec0:STR0/CALLEE1/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422dd0 CSWCCreature::IsDriveAnimationBlocked -> 0x00776e10  gate_class=REVIEW
- GOG-order neighbour after:  0x00422e80 CSWCCreature::SetEngagedState -> 0x00776ec0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004748e0 CSWCObject::AnimationDialog -> Steam 0x007ef780  in candidate callees: yes
  0x0064c2d0 CServerExoApp::ClientToServerObjectId -> Steam 0x0051c8b0  in candidate callees: yes
  0x0064c600 CServerExoApp::IsActionsPending -> Steam 0x0051cbe0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x48*,0x13c*} | Steam {0x4*,0x8*,0x48*,0x13c*}
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
prologue for 0x00776e40: 16-byte aligned: YES
  00776e40  55             PUSH EBP
  00776e41  8B EC          MOV EBP,ESP
  00776e43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776e40
string literals (0):
direct callees (3):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBE0  CServerExoApp::IsActionsPending
  0x007EF780  CSWCObject::AnimationDialog
struct offsets touched (top 4):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x13C [dec 316]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00776e40 FUN_00776e40 size=126 ret=0 callers=5 callees=3

undefined4 __fastcall FUN_00776e40(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  undefined4 local_8;
  
  local_8 = *(undefined4 *)(param_1 + 0x13c);
  uVar1 = FUN_0051c8b0(*(undefined4 *)(param_1 + 4));
  iVar2 = FUN_0051cbe0(uVar1);
  if (iVar2 != 0) {
    local_8 = 1;
  }
  iVar2 = FUN_007ef780(*(undefined2 *)(param_1 + 0x48));
  if (iVar2 != 0) {
    local_8 = 1;
  }
  return local_8;
}
```

### decoy: 0x00776ec0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00776ec0: 16-byte aligned: YES
  00776ec0  55             PUSH EBP
  00776ec1  8B EC          MOV EBP,ESP
  00776ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00776ec0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x13C [dec 316]  x1  via EAX
```

## Card B13a-2-19

GOG function: CSWCCreature::SetPrecalculatedPosition  gog 0x00421990  callconv=(blank)  param_size_bytes=4  gog_size=4873  gog_retn=4
Candidate Steam address: 0x00775aa0 (size 4873, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.74) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 13 | A-SIG 0.96 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR '<V1:%3.2f,%3.2f  V2:%3.2f,%3.2f IT:%d FR'@0099f1c7; '.\\nwccreature.cpp'@0099f208; CALLEE 13 (callees: TestDirectLine 0x7a7a60, GetCreaturePathfindInformation 0x51c9f0, ComputeHeight 0x51ca40); SIG 0.96
- best decoy (neighbour/other candidate) scores: 0x00776db0:STR0/CALLEE0/SIG0.107/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421970 CSWCCreatureStats::HasFeat  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00422ca0 CWalkHitInfo::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (39), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00413280 Vector::operator+ -> Steam ?  in candidate callees: n/a
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x004151f0 CSWCCreature::ComputeZValue -> Steam ?  in candidate callees: n/a
  0x00419020 Vector::operator* -> Steam ?  in candidate callees: n/a
  0x00422ca0 CWalkHitInfo::Constructor -> Steam ?  in candidate callees: n/a
  ... 31 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): '<V1:%3.2f,%3.2f  V2:%3.2f,%3.2f IT:%d FR'; '.\\\\nwccreature.cpp'
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0x14*,0x30*,0x3c*,0x48*,0x8c*,0x1b0*,0x260*,0x380*,0x3c0*,0x1114*} | Steam {0x4*,0x8*,0x14*,0x30*,0x3c*,0x48*,0x8c*,0x1b0*,0x260*,0x380*,0x3c0*,0x1114*}
imm sets (* = in both, 22 common): GOG {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x1e*,0x24*,0x30*,0x38*,0x41*,0x94*,0xa0*,0xff*,...(+9)} | Steam {0x2*,0x4*,0x5*,0x8*,0xc*,0x10*,0x1e*,0x24*,0x30*,0x38*,0x41*,0x94*,0xa0*,0xff*,...(+9)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00775aa0: 16-byte aligned: YES
  00775aa0  55             PUSH EBP
  00775aa1  8B EC          MOV EBP,ESP
  00775aa3  81 EC A4 05 00 00 SUB ESP,0x5a4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775aa0
string literals (1):
  0x0099F208  at 007762c1  '.\\\\nwccreature.cpp'
direct callees (39):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x00552E60  CSWSArea::ClosestPathPoint
  0x0051CA40  CServerExoApp::ComputeHeight
  0x005453C0  CSWSObject::GetArea
  0x0054B3B0  CSWSArea::GetRoomIndex
  0x0054B370  CSWSArea::GetRoom_2
  0x00651E30  (unnamed)
  0x006532B0  (unnamed)
  0x005E9E90  (unnamed)
  0x00436200  (unnamed)
  0x0044C8A0  Vector::operator+ (Vector3 add)
  0x0052BB60  CSWSArea::ClearLineOfSight
  0x0064E080  (unnamed)
  0x00650BE0  (unnamed)
  0x006DAA00  CSWSCreature::IsDebilitated
  0x00776E40  CSWCCreature::InCreatureCombat
  0x0073F430  CClientExoApp::GetModule
  0x004452B0  Vector::operator- (Vector3 subtract)
struct offsets touched (top 10):
  +0x4 [dec 4]  x23  via EAX,ECX,EDX
  +0x8 [dec 8]  x23  via EAX,ECX,EDX
  +0x3C [dec 60]  x10  via EAX,ECX,EDX
  +0x48 [dec 72]  x5  via EAX,ECX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x260 [dec 608]  x2  via EDX
  +0x380 [dec 896]  x2  via EAX
  +0x30 [dec 48]  x1  via EDX
  +0x8C [dec 140]  x1  via EDX
  +0x1B0 [dec 432]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00775aa0 FUN_00775aa0 size=4873 ret=4 callers=1 callees=39

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00775aa0(int *param_1,uint *param_2)

{
  bool bVar1;
  int iVar2;
  uint *puVar3;
  float10 fVar4;
  float10 fVar5;
  undefined8 uVar6;
  ulonglong uVar7;
  undefined4 uVar8;
  int *piVar9;
  undefined4 uVar10;
  int *piVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined1 local_3ac [12];
  undefined1 local_3a0 [12];
  undefined1 local_394 [12];
  undefined1 local_388 [12];
  undefined1 local_37c [12];
  undefined1 local_370 [12];
  undefined1 local_364 [12];
  undefined1 local_358 [12];
  float local_34c;
  float local_348;
  float local_344;
  undefined1 local_340 [12];
  undefined1 local_334 [12];
  float local_328;
  float local_324;
  float local_320;
  float local_31c;
  float local_318;
  float local_314;
  float local_310;
  float local_30c;
  float local_308;
  undefined1 local_304 [12];
  undefined1 local_2f8 [12];
  undefined1 local_2ec [12];
  undefined4 local_2e0;
  undefined4 local_2dc;
  undefined4 local_2d8;
  char local_2d4 [256];
  uint local_1d4;
  undefined1 local_1d0 [12];
  undefined1 local_1c4 [12];
  float local_1b8;
  float local_1b4;
  undefined4 local_1b0;
  undefined1 local_1ac [12];
  float local_1a0;
  undefined4 local_19c;
  undefined4 local_198;
  undefined1 local_194 [12];
  float local_188;
  float local_184;
  int local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  int local_16c;
  int local_168;
  uint local_164;
  uint local_160;
  uint local_15c;
  uint local_158;
  uint local_154;
  uint local_150;
  uint local_14c;
  uint local_148;
  uint local_144;
  uint local_140;
  uint local_13c;
  uint local_138;
/* ... 345 more lines truncated by --max-lines */
```

### decoy: 0x00776db0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00776db0: 16-byte aligned: YES
  00776db0  55             PUSH EBP
  00776db1  8B EC          MOV EBP,ESP
  00776db3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00776db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C0 [dec 960]  x3  via EAX,ECX,EDX
```

## Card B13a-2-20

GOG function: CSWCCreature::Constructor  gog 0x004102b0  callconv=(blank)  param_size_bytes=0  gog_size=2405  gog_retn=0
Candidate Steam address: 0x00764f80 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCObject::ShowShadowBlob | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00764f60:STR0/CALLEE0/SIG0.02/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410290 CSWCCreaturePartyStatus::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040ffe0 CSWCCombatAttackData::Constructor -> Steam ?  in candidate callees: n/a
  0x004721b0 CSWCObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00489500 CSWCSpellCastVisual::Constructor -> Steam ?  in candidate callees: n/a
  0x0048a160 Global::CSWCCreatureAppearance -> Steam ?  in candidate callees: n/a
  0x00490650 CSWCLevelUpStats::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00595eb0 CSWGuiInGameCharacter::Constructor -> Steam 0x0084c3a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x3c,0x40,0x48,0x58,0xfc,0x12c,0x130,0x134,0x138,0x13c,0x140,0x144,...(+101)} | Steam {0x68,0xac}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x5,0x6,0xc,0x11,0x14,0x30,0x48,0x6c,0x88,0x140,0x1cc,...(+17)} | Steam {}

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

### decoy: 0x00764f60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00764f60: 16-byte aligned: YES
  00764f60  55             PUSH EBP
  00764f61  8B EC          MOV EBP,ESP
  00764f63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00764f60
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B13a-2-21

GOG function: CSWCCreature::SetEngagedState  gog 0x00422e80  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00776ec0 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00776e40:STR0/CALLEE1/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00422ea0 Global::SetQuededAnimationCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x13c*} | Steam {0x13c*}
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
prologue for 0x00776ec0: 16-byte aligned: YES
  00776ec0  55             PUSH EBP
  00776ec1  8B EC          MOV EBP,ESP
  00776ec3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776ec0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x13C [dec 316]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00776ec0 FUN_00776ec0 size=25 ret=4 callers=2 callees=0

void __thiscall FUN_00776ec0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x13c) = param_2;
  return;
}
```

### decoy: 0x00776e40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00776e40: 16-byte aligned: YES
  00776e40  55             PUSH EBP
  00776e41  8B EC          MOV EBP,ESP
  00776e43  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00776e40
string literals (0):
direct callees (3):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051CBE0  CServerExoApp::IsActionsPending
  0x007EF780  CSWCObject::AnimationDialog
struct offsets touched (top 4):
  +0x8 [dec 8]  x2  via ECX
  +0x4 [dec 4]  x1  via EDX
  +0x48 [dec 72]  x1  via ECX
  +0x13C [dec 316]  x1  via EAX
```

## Card B13a-2-22

GOG function: CSWCCreature::StopLipSync  gog 0x004275c0  callconv=(blank)  param_size_bytes=0  gog_size=44  gog_retn=0
Candidate Steam address: 0x0077b600 (size 44, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: StopSoundPlayingInDialog 0x79be40)
- best decoy (neighbour/other candidate) scores: 0x0077b190:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00427150 CSWCCreature::LipSync -> 0x0077b190  gate_class=AUTO
- GOG-order neighbour after:  0x004275f0 CSWCCreature::IsLeaderUsableItem -> 0x0077b670  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00448440 CClientExoAppInternal::StopSoundPlayingInDialog -> Steam 0x0079be40  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x5c*,0x68*} | Steam {0x5c*,0x68*}
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
prologue for 0x0077b600: 16-byte aligned: YES
  0077b600  55             PUSH EBP
  0077b601  8B EC          MOV EBP,ESP
  0077b603  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077b600
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077b600 FUN_0077b600 size=44 ret=0 callers=2 callees=0

void __fastcall FUN_0077b600(int param_1)

{
  (**(code **)(**(int **)(param_1 + 0x68) + 0x5c))(0,0,0,0);
  return;
}
```

### decoy: 0x0077b190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077b190: 16-byte aligned: YES
  0077b190  55             PUSH EBP
  0077b191  8B EC          MOV EBP,ESP
  0077b193  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0077b190
string literals (2):
  0x00986BA8  at 0077b200  'Sound Options'
  0x00986B98  at 0077b211  '.\\\\swkotor2.ini'
direct callees (19):
  0x00736240  (unnamed)
  0x00733540  CExoString::Constructor
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091EF5C  (unnamed)
  0x007362C0  (unnamed)
  0x007109B0  (unnamed)
  0x00919723  operator new
  0x00944CA0  (unnamed)
  0x007335F0  CExoString::CExoString_copy
  0x00944D80  (unnamed)
  0x0084B570  (unnamed)
  0x00944F10  (unnamed)
  0x00944F80  (unnamed)
  0x00944F30  (unnamed)
  0x0077B600  CSWCCreature::StopLipSync
  0x00944EE0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 2):
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x5C [dec 92]  x1  via EDX
```

## Card B13a-2-23

GOG function: CSWCCreature::GetHeadHeightOffset  gog 0x00421860  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775990 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775970:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421840 CSWCCreature::GetFollowingBody -> 0x00775970  gate_class=REVIEW
- GOG-order neighbour after:  0x00421880 CSWCCreature::GetCameraOffset -> 0x00775950  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d3b0 CSWCCreatureAppearance::GetHeadHeightOffset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x224*} | Steam {0x224*}
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
prologue for 0x00775990: 16-byte aligned: YES
  00775990  55             PUSH EBP
  00775991  8B EC          MOV EBP,ESP
  00775993  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775990
string literals (0):
direct callees (1):
  0x0085D3D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775990 FUN_00775990 size=25 ret=0 callers=3 callees=1
THUNK -> 0x0085D3D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775990(undefined4 param_1)

{
  FUN_0085d3d0(param_1);
  return;
}
```

### decoy: 0x00775970 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775970: 16-byte aligned: YES
  00775970  55             PUSH EBP
  00775971  8B EC          MOV EBP,ESP
  00775973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775970
string literals (0):
direct callees (1):
  0x0085D320  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

## Card B13a-2-24

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077fac0 (size 77, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0077fa00:STR0/CALLEE0/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042ba20 CSWCCreature::PlayRollingFootstepSound -> 0x0077fa00  gate_class=AUTO
- GOG-order neighbour after:  0x0042bb30 CSWCCreature::UpdateRollingFootstepSound_2 -> 0x0077fb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x448*,0x450*,0x454*} | Steam {0x448*,0x450*,0x454*}
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
prologue for 0x0077fac0: 16-byte aligned: YES
  0077fac0  55             PUSH EBP
  0077fac1  8B EC          MOV EBP,ESP
  0077fac3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fac0
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 3):
  +0x448 [dec 1096]  x2  via EAX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fac0 FUN_0077fac0 size=77 ret=0 callers=1 callees=1

void __fastcall FUN_0077fac0(int param_1)

{
  if ((*(int *)(param_1 + 0x448) != 0) && (*(int *)(param_1 + 0x450) != 0)) {
    Decref();
    *(undefined4 *)(param_1 + 0x450) = 0;
    *(undefined4 *)(param_1 + 0x454) = 0;
  }
  return;
}
```

### decoy: 0x0077fa00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fa00: 16-byte aligned: YES
  0077fa00  55             PUSH EBP
  0077fa01  8B EC          MOV EBP,ESP
  0077fa03  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fa00
string literals (0):
direct callees (3):
  0x0070AE90  (unnamed)
  0x0070AFA0  (unnamed)
  0x0070AD50  (unnamed)
struct offsets touched (top 4):
  +0x448 [dec 1096]  x5  via EAX,ECX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x2  via ECX,EDX
  +0x44C [dec 1100]  x1  via EAX
```

## Card B13a-2-25

GOG function: CSWCCreature::IsDriveAnimationBlocked  gog 0x00422dd0  callconv=(blank)  param_size_bytes=8  gog_size=37  gog_retn=8
Candidate Steam address: 0x00776e10 (size 37, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: rotation:0x00422e00:0.83>0.09
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00776db0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422ca0 CWalkHitInfo::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x3c8*,0x3cc*} | Steam {0x3c8*,0x3cc*}
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
prologue for 0x00776e10: 16-byte aligned: YES
  00776e10  55             PUSH EBP
  00776e11  8B EC          MOV EBP,ESP
  00776e13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776e10
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C8 [dec 968]  x1  via ECX
  +0x3CC [dec 972]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00776e10 FUN_00776e10 size=37 ret=8 callers=1 callees=0

void __thiscall FUN_00776e10(int param_1,undefined4 param_2,undefined4 param_3)

{
  *(undefined4 *)(param_1 + 0x3cc) = param_3;
  *(undefined4 *)(param_1 + 0x3c8) = param_2;
  return;
}
```

### decoy: 0x00776db0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00776db0: 16-byte aligned: YES
  00776db0  55             PUSH EBP
  00776db1  8B EC          MOV EBP,ESP
  00776db3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00776db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C0 [dec 960]  x3  via EAX,ECX,EDX
```

## Card B13a-2-26

GOG function: CSWCCreature::PerformUseHostileItem  gog 0x00428d50  callconv=(blank)  param_size_bytes=8  gog_size=45  gog_retn=8
Candidate Steam address: 0x0077cdd0 (size 45, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077cd00:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428c80 CSWCCreature::PartyMenuUseHostileItem -> 0x0077cd00  gate_class=AUTO
- GOG-order neighbour after:  0x00428d80 CSWCCreature::ActionMenuAttack -> 0x0077ce00  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00428c00 CSWCCreature::PartyMenuUseItem -> Steam ?  in candidate callees: n/a
  0x0042bd40 CSWCCreature::SetCombatMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0077cdd0: 16-byte aligned: YES
  0077cdd0  55             PUSH EBP
  0077cdd1  8B EC          MOV EBP,ESP
  0077cdd3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077cdd0
string literals (0):
direct callees (2):
  0x0077FD20  CSWCCreature::SetCombatMode
  0x0077CC80  CSWCCreature::PartyMenuUseItem
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077cdd0 FUN_0077cdd0 size=45 ret=8 callers=2 callees=2

void FUN_0077cdd0(undefined4 param_1,int param_2)

{
  if (param_2 != 0) {
    FUN_0077fd20(1);
  }
  FUN_0077cc80(param_1,param_2);
  return;
}
```

### decoy: 0x0077cd00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077cd00: 16-byte aligned: YES
  0077cd00  55             PUSH EBP
  0077cd01  8B EC          MOV EBP,ESP
  0077cd03  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077cd00
string literals (0):
direct callees (4):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x006D6E30  CSWItem::GetBaseItem
  0x00740A20  CClientExoApp::ShowTutorialWindow
  0x0077CDD0  CSWCCreature::PerformUseHostileItem
struct offsets touched (top 3):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0xAC [dec 172]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
```

## Card B13a-2-27

GOG function: CSWCCreature::IsLeaderUsableItem  gog 0x004275f0  callconv=(blank)  param_size_bytes=4  gog_size=1291  gog_retn=12
Candidate Steam address: 0x0077b670 (size 1291, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=GuiInGame::IsItemUsableByCurrent | G7 mac: pass(0.94) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 7 (callees: GetServerArea 0x7aae00, GetPropertyByType 0x6042e0, CanUseItem 0x5d2f80; callers: CreateUsableItemEntry 0x77bcf0, CreateItemEntry 0x8a75f0, CheckFilter 0x8a8df0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077bb90:STR0/CALLEE1/SIG0.208/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004275c0 CSWCCreature::StopLipSync -> 0x0077b600  gate_class=REVIEW
- GOG-order neighbour after:  0x00427b10 CSWCCreature::IsHostileUsableItem -> 0x0077bb90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (10), mapped to Steam through trusted rows only:
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00457800 CSWCArea::GetServerArea -> Steam 0x007aae00  in candidate callees: yes
  0x0047b460 CSWParty::PartyHasRace -> Steam ?  in candidate callees: n/a
  0x004ecc00 CSWItem::GetBaseItem -> Steam 0x006d6e30  in candidate callees: yes
  0x004ed450 CSWSpellArray::GetSpell -> Steam ?  in candidate callees: n/a
  0x00667b10 CSWSItem::GetPropertyByType -> Steam 0x006042e0  in candidate callees: yes
  0x0067a4d0 CSWInventory::GetItemInInventory -> Steam ?  in candidate callees: n/a
  ... 2 more GOG callees
GOG callers mapped through trusted rows (3 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x00427c70 CSWCCreature::CreateUsableItemEntry -> Steam 0x0077bcf0  in candidate callers: yes
  0x0059b4f0 CSWGuiInGameInventory::CreateItemEntry -> Steam 0x008a75f0  in candidate callers: yes
  0x0059cc40 CSWGuiInGameInventory::CheckFilter -> Steam 0x008a8df0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 11 common): GOG {0x2*,0x4*,0xc*,0x20*,0xac*,0x104*,0x10c*,0x1a4*,0x2e4*,0x1150*,0x1198*} | Steam {0x2*,0x4*,0xc*,0x20*,0xac*,0x104*,0x10c*,0x1a4*,0x2e4*,0x1150*,0x1198*}
imm sets (* = in both, 13 common): GOG {0x2*,0x4*,0xa*,0xc*,0xd*,0x14*,0x19*,0x1a*,0x1c*,0x2d*,0x2e*,0x2f*,0xa4*} | Steam {0x2*,0x4*,0xa*,0xc*,0xd*,0x14*,0x19*,0x1a*,0x1c*,0x2d*,0x2e*,0x2f*,0xa4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 12)
```

### prologue
```
prologue for 0x0077b670: 16-byte aligned: YES
  0077b670  55             PUSH EBP
  0077b671  8B EC          MOV EBP,ESP
  0077b673  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077b670
string literals (0):
direct callees (10):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x007AAE00  CSWCArea::GetServerArea
  0x006D6E30  CSWItem::GetBaseItem
  0x006042E0  CSWSItem::GetPropertyByType
  0x006B7A00  CSWSCreatureStats::GetCanUseSkill
  0x005D2F80  CSWSCreature::CanUseItem
  0x006D0540  (unnamed)
  0x006C1450  (unnamed)
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E8830  (unnamed)
struct offsets touched (top 10):
  +0xAC [dec 172]  x9  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x2 [dec 2]  x2  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX
  +0x104 [dec 260]  x2  via EAX,ECX
  +0x1A4 [dec 420]  x2  via ECX,EDX
  +0x1150 [dec 4432]  x2  via EAX,EDX
  +0x1198 [dec 4504]  x2  via EDX
  +0x20 [dec 32]  x1  via EAX
  +0x10C [dec 268]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0077b670 FUN_0077b670 size=1291 ret=12 callers=4 callees=10

undefined4 FUN_0077b670(int param_1,undefined4 param_2,byte param_3)

{
  byte bVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  int local_c;
  uint local_8;
  
  if (param_1 != 0) {
    iVar2 = FUN_0077d800();
    iVar3 = FUN_007aae00();
    if (iVar2 == 0) {
      return 0;
    }
    if (*(int *)(param_1 + 0x10c) == 0) {
      if (*(int *)(iVar3 + 0x2e4) == 0) {
        if (((param_3 & 2) != 0) && (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1c')) {
          iVar2 = FUN_006042e0(&local_c,0x2e,0);
          if ((iVar2 != 0) &&
             ((*(char *)(local_c + 0xc) == '\0' || (iVar2 = FUN_006b7a00(1), iVar2 == 0)))) {
            return 0;
          }
          uVar4 = FUN_005d2f80(param_1,1,1);
          return uVar4;
        }
        if (((param_3 & 1) != 0) &&
           ((((iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x19' ||
              (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '-')) ||
             (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '/')) ||
            (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1a')))) {
          iVar2 = FUN_006042e0(&local_c,10,0);
          if ((iVar2 != 0) && (*(char *)(local_c + 0xc) == '\0')) {
            return 0;
          }
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) != '\x1a') {
            uVar5 = (uint)DAT_0099a8bc;
            FUN_0073fb90(uVar5);
            iVar2 = FUN_007e8830(uVar5);
            if (iVar2 == 0) {
              return 0;
            }
            return 1;
          }
          uVar5 = (uint)DAT_0099a8b8;
          FUN_0073fb90(uVar5);
          iVar2 = FUN_007e8830(uVar5);
          if (iVar2 == 0) {
            return 0;
          }
          return 1;
        }
        iVar2 = FUN_006042e0(&local_c,10,0);
        if (iVar2 != 0) {
          if (*(char *)(local_c + 0xc) == '\0') {
            return 0;
          }
          iVar2 = FUN_006d6e30();
          if (iVar2 != 0) {
            bVar1 = *(byte *)(iVar2 + 0xac);
            if ((bVar1 < 0xc) || ((0xd < bVar1 && (bVar1 != 0x14)))) {
              if ((param_3 & 4) == 0) {
                return 0;
              }
            }
            else {
              iVar2 = FUN_006d0540(param_1);
              if ((iVar2 == 0) || ((param_3 & 4) == 0)) {
                return 0;
              }
            }
          }
          local_8 = (uint)*(ushort *)(local_c + 2);
          iVar2 = FUN_006c1450(local_8);
          if ((iVar2 != 0) && (*(char *)(iVar2 + 0x1a4) == '\x01')) {
            uVar4 = FUN_005d2f80(param_1,1,1);
/* ... 43 more lines truncated by --max-lines */
```

### decoy: 0x0077bb90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077bb90: 16-byte aligned: YES
  0077bb90  55             PUSH EBP
  0077bb91  8B EC          MOV EBP,ESP
  0077bb93  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077bb90
string literals (0):
direct callees (4):
  0x007AAE00  CSWCArea::GetServerArea
  0x006D6E30  CSWItem::GetBaseItem
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x006D0620  CSWInventory::GetItemInSlot
struct offsets touched (top 4):
  +0xAC [dec 172]  x4  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via EAX,ECX
  +0x20 [dec 32]  x1  via EAX
  +0x2E4 [dec 740]  x1  via EDX
```

## Card B13a-2-28

GOG function: CSWCCreature::Constructor  gog 0x004102b0  callconv=(blank)  param_size_bytes=0  gog_size=2405  gog_retn=0
Candidate Steam address: 0x007645d0 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007645f0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410290 CSWCCreaturePartyStatus::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00410c20 Global::return_zero  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040ffe0 CSWCCombatAttackData::Constructor -> Steam ?  in candidate callees: n/a
  0x004721b0 CSWCObject::Constructor -> Steam ?  in candidate callees: n/a
  0x00489500 CSWCSpellCastVisual::Constructor -> Steam ?  in candidate callees: n/a
  0x0048a160 Global::CSWCCreatureAppearance -> Steam ?  in candidate callees: n/a
  0x00490650 CSWCLevelUpStats::Constructor -> Steam ?  in candidate callees: n/a
  0x00567c10 CExoArrayList_int_::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (1 of 8): the card's A-CALLEE credit may come from these, not from callees
  0x00595eb0 CSWGuiInGameCharacter::Constructor -> Steam 0x0084c3a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x3c,0x40,0x48,0x58,0xfc,0x12c,0x130,0x134,0x138,0x13c,0x140,0x144,...(+101)} | Steam {}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x5,0x6,0xc,0x11,0x14,0x30,0x48,0x6c,0x88,0x140,0x1cc,...(+17)} | Steam {0x18}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007645d0: 16-byte aligned: YES
  007645d0  55             PUSH EBP
  007645d1  8B EC          MOV EBP,ESP
  007645d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007645d0
string literals (0):
direct callees (1):
  0x00733780  CExoString::~CExoString
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007645d0 FUN_007645d0 size=22 ret=0 callers=1 callees=1
THUNK -> 0x00733780 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_007645d0(undefined4 param_1)

{
  FUN_00733780(param_1);
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

## Card B13a-2-29

GOG function: CSWCCreature::GetServerCreature  gog 0x00429780  callconv=(blank)  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0077d800 (size 53, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Party | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 35 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 35 (callees: GetServerObject 0x7f2540; callers: ChangeCharacter 0x797f30, OnObjectSelected 0x798d70, DoPassiveSelection 0x79a4c0)
- best decoy (neighbour/other candidate) scores: 0x0077d840:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00429310 CSWCCreature::CGPauseCycle -> 0x0077d390  gate_class=AUTO
- GOG-order neighbour after:  0x004297c0 CSWCCreature::AttachToScene -> 0x0077d840  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004776e0 CSWCObject::GetServerObject -> Steam 0x007f2540  in candidate callees: yes
GOG callers mapped through trusted rows (34 of 133): the card's A-CALLEE credit may come from these, not from callees
  0x00444550 CClientExoAppInternal::ChangeCharacter -> Steam 0x00797f30  in candidate callers: yes
  0x00445390 CClientExoAppInternal::ShowObject -> Steam 0x00798d70  in candidate callers: yes
  0x00446ae0 CClientExoAppInternal::DoPassiveSelection -> Steam 0x0079a4c0  in candidate callers: yes
  0x00448660 CClientExoAppInternal::GetControlledCharacterDead -> Steam 0x0079c060  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x30*} | Steam {0x30*}
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
prologue for 0x0077d800: 16-byte aligned: YES
  0077d800  55             PUSH EBP
  0077d801  8B EC          MOV EBP,ESP
  0077d803  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077d800
string literals (0):
direct callees (1):
  0x007F2540  CSWCObject::GetServerObject
struct offsets touched (top 1):
  +0x30 [dec 48]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077d800 FUN_0077d800 size=53 ret=0 callers=132 callees=1

undefined4 FUN_0077d800(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_007f2540();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_007f2540();
    uVar3 = (**(code **)(*piVar2 + 0x30))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x0077d840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077d840: 16-byte aligned: YES
  0077d840  55             PUSH EBP
  0077d841  8B EC          MOV EBP,ESP
  0077d843  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077d840
string literals (0):
direct callees (2):
  0x007400A0  CClientExoApp::GetMiniGame
  0x007ED7F0  CSWCObject::AttachToScene
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x3BC [dec 956]  x1  via EAX
```

## Card B13a-2-30

GOG function: CSWCCreature::EnableFeatForMenu  gog 0x00424900  callconv=(blank)  param_size_bytes=28  gog_size=600  gog_retn=28
Candidate Steam address: 0x00778940 (size 600, retn 28)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=Global::SceneHitCheck | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00778ba0:STR0/CALLEE0/SIG0.216/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004248c0 CSWCCreature::GetEquipmentForcePowers -> 0x00778900  gate_class=REVIEW
- GOG-order neighbour after:  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> 0x00778ba0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  0x0050e730 CSWRules::GetFeat -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  0x00605f00 CExoString::operator+ -> Steam 0x00733df0  in candidate callees: yes
  0x0061a5f0 CResRef::operator= -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x20*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x20*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x18*,0x1c*,0x3c*,0x84*,0x9475*,0x10000000*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x18*,0x1c*,0x3c*,0x84*,0x9475*,0x10000000*}

### abi
```
GOG param bytes: 28; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(28/4)*4 = 28 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=28 ; asm RET = 28
Stack Slot Match: TRUE (GOG 28 bytes -> x86 slots 28; ASM RET 28)
```

### prologue
```
prologue for 0x00778940: 16-byte aligned: YES
  00778940  55             PUSH EBP
  00778941  8B EC          MOV EBP,ESP
  00778943  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00778940
string literals (0):
direct callees (8):
  0x006A20F0  (unnamed)
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007408F0  CClientExoApp::GetGUIString
  0x00733570  CExoString::CExoString_char
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710CA0  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00778940 FUN_00778940 size=600 ret=28 callers=1 callees=8

void __thiscall
FUN_00778940(int param_1,uint param_2,int *param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6,undefined4 param_7,int param_8)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined1 local_60 [16];
  undefined1 local_50 [8];
  undefined1 local_48 [8];
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [8];
  int local_20;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009650f8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (param_2 != 0xffffffff) {
    local_14 = FUN_006a20f0(param_2 & 0xffff);
    local_20 = param_3[1 = byte +0x4];
    FUN_0058c3b0(local_20 + 1);
    FUN_007408f0(local_1c,*(undefined4 *)(local_14 + 8));
    local_8 = 0;
    if (param_8 == param_1) {
      FUN_00733570(&DAT_0099d77c);
      local_8._0_1_ = 1;
      FUN_00733570(&DAT_0099761c);
      local_8._0_1_ = 2;
      uVar2 = FUN_007408f0(local_38,0x9475);
      local_8._0_1_ = 3;
      FUN_00733df0(local_30,local_28);
      local_8._0_1_ = 4;
      FUN_00733df0(local_40,uVar2);
      local_8._0_1_ = 5;
      uVar2 = FUN_00733df0(local_50,local_48);
      local_8._0_1_ = 6;
      FUN_007337c0(uVar2);
      local_8._0_1_ = 5;
      FUN_00733780();
      local_8._0_1_ = 4;
      FUN_00733780();
      local_8._0_1_ = 3;
      FUN_00733780();
      local_8._0_1_ = 2;
      FUN_00733780();
      local_8._0_1_ = 1;
      FUN_00733780();
      local_8 = (uint)local_8._1_3_ << 8;
      FUN_00733780();
    }
    FUN_007337c0(local_1c);
    puVar1 = (undefined4 *)(*param_3 + 0xc + local_20 * 0x3c);
    *puVar1 = param_4;
    puVar1[1 = byte +0x4] = param_5;
    puVar1[2 = byte +0x8] = param_6;
    puVar1[3 = byte +0xc] = param_7;
    *(undefined4 *)(*param_3 + 0x1c + local_20 * 0x3c) = *(undefined4 *)(param_8 + 4);
    *(uint *)(*param_3 + 0x8 [dec 8] {int*: byte +0x20} + local_20 * 0x3c) = param_2 | 0x10000000;
    FUN_00710ca0(local_60,local_14 + 0x18);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00778ba0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00778ba0: 16-byte aligned: YES
  00778ba0  55             PUSH EBP
  00778ba1  8B EC          MOV EBP,ESP
  00778ba3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00778ba0
string literals (0):
direct callees (20):
  0x0084A640  (unnamed)
  0x005933B0  (unnamed)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x007FDB50  CSWCItem::GetActivePropertySpellID
  0x006C1450  (unnamed)
  0x006C11C0  (unnamed)
  0x005934C0  (unnamed)
  0x00414280  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x00593450  CExoArrayList<uint16>::Add
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x006C12D0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00710C20  (unnamed)
  0x00710CA0  (unnamed)
  0x007FEE70  CSWCItem::GetServerItem
  0x00604260  CSWSItem::GetActiveProperty
  0x00603B50  CSWSItem::GetUsedActivePropertyUsesLeft
  0x0064DDB0  Free(ptr*) delete-and-zero helper
struct offsets touched (top 8):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x6 [dec 6]  x2  via EDX
  +0x104 [dec 260]  x2  via EAX,EDX
  +0x19C [dec 412]  x2  via EAX,ECX
  +0x14 [dec 20]  x1  via EAX
  +0x13C [dec 316]  x1  via EAX
  +0x194 [dec 404]  x1  via EAX
  +0x198 [dec 408]  x1  via EAX
```

## Card B13a-2-31

GOG function: CSWCCreature::UpdateRelativePerception_2  gog 0x00419dc0  callconv=(blank)  param_size_bytes=4  gog_size=470  gog_retn=4
Candidate Steam address: 0x0076e610 (size 429, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetEquippedItemID | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0076e130:STR0/CALLEE0/SIG0.119/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00419c50 CSWCCreature::UpdateRelativePerception -> 0x0076dde0  gate_class=REVIEW
- GOG-order neighbour after:  0x00419fa0 CSWCCreature::EquipToInventorySlot -> 0x0076e130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: no
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x20,0xb4,0xb8,0x1b8} | Steam {0x228,0x22c,0x230,0x234,0x238,0x23c,0x240,0x244,0x248,0x24c,0x250,0x254,0x258,0x25c,...(+3)}
imm sets (* = in both, 1 common): GOG {0x4*,0x24,0x44,0x378,0x7d1,0x7f000000} | Steam {0x4*,0x8,0x7f,0x100,0x200,0x400,0x4000,0x8000,0x10000,0x20000,0x40000,0x80000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076e610: 16-byte aligned: YES
  0076e610  55             PUSH EBP
  0076e611  8B EC          MOV EBP,ESP
  0076e613  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076e610
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x228 [dec 552]  x1  via ECX
  +0x22C [dec 556]  x1  via EDX
  +0x230 [dec 560]  x1  via EAX
  +0x234 [dec 564]  x1  via ECX
  +0x238 [dec 568]  x1  via EDX
  +0x23C [dec 572]  x1  via EAX
  +0x240 [dec 576]  x1  via ECX
  +0x244 [dec 580]  x1  via EDX
  +0x248 [dec 584]  x1  via EAX
  +0x24C [dec 588]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0076e610 FUN_0076e610 size=429 ret=4 callers=41 callees=0

undefined4 __thiscall FUN_0076e610(int param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (param_2 < 0x101) {
    if (param_2 == 0x100) {
      return *(undefined4 *)(param_1 + 0x248);
    }
    switch(param_2) {
    case 1:
      uVar1 = *(undefined4 *)(param_1 + 0x228);
      break;
    case 2:
      uVar1 = *(undefined4 *)(param_1 + 0x22c);
      break;
    default:
      goto switchD_0076e652_caseD_3;
    case 4:
      uVar1 = *(undefined4 *)(param_1 + 0x230);
      break;
    case 8:
      uVar1 = *(undefined4 *)(param_1 + 0x234);
      break;
    case 0x10:
      uVar1 = *(undefined4 *)(param_1 + 0x238);
      break;
    case 0x20:
      uVar1 = *(undefined4 *)(param_1 + 0x23c);
      break;
    case 0x40:
      uVar1 = *(undefined4 *)(param_1 + 0x240);
      break;
    case 0x80:
      uVar1 = *(undefined4 *)(param_1 + 0x244);
    }
  }
  else {
    if (param_2 < 0x10001) {
      if (param_2 == 0x10000) {
        return *(undefined4 *)(param_1 + 0x25c);
      }
      if (param_2 < 0x4001) {
        if (param_2 == 0x4000) {
          return *(undefined4 *)(param_1 + 0x254);
        }
        if (param_2 == 0x200) {
          return *(undefined4 *)(param_1 + 0x24c);
        }
        if (param_2 == 0x400) {
          return *(undefined4 *)(param_1 + 0x250);
        }
      }
      else if (param_2 == 0x8000) {
        return *(undefined4 *)(param_1 + 0x258 [dec 600]);
      }
    }
    else {
      if (param_2 == 0x20000) {
        return *(undefined4 *)(param_1 + 0x260);
      }
      if (param_2 == 0x40000) {
        return *(undefined4 *)(param_1 + 0x264);
      }
      if (param_2 == 0x80000) {
        return *(undefined4 *)(param_1 + 0x268);
      }
    }
switchD_0076e652_caseD_3:
    uVar1 = DAT_00994484;
  }
  return uVar1;
}
```

### decoy: 0x0076e130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076e130: 16-byte aligned: YES
  0076e130  55             PUSH EBP
  0076e131  8B EC          MOV EBP,ESP
  0076e133  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076e130
string literals (0):
direct callees (9):
  0x007FD700  CSWCItem::Destructor
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0076EF70  CSWCCreature::GetLastInstant
  0x0076E8A0  CSWCCreature::SetEquippedItemID
  0x0076D590  CSWCCreature::UpdateWeaponClass
  0x0076EDA0  CSWCCreature::UpdateRangedAttackData
  0x0076D890  CSWCCreature::UpdateVisibleWeapons
  0x00768060  CSWCCreature::ForceAppearanceUpdate
struct offsets touched (top 8):
  +0x10 [dec 16]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x98 [dec 152]  x2  via EDX
  +0x14 [dec 20]  x1  via ECX
  +0x4C [dec 76]  x1  via EAX
  +0x80 [dec 128]  x1  via EAX
  +0x84 [dec 132]  x1  via EDX
  +0x164 [dec 356]  x1  via EDX
```

## Card B13a-2-32

GOG function: CSWCCreature::GetPersonalActions  gog 0x0042a5d0  callconv=(blank)  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x0077e650 (size 129, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0077e6f0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042a2f0 CSWCCreature::GetTargetActions -> 0x0077e370  gate_class=AUTO
- GOG-order neighbour after:  0x0042a670 CSWCCreature::GetReticlePoint -> 0x0077e6f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004254d0 CSWCCreature::GetSelfForcePowers -> Steam ?  in candidate callees: n/a
  0x004269a0 CSWCCreature::GetBehaviors -> Steam ?  in candidate callees: n/a
  0x00426e70 CSWCCreature::GetStances -> Steam ?  in candidate callees: n/a
  0x00428490 CSWCCreature::GetUsableItems -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x5*,0x8*} | Steam {0x2*,0x4*,0x5*,0x8*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077e650: 16-byte aligned: YES
  0077e650  55             PUSH EBP
  0077e651  8B EC          MOV EBP,ESP
  0077e653  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077e650
string literals (0):
direct callees (4):
  0x00779510  CSWCCreature::GetSelfForcePowers
  0x0077C510  (unnamed)
  0x0077AEB0  (unnamed)
  0x0077A9E0  CSWGuiTargetActionMenu::AddStanceActions
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077e650 FUN_0077e650 size=129 ret=8 callers=1 callees=4

void FUN_0077e650(undefined4 param_1,undefined4 param_2)

{
  switch(param_1) {
  case 0:
    FUN_00779510(param_2);
    break;
  case 1:
    FUN_0077c510(param_2,1);
    break;
  case 2:
    FUN_0077c510(param_2,4);
    break;
  case 3:
    FUN_0077c510(param_2,2);
    break;
  case 4:
    FUN_0077a9e0(param_2);
    break;
  case 5:
    FUN_0077aeb0(param_2,param_1);
  }
  return;
}
```

### decoy: 0x0077e6f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077e6f0: 16-byte aligned: YES
  0077e6f0  55             PUSH EBP
  0077e6f1  8B EC          MOV EBP,ESP
  0077e6f3  81 EC A8 00 00 00 SUB ESP,0xa8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077e6f0
string literals (3):
  0x0099F37C  at 0077e730  'head_g'
  0x0099F37C  at 0077e775  'head_g'
  0x0099F370  at 0077e793  'rootdummy'
direct callees (4):
  0x0073F490  CClientExoApp::BeginIntro
  0x00463120  (unnamed)
  0x0044B8A0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
struct offsets touched (top 10):
  +0x4 [dec 4]  x10  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x68 [dec 104]  x4  via EAX,ECX,EDX
  +0x10C [dec 268]  x3  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via EDX
  +0x6C [dec 108]  x1  via EAX
  +0x6E [dec 110]  x1  via EAX
```

## Card B13a-2-33

GOG function: CSWCCreature::ActionMenuSwapImplantAgi  gog 0x00426470  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a4b0 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a490:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426450 CSWCCreature::ActionMenuSwapImplantStr -> 0x0077a4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x00426490 CSWCCreature::ActionMenuSwapImplantEnd -> 0x0077a4b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x8*,0x40b*} | Steam {0x8*,0x40b*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a4b0: 16-byte aligned: YES
  0077a4b0  55             PUSH EBP
  0077a4b1  8B EC          MOV EBP,ESP
  0077a4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a4b0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a4b0 FUN_0077a4b0 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a4b0(void)

{
  FUN_0077a280(0x40b);
  return;
}
```

### decoy: 0x0077a490 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a490: 16-byte aligned: YES
  0077a490  55             PUSH EBP
  0077a491  8B EC          MOV EBP,ESP
  0077a493  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a490
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

## Card B13a-2-34

GOG function: CSWCCreature::SetAlignmentGoodEvil  gog 0x004291f0  callconv=(blank)  param_size_bytes=2  gog_size=155  gog_retn=4
Candidate Steam address: 0x0077d270 (size 155, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::ActionMenuForm | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0077d310:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004290a0 CSWCCreature::ActionMenuForm -> 0x0077d120  gate_class=AUTO
- GOG-order neighbour after:  0x00429290 CSWCCreature::SetBaseCNPCAlignment -> 0x0077d310  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d4c0 CSWCCreatureAppearance::SetAppearanceAlignment -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x80*,0x224*,0x310*} | Steam {0x80*,0x224*,0x310*}
imm sets (* = in both, 3 common): GOG {0x4*,0x14*,0x64*} | Steam {0x4*,0x14*,0x64*}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 2 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0077d270: 16-byte aligned: YES
  0077d270  55             PUSH EBP
  0077d271  8B EC          MOV EBP,ESP
  0077d273  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077d270
string literals (0):
direct callees (1):
  0x0085D430  (unnamed)
struct offsets touched (top 3):
  +0x224 [dec 548]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x80 [dec 128]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077d270 FUN_0077d270 size=155 ret=4 callers=1 callees=1

void __thiscall FUN_0077d270(int param_1,short param_2)

{
  short sVar1;
  
  if (*(int *)(param_1 + 0x310) != 0) {
    sVar1 = param_2;
    if (param_2 < 0) {
      sVar1 = 0;
    }
    if (sVar1 < 0x65) {
      sVar1 = param_2;
      if (param_2 < 0) {
        sVar1 = 0;
      }
    }
    else {
      sVar1 = 100;
    }
    *(short *)(*(int *)(param_1 + 0x310) + 0x80) = sVar1;
  }
  if (*(int *)(param_1 + 0x224) != 0) {
    FUN_0085d430(param_2);
  }
  return;
}
```

### decoy: 0x0077d310 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077d310: 16-byte aligned: YES
  0077d310  55             PUSH EBP
  0077d311  8B EC          MOV EBP,ESP
  0077d313  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077d310
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x82 [dec 130]  x1  via EDX
```

## Card B13a-2-35

GOG function: CSWCCreature::BlurWeapon  gog 0x00413850  callconv=(blank)  param_size_bytes=4  gog_size=152  gog_retn=4
Candidate Steam address: 0x007682b0 (size 397, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRollingFootstepSound | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.31 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007680a0:STR0/CALLEE0/SIG0.086/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004137c0 CSWCCreature::ClearVisibilityList -> 0x007679f0  gate_class=REVIEW
- GOG-order neighbour after:  0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0xc,0x34*,0x198,0x2e4} | Steam {0x4*,0x34*,0x94,0x9c,0x138,0x11ac}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*,0x1c} | Steam {0x4*,0x10*,0x20,0x7f000000}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x007682b0: 16-byte aligned: YES
  007682b0  55             PUSH EBP
  007682b1  8B EC          MOV EBP,ESP
  007682b3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007682b0
string literals (0):
direct callees (6):
  0x007680A0  (unnamed)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x00773BA0  (unnamed)
struct offsets touched (top 6):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x2  via EAX
  +0x138 [dec 312]  x2  via EAX,EDX
  +0x94 [dec 148]  x1  via EAX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007682b0 FUN_007682b0 size=397 ret=4 callers=3 callees=6

void __thiscall FUN_007682b0(int param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  int *piVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  
  iVar4 = *(int *)(param_1 + 0x138);
  *(int *)(param_1 + 0x138) = param_2;
  if (param_2 != 0) {
    FUN_007680a0(0,1);
  }
  if (iVar4 != param_2) {
    piVar3 = (int *)FUN_0077d800();
    if (piVar3 != (int *)0x0) {
      iVar4 = (**(code **)(*piVar3 + 0x94))();
      if (iVar4 == 0) {
        if (piVar3[0x46b = byte +0x11ac] == 0) {
          bVar1 = false;
        }
        else {
          sVar2 = (**(code **)(*piVar3 + 0x9c))(0);
          bVar1 = sVar2 < 1;
        }
        if (!bVar1) {
          iVar4 = FUN_0076e610(0x10);
          if (iVar4 != 0x7f000000) {
            piVar3 = (int *)FUN_0073f4d0(iVar4);
            if (piVar3 != (int *)0x0) {
              iVar4 = (**(code **)(*piVar3 + 0x34))();
              if (iVar4 != 0) {
                uVar7 = 0;
                uVar6 = 1;
                uVar5 = 0;
                iVar4 = param_2;
                (**(code **)(*piVar3 + 0x34))(param_2,0,1,0);
                FUN_007fe6c0(iVar4,uVar5,uVar6,uVar7);
              }
            }
          }
          iVar4 = FUN_0076e610(0x20);
          if (iVar4 != 0x7f000000) {
            uVar5 = *(undefined4 *)(DAT_00a1b4a4 + 4);
            piVar3 = (int *)FUN_0073f4d0(iVar4);
            if (piVar3 != (int *)0x0) {
              iVar4 = (**(code **)(*piVar3 + 0x34))(param_1,uVar5);
              if (iVar4 != 0) {
                uVar7 = 0;
                uVar6 = 1;
                uVar5 = 0;
                iVar4 = param_2;
                (**(code **)(*piVar3 + 0x34))(param_2,0,1,0);
                FUN_007fe6c0(iVar4,uVar5,uVar6,uVar7);
              }
            }
          }
        }
      }
    }
  }
  FUN_00773ba0(param_2);
  return;
}
```

### decoy: 0x007680a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007680a0: 16-byte aligned: YES
  007680a0  55             PUSH EBP
  007680a1  8B EC          MOV EBP,ESP
  007680a3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007680a0
string literals (0):
direct callees (3):
  0x007ED830  CSWCObject::GetAnimationBase
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 6):
  +0x4C [dec 76]  x8  via EDX
  +0x4 [dec 4]  x2  via EDX
  +0xC5 [dec 197]  x1  via EAX
  +0x138 [dec 312]  x1  via ECX
  +0x2E8 [dec 744]  x1  via EDX
  +0x3B0 [dec 944]  x1  via ECX
```

## Card B13a-2-36

GOG function: CSWCCreature::ActionMenuSwapImplantEnd  gog 0x00426490  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=8
Candidate Steam address: 0x0077a4b0 (size 26, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077a4d0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00426470 CSWCCreature::ActionMenuSwapImplantAgi -> 0x0077a4b0  gate_class=REVIEW
- GOG-order neighbour after:  0x004264b0 CSWCCreature::GetHostileForcePowers -> 0x0077a4f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00426240 CSWCCreature::ActionSwapImplant -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*,0x40a} | Steam {0x8*,0x40b}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0077a4b0: 16-byte aligned: YES
  0077a4b0  55             PUSH EBP
  0077a4b1  8B EC          MOV EBP,ESP
  0077a4b3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077a4b0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0077a4b0 FUN_0077a4b0 size=26 ret=8 callers=0 callees=1
THUNK -> 0x0077A280 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0077a4b0(void)

{
  FUN_0077a280(0x40b);
  return;
}
```

### decoy: 0x0077a4d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a4d0: 16-byte aligned: YES
  0077a4d0  55             PUSH EBP
  0077a4d1  8B EC          MOV EBP,ESP
  0077a4d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a4d0
string literals (0):
direct callees (1):
  0x0077A280  CSWCCreature::ActionSwapImplant
struct offsets touched (top 0):
```

## Card B13a-2-37

GOG function: CSWCCreature::SetEquippedItemID  gog 0x0041a710  callconv=(blank)  param_size_bytes=8  gog_size=473  gog_retn=8
Candidate Steam address: 0x0076eb20 (size 473, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::SetLastInstant | G7 mac: pass(0.15) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0076e8a0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041a480 CSWCCreature::GetEquippedItemID -> 0x0076e610  gate_class=AUTO
- GOG-order neighbour after:  0x0041a990 CSWCCreature::SetLastInstant -> 0x0076eb20  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x228,0x22c,0x230,0x234,0x238,0x23c,0x240,0x244,0x248,0x24c,0x250,0x254,0x258,0x25c,...(+3)} | Steam {0x280,0x284,0x288,0x28c,0x290,0x294,0x298,0x29c,0x2a0,0x2a4,0x2a8,0x2ac,0x2b0,0x2b4,...(+3)}
imm sets (* = in both, 11 common): GOG {0x8*,0x7f*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*,0x40000*,0x80000*} | Steam {0x8*,0x7f*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*,0x40000*,0x80000*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0076eb20: 16-byte aligned: YES
  0076eb20  55             PUSH EBP
  0076eb21  8B EC          MOV EBP,ESP
  0076eb23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076eb20
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x280 [dec 640]  x1  via ECX
  +0x284 [dec 644]  x1  via EAX
  +0x288 [dec 648]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x290 [dec 656]  x1  via EAX
  +0x294 [dec 660]  x1  via EDX
  +0x298 [dec 664]  x1  via ECX
  +0x29C [dec 668]  x1  via EAX
  +0x2A0 [dec 672]  x1  via EDX
  +0x2A4 [dec 676]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0076eb20 FUN_0076eb20 size=473 ret=8 callers=2 callees=0

void __thiscall FUN_0076eb20(int param_1,uint param_2,undefined4 param_3)

{
  if (param_2 < 0x101) {
    if (param_2 == 0x100) {
      *(undefined4 *)(param_1 + 0x2a0) = param_3;
    }
    else {
      switch(param_2) {
      case 1:
        *(undefined4 *)(param_1 + 0x280) = param_3;
        break;
      case 2:
        *(undefined4 *)(param_1 + 0x284) = param_3;
        break;
      case 4:
        *(undefined4 *)(param_1 + 0x288) = param_3;
        break;
      case 8:
        *(undefined4 *)(param_1 + 0x28c) = param_3;
        break;
      case 0x10:
        *(undefined4 *)(param_1 + 0x290) = param_3;
        break;
      case 0x20:
        *(undefined4 *)(param_1 + 0x294) = param_3;
        break;
      case 0x40:
        *(undefined4 *)(param_1 + 0x298) = param_3;
        break;
      case 0x80:
        *(undefined4 *)(param_1 + 0x29c) = param_3;
      }
    }
  }
  else if (param_2 < 0x10001) {
    if (param_2 == 0x10000) {
      *(undefined4 *)(param_1 + 0x2b4) = param_3;
    }
    else if (param_2 < 0x4001) {
      if (param_2 == 0x4000) {
        *(undefined4 *)(param_1 + 0x2ac) = param_3;
      }
      else if (param_2 == 0x200) {
        *(undefined4 *)(param_1 + 0x2a4) = param_3;
      }
      else if (param_2 == 0x400) {
        *(undefined4 *)(param_1 + 0x2a8) = param_3;
      }
    }
    else if (param_2 == 0x8000) {
      *(undefined4 *)(param_1 + 0x2b0) = param_3;
    }
  }
  else if (param_2 == 0x20000) {
    *(undefined4 *)(param_1 + 0x2b8) = param_3;
  }
  else if (param_2 == 0x40000) {
    *(undefined4 *)(param_1 + 0x2bc [dec 700]) = param_3;
  }
  else if (param_2 == 0x80000) {
    *(undefined4 *)(param_1 + 0x2c0) = param_3;
  }
  return;
}
```

### decoy: 0x0076e8a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076e8a0: 16-byte aligned: YES
  0076e8a0  55             PUSH EBP
  0076e8a1  8B EC          MOV EBP,ESP
  0076e8a3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0076e8a0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x228 [dec 552]  x1  via ECX
  +0x22C [dec 556]  x1  via EAX
  +0x230 [dec 560]  x1  via EDX
  +0x234 [dec 564]  x1  via ECX
  +0x238 [dec 568]  x1  via EAX
  +0x23C [dec 572]  x1  via EDX
  +0x240 [dec 576]  x1  via ECX
  +0x244 [dec 580]  x1  via EAX
  +0x248 [dec 584]  x1  via EDX
  +0x24C [dec 588]  x1  via ECX
```

## Card B13a-2-38

GOG function: CSWCCreature::GetBehaviors  gog 0x004269a0  callconv=(blank)  param_size_bytes=4  gog_size=813  gog_retn=4
Candidate Steam address: 0x0077a9e0 (size 1206, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWGuiTargetActionMenu::AddStanceActions | G7 mac: pass(0.82) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 0.82 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'AISTATE'@0099f2dc; 'NAME_STRREF'@0099f2e4; SIG 0.82
- best decoy (neighbour/other candidate) scores: 0x0077a4f0:STR0/CALLEE0/SIG0.318/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004264b0 CSWCCreature::GetHostileForcePowers -> 0x0077a4f0  gate_class=AUTO
- GOG-order neighbour after:  0x00426e70 CSWCCreature::GetStances -> 0x0077aeb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x0040ddb0 CClientExoApp::GetGUIString -> Steam 0x007408f0  in candidate callees: yes
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x0042c190 CExoArrayList_CSWGuiInterfaceAction_::SetSize -> Steam ?  in candidate callees: n/a
  0x0048f9b0 CSWCLevelUpStats::GetClass -> Steam ?  in candidate callees: n/a
  0x005110e0 Global::IsJedi -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x006058d0 CExoString::operator= -> Steam 0x007337c0  in candidate callees: yes
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'AISTATE'; 'NAME_STRREF'
disp sets (* = in both, 12 common): GOG {0x4*,0x8*,0xc*,0x1c*,0x24*,0x32*,0x38*,0x130*,0x146*,0x158*,0x310*,0x1198*} | Steam {0x4*,0x8*,0xc*,0x1c*,0x20,0x24*,0x32*,0x38*,0x130*,0x146*,0x158*,0x310*,0x1198*}
imm sets (* = in both, 10 common): GOG {0x2*,0x4*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0xec*,0x1ec65*} | Steam {0x2*,0x4*,0x9*,0xa*,0xb*,0xc*,0xd*,0x3c*,0xec*,0x410,0x412,0x413,0x414,0x1ec65*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0077a9e0: 16-byte aligned: YES
  0077a9e0  55             PUSH EBP
  0077a9e1  8B EC          MOV EBP,ESP
  0077a9e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077a9e0
string literals (7):
  0x0099F2E4  at 0077aaf8  'NAME_STRREF'
  0x0099F2DC  at 0077ab2f  'AISTATE'
  0x0099F2CC  at 0077ad16  'ib_aggressive'
  0x0099F2C0  at 0077ad57  'ib_ranged'
  0x0099F2B0  at 0077ad98  'ib_stationary'
  0x0099F2A0  at 0077ae32  'ib_jedisupport'
  0x0099F290  at 0077ae60  'ib_grenadier'
direct callees (10):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x00844E40  CSWSCreatureStats::GetClassByPosition
  0x006F2A30  IsForceUserClass(classId) {3,4,5,11..16}
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007408F0  CClientExoApp::GetGUIString
  0x007337C0  CExoString::operator=
  0x00710EB0  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x310 [dec 784]  x4  via EAX,ECX,EDX
  +0x32 [dec 50]  x2  via ECX,EDX
  +0x146 [dec 326]  x2  via EAX,ECX
  +0x1198 [dec 4504]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
  +0x24 [dec 36]  x1  via EAX
  +0x130 [dec 304]  x1  via EAX
  +0x158 [dec 344]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077a9e0 FUN_0077a9e0 size=1206 ret=4 callers=1 callees=10

void __thiscall FUN_0077a9e0(int param_1,int *param_2)

{
  undefined4 *puVar1;
  undefined1 uVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined1 local_a0 [16];
  undefined1 local_90 [16];
  undefined1 local_80 [16];
  undefined1 local_70 [16];
  undefined1 local_60 [16];
  code *local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  int local_28;
  undefined4 local_24;
  int local_20;
  uint local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965298;
  local_10 = ExceptionList;
  uVar3 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = *(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x158);
  iVar4 = FUN_0077d800(uVar3);
  local_1c = (uint)*(ushort *)(*(int *)(iVar4 + 0x1198) + 0x146);
  if ((((local_1c != 9) && (local_1c != 0xb)) && (local_1c != 0xc)) && (local_1c != 0xd)) {
    local_1c = 9;
    iVar4 = FUN_0077d800(uVar3);
    *(undefined2 *)(*(int *)(iVar4 + 0x1198) + 0x146) = (undefined2)local_1c;
  }
  local_20 = param_2[1 = byte +0x4];
  local_24 = 0xffffffff;
  local_18 = -1;
  for (local_28 = 0; local_28 < *(int *)(local_14 + 0x24); local_28 = local_28 + 1) {
    FUN_00733570("NAME_STRREF");
    local_8 = 0;
    FUN_0071bcf0(local_28,local_30,&local_24);
    local_8 = 0xffffffff;
    FUN_00733780();
    FUN_00733570("AISTATE");
    local_8 = 1;
    FUN_0071bcf0(local_28,local_38,&local_18);
    local_8 = 0xffffffff;
    FUN_00733780();
    if (local_18 != 10) {
      if (local_28 == 4) {
        uVar2 = FUN_00844e40(*(byte *)(*(int *)(param_1 + 0x310) + 0x32) - 1);
        iVar4 = FUN_006f2a30(uVar2);
        if (iVar4 == 0) {
          local_24 = 0x1ec65;
        }
      }
      FUN_0058c3b0(local_20 + 1);
      uVar5 = FUN_007408f0(local_40,local_24);
      local_8 = 2;
      FUN_007337c0(uVar5);
      local_8 = 0xffffffff;
      FUN_00733780();
      local_50 = FUN_0077d890;
      local_4c = 0;
      local_48 = 0;
      local_44 = 0;
      puVar1 = (undefined4 *)(*param_2 + 0xc + local_20 * 0x3c);
      *puVar1 = FUN_0077d890;
      puVar1[1 = byte +0x4] = 0;
      puVar1[2 = byte +0x8] = 0;
/* ... 33 more lines truncated by --max-lines */
```

### decoy: 0x0077a4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077a4f0: 16-byte aligned: YES
  0077a4f0  55             PUSH EBP
  0077a4f1  8B EC          MOV EBP,ESP
  0077a4f3  81 EC D4 00 00 00 SUB ESP,0xd4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077a4f0
string literals (0):
direct callees (11):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005453C0  CSWSObject::GetArea
  0x0058AD80  CSWSCreature::GetForceItemMask
  0x0084A1D0  (unnamed)
  0x0077A030  CSWCCreature::GetDroidConfusion
  0x00778BA0  CSWCCreature::GetEquipmentForcePowers_2
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0054B4F0  CSWSArea::GetRoomForceRating
  0x005E4E60  CGameEffect::GetInteger
  0x0044B8A0  (unnamed)
  0x008495C0  (unnamed)
struct offsets touched (top 10):
  +0x310 [dec 784]  x7  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX
  +0x2C [dec 44]  x4  via EAX,EDX
  +0x24 [dec 36]  x3  via EAX,EDX
  +0x8 [dec 8]  x2  via ECX
  +0x9C [dec 156]  x2  via EDX
  +0x130 [dec 304]  x2  via ECX,EDX
  +0x1198 [dec 4504]  x2  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x30 [dec 48]  x1  via EAX
```

## Card B13a-2-39

GOG function: CSWCCreature::GetPauseReadyAnimation  gog 0x004218a0  callconv=(blank)  param_size_bytes=0  gog_size=188  gog_retn=0
Candidate Steam address: 0x007759b0 (size 188, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetCameraFollowPart | G7 mac: pass(0.17) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: IsInjured 0x51c6f0; callers: GetPauseReadyAnimation 0x56d6a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775990:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00421880 CSWCCreature::GetCameraOffset -> 0x00775950  gate_class=REVIEW
- GOG-order neighbour after:  0x00421970 CSWCCreatureStats::HasFeat  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x0064c110 CServerExoApp::IsInjured -> Steam 0x0051c6f0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x006862d0 CSWSCreature::GetPauseReadyAnimation -> Steam 0x0056d6a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x75*,0x138*} | Steam {0x4*,0x8*,0x75*,0x138*}
imm sets (* = in both, 5 common): GOG {0x5*,0x24*,0x2710*,0x2711*,0x276c*} | Steam {0x5*,0x24*,0x2710*,0x2711*,0x276c*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007759b0: 16-byte aligned: YES
  007759b0  55             PUSH EBP
  007759b1  8B EC          MOV EBP,ESP
  007759b3  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007759b0
string literals (0):
direct callees (4):
  0x0051C6F0  CServerExoApp::IsInjured
  0x0072FB00  CClientExoApp::GetClientOptions
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x75 [dec 117]  x1  via EDX
  +0x138 [dec 312]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007759b0 FUN_007759b0 size=188 ret=0 callers=4 callees=4

undefined4 __fastcall FUN_007759b0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if (*(int *)(param_1 + 0x138) != 0) {
    return 0x2711;
  }
  if (*(int *)(DAT_00a1b4a4 + 8) == 0) {
    return 10000;
  }
  iVar1 = FUN_0051c6f0(*(undefined4 *)(param_1 + 4));
  if (iVar1 == 0) {
LAB_00775a63:
    uVar2 = 10000;
  }
  else {
    iVar1 = FUN_0072fb00();
    if (*(char *)(iVar1 + 0x75) == '\x05') {
      uVar2 = 0;
      FUN_0073fb90(0,param_1,*(undefined4 *)(DAT_00a1b4a4 + 4));
      iVar1 = FUN_007e5da0(uVar2);
      if (param_1 == iVar1) goto LAB_00775a63;
    }
    uVar2 = 0x276c;
  }
  return uVar2;
}
```

### decoy: 0x00775990 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775990: 16-byte aligned: YES
  00775990  55             PUSH EBP
  00775991  8B EC          MOV EBP,ESP
  00775993  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775990
string literals (0):
direct callees (1):
  0x0085D3D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```
