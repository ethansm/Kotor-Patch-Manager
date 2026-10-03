# Review cards B12b chunk 1

Answer each card with one TSV row in `steam_db_batches/B12b/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B12b-1-01

GOG function: CSWSCreature::AIActionChangeFacingPoint  gog 0x006c6930  callconv=(blank)  param_size_bytes=4  gog_size=962  gog_retn=4
Candidate Steam address: 0x005bbb70 (size 962, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.27) | G8 mac_rotation: rotation:0x006c6230:0.81>0.36
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: AddMoveToPointActionToFront 0x5644b0, SetIntraAreaGoal 0x55e610); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005bbf40:STR0/CALLEE1/SIG0.267/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006c66e0 CSWSCreature::AIActionCheckInterAreaPathfinding -> 0x005bb920  gate_class=REVIEW
- GOG-order neighbour after:  0x006c6d00 CSWSCreature::AIActionCheckMoveToObject -> 0x005bbf40  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00647b30 CPathfindInformation::ResetAtEndOfPath -> Steam ?  in candidate callees: n/a
  0x0064b7b0 CServerExoApp::GetModule -> Steam 0x0051bd90  in candidate callees: yes
  0x0067d0e0 CSWSCreature::AddMoveToPointActionToFront -> Steam 0x005644b0  in candidate callees: yes
  0x0067d2f0 CSWSCreature::AddPathfindingWaitActionToFront -> Steam ?  in candidate callees: n/a
  0x006a7980 CSWSObject::AddActionToFront -> Steam 0x00540ca0  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00730dc0 CSWSModule::SetIntraAreaGoal -> Steam 0x0055e610  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x4*,0x8*,0x5c*,0x60*,0x64*,0x6c*,0x70*,0x94*,0x9c*,0xa4*,0xa8*,0xac*,0xb0*,0x380*,...(+1)} | Steam {0x4*,0x8*,0x5c*,0x60*,0x64*,0x6c*,0x70*,0x94*,0x9c*,0xa4*,0xa8*,0xac*,0xb0*,0x380*,...(+1)}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x1e*,0x7c*,0x7f000000*} | Steam {0x2*,0x4*,0x1e*,0x7c*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005bbb70: 16-byte aligned: YES
  005bbb70  55             PUSH EBP
  005bbb71  8B EC          MOV EBP,ESP
  005bbb73  83 EC 7C       SUB ESP,0x7c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bbb70
string literals (0):
direct callees (9):
  0x005453C0  CSWSObject::GetArea
  0x00919723  operator new
  0x009196FD  operator delete
  0x0051BD90  CServerExoApp::GetModule
  0x0055E610  CSWSModule::SetIntraAreaGoal
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
  0x006F9D40  CPathfindInformation::ResetAtEndOfPath
  0x00540CA0  CSWSObject::AddActionToFront
struct offsets touched (top 10):
  +0x380 [dec 896]  x18  via EAX,ECX,EDX
  +0xAC [dec 172]  x5  via EAX,ECX
  +0x6C [dec 108]  x3  via ECX,EDX
  +0xB0 [dec 176]  x3  via EAX,ECX
  +0x5C [dec 92]  x2  via EAX,ECX
  +0xA8 [dec 168]  x2  via EAX,ECX
  +0x8 [dec 8]  x1  via ECX
  +0x60 [dec 96]  x1  via EDX
  +0x64 [dec 100]  x1  via ECX
  +0x70 [dec 112]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005bbb70 FUN_005bbb70 size=962 ret=4 callers=1 callees=9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005bbb70(int *param_1,int param_2)

{
  bool bVar1;
  longlong lVar2;
  short sVar3;
  int iVar4;
  undefined4 uVar5;
  float local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_30;
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  iVar4 = (**(code **)(*param_1 + 0x94))();
  uVar5 = DAT_009944bc;
  if (iVar4 != 1) {
    if (param_1[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    uVar5 = DAT_009944bc;
    if ((!bVar1) && (iVar4 = FUN_005453c0(), uVar5 = DAT_009944b4, iVar4 != 0)) {
      local_c = *(int *)(param_1[0xe0 = byte +0x380] + 0xa8);
      if (local_c < 2) {
        FUN_006f9d40();
      }
      else {
        lVar2 = (ulonglong)(local_c - 1) * 4;
        local_2c = FUN_00919723(-(uint)((int)((ulonglong)lVar2 >> 0x20) != 0) | (uint)lVar2);
        local_14 = 0;
        local_18 = local_2c;
        if (2 < local_c) {
          lVar2 = (ulonglong)(local_c - 2) * 4;
          local_30 = FUN_00919723(-(uint)((int)((ulonglong)lVar2 >> 0x20) != 0) | (uint)lVar2);
          for (local_10 = 0; local_14 = local_30, local_10 < local_c + -2; local_10 = local_10 + 1)
          {
            *(undefined4 *)(local_30 + local_10 * 4) =
                 *(undefined4 *)(*(int *)(param_1[0xe0 = byte +0x380] + 0xac) + 4 + local_10 * 4);
          }
        }
        for (local_10 = 0; local_10 < local_c + -1; local_10 = local_10 + 1) {
          *(undefined4 *)(local_18 + local_10 * 4) =
               *(undefined4 *)(*(int *)(param_1[0xe0 = byte +0x380] + 0xac) + 4 + local_10 * 4);
        }
        if (*(int *)(param_1[0xe0 = byte +0x380] + 0xac) != 0) {
          local_34 = *(undefined4 *)(param_1[0xe0 = byte +0x380] + 0xac);
          FUN_009196fd(local_34);
        }
        if (*(int *)(param_1[0xe0 = byte +0x380] + 0xb0) != 0) {
          local_38 = *(undefined4 *)(param_1[0xe0 = byte +0x380] + 0xb0);
          FUN_009196fd(local_38);
        }
        *(int *)(param_1[0xe0 = byte +0x380] + 0xb0) = local_14;
        *(int *)(param_1[0xe0 = byte +0x380] + 0xac) = local_18;
        *(int *)(param_1[0xe0 = byte +0x380] + 0xa8) = local_c + -1;
        iVar4 = FUN_005453c0();
        if (iVar4 != 0) {
          iVar4 = param_1[0xe0 = byte +0x380];
          FUN_0051bd90(iVar4);
/* ... 26 more lines truncated by --max-lines */
```

### decoy: 0x005bbf40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bbf40: 16-byte aligned: YES
  005bbf40  55             PUSH EBP
  005bbf41  8B EC          MOV EBP,ESP
  005bbf43  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bbf40
string literals (0):
direct callees (12):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x00585F10  CSWSCreature::GetUseRange
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x00585CC0  CSWSCreature::GetIsInUseRange
  0x00540CA0  CSWSObject::AddActionToFront
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
  0x0057D370  CSWSCreature::SetLockOrientationToObject
struct offsets touched (top 8):
  +0x8 [dec 8]  x3  via ECX
  +0x6C [dec 108]  x3  via EAX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x4 [dec 4]  x1  via ECX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x124 [dec 292]  x1  via ECX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-02

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest  gog 0x006d5a90  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=16
Candidate Steam address: 0x005bd3e0 (size 537, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands2? | G7 mac: pass(0.29) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005bd1d0:STR0/CALLEE0/SIG0.104/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5460 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath -> 0x0067df50  gate_class=REJECT
- GOG-order neighbour after:  0x006d5bc0 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest -> 0x005bd1d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 1 common): GOG {0x5,0x8,0xc,0x10*,0x18,0x28,0x5c,0x64} | Steam {0x2,0x4,0x10*,0x40,0x40000,0x80000}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005bd3e0: 16-byte aligned: YES
  005bd3e0  55             PUSH EBP
  005bd3e1  8B EC          MOV EBP,ESP
  005bd3e3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd3e0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd3e0 FUN_005bd3e0 size=537 ret=16 callers=1 callees=4

undefined4 FUN_005bd3e0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x40000) || (*param_2 == 0x80000)) {
    iVar1 = FUN_006d0620(0x40000);
    iVar2 = FUN_006d0620(0x80000);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x80000) {
        *param_2 = 0x40000;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x40000;
        return 4;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x40000;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x40000;
        return 2;
      }
      if (*param_2 == 0x80000) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x40000;
        return 4;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x40000;
      return 4;
    }
  }
  return 0;
}
```

### decoy: 0x005bd1d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

## Card B12b-1-03

GOG function: CSWSCreature::GetCreatureReputation_2  gog 0x006dc300  callconv=(blank)  param_size_bytes=4  gog_size=106  gog_retn=4
Candidate Steam address: 0x005d16e0 (size 106, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005d1750:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006dc210 CSWSCreature::RemoveFromVisibleList -> 0x005d15f0  gate_class=AUTO
- GOG-order neighbour after:  0x006dc370 CSWSCreature::GetVisibleListElement -> 0x005d1750  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1028*,0x102c*} | Steam {0x1028*,0x102c*}
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
prologue for 0x005d16e0: 16-byte aligned: YES
  005d16e0  55             PUSH EBP
  005d16e1  8B EC          MOV EBP,ESP
  005d16e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d16e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x1  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d16e0 FUN_005d16e0 size=106 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_005d16e0(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x102c) <= local_8) {
      return 0;
    }
    if (**(int **)(*(int *)(param_1 + 0x1028) + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### decoy: 0x005d1750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d1750: 16-byte aligned: YES
  005d1750  55             PUSH EBP
  005d1751  8B EC          MOV EBP,ESP
  005d1753  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d1750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x2  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

## Card B12b-1-04

GOG function: CSWSCreature::LearnScroll  gog 0x006a4da0  callconv=(blank)  param_size_bytes=8  gog_size=97  gog_retn=8
Candidate Steam address: 0x0058c300 (size 97, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058c250:STR0/CALLEE1/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a4d30 CSWSCreature::UseLoreOnItem -> 0x0058c290  gate_class=REVIEW
- GOG-order neighbour after:  0x006a4e10 CScriptLocation::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006a4d30 CSWSCreature::UseLoreOnItem -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00799310 Global::ExecuteCommandGetNearestObject -> Steam 0x00683d40  in candidate callers: yes
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
prologue for 0x0058c300: 16-byte aligned: YES
  0058c300  55             PUSH EBP
  0058c301  8B EC          MOV EBP,ESP
  0058c303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058c300
string literals (0):
direct callees (1):
  0x0058C290  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058c300 FUN_0058c300 size=97 ret=8 callers=2 callees=1

void __thiscall FUN_0058c300(int *param_1,undefined4 param_2,int param_3)

{
  int local_8;
  
  local_8 = param_1[1 = byte +0x4];
  FUN_0058c290(param_2);
  for (; param_3 < local_8; local_8 = local_8 + -1) {
    *(undefined4 *)(*param_1 + local_8 * 4) = *(undefined4 *)(*param_1 + -0x4 [dec -4] {int*: byte -0x10} + local_8 * 4);
  }
  *(undefined4 *)(*param_1 + param_3 * 4) = param_2;
  return;
}
```

### decoy: 0x0058c250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058c250: 16-byte aligned: YES
  0058c250  55             PUSH EBP
  0058c251  8B EC          MOV EBP,ESP
  0058c253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058c250
string literals (0):
direct callees (1):
  0x0058C4A0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B12b-1-05

GOG function: CSWSCreature::AdjustReputation  gog 0x006dba10  callconv=(blank)  param_size_bytes=8  gog_size=178  gog_retn=8
Candidate Steam address: 0x005d0d00 (size 238, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetCreatureReputation | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x006db8d0:0.91>0.67
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005d0df0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006db920 CSWSCreature::GetCreatureReputation -> 0x005d0d00  gate_class=AUTO
- GOG-order neighbour after:  0x006dbad0 CSWSCreature::SetVisibilityList -> 0x005d0eb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam ?  in candidate callees: n/a
  0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam ?  in candidate callees: n/a
  0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00782790 Global::ExecuteCommandAdjustReputation -> Steam 0x0066d1b0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*,0x11ac}
imm sets (* = in both, 1 common): GOG {0x8*,0x2c} | Steam {0x8*,0x32,0x38,0x64}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005d0d00: 16-byte aligned: YES
  005d0d00  55             PUSH EBP
  005d0d01  8B EC          MOV EBP,ESP
  005d0d03  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d0d00
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664580  CFactionManager::GetNPCFactionReputation(a,b) = data[b*N+a-1]
struct offsets touched (top 6):
  +0x11AC [dec 4524]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
  +0x10054 [dec 65620]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005d0d00 FUN_005d0d00 size=238 ret=8 callers=1 callees=2

undefined4 __thiscall FUN_005d0d00(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = *(undefined4 *)(*(int *)(param_1 + 0x1198) + 0x78);
  local_10 = param_3;
  iVar1 = FUN_0051c100(param_2);
  if (((*(int *)(param_1 + 0x11ac) == 0) || (iVar1 == 0)) || (*(int *)(iVar1 + 0x11ac) == 0)) {
    if (*(int *)(param_1 + 0x11ac) != 0) {
      local_8 = 0;
    }
    if ((iVar1 != 0) && (*(int *)(iVar1 + 0x11ac) != 0)) {
      local_10 = 0;
    }
    local_c = FUN_00664580(local_10,local_8);
  }
  else {
    local_c = 100;
  }
  return local_c;
}
```

### decoy: 0x005d0df0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d0df0: 16-byte aligned: YES
  005d0df0  55             PUSH EBP
  005d0df1  8B EC          MOV EBP,ESP
  005d0df3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d0df0
string literals (0):
direct callees (3):
  0x00664650  CFactionManager::GetIsNPCFaction(id) (1..N)
  0x00664580  CFactionManager::GetNPCFactionReputation(a,b) = data[b*N+a-1]
  0x006644D0  CFactionManager::SetNPCFactionReputation(row,col,v) = data[row*N+col-1]
struct offsets touched (top 5):
  +0x78 [dec 120]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

## Card B12b-1-06

GOG function: CSWSCreature::GetLastInstantEquip  gog 0x006a3dc0  callconv=(blank)  param_size_bytes=4  gog_size=59  gog_retn=4
Candidate Steam address: 0x0058b2e0 (size 59, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0058b320:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3cf0 CSWSCreature::GetListenCheckDistance -> 0x0058b210  gate_class=REVIEW
- GOG-order neighbour after:  0x006a3e00 CSWSCreature::HasEffect -> 0x0058b320  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x11d0*,0x11d4*} | Steam {0x11d0*,0x11d4*}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x20*} | Steam {0x4*,0x8*,0x10*,0x20*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0058b2e0: 16-byte aligned: YES
  0058b2e0  55             PUSH EBP
  0058b2e1  8B EC          MOV EBP,ESP
  0058b2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058b2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x11D0 [dec 4560]  x1  via EDX
  +0x11D4 [dec 4564]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b2e0 FUN_0058b2e0 size=59 ret=4 callers=2 callees=0

undefined4 __thiscall FUN_0058b2e0(int param_1,int param_2)

{
  undefined4 uVar1;
  
  if (param_2 == 0x10) {
    uVar1 = *(undefined4 *)(param_1 + 0x11d4);
  }
  else if (param_2 == 0x20) {
    uVar1 = *(undefined4 *)(param_1 + 0x11d0);
  }
  else {
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x0058b320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b320: 16-byte aligned: YES
  0058b320  55             PUSH EBP
  0058b321  8B EC          MOV EBP,ESP
  0058b323  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058b320
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x11D0 [dec 4560]  x1  via EAX
  +0x11D4 [dec 4564]  x1  via ECX
```

## Card B12b-1-07

GOG function: CSWSCreature::GetVisibleListElement  gog 0x006dc370  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=4
Candidate Steam address: 0x005d16e0 (size 106, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetCreatureReputation_2 | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005d1750:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006dc300 CSWSCreature::GetCreatureReputation_2 -> 0x005d16e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006dc3f0 CSWSCreature::AIActionRest -> 0x00983c00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (7 of 26): the card's A-CALLEE credit may come from these, not from callees
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: no
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: no
  0x006e1590 CSWSCreatureStats::GetArmorClassVersus -> Steam 0x006a5110  in candidate callers: no
  0x00744740 CSWSEffectListHandler::OnApplySanctuary -> Steam 0x005a1e70  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1028*,0x102c*} | Steam {0x1028*,0x102c*}
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
prologue for 0x005d16e0: 16-byte aligned: YES
  005d16e0  55             PUSH EBP
  005d16e1  8B EC          MOV EBP,ESP
  005d16e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d16e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x1  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d16e0 FUN_005d16e0 size=106 ret=4 callers=1 callees=0

undefined4 __thiscall FUN_005d16e0(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x102c) <= local_8) {
      return 0;
    }
    if (**(int **)(*(int *)(param_1 + 0x1028) + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### decoy: 0x005d1750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d1750: 16-byte aligned: YES
  005d1750  55             PUSH EBP
  005d1751  8B EC          MOV EBP,ESP
  005d1753  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d1750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x2  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

## Card B12b-1-08

GOG function: CSWSCreature::RemoveGold  gog 0x00699fd0  callconv=(blank)  param_size_bytes=8  gog_size=214  gog_retn=8
Candidate Steam address: 0x005814a0 (size 214, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x005813c0:STR0/CALLEE1/SIG0.286/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00699ef0 CSWSCreature::AddGold -> 0x005813c0  gate_class=AUTO
- GOG-order neighbour after:  0x0069a0b0 CSWSCreature::GetComponent -> 0x00581580  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x00699e10 CSWSCreature::GetGold -> Steam ?  in candidate callees: n/a
  0x00699e60 CSWSCreature::SetGold -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x007aa040 Global::ExecuteCommandTakeGoldFromCreature -> Steam 0x006949f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x1c*,0x34*,0x95*} | Steam {0x4*,0x8*,0x1c*,0x34*,0x95*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005814a0: 16-byte aligned: YES
  005814a0  55             PUSH EBP
  005814a1  8B EC          MOV EBP,ESP
  005814a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005814a0
string literals (0):
direct callees (6):
  0x005812E0  CSWSCreature::GetGold
  0x00581330  CSWSCreature::SetGold
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005814a0 FUN_005814a0 size=214 ret=8 callers=4 callees=6

void FUN_005814a0(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961a8b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_005812e0(uVar1);
  local_14 = local_14 - param_1;
  if (local_14 < 0) {
    local_14 = 0;
  }
  iVar2 = FUN_005812e0(uVar1);
  FUN_00581330(local_14);
  if ((param_2 == 1) && (iVar2 - local_14 != 0)) {
    iVar3 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar3 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(0,iVar2 - local_14);
    FUN_00584da0(0x95,local_2c);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005813c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005813c0: 16-byte aligned: YES
  005813c0  55             PUSH EBP
  005813c1  8B EC          MOV EBP,ESP
  005813c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005813c0
string literals (0):
direct callees (6):
  0x005812E0  CSWSCreature::GetGold
  0x00581330  CSWSCreature::SetGold
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

## Card B12b-1-09

GOG function: CSWSCreature::AddChemical  gog 0x0069a160  callconv=(blank)  param_size_bytes=8  gog_size=223  gog_retn=8
Candidate Steam address: 0x005814a0 (size 214, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::RemoveGold | G7 mac: pass(0.33) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.29 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005813c0:STR0/CALLEE1/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0069a100 CSWSCreature::SetComponent -> 0x005815d0  gate_class=AUTO
- GOG-order neighbour after:  0x0069a240 CSWSCreature::AddComponent -> 0x00581710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x0069a0b0 CSWSCreature::GetComponent -> Steam ?  in candidate callees: n/a
  0x0069a100 CSWSCreature::SetComponent -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x1c*,0x34*,0xe9,0x3b9ac9ff} | Steam {0x4*,0x8*,0x1c*,0x34*,0x95}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005814a0: 16-byte aligned: YES
  005814a0  55             PUSH EBP
  005814a1  8B EC          MOV EBP,ESP
  005814a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x005814a0
string literals (0):
direct callees (6):
  0x005812E0  CSWSCreature::GetGold
  0x00581330  CSWSCreature::SetGold
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005814a0 FUN_005814a0 size=214 ret=8 callers=4 callees=6

void FUN_005814a0(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961a8b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_005812e0(uVar1);
  local_14 = local_14 - param_1;
  if (local_14 < 0) {
    local_14 = 0;
  }
  iVar2 = FUN_005812e0(uVar1);
  FUN_00581330(local_14);
  if ((param_2 == 1) && (iVar2 - local_14 != 0)) {
    iVar3 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar3 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(0,iVar2 - local_14);
    FUN_00584da0(0x95,local_2c);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x005813c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005813c0: 16-byte aligned: YES
  005813c0  55             PUSH EBP
  005813c1  8B EC          MOV EBP,ESP
  005813c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x005813c0
string literals (0):
direct callees (6):
  0x005812E0  CSWSCreature::GetGold
  0x00581330  CSWSCreature::SetGold
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

## Card B12b-1-10

GOG function: CSWSCreature::UseLoreOnItem  gog 0x006a4d30  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0058c290 (size 102, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: Constructor 0x622c90, LoadFractionalCRData 0x627e50); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0058c250:STR0/CALLEE1/SIG0.667/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a4cc0 CSWSCreature::BroadcastAttackDataToParty -> 0x0058c1e0  gate_class=AUTO
- GOG-order neighbour after:  0x006a4da0 CSWSCreature::LearnScroll -> 0x0058c300  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0054c0a0 CExoArrayList_float_::Allocate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x007e9150 CSWSEncounter::Constructor -> Steam 0x00622c90  in candidate callers: yes
  0x007ee300 CSWSEncounter::LoadFractionalCRData -> Steam 0x00627e50  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x10*} | Steam {0x4*,0x8*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0058c290: 16-byte aligned: YES
  0058c290  55             PUSH EBP
  0058c291  8B EC          MOV EBP,ESP
  0058c293  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058c290
string literals (0):
direct callees (1):
  0x0058C4A0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0058c290 FUN_0058c290 size=102 ret=4 callers=5 callees=1

void __thiscall FUN_0058c290(int *param_1,undefined4 param_2)

{
  int local_c;
  
  if (param_1[1 = byte +0x4] == param_1[2 = byte +0x8]) {
    if (param_1[2 = byte +0x8] == 0) {
      local_c = 0x10;
    }
    else {
      local_c = param_1[2 = byte +0x8] << 1;
    }
    FUN_0058c4a0(local_c);
  }
  *(undefined4 *)(*param_1 + param_1[1 = byte +0x4] * 4) = param_2;
  param_1[1 = byte +0x4] = param_1[1 = byte +0x4] + 1;
  return;
}
```

### decoy: 0x0058c250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058c250: 16-byte aligned: YES
  0058c250  55             PUSH EBP
  0058c251  8B EC          MOV EBP,ESP
  0058c253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058c250
string literals (0):
direct callees (1):
  0x0058C4A0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

## Card B12b-1-11

GOG function: CSWSCreature::AddComponent  gog 0x0069a240  callconv=(blank)  param_size_bytes=8  gog_size=223  gog_retn=8
Candidate Steam address: 0x00581710 (size 223, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00581630:STR0/CALLEE1/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0069a160 CSWSCreature::AddChemical -> 0x00581630  gate_class=REVIEW
- GOG-order neighbour after:  0x0069a320 CSWSCreature::GetChemical -> 0x005817f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x0069a320 CSWSCreature::GetChemical -> Steam ?  in candidate callees: n/a
  0x0069a370 CSWSCreature::SetChemical -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x1c*,0x34*,0xea*,0x3b9ac9ff*} | Steam {0x4*,0x8*,0x1c*,0x34*,0xea*,0x3b9ac9ff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00581710: 16-byte aligned: YES
  00581710  55             PUSH EBP
  00581711  8B EC          MOV EBP,ESP
  00581713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00581710
string literals (0):
direct callees (6):
  0x005817F0  CSWSCreature::GetChemical
  0x00581840  CSWSCreature::SetChemical
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00581710 FUN_00581710 size=223 ret=8 callers=1 callees=6

void FUN_00581710(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961a8b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_005817f0(uVar1);
  local_14 = local_14 + param_1;
  if (999999999 < local_14) {
    local_14 = 999999999;
  }
  iVar2 = FUN_005817f0(uVar1);
  FUN_00581840(local_14);
  if ((param_2 == 1) && (local_14 - iVar2 != 0)) {
    iVar3 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar3 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(0,local_14 - iVar2);
    FUN_00584da0(0xea,local_2c);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00581630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00581630: 16-byte aligned: YES
  00581630  55             PUSH EBP
  00581631  8B EC          MOV EBP,ESP
  00581633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00581630
string literals (0):
direct callees (6):
  0x00581580  CSWSCreature::GetComponent
  0x005815D0  CSWSCreature::SetComponent
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

## Card B12b-1-12

GOG function: CSWSCreature::HasEffect  gog 0x006a3e00  callconv=(blank)  param_size_bytes=8  gog_size=61  gog_retn=8
Candidate Steam address: 0x0058b320 (size 61, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0058b2e0:STR0/CALLEE0/SIG0.875/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3dc0 CSWSCreature::GetLastInstantEquip -> 0x0058b2e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006a3e40 CSWSCreature::InstantUseItem -> 0x0058b360  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x11d0*,0x11d4*} | Steam {0x11d0*,0x11d4*}
imm sets (* = in both, 3 common): GOG {0x8*,0x10*,0x20*} | Steam {0x8*,0x10*,0x20*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0058b320: 16-byte aligned: YES
  0058b320  55             PUSH EBP
  0058b321  8B EC          MOV EBP,ESP
  0058b323  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058b320
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x11D0 [dec 4560]  x1  via EAX
  +0x11D4 [dec 4564]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b320 FUN_0058b320 size=61 ret=8 callers=3 callees=0

void __thiscall FUN_0058b320(int param_1,int param_2,undefined4 param_3)

{
  if (param_2 == 0x10) {
    *(undefined4 *)(param_1 + 0x11d4) = param_3;
  }
  else if (param_2 == 0x20) {
    *(undefined4 *)(param_1 + 0x11d0) = param_3;
  }
  return;
}
```

### decoy: 0x0058b2e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b2e0: 16-byte aligned: YES
  0058b2e0  55             PUSH EBP
  0058b2e1  8B EC          MOV EBP,ESP
  0058b2e3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058b2e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x11D0 [dec 4560]  x1  via EDX
  +0x11D4 [dec 4564]  x1  via ECX
```

## Card B12b-1-13

GOG function: CSWSCreature::UpdateAppearanceForEquippedItems_2  gog 0x00698df0  callconv=(blank)  param_size_bytes=0  gog_size=62  gog_retn=0
Candidate Steam address: 0x005802c0 (size 62, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetDetectModeState | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00580300:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00698a80 CSWSCreature::UpdateAppearanceDependantInfo -> 0x0057ff50  gate_class=AUTO
- GOG-order neighbour after:  0x00698e30 CSWSCreature::GetJournal -> 0x00580300  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xd8*,0x510*} | Steam {0xd8*,0x510*}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x2710*} | Steam {0x2*,0x8*,0x2710*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x005802c0: 16-byte aligned: YES
  005802c0  55             PUSH EBP
  005802c1  8B EC          MOV EBP,ESP
  005802c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005802c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x510 [dec 1296]  x2  via ECX,EDX
  +0xD8 [dec 216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005802c0 FUN_005802c0 size=62 ret=0 callers=6 callees=0

undefined1 __fastcall FUN_005802c0(int param_1)

{
  undefined1 uVar1;
  
  if ((*(int *)(param_1 + 0xd8) == 10000) && (*(char *)(param_1 + 0x510) != '\x01')) {
    uVar1 = 2;
  }
  else {
    uVar1 = *(undefined1 *)(param_1 + 0x510);
  }
  return uVar1;
}
```

### decoy: 0x00580300 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00580300: 16-byte aligned: YES
  00580300  55             PUSH EBP
  00580301  8B EC          MOV EBP,ESP
  00580303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00580300
string literals (0):
direct callees (2):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F7AE0  CSWPartyTable::GetPartyJournal
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

## Card B12b-1-14

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest  gog 0x006d5a90  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=16
Candidate Steam address: 0x005bd1d0 (size 519, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands? | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005bcdc0:STR0/CALLEE0/SIG0.263/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5460 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_WaitingForPath -> 0x0067df50  gate_class=REJECT
- GOG-order neighbour after:  0x006d5bc0 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest -> 0x005bd1d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 1 common): GOG {0x5,0x8,0xc,0x10*,0x18,0x28,0x5c,0x64} | Steam {0x2,0x3,0x10*,0x20,0x40}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd1d0 FUN_005bd1d0 size=519 ret=16 callers=1 callees=4

undefined4 FUN_005bd1d0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x10) || (*param_2 == 0x20)) {
    iVar1 = FUN_006d0620(0x10);
    iVar2 = FUN_006d0620(0x20);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x20) {
        *param_2 = 0x10;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x10;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x10;
        return 2;
      }
      if (*param_2 == 0x20) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x10;
      return 3;
    }
  }
  return 0;
}
```

### decoy: 0x005bcdc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bcdc0: 16-byte aligned: YES
  005bcdc0  55             PUSH EBP
  005bcdc1  8B EC          MOV EBP,ESP
  005bcdc3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bcdc0
string literals (0):
direct callees (9):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-15

GOG function: CSWSCreature::GetClientCreature  gog 0x006a3970  callconv=__thiscall  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0058ad80 (size 223, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:name_held_elsewhere=0x0058ae90 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058ae90:STR0/CALLEE12/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3860 CSWSCreature::GetForceItemMask -> 0x0058ad80  gate_class=AUTO
- GOG-order neighbour after:  0x006a39b0 CSWSCreature::AddPazaakCard -> 0x0058aed0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: no
GOG callers mapped through trusted rows (11 of 34): the card's A-CALLEE credit may come from these, not from callees
  0x0059cc40 CSWGuiInGameInventory::CheckFilter -> Steam 0x008a8df0  in candidate callers: no
  0x006fe7d0 CSWPartyTable::RemovePuppet -> Steam 0x005f8c60  in candidate callers: no
  0x00703350 CSWPartyTable::KillNPCObject -> Steam 0x005fd9a0  in candidate callers: no
  0x007034a0 CSWPartyTable::KillPUPObject -> Steam 0x005fdaf0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2c} | Steam {0xac,0x1150}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x2,0x4,0x8*,0xa,0x10,0x14,0x1f,0x20,0x40}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058ad80: 16-byte aligned: YES
  0058ad80  55             PUSH EBP
  0058ad81  8B EC          MOV EBP,ESP
  0058ad83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058ad80
string literals (0):
direct callees (2):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D6E30  CSWItem::GetBaseItem
struct offsets touched (top 2):
  +0xAC [dec 172]  x1  via ECX
  +0x1150 [dec 4432]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0058ad80 FUN_0058ad80 size=223 ret=0 callers=3 callees=2

uint FUN_0058ad80(void)

{
  int iVar1;
  int local_c;
  uint local_8;
  
  local_8 = 0;
  for (local_c = 0; local_c < 0x14; local_c = local_c + 1) {
    iVar1 = FUN_006d0620(local_c);
    if (iVar1 != 0) {
      iVar1 = FUN_006d6e30();
      switch(*(undefined1 *)(iVar1 + 0xac)) {
      case 0x1f:
        local_8 = local_8 | 1;
        break;
      case 0x20:
        local_8 = local_8 | 2;
        break;
      case 0x21:
        local_8 = local_8 | 4;
        break;
      case 0x22:
        local_8 = local_8 | 8;
        break;
      case 0x23:
        local_8 = local_8 | 0x10;
        break;
      case 0x24:
        local_8 = local_8 | 0x20;
        break;
      case 0x27:
      case 0x28:
      case 0x29:
        local_8 = local_8 | 0x40;
      }
    }
  }
  return local_8;
}
```

### decoy: 0x0058ae90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058ae90: 16-byte aligned: YES
  0058ae90  55             PUSH EBP
  0058ae91  8B EC          MOV EBP,ESP
  0058ae93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058ae90
string literals (0):
direct callees (1):
  0x0054A840  CSWSObject::GetClientObject
struct offsets touched (top 1):
  +0x2C [dec 44]  x1  via EDX
```

## Card B12b-1-16

GOG function: CSWSCreature::GetJournal  gog 0x00698e30  callconv=(blank)  param_size_bytes=0  gog_size=39  gog_retn=0
Candidate Steam address: 0x00580300 (size 39, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callees: GetPartyJournal 0x5f7ae0; callers: ExecuteCommandAddJournalQuestEntry 0x66c160, ExecuteCommandGetJournalEntry 0x681450, ExecuteCommandRemoveJournalQuestEntry 0x68feb0)
- best decoy (neighbour/other candidate) scores: 0x005802c0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00698df0 CSWSCreature::UpdateAppearanceForEquippedItems_2 -> 0x005802c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00698e60 CSWSCreature::EquipMostDamagingMeleeWeapon -> 0x00580330  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0064c2f0 CTwoDimArrays::GetSwoopUpgradeRowProvider -> Steam ?  in candidate callees: n/a
  0x006fd650 CSWPartyTable::GetPartyJournal -> Steam 0x005f7ae0  in candidate callees: yes
GOG callers mapped through trusted rows (4 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x00781740 Global::ExecuteCommandAddJournalQuestEntry -> Steam 0x0066c160  in candidate callers: yes
  0x00796a30 Global::ExecuteCommandGetJournalEntry -> Steam 0x00681450  in candidate callers: yes
  0x007a5500 Global::ExecuteCommandRemoveJournalQuestEntry -> Steam 0x0068feb0  in candidate callers: yes
  0x007f0290 CSWSScriptVarTable::SetInt -> Steam 0x005e6a00  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x00580300: 16-byte aligned: YES
  00580300  55             PUSH EBP
  00580301  8B EC          MOV EBP,ESP
  00580303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00580300
string literals (0):
direct callees (2):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005F7AE0  CSWPartyTable::GetPartyJournal
struct offsets touched (top 1):
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00580300 FUN_00580300 size=39 ret=0 callers=7 callees=2

void __fastcall FUN_00580300(undefined4 param_1)

{
  FUN_0051c8d0(param_1,*(undefined4 *)(DAT_00a1b4a4 + 8));
  FUN_005f7ae0();
  return;
}
```

### decoy: 0x005802c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005802c0: 16-byte aligned: YES
  005802c0  55             PUSH EBP
  005802c1  8B EC          MOV EBP,ESP
  005802c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005802c0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x510 [dec 1296]  x2  via ECX,EDX
  +0xD8 [dec 216]  x1  via EAX
```

## Card B12b-1-17

GOG function: CSWSCreature::HasAction  gog 0x006dfcc0  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=4
Candidate Steam address: 0x005d50a0 (size 116, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005d5120:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006df780 CSWSCreature::SetFormation -> 0x005d4b60  gate_class=AUTO
- GOG-order neighbour after:  0x006dfdc0 CSWSCreature::SurrenderToEnemies -> 0x005d51a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x005d50a0: 16-byte aligned: YES
  005d50a0  55             PUSH EBP
  005d50a1  8B EC          MOV EBP,ESP
  005d50a3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d50a0
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005d50a0 FUN_005d50a0 size=116 ret=4 callers=3 callees=4

undefined4 __thiscall FUN_005d50a0(undefined4 param_1,int param_2)

{
  int iVar1;
  int local_c;
  int *local_8;
  
  iVar1 = FUN_007a1780(param_1);
  if (iVar1 == 0) {
    local_c = FUN_007a1720();
    while (local_c != 0) {
      local_8 = (int *)FUN_00561430(local_c);
      if (*local_8 == param_2) {
        return 1;
      }
      FUN_0058c370(&local_c);
    }
  }
  return 0;
}
```

### decoy: 0x005d5120 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d5120: 16-byte aligned: YES
  005d5120  55             PUSH EBP
  005d5121  8B EC          MOV EBP,ESP
  005d5123  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d5120
string literals (0):
direct callees (4):
  0x007A1780  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B12b-1-18

GOG function: CSWSCreature::GetVisibleListElement  gog 0x006dc370  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=4
Candidate Steam address: 0x005d17d0 (size 18, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=AIAction | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005d1750:STR0/CALLEE7/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006dc300 CSWSCreature::GetCreatureReputation_2 -> 0x005d16e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006dc3f0 CSWSCreature::AIActionRest -> 0x00983c00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (7 of 26): the card's A-CALLEE credit may come from these, not from callees
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: no
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: no
  0x006e1590 CSWSCreatureStats::GetArmorClassVersus -> Steam 0x006a5110  in candidate callers: no
  0x00744740 CSWSEffectListHandler::OnApplySanctuary -> Steam 0x005a1e70  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1028,0x102c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0x18} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005d17d0: 16-byte aligned: YES
  005d17d0  55             PUSH EBP
  005d17d1  8B EC          MOV EBP,ESP
  005d17d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d17d0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 005d17d0 FUN_005d17d0 size=18 ret=4 callers=1 callees=0

undefined4 FUN_005d17d0(void)

{
  return DAT_009944b8;
}
```

### decoy: 0x005d1750 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d1750: 16-byte aligned: YES
  005d1750  55             PUSH EBP
  005d1751  8B EC          MOV EBP,ESP
  005d1753  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d1750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x2  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

## Card B12b-1-19

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest  gog 0x006d5cf0  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=16
Candidate Steam address: 0x005bd1d0 (size 519, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands? | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066b770:STR0/CALLEE0/SIG0.183/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5bc0 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest -> 0x005bd1d0  gate_class=REVIEW
- GOG-order neighbour after:  0x006d5e20 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest -> 0x005bd1d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 1 common): GOG {0x4,0x5,0x8,0xc,0x10*,0x18,0x28,0x5c,0x64} | Steam {0x2,0x3,0x10*,0x20,0x40}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd1d0 FUN_005bd1d0 size=519 ret=16 callers=1 callees=4

undefined4 FUN_005bd1d0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x10) || (*param_2 == 0x20)) {
    iVar1 = FUN_006d0620(0x10);
    iVar2 = FUN_006d0620(0x20);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x20) {
        *param_2 = 0x10;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x10;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x10;
        return 2;
      }
      if (*param_2 == 0x20) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x10;
      return 3;
    }
  }
  return 0;
}
```

### decoy: 0x0066b770 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066b770: 16-byte aligned: YES
  0066b770  55             PUSH EBP
  0066b771  8B EC          MOV EBP,ESP
  0066b773  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066b770
string literals (0):
direct callees (4):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x006FD9A0  CVirtualMachine::StackPopInteger
  0x0051C0A0  CServerExoApp::GetGameObject
  0x00585120  (unnamed)
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x30 [dec 48]  x1  via EAX
```

## Card B12b-1-20

GOG function: CSWSCreature::SetInParty  gog 0x006a2e10  callconv=(blank)  param_size_bytes=8  gog_size=318  gog_retn=8
Candidate Steam address: 0x0058a610 (size 810, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.62) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.13 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058a330:STR0/CALLEE9/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a2da0 CSWSCreature::GetItemRepository -> 0x0058a2c0  gate_class=AUTO
- GOG-order neighbour after:  0x006a2f50 CSWSCreature::SetIsPuppet -> 0x0058a470  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0064bca0 CServerExoApp::GetServerAIMaster -> Steam 0x0051c280  in candidate callees: no
  0x0064db40 CServerAIMaster::SetAILevel -> Steam 0x0051e120  in candidate callees: no
  0x0068ddf0 CSWSCreature::RemoveMindTrickEffects -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (8 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x006fe4d0 CSWPartyTable::TransferInventory -> Steam 0x005f8960  in candidate callers: no
  0x006fe8f0 CSWPartyTable::AddMember -> Steam 0x005f8d80  in candidate callers: no
  0x006fed90 CSWPartyTable::CreateParty -> Steam 0x005f9220  in candidate callers: no
  0x00703e60 CSWPartyTable::SwitchPlayerCharacter -> Steam 0x005fe4b0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*,0x78,0x500,0x11ac} | Steam {0x4,0x8*,0x10,0x14,0x34,0xac,0x298,0x2cc}
imm sets (* = in both, 2 common): GOG {0x2,0x4,0x8*,0x3c*,0x40} | Steam {0x8*,0x1b,0x20,0x25,0x35,0x3c*,0x98,0x2c0,0x40000000}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0058a610: 16-byte aligned: YES
  0058a610  55             PUSH EBP
  0058a611  8B EC          MOV EBP,ESP
  0058a613  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0058a610
string literals (0):
direct callees (13):
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00733540  CExoString::Constructor
  0x0073F350  CClientExoApp::GetClientLanguage
  0x00606100  CItemRepository::ItemListGetItem
  0x006D6E30  CSWItem::GetBaseItem
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042E0  CSWSItem::GetPropertyByType
  0x0058C3B0  CExoArrayList<ActionEntry(0x3C)>::SetSize(n)
  0x007356B0  CExoLocString::GetString_3
  0x007337C0  CExoString::operator=
  0x00615CC0  CSWSItem::GetIcon
  0x00710CA0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 6):
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x14 [dec 20]  x2  via ECX
  +0x298 [dec 664]  x2  via EAX,ECX
  +0x2CC [dec 716]  x2  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0xAC [dec 172]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058a610 FUN_0058a610 size=810 ret=8 callers=2 callees=13

void FUN_0058a610(int *param_1,char param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_a8;
  undefined1 local_70 [16];
  undefined1 local_60 [16];
  undefined1 local_50 [16];
  undefined1 local_40 [16];
  int local_30;
  int local_2c;
  int local_28;
  undefined1 local_24 [8];
  uint local_1c;
  undefined4 local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009516d8;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_28 = FUN_0058a2c0(1);
  FUN_00733540(uVar1);
  local_8 = 0;
  local_18 = FUN_0073f350();
  local_30 = param_1[1 = byte +0x4];
  if (local_28 != 0) {
    for (local_1c = 0; local_1c < *(uint *)(local_28 + 0x10); local_1c = local_1c + 1) {
      local_2c = FUN_00606100(local_1c);
      if ((local_2c == 0) || (iVar2 = FUN_006d6e30(), *(char *)(iVar2 + 0xac) == '\x1b')) {
        if ((param_2 == '\x01') && (iVar2 = FUN_00603f60(0x25,0), iVar2 != 0)) {
          iVar2 = FUN_006042e0(&local_14,0x25,0);
          if ((iVar2 != 0) && (local_14 != 0)) {
            FUN_0058c3b0(local_30 + 1);
            FUN_007356b0(local_18,local_24,0);
            FUN_007337c0(local_24);
            *(uint *)(*param_1 + 0x8 [dec 8] {int*: byte +0x20} + local_30 * 0x3c) = *(uint *)(local_2c + 0x14) | 0x40000000;
            uVar3 = FUN_00615cc0(local_40);
            FUN_00710ca0(local_50,uVar3);
            if (*(int *)(local_2c + 0x298) < *(int *)(local_2c + 0x2cc)) {
              local_a8 = *(undefined4 *)(local_2c + 0x2cc);
            }
            else {
              local_a8 = *(undefined4 *)(local_2c + 0x298);
            }
            *(undefined4 *)(*param_1 + 0x34 + local_30 * 0x3c) = local_a8;
            local_30 = local_30 + 1;
          }
        }
        else if ((((param_2 == '\0') && (iVar2 = FUN_00603f60(0x35,0), iVar2 != 0)) &&
                 (iVar2 = FUN_006042e0(&local_14,0x35,0), iVar2 != 0)) && (local_14 != 0)) {
          FUN_0058c3b0(local_30 + 1);
          FUN_007356b0(local_18,local_24,0);
          FUN_007337c0(local_24);
          *(uint *)(*param_1 + 0x8 [dec 8] {int*: byte +0x20} + local_30 * 0x3c) = *(uint *)(local_2c + 0x14) | 0x40000000;
          uVar3 = FUN_00615cc0(local_60);
          FUN_00710ca0(local_70,uVar3);
          local_30 = local_30 + 1;
        }
      }
    }
  }
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0058a330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058a330: 16-byte aligned: YES
  0058a330  55             PUSH EBP
  0058a331  8B EC          MOV EBP,ESP
  0058a333  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0058a330
string literals (0):
direct callees (5):
  0x00919723  operator new
  0x005752C0  CFollowInfo::CFollowInfo
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051E120  CServerAIMaster::SetAILevel
  0x009196FD  operator delete
struct offsets touched (top 4):
  +0x500 [dec 1280]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EAX,EDX
  +0x78 [dec 120]  x2  via EAX,ECX
  +0x11AC [dec 4524]  x1  via EAX
```

## Card B12b-1-21

GOG function: CSWSCreature::AddChemical  gog 0x0069a160  callconv=(blank)  param_size_bytes=8  gog_size=223  gog_retn=8
Candidate Steam address: 0x00581630 (size 223, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00581710:STR0/CALLEE1/SIG0.357/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0069a100 CSWSCreature::SetComponent -> 0x005815d0  gate_class=AUTO
- GOG-order neighbour after:  0x0069a240 CSWSCreature::AddComponent -> 0x00581710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x0069a0b0 CSWSCreature::GetComponent -> Steam ?  in candidate callees: n/a
  0x0069a100 CSWSCreature::SetComponent -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x4*,0x8*,0x1c*,0x34*,0xe9*,0x3b9ac9ff*} | Steam {0x4*,0x8*,0x1c*,0x34*,0xe9*,0x3b9ac9ff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00581630: 16-byte aligned: YES
  00581630  55             PUSH EBP
  00581631  8B EC          MOV EBP,ESP
  00581633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00581630
string literals (0):
direct callees (6):
  0x00581580  CSWSCreature::GetComponent
  0x005815D0  CSWSCreature::SetComponent
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00581630 FUN_00581630 size=223 ret=8 callers=1 callees=6

void FUN_00581630(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961a8b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_00581580(uVar1);
  local_14 = local_14 + param_1;
  if (999999999 < local_14) {
    local_14 = 999999999;
  }
  iVar2 = FUN_00581580(uVar1);
  FUN_005815d0(local_14);
  if ((param_2 == 1) && (local_14 - iVar2 != 0)) {
    iVar3 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar3 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(0,local_14 - iVar2);
    FUN_00584da0(0xe9,local_2c);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00581710 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00581710: 16-byte aligned: YES
  00581710  55             PUSH EBP
  00581711  8B EC          MOV EBP,ESP
  00581713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00581710
string literals (0):
direct callees (6):
  0x005817F0  CSWSCreature::GetChemical
  0x00581840  CSWSCreature::SetChemical
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

## Card B12b-1-22

GOG function: CSWSCreature::UpdatePureGoodEvilPowers  gog 0x006a2600  callconv=(blank)  param_size_bytes=0  gog_size=204  gog_retn=0
Candidate Steam address: 0x00589b20 (size 204, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00589bf0:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a2590 CSWSObjectActionNode::Clear  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006a26d0 CSWSCreature::ComputeArmourClass -> 0x00589bf0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x006b3410 CSWSObject::GetAQActionIDByID -> Steam ?  in candidate callees: n/a
  0x00775b00 unnamed -> Steam ?  in candidate callees: n/a
  0x00775bc0 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x3e8*,0x3ec*,0x10dc*} | Steam {0x3e8*,0x3ec*,0x10dc*}
imm sets (* = in both, 3 common): GOG {0x9*,0x1c*,0x20*} | Steam {0x9*,0x1c*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00589b20: 16-byte aligned: YES
  00589b20  55             PUSH EBP
  00589b21  8B EC          MOV EBP,ESP
  00589b23  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00589b20
string literals (0):
direct callees (3):
  0x00591510  (unnamed)
  0x00591450  (unnamed)
  0x005D7220  CSWSObject::GetAQActionIDByID
struct offsets touched (top 1):
  +0x10DC [dec 4316]  x2  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00589b20 FUN_00589b20 size=204 ret=0 callers=1 callees=3

/* WARNING: Removing unreachable block (ram,0x00589b3d) */

int __fastcall FUN_00589b20(int param_1)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 uVar3;
  undefined4 local_14;
  undefined4 local_c;
  
  local_14 = 0;
  for (local_c = 0; local_c < 0; local_c = local_c + 1) {
    uVar1 = FUN_00591510(local_c);
    sVar2 = FUN_00591450(local_c);
    if (sVar2 != 0) {
      uVar3 = FUN_005d7220(sVar2,1);
      *(undefined2 *)(param_1 + 0x3ec + local_14 * 0x1c) = uVar3;
      *(undefined2 *)(param_1 + 0x3e8 [dec 1000] + local_14 * 0x1c) = uVar1;
      local_14 = local_14 + 1;
    }
  }
  return local_14;
}
```

### decoy: 0x00589bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00589bf0: 16-byte aligned: YES
  00589bf0  55             PUSH EBP
  00589bf1  8B EC          MOV EBP,ESP
  00589bf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00589bf0
string literals (0):
direct callees (2):
  0x00590180  CSWSCombatRound::GetSpellActionPending
  0x00591A70  CSWSCombatRound::RemoveSpellAction
struct offsets touched (top 1):
  +0x10DC [dec 4316]  x2  via EAX,EDX
```

## Card B12b-1-23

GOG function: CSWSCreature::AIActionCheckMoveToPoint_2  gog 0x006c7820  callconv=(blank)  param_size_bytes=4  gog_size=848  gog_retn=4
Candidate Steam address: 0x005bca60 (size 848, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: rotation_in:0x006c6930:0.55>0.36
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005bc700:STR0/CALLEE1/SIG0.685/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006c74c0 CSWSCreature::AIActionCheckMoveToPoint -> 0x005bc700  gate_class=AUTO
- GOG-order neighbour after:  0x006c7b80 CSWSCreature::AIActionCheckMoveToFollowRadius -> 0x005bcdc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x0064baa0 CServerExoApp::GetObjectArray -> Steam 0x0051c080  in candidate callees: yes
  0x0067d0e0 CSWSCreature::AddMoveToPointActionToFront -> Steam 0x005644b0  in candidate callees: yes
  0x0067d2f0 CSWSCreature::AddPathfindingWaitActionToFront -> Steam ?  in candidate callees: n/a
  0x006a7980 CSWSObject::AddActionToFront -> Steam 0x00540ca0  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x8*,0x6c*,0x94*,0x9c*,0x11ac*} | Steam {0x4*,0x8*,0x6c*,0x94*,0x9c*,0x11ac*}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x12*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x8*,0xc*,0x12*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x50*,...(+4)}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005bca60: 16-byte aligned: YES
  005bca60  55             PUSH EBP
  005bca61  8B EC          MOV EBP,ESP
  005bca63  81 EC A0 00 00 00 SUB ESP,0xa0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bca60
string literals (0):
direct callees (9):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x00540CA0  CSWSObject::AddActionToFront
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
struct offsets touched (top 6):
  +0x6C [dec 108]  x3  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bca60 FUN_005bca60 size=848 ret=4 callers=1 callees=9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005bca60(int *param_1,int param_2)

{
  bool bVar1;
  float fVar2;
  short sVar3;
  int iVar4;
  undefined4 uVar5;
  float10 fVar6;
  undefined1 *puVar7;
  int local_a0;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  int local_3c;
  int local_38;
  float local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [4];
  undefined4 local_18;
  char local_11;
  undefined1 local_10 [8];
  undefined4 local_8;
  
  iVar4 = (**(code **)(*param_1 + 0x94))();
  uVar5 = DAT_009944bc;
  if (iVar4 != 1) {
    if (param_1[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    uVar5 = DAT_009944bc;
    if (!bVar1) {
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_28,(void *)(param_2 + 0x38),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_2c,(void *)(param_2 + 0x3c),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_30,(void *)(param_2 + 0x40),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_38,(void *)(param_2 + 0x44),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_24,(void *)(param_2 + 0x48),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_20,(void *)(param_2 + 0x4c),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_34,(void *)(param_2 + 0x50),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_18,(void *)(param_2 + 0x54),4);
      puVar7 = local_1c;
      iVar4 = local_38;
      FUN_0051c080(local_38,puVar7);
      local_11 = FUN_0053dfb0(iVar4,puVar7);
      uVar5 = DAT_009944bc;
      if (local_11 == DAT_0099447f) {
        iVar4 = FUN_005453c0();
        if ((iVar4 == 0) || (iVar4 = FUN_005453c0(), local_38 == *(int *)(iVar4 + 0x124))) {
          local_a0 = 0;
        }
        else {
          local_a0 = 1;
        }
        local_3c = local_a0;
        local_48 = local_28;
        local_44 = local_2c;
        local_40 = local_30;
        FUN_004452b0(local_10,param_1 + 0x25,&local_48);
        local_8 = 0;
        if ((local_3c != 0) ||
           (fVar2 = local_34 + (float)_DAT_00987538,
           fVar6 = (float10)FUN_0044b8a0(local_10,local_10), uVar5 = DAT_009944b8,
           (float10)(fVar2 * fVar2) < fVar6)) {
/* ... 19 more lines truncated by --max-lines */
```

### decoy: 0x005bc700 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bc700: 16-byte aligned: YES
  005bc700  55             PUSH EBP
  005bc701  8B EC          MOV EBP,ESP
  005bc703  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bc700
string literals (0):
direct callees (9):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x00540CA0  CSWSObject::AddActionToFront
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
struct offsets touched (top 8):
  +0x6C [dec 108]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x380 [dec 896]  x1  via ECX
  +0x1114 [dec 4372]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-24

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward  gog 0x006d5f50  callconv=(blank)  param_size_bytes=12  gog_size=412  gog_retn=16
Candidate Steam address: 0x005bd1d0 (size 519, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands? | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005bd610:STR0/CALLEE0/SIG0.15/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5e20 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest -> 0x005bd1d0  gate_class=REVIEW
- GOG-order neighbour after:  0x006d60f0 CSWSCreature::ComputeSpeedFactor -> 0x005cb330  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x007059f0 CSWSArea::TestSafeLocationPoint -> Steam 0x0054b650  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0x380,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 2 common): GOG {0x2*,0x8,0xc,0x10*,0x18,0x54,0x5c,0x64} | Steam {0x2*,0x3,0x10*,0x20,0x40}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 16)
```

### prologue
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd1d0 FUN_005bd1d0 size=519 ret=16 callers=1 callees=4

undefined4 FUN_005bd1d0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x10) || (*param_2 == 0x20)) {
    iVar1 = FUN_006d0620(0x10);
    iVar2 = FUN_006d0620(0x20);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x20) {
        *param_2 = 0x10;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x10;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x10;
        return 2;
      }
      if (*param_2 == 0x20) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x10;
      return 3;
    }
  }
  return 0;
}
```

### decoy: 0x005bd610 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bd610: 16-byte aligned: YES
  005bd610  55             PUSH EBP
  005bd611  8B EC          MOV EBP,ESP
  005bd613  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bd610
string literals (0):
direct callees (3):
  0x005BDA40  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
  0x006D0620  CSWInventory::GetItemInSlot
struct offsets touched (top 1):
  +0x1150 [dec 4432]  x1  via EAX
```

## Card B12b-1-25

GOG function: CSWSCreature::GetListenCheckDistance  gog 0x006a3cf0  callconv=(blank)  param_size_bytes=0  gog_size=201  gog_retn=0
Candidate Steam address: 0x0058b5a0 (size 475, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::ForceEquipClothing | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058b360:STR0/CALLEE0/SIG0.077/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3c20 CSWSCreature::GetSpotCheckDistance -> 0x0058b140  gate_class=AUTO
- GOG-order neighbour after:  0x006a3dc0 CSWSCreature::GetLastInstantEquip -> 0x0058b2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: no
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'SecondaryRange'
disp sets (* = in both, 1 common): GOG {0x8*,0xbc,0x130,0x520,0x1020} | Steam {0x4,0x8*,0x2c8,0x11e0}
imm sets (* = in both, 0 common): GOG {0x12,0x20} | Steam {0x2,0x3,0x4,0x4c,0x30c,0x7f000000,0xfffffff7}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058b5a0: 16-byte aligned: YES
  0058b5a0  55             PUSH EBP
  0058b5a1  8B EC          MOV EBP,ESP
  0058b5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0058b5a0
string literals (2):
  0x00995918  at 0058b5ec  'g_a_clothes01'
  0x00995918  at 0058b695  'g_a_clothes01'
direct callees (13):
  0x0058A2C0  CSWSCreature::GetItemRepository
  0x00733570  CExoString::CExoString_char
  0x00605CB0  CItemRepository::FindItemWithTag
  0x00733780  CExoString::~CExoString
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00919723  operator new
  0x005FFD60  CSWSItem::Constructor
  0x00710840  (unnamed)
  0x00601610  CSWSItem::LoadFromTemplate
  0x00600380  CSWSItem::SetPossessor
  0x00605D80  CItemRepository::GetItemInRepository
  0x00605E80  CItemRepository::RemoveItem
  0x0056D6F0  CSWSCreature::EquipItem
struct offsets touched (top 4):
  +0x2C8 [dec 712]  x2  via EAX,ECX
  +0x11E0 [dec 4576]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b5a0 FUN_0058b5a0 size=475 ret=0 callers=2 callees=13

void __fastcall FUN_0058b5a0(int param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 *local_58;
  undefined1 local_3c [16];
  int local_2c;
  undefined4 *local_28;
  undefined1 local_24 [8];
  undefined4 *local_1c;
  undefined4 local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00951783;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x11e0) != 0) {
    local_18 = FUN_0058a2c0(1);
    FUN_00733570("g_a_clothes01");
    local_8 = 0;
    local_14 = FUN_00605cb0(local_24);
    local_8 = 0xffffffff;
    FUN_00733780(uVar1);
    local_1c = (undefined4 *)FUN_0051c0e0(local_14);
    if (local_1c == (undefined4 *)0x0) {
      local_2c = FUN_00919723(0x30c);
      local_8 = 1;
      if (local_2c == 0) {
        local_58 = (undefined4 *)0x0;
      }
      else {
        local_58 = (undefined4 *)FUN_005ffd60(0x7f000000);
      }
      local_28 = local_58;
      local_8 = 0xffffffff;
      local_1c = local_58;
      if (local_58 != (undefined4 *)0x0) {
        FUN_00710840("g_a_clothes01");
        iVar2 = FUN_00601610(local_3c);
        if (iVar2 == 0) {
          if (local_1c != (undefined4 *)0x0) {
            (**(code **)*local_1c)(1);
          }
          local_1c = (undefined4 *)0x0;
        }
        else {
          local_1c[0xb2 = byte +0x2c8] = local_1c[0xb2 = byte +0x2c8] & 0xfffffff7 | 8;
          FUN_00600380(*(undefined4 *)(param_1 + 4),0,0,0x7f000000);
        }
      }
    }
    if (local_1c != (undefined4 *)0x0) {
      iVar2 = FUN_00605d80(local_1c,0);
      if (iVar2 != 0) {
        FUN_00605e80(local_1c);
      }
      FUN_0056d6f0(2,local_1c,1,0,0);
    }
    *(undefined4 *)(param_1 + 0x11e0) = 0;
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0058b360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b360: 16-byte aligned: YES
  0058b360  55             PUSH EBP
  0058b361  8B EC          MOV EBP,ESP
  0058b363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0058b360
string literals (0):
direct callees (11):
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x00604260  CSWSItem::GetActiveProperty
  0x005482A0  (unnamed)
  0x006070A0  CSWSItem::DecrementUses
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
  0x005453C0  CSWSObject::GetArea
  0x00919723  operator new
  0x005E1A60  (unnamed)
  0x005E1BD0  (unnamed)
  0x0051C280  CServerExoApp::GetServerAIMaster
  0x0051E170  CServerAIMaster::AddEventDeltaTime(dayD,msD,callerId,targetId,eventId,data)
struct offsets touched (top 10):
  +0x8 [dec 8]  x3  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x124 [dec 292]  x2  via ECX,EDX
  +0x2 [dec 2]  x1  via ECX
  +0x24C [dec 588]  x1  via EAX
  +0x250 [dec 592]  x1  via EAX
  +0x288 [dec 648]  x1  via ECX
  +0x520 [dec 1312]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
  +0x11D8 [dec 4568]  x1  via ECX
```

## Card B12b-1-26

GOG function: CSWSCreature::AIActionCheckMoveAwayFromLocation  gog 0x006c6230  callconv=(blank)  param_size_bytes=4  gog_size=299  gog_retn=4
Candidate Steam address: 0x005bb470 (size 299, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: rotation:0x006c66e0:0.90>0.71
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005bb2c0:STR0/CALLEE0/SIG0.625/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006c6080 CSWSCreature::AIActionChangeFacingObject -> 0x005bb2c0  gate_class=AUTO
- GOG-order neighbour after:  0x006c6360 CSWSCreature::AIActionCheckMoveToPointRadius -> 0x005bb5a0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x006aabf0 CSWSObject::SetOrientation -> Steam ?  in candidate callees: n/a
  0x0080c170 Global::normalize -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x94*,0x9c*,0x11ac*} | Steam {0x94*,0x9c*,0x11ac*}
imm sets (* = in both, 8 common): GOG {0x4*,0x8*,0xc*,0x38*,0x3c*,0x40*,0x50*,0x94*} | Steam {0x4*,0x8*,0xc*,0x38*,0x3c*,0x40*,0x50*,0x94*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005bb470: 16-byte aligned: YES
  005bb470  55             PUSH EBP
  005bb471  8B EC          MOV EBP,ESP
  005bb473  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bb470
string literals (0):
direct callees (5):
  0x0091DBA0  _memcpy
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 3):
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bb470 FUN_005bb470 size=299 ret=4 callers=1 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_005bb470(int *param_1,int param_2)

{
  bool bVar1;
  short sVar2;
  int iVar3;
  undefined4 uVar4;
  float10 fVar5;
  undefined1 local_34 [12];
  undefined1 local_28 [8];
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar3 = (**(code **)(*param_1 + 0x94))();
  uVar4 = DAT_009944bc;
  if (iVar3 != 1) {
    if (param_1[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar2 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar2 < 1;
    }
    uVar4 = DAT_009944bc;
    if (!bVar1) {
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_8,(void *)(param_2 + 0x38),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_c,(void *)(param_2 + 0x3c),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_1c,(void *)(param_2 + 0x40),4);
      local_18 = local_8;
      local_14 = local_c;
      local_10 = local_1c;
      FUN_004452b0(local_28,&local_18,param_1 + 0x25);
      local_20 = 0;
      fVar5 = (float10)FUN_0044b8a0(local_28,local_28);
      uVar4 = DAT_009944b8;
      if ((float10)_DAT_00993768 <= fVar5) {
        uVar4 = FUN_005143c0(local_34,local_28);
        FUN_00543f10(uVar4);
        uVar4 = DAT_009944b8;
      }
    }
  }
  return uVar4;
}
```

### decoy: 0x005bb2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bb2c0: 16-byte aligned: YES
  005bb2c0  55             PUSH EBP
  005bb2c1  8B EC          MOV EBP,ESP
  005bb2c3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bb2c0
string literals (0):
direct callees (8):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x005143C0  (unnamed)
  0x00543F10  CSWSObject::SetOrientation
struct offsets touched (top 6):
  +0x8 [dec 8]  x3  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x10 [dec 16]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-27

GOG function: CSWSCreature::CheckItemClassRestrictions  gog 0x006de780  callconv=(blank)  param_size_bytes=4  gog_size=298  gog_retn=4
Candidate Steam address: 0x005d3b60 (size 298, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.71) | G8 mac_rotation: rotation:0x006ddee0:0.83>0.66
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005d3a20:STR0/CALLEE2/SIG0.458/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006de640 CSWSCreature::CheckItemFeatRestrictions -> 0x005d3a20  gate_class=AUTO
- GOG-order neighbour after:  0x006de8b0 CSWSCreature::CheckItemPCRestrictions -> 0x005d3c90  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00667790 unnamed -> Steam ?  in candidate callees: n/a
  0x00667ad0 CSWSItem::GetPassiveProperty -> Steam 0x006042a0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x006ddba0 CSWSCreature::CanUseItem -> Steam 0x005d2f80  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x2*,0x11*,0x88*,0x28c*,0x2d4*} | Steam {0x2*,0x11*,0x88*,0x28c*,0x2d4*}
imm sets (* = in both, 5 common): GOG {0x2*,0x4*,0x30*,0x3c*,0xff*} | Steam {0x2*,0x4*,0x30*,0x3c*,0xff*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005d3b60: 16-byte aligned: YES
  005d3b60  55             PUSH EBP
  005d3b61  8B EC          MOV EBP,ESP
  005d3b63  83 EC 30       SUB ESP,0x30
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d3b60
string literals (0):
direct callees (2):
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006042A0  CSWSItem::GetPassiveProperty
struct offsets touched (top 5):
  +0x2 [dec 2]  x1  via EAX
  +0x11 [dec 17]  x1  via EDX
  +0x88 [dec 136]  x1  via EDX
  +0x28C [dec 652]  x1  via ECX
  +0x2D4 [dec 724]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d3b60 FUN_005d3b60 size=298 ret=4 callers=2 callees=2

undefined4 __thiscall FUN_005d3b60(int *param_1,int param_2)

{
  ushort uVar1;
  bool bVar2;
  ushort uVar3;
  int iVar4;
  short *psVar5;
  int local_c;
  
  iVar4 = FUN_00603f60(0x3c,0);
  if (iVar4 != 0) {
    for (local_c = 0; local_c < *(int *)(param_2 + 0x28c); local_c = local_c + 1) {
      iVar4 = FUN_006042a0(local_c);
      if ((*(byte *)(iVar4 + 0x11) == 0xff) ||
         ((1 << (*(byte *)(iVar4 + 0x11) & 0x1f) & *(uint *)(param_2 + 0x2d4)) != 0)) {
        bVar2 = true;
      }
      else {
        bVar2 = false;
      }
      if ((bVar2) && (psVar5 = (short *)FUN_006042a0(local_c), *psVar5 == 0x3c)) {
        iVar4 = FUN_006042a0(local_c);
        uVar1 = *(ushort *)(iVar4 + 2);
        uVar3 = (**(code **)(*param_1 + 0x88))();
        if (uVar1 != (uVar3 & 0xff)) {
          if ((uVar1 != 0) && (uVar1 != 1)) {
            return 0;
          }
          if ((uVar3 & 0xff) != 2) {
            return 0;
          }
        }
      }
    }
  }
  return 1;
}
```

### decoy: 0x005d3a20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d3a20: 16-byte aligned: YES
  005d3a20  55             PUSH EBP
  005d3a21  8B EC          MOV EBP,ESP
  005d3a23  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d3a20
string literals (0):
direct callees (4):
  0x00603F60  CSWSItem::GetPropertyByTypeExists
  0x006B83F0  CSWSCreatureStats::HasFeat
  0x006042A0  CSWSItem::GetPassiveProperty
  0x006B8440  CSWSCreatureStats::GetFeatRemainingUses
struct offsets touched (top 5):
  +0x1198 [dec 4504]  x2  via EAX
  +0x2 [dec 2]  x1  via EAX
  +0x11 [dec 17]  x1  via ECX
  +0x28C [dec 652]  x1  via EAX
  +0x2D4 [dec 724]  x1  via EAX
```

## Card B12b-1-28

GOG function: CSWSCreature::BroadcastAttackOfOpportunity  gog 0x0069df00  callconv=(blank)  param_size_bytes=0  gog_size=85  gog_retn=0
Candidate Steam address: 0x00585420 (size 85, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetIsBlindOrInDarkness | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00585480:STR0/CALLEE0/SIG0.118/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0069ddf0 CSWSCreature::UnDisguise -> 0x00585310  gate_class=AUTO
- GOG-order neighbour after:  0x0069df60 CSWSCreature::GetInvisible -> 0x00585480  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x006f4030 CSWSCreatureStats::GetSkillRank -> Steam 0x006b7bb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0xff4*} | Steam {0xff4*}
imm sets (* = in both, 3 common): GOG {0x2*,0x8*,0x10*} | Steam {0x2*,0x8*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00585420: 16-byte aligned: YES
  00585420  55             PUSH EBP
  00585421  8B EC          MOV EBP,ESP
  00585423  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00585420
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xFF4 [dec 4084]  x3  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00585420 FUN_00585420 size=85 ret=0 callers=4 callees=0

undefined4 __fastcall FUN_00585420(int param_1)

{
  undefined4 uVar1;
  
  if (((*(byte *)(param_1 + 0xff4) & 0x10) == 0) &&
     (((*(byte *)(param_1 + 0xff4) & 8) == 0 || ((*(byte *)(param_1 + 0xff4) & 2) != 0)))) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}
```

### decoy: 0x00585480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00585480: 16-byte aligned: YES
  00585480  55             PUSH EBP
  00585481  8B EC          MOV EBP,ESP
  00585483  83 EC 74       SUB ESP,0x74
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00585480
string literals (0):
direct callees (6):
  0x005E4E60  CGameEffect::GetInteger
  0x005E1680  CSWSObject::GetAIStateReputation
  0x005D1750  CSWSCreature::GetVisibleListElement
  0x006E4630  (unnamed)
  0x005D1400  CSWSCreature::AddToVisibleList(id,seen,heard,invis,hidden)
  0x00586640  CSWSCreature::PacifyCreature_2
struct offsets touched (top 9):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0xFF4 [dec 4084]  x5  via ECX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX,EDX
  +0x30 [dec 48]  x2  via EDX
  +0x122 [dec 290]  x2  via EDX
  +0xE0 [dec 224]  x1  via ECX
  +0x14C [dec 332]  x1  via EDX
  +0x18A [dec 394]  x1  via EAX
```

## Card B12b-1-29

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_PathWaitDest  gog 0x006d5bc0  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=16
Candidate Steam address: 0x005bd1d0 (size 519, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands? | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x005bcdc0:STR0/CALLEE0/SIG0.259/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5a90 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_OffsetWaitDest -> 0x005bd1d0  gate_class=REVIEW
- GOG-order neighbour after:  0x006d5cf0 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest -> 0x005bd1d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 2 common): GOG {0x3*,0x5,0x8,0xc,0x10*,0x18,0x28,0x5c,0x64} | Steam {0x2,0x3*,0x10*,0x20,0x40}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd1d0 FUN_005bd1d0 size=519 ret=16 callers=1 callees=4

undefined4 FUN_005bd1d0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x10) || (*param_2 == 0x20)) {
    iVar1 = FUN_006d0620(0x10);
    iVar2 = FUN_006d0620(0x20);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x20) {
        *param_2 = 0x10;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x10;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x10;
        return 2;
      }
      if (*param_2 == 0x20) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x10;
      return 3;
    }
  }
  return 0;
}
```

### decoy: 0x005bcdc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bcdc0: 16-byte aligned: YES
  005bcdc0  55             PUSH EBP
  005bcdc1  8B EC          MOV EBP,ESP
  005bcdc3  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bcdc0
string literals (0):
direct callees (9):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0073FEC0  CClientExoApp::ServerToClientObjectId
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
struct offsets touched (top 5):
  +0x4 [dec 4]  x6  via ECX,EDX
  +0x8 [dec 8]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-30

GOG function: CSWSCreature::GetListenCheckDistance  gog 0x006a3cf0  callconv=(blank)  param_size_bytes=0  gog_size=201  gog_retn=0
Candidate Steam address: 0x0058b140 (size 201, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetSpotCheckDistance | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.83 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058b210:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3c20 CSWSCreature::GetSpotCheckDistance -> 0x0058b140  gate_class=AUTO
- GOG-order neighbour after:  0x006a3dc0 CSWSCreature::GetLastInstantEquip -> 0x0058b2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'SecondaryRange'
disp sets (* = in both, 4 common): GOG {0x8*,0xbc*,0x130*,0x520*,0x1020} | Steam {0x8*,0xbc*,0x130*,0x520*,0x101c}
imm sets (* = in both, 2 common): GOG {0x12*,0x20*} | Steam {0x12*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058b140: 16-byte aligned: YES
  0058b140  55             PUSH EBP
  0058b141  8B EC          MOV EBP,ESP
  0058b143  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0058b140
string literals (1):
  0x00995574  at 0058b1a7  'PrimaryRange'
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0xBC [dec 188]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x101C [dec 4124]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b140 FUN_0058b140 size=201 ret=0 callers=8 callees=5

float10 __fastcall FUN_0058b140(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_1c [8];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972f58;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = *(float *)(param_1 + 0x101c);
  if (*(int *)(param_1 + 0x520) != 0) {
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar2 = FUN_0051c8f0(uVar1);
    iVar3 = FUN_0051c100(uVar2);
    if (iVar3 != param_1) {
      FUN_00733570("PrimaryRange");
      local_8 = 0;
      iVar3 = *(int *)(DAT_00a1b4d0 + 0x130);
      uVar2 = *(undefined4 *)(iVar3 + 0xbc);
      FUN_0071b670(0x12,local_1c,&local_14);
      local_8 = 0xffffffff;
      FUN_00733780(uVar1,param_1,uVar2,iVar3,uVar4);
    }
  }
  ExceptionList = local_10;
  return (float10)local_14;
}
```

### decoy: 0x0058b210 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b210: 16-byte aligned: YES
  0058b210  55             PUSH EBP
  0058b211  8B EC          MOV EBP,ESP
  0058b213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0058b210
string literals (1):
  0x00995908  at 0058b277  'SecondaryRange'
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0xBC [dec 188]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x1020 [dec 4128]  x1  via EAX
```

## Card B12b-1-31

GOG function: CSWSCreature::AIActionCheckInterAreaPathfinding  gog 0x006c66e0  callconv=(blank)  param_size_bytes=4  gog_size=581  gog_retn=4
Candidate Steam address: 0x005bb920 (size 581, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.43) | G8 mac_rotation: rotation:0x006c6930:0.97>0.55
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005bb5a0:STR0/CALLEE2/SIG0.669/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006c6360 CSWSCreature::AIActionCheckMoveToPointRadius -> 0x005bb5a0  gate_class=AUTO
- GOG-order neighbour after:  0x006c6930 CSWSCreature::AIActionChangeFacingPoint -> 0x005bbb70  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0067d0e0 CSWSCreature::AddMoveToPointActionToFront -> Steam 0x005644b0  in candidate callees: yes
  0x0067d2f0 CSWSCreature::AddPathfindingWaitActionToFront -> Steam ?  in candidate callees: n/a
  0x006a7980 CSWSObject::AddActionToFront -> Steam 0x00540ca0  in candidate callees: yes
  0x006ac0a0 CSWSObject::GetArea -> Steam ?  in candidate callees: n/a
  0x00704df0 CSWSArea::ComputeAwayVector -> Steam 0x0054aa50  in candidate callees: yes
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x6c*,0x94*,0x9c*,0x124*,0x380*,0x11ac*} | Steam {0x6c*,0x94*,0x9c*,0x124*,0x380*,0x11ac*}
imm sets (* = in both, 14 common): GOG {0x2*,0x4*,0x8*,0xc*,0x2c*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x60*,0x94*,0x7f000000*} | Steam {0x2*,0x4*,0x8*,0xc*,0x2c*,0x38*,0x3c*,0x40*,0x44*,0x48*,0x4c*,0x60*,0x94*,0x7f000000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x005bb920: 16-byte aligned: YES
  005bb920  55             PUSH EBP
  005bb921  8B EC          MOV EBP,ESP
  005bb923  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bb920
string literals (0):
direct callees (8):
  0x005453C0  CSWSObject::GetArea
  0x0091DBA0  _memcpy
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0054AA50  CSWSArea::ComputeAwayVector
  0x00540CA0  CSWSObject::AddActionToFront
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
struct offsets touched (top 6):
  +0x6C [dec 108]  x3  via EAX,ECX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x124 [dec 292]  x1  via EDX
  +0x380 [dec 896]  x1  via EDX
  +0x11AC [dec 4524]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bb920 FUN_005bb920 size=581 ret=4 callers=1 callees=8

undefined4 __thiscall FUN_005bb920(int *param_1,int param_2)

{
  bool bVar1;
  float fVar2;
  short sVar3;
  int iVar4;
  undefined4 uVar5;
  float10 fVar6;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined1 local_38 [12];
  int local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined1 local_18 [4];
  float local_14;
  undefined1 local_10 [12];
  
  iVar4 = (**(code **)(*param_1 + 0x94))();
  uVar5 = DAT_009944bc;
  if (iVar4 != 1) {
    if (param_1[0x46b = byte +0x11ac] == 0) {
      bVar1 = false;
    }
    else {
      sVar3 = (**(code **)(*param_1 + 0x9c))(0);
      bVar1 = sVar3 < 1;
    }
    uVar5 = DAT_009944bc;
    if (!bVar1) {
      local_28 = 0;
      local_24 = 0;
      local_20 = 0;
      local_2c = FUN_005453c0();
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_28,(void *)(param_2 + 0x38),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_24,(void *)(param_2 + 0x3c),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_20,(void *)(param_2 + 0x40),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_1c,(void *)(param_2 + 0x44),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](&local_14,(void *)(param_2 + 0x48),4);
      _memcpy [GHIDRA-AUTO-LABEL: unreliable](local_18,(void *)(param_2 + 0x4c),4);
      FUN_004452b0(local_10,param_1 + 0x25,&local_28);
      fVar2 = local_14 * local_14;
      fVar6 = (float10)FUN_0044b8a0(local_10,local_10);
      uVar5 = DAT_009944b8;
      if (fVar6 <= (float10)fVar2) {
        FUN_0054aa50(local_38,param_1 + 0x25,&local_28,local_14,param_1[0xe0 = byte +0x380]);
        local_3c = *(undefined4 *)(local_2c + 0x124);
        FUN_00540ca0(0x2c,*(undefined2 *)(param_2 + 0x6c),2,&local_28,2,&local_24,2,&local_20,1,
                     &local_1c,2,&local_14,1,local_18,0,0,0,0,0,0,0,0,0,0,0,0,0,0);
        local_48 = 0;
        local_44 = 0;
        local_40 = 0;
        FUN_005644b0(*(undefined2 *)(param_2 + 0x6c),local_38,local_3c,0x7f000000,local_1c,0,0,0,0,0
                     ,0,1,0,&local_48,0,0,0);
        FUN_005646c0(*(undefined2 *)(param_2 + 0x6c),0);
        uVar5 = DAT_009944b8;
      }
    }
  }
  return uVar5;
}
```

### decoy: 0x005bb5a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005bb5a0: 16-byte aligned: YES
  005bb5a0  55             PUSH EBP
  005bb5a1  8B EC          MOV EBP,ESP
  005bb5a3  81 EC 84 00 00 00 SUB ESP,0x84
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005bb5a0
string literals (0):
direct callees (14):
  0x0091DBA0  _memcpy
  0x0051C080  CServerExoApp::GetObjectArray
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x005453C0  CSWSObject::GetArea
  0x00521060  (unnamed)
  0x007A1720  (unnamed)
  0x00576CA0  (unnamed)
  0x0058C370  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  (unnamed)
  0x0054AA50  CSWSArea::ComputeAwayVector
  0x00540CA0  CSWSObject::AddActionToFront
  0x005644B0  CSWSCreature::AddMoveToPointActionToFront
  0x005646C0  CSWSCreature::AddPathfindingWaitActionToFront
struct offsets touched (top 8):
  +0x6C [dec 108]  x4  via EAX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x4 [dec 4]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EDX
  +0x124 [dec 292]  x1  via EAX
  +0x380 [dec 896]  x1  via EAX
  +0x11AC [dec 4524]  x1  via EDX
```

## Card B12b-1-32

GOG function: CSWSCreature::GetVisibleListElement  gog 0x006dc370  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=4
Candidate Steam address: 0x005d1750 (size 116, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: OnPanelAdded 0x894320, AddAttackActions 0x568540, GetArmorClassVersus 0x6a5110)
- best decoy (neighbour/other candidate) scores: 0x005d16e0:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006dc300 CSWSCreature::GetCreatureReputation_2 -> 0x005d16e0  gate_class=REVIEW
- GOG-order neighbour after:  0x006dc3f0 CSWSCreature::AIActionRest -> 0x00983c00  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (7 of 26): the card's A-CALLEE credit may come from these, not from callees
  0x00563290 CSWGuiInGameMap::OnPanelAdded -> Steam 0x00894320  in candidate callers: yes
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: yes
  0x006e1590 CSWSCreatureStats::GetArmorClassVersus -> Steam 0x006a5110  in candidate callers: yes
  0x00744740 CSWSEffectListHandler::OnApplySanctuary -> Steam 0x005a1e70  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x1028*,0x102c*} | Steam {0x1028*,0x102c*}
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
prologue for 0x005d1750: 16-byte aligned: YES
  005d1750  55             PUSH EBP
  005d1751  8B EC          MOV EBP,ESP
  005d1753  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d1750
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x2  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005d1750 FUN_005d1750 size=116 ret=4 callers=26 callees=0

undefined4 __thiscall FUN_005d1750(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (*(int *)(param_1 + 0x102c) <= local_8) {
      return 0;
    }
    if (**(int **)(*(int *)(param_1 + 0x1028) + local_8 * 4) == param_2) break;
    local_8 = local_8 + 1;
  }
  return *(undefined4 *)(*(int *)(param_1 + 0x1028) + local_8 * 4);
}
```

### decoy: 0x005d16e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d16e0: 16-byte aligned: YES
  005d16e0  55             PUSH EBP
  005d16e1  8B EC          MOV EBP,ESP
  005d16e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d16e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x1028 [dec 4136]  x1  via ECX
  +0x102C [dec 4140]  x1  via EAX
```

## Card B12b-1-33

GOG function: CSWSCreature::CalculateDamagePower  gog 0x00698350  callconv=(blank)  param_size_bytes=4  gog_size=75  gog_retn=4
Candidate Steam address: 0x0057f820 (size 75, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0057f870:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00697f80 CSWSCreature::DoDamage -> 0x0057f450  gate_class=AUTO
- GOG-order neighbour after:  0x006983a0 CSWSCreature::GetDamageFlags -> 0x0057f870  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0069aaf0 CSWSCreature::GetTotalEffectBonus -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006ea340 CSWSCreatureStats::GetDamageRoll -> Steam 0x006adec0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0xff*} | Steam {0x4*,0xc*,0xff*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0057f820: 16-byte aligned: YES
  0057f820  55             PUSH EBP
  0057f821  8B EC          MOV EBP,ESP
  0057f823  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057f820
string literals (0):
direct callees (1):
  0x00582010  CSWSCreature::GetTotalEffectBonus
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0057f820 FUN_0057f820 size=75 ret=4 callers=1 callees=1

undefined4 __thiscall FUN_0057f820(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 local_8;
  
  local_8 = 0;
  uVar1 = FUN_00582010(1,param_2,&local_8,0,0,0,0,0xff,0xff,0,0,param_1,0);
  return uVar1;
}
```

### decoy: 0x0057f870 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057f870: 16-byte aligned: YES
  0057f870  55             PUSH EBP
  0057f871  8B EC          MOV EBP,ESP
  0057f873  83 EC 24       SUB ESP,0x24
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057f870
string literals (0):
direct callees (3):
  0x005908C0  CSWSCombatRound::GetAttack
  0x00591E10  CSWSCombatRound::GetCurrentAttackWeapon
  0x00606BD0  CSWSItem::GetDamageFlags
struct offsets touched (top 3):
  +0x10DC [dec 4316]  x3  via EAX,EDX
  +0x3A [dec 58]  x1  via EAX
  +0xABC [dec 2748]  x1  via EDX
```

## Card B12b-1-34

GOG function: CSWSCreature::GetListenCheckDistance  gog 0x006a3cf0  callconv=(blank)  param_size_bytes=0  gog_size=201  gog_retn=0
Candidate Steam address: 0x0058b210 (size 201, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.83) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0058b140:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3c20 CSWSCreature::GetSpotCheckDistance -> 0x0058b140  gate_class=AUTO
- GOG-order neighbour after:  0x006a3dc0 CSWSCreature::GetLastInstantEquip -> 0x0058b2e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00621110 C2DA::GetFLOATEntry_2 -> Steam ?  in candidate callees: n/a
  0x0064bb20 CServerExoApp::GetCreatureByGameObjectID -> Steam 0x0051c100  in candidate callees: yes
  0x0064c310 CServerExoApp::GetPlayerCreatureId -> Steam 0x0051c8f0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 7): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'SecondaryRange'
disp sets (* = in both, 5 common): GOG {0x8*,0xbc*,0x130*,0x520*,0x1020*} | Steam {0x8*,0xbc*,0x130*,0x520*,0x1020*}
imm sets (* = in both, 2 common): GOG {0x12*,0x20*} | Steam {0x12*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058b210: 16-byte aligned: YES
  0058b210  55             PUSH EBP
  0058b211  8B EC          MOV EBP,ESP
  0058b213  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0058b210
string literals (1):
  0x00995908  at 0058b277  'SecondaryRange'
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0xBC [dec 188]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x1020 [dec 4128]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b210 FUN_0058b210 size=201 ret=0 callers=7 callees=5

float10 __fastcall FUN_0058b210(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_1c [8];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972f58;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = *(float *)(param_1 + 0x1020);
  if (*(int *)(param_1 + 0x520) != 0) {
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar2 = FUN_0051c8f0(uVar1);
    iVar3 = FUN_0051c100(uVar2);
    if (iVar3 != param_1) {
      FUN_00733570("SecondaryRange");
      local_8 = 0;
      iVar3 = *(int *)(DAT_00a1b4d0 + 0x130);
      uVar2 = *(undefined4 *)(iVar3 + 0xbc);
      FUN_0071b670(0x12,local_1c,&local_14);
      local_8 = 0xffffffff;
      FUN_00733780(uVar1,param_1,uVar2,iVar3,uVar4);
    }
  }
  ExceptionList = local_10;
  return (float10)local_14;
}
```

### decoy: 0x0058b140 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b140: 16-byte aligned: YES
  0058b140  55             PUSH EBP
  0058b141  8B EC          MOV EBP,ESP
  0058b143  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0058b140
string literals (1):
  0x00995574  at 0058b1a7  'PrimaryRange'
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0xBC [dec 188]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x101C [dec 4124]  x1  via EAX
```

## Card B12b-1-35

GOG function: CSWSCreature::AddChemical  gog 0x0069a160  callconv=(blank)  param_size_bytes=8  gog_size=223  gog_retn=8
Candidate Steam address: 0x00581710 (size 223, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::AddComponent | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.36 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00581630:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0069a100 CSWSCreature::SetComponent -> 0x005815d0  gate_class=AUTO
- GOG-order neighbour after:  0x0069a240 CSWSCreature::AddComponent -> 0x00581710  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x00549c20 CSWCCMessageData::Constructor -> Steam ?  in candidate callees: n/a
  0x0069a0b0 CSWSCreature::GetComponent -> Steam ?  in candidate callees: n/a
  0x0069a100 CSWSCreature::SetComponent -> Steam ?  in candidate callees: n/a
  0x0069d880 CSWSCreature::SendFeedbackMessage -> Steam ?  in candidate callees: n/a
  0x0077ab20 CSWCCMessageData::SetInteger -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x006688c0 CItemRepository::AddItem -> Steam 0x00605090  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 5 common): GOG {0x4*,0x8*,0x1c*,0x34*,0xe9,0x3b9ac9ff*} | Steam {0x4*,0x8*,0x1c*,0x34*,0xea,0x3b9ac9ff*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00581710: 16-byte aligned: YES
  00581710  55             PUSH EBP
  00581711  8B EC          MOV EBP,ESP
  00581713  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00581710
string literals (0):
direct callees (6):
  0x005817F0  CSWSCreature::GetChemical
  0x00581840  CSWSCreature::SetChemical
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00581710 FUN_00581710 size=223 ret=8 callers=1 callees=6

void FUN_00581710(int param_1,int param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  undefined4 local_2c;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961a8b;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = FUN_005817f0(uVar1);
  local_14 = local_14 + param_1;
  if (999999999 < local_14) {
    local_14 = 999999999;
  }
  iVar2 = FUN_005817f0(uVar1);
  FUN_00581840(local_14);
  if ((param_2 == 1) && (local_14 - iVar2 != 0)) {
    iVar3 = FUN_00919723(0x34);
    local_8 = 0;
    if (iVar3 == 0) {
      local_2c = 0;
    }
    else {
      local_2c = FUN_005e2540();
    }
    local_8 = 0xffffffff;
    FUN_005e1bd0(0,local_14 - iVar2);
    FUN_00584da0(0xea,local_2c);
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00581630 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00581630: 16-byte aligned: YES
  00581630  55             PUSH EBP
  00581631  8B EC          MOV EBP,ESP
  00581633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00581630
string literals (0):
direct callees (6):
  0x00581580  CSWSCreature::GetComponent
  0x005815D0  CSWSCreature::SetComponent
  0x00919723  operator new
  0x005E2540  (unnamed)
  0x005E1BD0  (unnamed)
  0x00584DA0  CSWSCreature::SendFeedbackMessage
struct offsets touched (top 0):
```

## Card B12b-1-36

GOG function: CSWSCreature::SetAttemptedAttackTarget  gog 0x00696f00  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x0057e3d0 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: ClearAllActions 0x51cb10, AIActionPhysicalAttacks 0x6d7390, InitializeNewAttackTarget 0x6dac70)
- best decoy (neighbour/other candidate) scores: 0x0057e350:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00696e80 CSWSCreature::Rest_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00696f20 CSWSCreature::GetItemCount -> 0x0057e3f0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (7 of 11): the card's A-CALLEE credit may come from these, not from callees
  0x0064c530 CServerExoApp::ClearAllActions -> Steam 0x0051cb10  in candidate callers: yes
  0x00653640 CSWSCreature::AIActionPhysicalAttacks -> Steam 0x006d7390  in candidate callers: yes
  0x00656f20 CSWSCreature::InitializeNewAttackTarget -> Steam 0x006dac70  in candidate callers: yes
  0x00681170 CSWSCreature::AddAttackActions -> Steam 0x00568540  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x550*} | Steam {0x550*}
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
prologue for 0x0057e3d0: 16-byte aligned: YES
  0057e3d0  55             PUSH EBP
  0057e3d1  8B EC          MOV EBP,ESP
  0057e3d3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0057e3d0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x550 [dec 1360]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0057e3d0 FUN_0057e3d0 size=25 ret=4 callers=11 callees=0

void __thiscall FUN_0057e3d0(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x550) = param_2;
  return;
}
```

### decoy: 0x0057e350 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0057e350: 16-byte aligned: YES
  0057e350  55             PUSH EBP
  0057e351  8B EC          MOV EBP,ESP
  0057e353  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0057e350
string literals (0):
direct callees (1):
  0x006D5200  (unnamed)
struct offsets touched (top 4):
  +0xC [dec 12]  x1  via EDX
  +0x1B [dec 27]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x53C [dec 1340]  x1  via EAX
```

## Card B12b-1-37

GOG function: CSWSCreature::GetCurrentlyFollowingLeader  gog 0x006a3810  callconv=(blank)  param_size_bytes=0  gog_size=80  gog_retn=0
Candidate Steam address: 0x0058ad30 (size 80, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0058ac50:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3730 CSWSCreature::RemoveAction -> 0x0058ac50  gate_class=AUTO
- GOG-order neighbour after:  0x006a3860 CSWSCreature::GetForceItemMask -> 0x0058ad80  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x007f3870 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0xc*,0x3d*,0x100*} | Steam {0xc*,0x3d*,0x100*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058ad30: 16-byte aligned: YES
  0058ad30  55             PUSH EBP
  0058ad31  8B EC          MOV EBP,ESP
  0058ad33  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058ad30
string literals (0):
direct callees (2):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0058ad30 FUN_0058ad30 size=80 ret=0 callers=1 callees=2

undefined4 __fastcall FUN_0058ad30(undefined4 param_1)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = FUN_007a1720(param_1);
  if (((iVar1 != 0) && (piVar2 = (int *)FUN_00561430(iVar1), piVar2 != (int *)0x0)) &&
     (*piVar2 == 0x3d)) {
    return 1;
  }
  return 0;
}
```

### decoy: 0x0058ac50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058ac50: 16-byte aligned: YES
  0058ac50  55             PUSH EBP
  0058ac51  8B EC          MOV EBP,ESP
  0058ac53  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058ac50
string literals (0):
direct callees (6):
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0064DDE0  (unnamed)
  0x0053E740  Action-list free params
  0x009196FD  operator delete
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x74 [dec 116]  x1  via EDX
```

## Card B12b-1-38

GOG function: CSWSCreature::AdjustReputation  gog 0x006dba10  callconv=(blank)  param_size_bytes=8  gog_size=178  gog_retn=8
Candidate Steam address: 0x005d0df0 (size 178, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x006db8d0:0.91>0.67
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ExecuteCommandAdjustReputation 0x66d1b0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005d0d00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006db920 CSWSCreature::GetCreatureReputation -> 0x005d0d00  gate_class=AUTO
- GOG-order neighbour after:  0x006dbad0 CSWSCreature::SetVisibilityList -> 0x005d0eb0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x007ef060 CFactionManager::SetNPCFactionReputation -> Steam ?  in candidate callees: n/a
  0x007ef110 CFactionManager::GetNPCFactionReputation -> Steam ?  in candidate callees: n/a
  0x007ef1e0 CFactionManager::GetIsNPCFaction -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00782790 Global::ExecuteCommandAdjustReputation -> Steam 0x0066d1b0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x78*,0x1198*} | Steam {0x4*,0x8*,0x78*,0x1198*}
imm sets (* = in both, 2 common): GOG {0x8*,0x2c*} | Steam {0x8*,0x2c*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x005d0df0: 16-byte aligned: YES
  005d0df0  55             PUSH EBP
  005d0df1  8B EC          MOV EBP,ESP
  005d0df3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d0df0
string literals (0):
direct callees (3):
  0x00664650  CFactionManager::GetIsNPCFaction(id) (1..N)
  0x00664580  CFactionManager::GetNPCFactionReputation(a,b) = data[b*N+a-1]
  0x006644D0  CFactionManager::SetNPCFactionReputation(row,col,v) = data[row*N+col-1]
struct offsets touched (top 5):
  +0x78 [dec 120]  x3  via EAX,EDX
  +0x1198 [dec 4504]  x3  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0x8 [dec 8]  x1  via EAX
  +0x10054 [dec 65620]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 005d0df0 FUN_005d0df0 size=178 ret=8 callers=1 callees=3

void __thiscall FUN_005d0df0(int param_1,int param_2,int param_3)

{
  int iVar1;
  
  iVar1 = FUN_00664650(param_2);
  if ((iVar1 != 0) &&
     (iVar1 = FUN_00664580(*(undefined4 *)(*(int *)(param_1 + 0x1198) + 0x78),param_2),
     *(int *)(*(int *)(param_1 + 0x1198) + 0x78) != param_2)) {
    FUN_006644d0(*(undefined4 *)(*(int *)(param_1 + 0x1198) + 0x78),param_2,iVar1 + param_3);
  }
  return;
}
```

### decoy: 0x005d0d00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d0d00: 16-byte aligned: YES
  005d0d00  55             PUSH EBP
  005d0d01  8B EC          MOV EBP,ESP
  005d0d03  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d0d00
string literals (0):
direct callees (2):
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00664580  CFactionManager::GetNPCFactionReputation(a,b) = data[b*N+a-1]
struct offsets touched (top 6):
  +0x11AC [dec 4524]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x78 [dec 120]  x1  via EDX
  +0x1198 [dec 4504]  x1  via EAX
  +0x10054 [dec 65620]  x1  via EDX
```

## Card B12b-1-39

GOG function: CSWSCreature::AddToVisibleList  gog 0x006dc020  callconv=(blank)  param_size_bytes=17  gog_size=269  gog_retn=20
Candidate Steam address: 0x005d1400 (size 269, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.33) | G8 mac_rotation: rotation:0x006dbad0:0.98>0.62
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: ReadStatsFromGff 0x6afed0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005d1510:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006dbb50 CSWSCreature::UpdateVisibleList -> 0x005c3270  gate_class=REJECT
- GOG-order neighbour after:  0x006dc130 CSWSCreature::ClearVisibleList -> 0x005d1510  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x004c7640 CExoArrayList::Add -> Steam ?  in candidate callees: n/a
  0x006dc300 CSWSCreature::GetCreatureReputation_2 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 6): the card's A-CALLEE credit may come from these, not from callees
  0x006ec350 CSWSCreatureStats::ReadStatsFromGff -> Steam 0x006afed0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x8*,0x14*,0xef*,0xf3*,0xfd*,0xfe*,0x1028*,0x7f000000*} | Steam {0x2*,0x3*,0x4*,0x8*,0x14*,0xef*,0xf3*,0xfd*,0xfe*,0x1028*,0x7f000000*}

### abi
```
GOG param bytes: 17; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(17/4)*4 = 20 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 17 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x005d1400: 16-byte aligned: YES
  005d1400  55             PUSH EBP
  005d1401  8B EC          MOV EBP,ESP
  005d1403  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005d1400
string literals (0):
direct callees (3):
  0x005D16E0  CSWSCreature::GetCreatureReputation_2
  0x00919723  operator new
  0x0083EA60  CExoArrayList<uint32>::Add
struct offsets touched (top 1):
  +0x4 [dec 4]  x16  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005d1400 FUN_005d1400 size=269 ret=20 callers=6 callees=3

void FUN_005d1400(undefined4 param_1,byte param_2,byte param_3,byte param_4,byte param_5)

{
  int iVar1;
  undefined4 *local_18;
  
  iVar1 = FUN_005d16e0(param_1);
  if (iVar1 == 0) {
    local_18 = (undefined4 *)FUN_00919723(8);
    if (local_18 == (undefined4 *)0x0) {
      local_18 = (undefined4 *)0x0;
    }
    else {
      *local_18 = 0x7f000000;
      *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xfe;
      *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xfd;
      *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xef;
      *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xf3;
    }
    *local_18 = param_1;
    *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xfe | param_2 & 1;
    *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xfd | (param_3 & 1) << 1;
    *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xf3 | (param_4 & 3) << 2;
    *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) = *(byte *)(local_18 + 0x1 [dec 1] {undefined4*: byte +0x4}) & 0xef | (param_5 & 1) << 4;
    FUN_0083ea60(local_18);
  }
  return;
}
```

### decoy: 0x005d1510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005d1510: 16-byte aligned: YES
  005d1510  55             PUSH EBP
  005d1511  8B EC          MOV EBP,ESP
  005d1513  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005d1510
string literals (0):
direct callees (2):
  0x0049B840  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 3):
  +0x1028 [dec 4136]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x102C [dec 4140]  x1  via EAX
```

## Card B12b-1-40

GOG function: CSWSCreature::GetClientCreature  gog 0x006a3970  callconv=__thiscall  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0058ae90 (size 53, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 12 (callees: GetClientObject 0x54a840; callers: CheckFilter 0x8a8df0, RemovePuppet 0x5f8c60, KillNPCObject 0x5fd9a0)
- best decoy (neighbour/other candidate) scores: 0x0058ad80:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3860 CSWSCreature::GetForceItemMask -> 0x0058ad80  gate_class=AUTO
- GOG-order neighbour after:  0x006a39b0 CSWSCreature::AddPazaakCard -> 0x0058aed0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: yes
GOG callers mapped through trusted rows (11 of 34): the card's A-CALLEE credit may come from these, not from callees
  0x0059cc40 CSWGuiInGameInventory::CheckFilter -> Steam 0x008a8df0  in candidate callers: yes
  0x006fe7d0 CSWPartyTable::RemovePuppet -> Steam 0x005f8c60  in candidate callers: yes
  0x00703350 CSWPartyTable::KillNPCObject -> Steam 0x005fd9a0  in candidate callers: yes
  0x007034a0 CSWPartyTable::KillPUPObject -> Steam 0x005fdaf0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2c*} | Steam {0x2c*}
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
prologue for 0x0058ae90: 16-byte aligned: YES
  0058ae90  55             PUSH EBP
  0058ae91  8B EC          MOV EBP,ESP
  0058ae93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0058ae90
string literals (0):
direct callees (1):
  0x0054A840  CSWSObject::GetClientObject
struct offsets touched (top 1):
  +0x2C [dec 44]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0058ae90 FUN_0058ae90 size=53 ret=0 callers=34 callees=1

undefined4 FUN_0058ae90(void)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  undefined4 extraout_var;
  
  iVar1 = FUN_0054a840();
  if (iVar1 == 0) {
    uVar3 = 0;
  }
  else {
    uVar3 = extraout_var;
    piVar2 = (int *)FUN_0054a840();
    uVar3 = (**(code **)(*piVar2 + 0x2c))(piVar2,uVar3);
  }
  return uVar3;
}
```

### decoy: 0x0058ad80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058ad80: 16-byte aligned: YES
  0058ad80  55             PUSH EBP
  0058ad81  8B EC          MOV EBP,ESP
  0058ad83  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058ad80
string literals (0):
direct callees (2):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D6E30  CSWItem::GetBaseItem
struct offsets touched (top 2):
  +0xAC [dec 172]  x1  via ECX
  +0x1150 [dec 4432]  x1  via ECX
```

## Card B12b-1-41

GOG function: CSWSCreature::GetClientCreature  gog 0x006a3970  callconv=__thiscall  param_size_bytes=0  gog_size=53  gog_retn=0
Candidate Steam address: 0x0058b140 (size 201, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWSCreature::GetSpotCheckDistance+name_held_elsewhere=0x0058ae90 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0058b090:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a3860 CSWSCreature::GetForceItemMask -> 0x0058ad80  gate_class=AUTO
- GOG-order neighbour after:  0x006a39b0 CSWSCreature::AddPazaakCard -> 0x0058aed0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x006b1520 CSWSObject::GetClientObject -> Steam 0x0054a840  in candidate callees: no
GOG callers mapped through trusted rows (11 of 34): the card's A-CALLEE credit may come from these, not from callees
  0x0059cc40 CSWGuiInGameInventory::CheckFilter -> Steam 0x008a8df0  in candidate callers: no
  0x006fe7d0 CSWPartyTable::RemovePuppet -> Steam 0x005f8c60  in candidate callers: no
  0x00703350 CSWPartyTable::KillNPCObject -> Steam 0x005fd9a0  in candidate callers: no
  0x007034a0 CSWPartyTable::KillPUPObject -> Steam 0x005fdaf0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x2c} | Steam {0x8,0xbc,0x130,0x520,0x101c}
imm sets (* = in both, 0 common): GOG {0x8} | Steam {0x12,0x20}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0058b140: 16-byte aligned: YES
  0058b140  55             PUSH EBP
  0058b141  8B EC          MOV EBP,ESP
  0058b143  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0058b140
string literals (1):
  0x00995574  at 0058b1a7  'PrimaryRange'
direct callees (5):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00733570  CExoString::CExoString_char
  0x0071B670  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 5):
  +0x8 [dec 8]  x1  via EAX
  +0xBC [dec 188]  x1  via ECX
  +0x130 [dec 304]  x1  via EDX
  +0x520 [dec 1312]  x1  via ECX
  +0x101C [dec 4124]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0058b140 FUN_0058b140 size=201 ret=0 callers=8 callees=5

float10 __fastcall FUN_0058b140(int param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined1 local_1c [8];
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00972f58;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_14 = *(float *)(param_1 + 0x101c);
  if (*(int *)(param_1 + 0x520) != 0) {
    uVar4 = *(undefined4 *)(DAT_00a1b4a4 + 8);
    uVar2 = FUN_0051c8f0(uVar1);
    iVar3 = FUN_0051c100(uVar2);
    if (iVar3 != param_1) {
      FUN_00733570("PrimaryRange");
      local_8 = 0;
      iVar3 = *(int *)(DAT_00a1b4d0 + 0x130);
      uVar2 = *(undefined4 *)(iVar3 + 0xbc);
      FUN_0071b670(0x12,local_1c,&local_14);
      local_8 = 0xffffffff;
      FUN_00733780(uVar1,param_1,uVar2,iVar3,uVar4);
    }
  }
  ExceptionList = local_10;
  return (float10)local_14;
}
```

### decoy: 0x0058b090 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0058b090: 16-byte aligned: YES
  0058b090  55             PUSH EBP
  0058b091  8B EC          MOV EBP,ESP
  0058b093  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0058b090
string literals (0):
direct callees (1):
  0x0051C8D0  CTwoDimArrays::GetSwoopUpgradeRowProvider
struct offsets touched (top 3):
  +0x2CC [dec 716]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via ECX
  +0x2A4 [dec 676]  x1  via EAX
```

## Card B12b-1-42

GOG function: CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToPathWaitDest  gog 0x006d5e20  callconv=(blank)  param_size_bytes=8  gog_size=286  gog_retn=16
Candidate Steam address: 0x005bd1d0 (size 519, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CanEquip_WeaponHands? | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.15 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0066b8a0:STR0/CALLEE0/SIG0.204/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x006d5cf0 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_ToOffsetWaitDest -> 0x005bd1d0  gate_class=REVIEW
- GOG-order neighbour after:  0x006d5f50 CSWSCreature::WalkUpdateLocation_QuickWalk_FollowLeader_TeleportForward -> 0x005bd1d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00413720 Vector::operator- -> Steam ?  in candidate callees: n/a
  0x00413780 Vector::Dot -> Steam ?  in candidate callees: n/a
  0x0047b610 CSWPartyFollowDataArray::GetPathData -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0x24,0x34*,0xfa4} | Steam {0xc,0x1a,0x34*,0x1150}
imm sets (* = in both, 2 common): GOG {0x2*,0x5,0x8,0xc,0x10*,0x18,0x28,0x5c,0x64} | Steam {0x2*,0x3,0x10*,0x20,0x40}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: FALSE (GOG 8 bytes -> x86 slots 8; ASM RET 16)
```

### prologue
```
prologue for 0x005bd1d0: 16-byte aligned: YES
  005bd1d0  55             PUSH EBP
  005bd1d1  8B EC          MOV EBP,ESP
  005bd1d3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005bd1d0
string literals (0):
direct callees (4):
  0x006D0620  CSWInventory::GetItemInSlot
  0x006D5200  (unnamed)
  0x006D5090  (unnamed)
  0x006D50E0  (unnamed)
struct offsets touched (top 4):
  +0x1A [dec 26]  x4  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,ECX,EDX
  +0x34 [dec 52]  x3  via EAX,ECX,EDX
  +0x1150 [dec 4432]  x2  via ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 005bd1d0 FUN_005bd1d0 size=519 ret=16 callers=1 callees=4

undefined4 FUN_005bd1d0(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int local_8;
  
  local_8 = 0;
  if ((*param_2 == 0x10) || (*param_2 == 0x20)) {
    iVar1 = FUN_006d0620(0x10);
    iVar2 = FUN_006d0620(0x20);
    if (iVar1 != 0) {
      local_8 = FUN_006d5200(*(undefined4 *)(iVar1 + 0xc));
    }
    if (iVar2 != 0) {
      FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
    }
    if (param_1 == 0) {
      return 0;
    }
    iVar3 = FUN_006d5200(*(undefined4 *)(param_1 + 0xc));
    if ((iVar1 == 0) && (iVar2 == 0)) {
      if (*param_2 == 0x20) {
        *param_2 = 0x10;
      }
      return 1;
    }
    if ((iVar1 != 0) && (iVar2 == 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') != (*(char *)(iVar3 + 0x1a) != '\0')) {
        *param_2 = 0x10;
        return 2;
      }
      iVar1 = FUN_006d50e0();
      if (iVar1 != 1) {
        *param_2 = 0x10;
        return 2;
      }
      if (*param_2 == 0x20) {
        return 1;
      }
      return 2;
    }
    if ((iVar1 != 0) && (iVar2 != 0)) {
      iVar1 = FUN_006d5090();
      if (iVar1 == 1) {
        *param_2 = 0x10;
        return 3;
      }
      if ((*(char *)(local_8 + 0x1a) != '\0') == (*(char *)(iVar3 + 0x1a) != '\0')) {
        return 2;
      }
      *param_2 = 0x10;
      return 3;
    }
  }
  return 0;
}
```

### decoy: 0x0066b8a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0066b8a0: 16-byte aligned: YES
  0066b8a0  55             PUSH EBP
  0066b8a1  8B EC          MOV EBP,ESP
  0066b8a3  83 EC 48       SUB ESP,0x48
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0066b8a0
string literals (0):
direct callees (11):
  0x006FDAF0  CVirtualMachine::StackPopObject
  0x0051C0A0  CServerExoApp::GetGameObject
  0x0058AE90  CSWSCreature::GetClientCreature
  0x007680A0  (unnamed)
  0x007ED830  CSWCObject::GetAnimationBase
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x007FE6C0  CSWCItem::PowerItem
  0x0057A560  CSWSCreature::SetCombatState
  0x00740AA0  CClientExoApp::GetPlayerMovementInput
  0x007EFC40  CSWCObject::InsertFireForgetAnimation
struct offsets touched (top 10):
  +0x34 [dec 52]  x4  via EAX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x464 [dec 1124]  x2  via EAX,ECX
  +0x30 [dec 48]  x1  via EDX
  +0x44 [dec 68]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
  +0x9C [dec 156]  x1  via EAX
  +0xC5 [dec 197]  x1  via EDX
  +0xE0 [dec 224]  x1  via EDX
```

## Card B12b-1-43

GOG function: CSWSCreature::ComputeArmourClass  gog 0x006a26d0  callconv=(blank)  param_size_bytes=0  gog_size=58  gog_retn=0
Candidate Steam address: 0x00589bf0 (size 58, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: GetSpellActionPending 0x590180, RemoveSpellAction 0x591a70)
- best decoy (neighbour/other candidate) scores: 0x00589b20:STR0/CALLEE0/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006a2600 CSWSCreature::UpdatePureGoodEvilPowers -> 0x00589b20  gate_class=REVIEW
- GOG-order neighbour after:  0x006a2710 CSWSCreature::ProcessPendingCombatActions -> 0x00589c30  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00774830 CSWSCombatRound::GetSpellActionPending -> Steam 0x00590180  in candidate callees: yes
  0x00776120 CSWSCombatRound::RemoveSpellAction -> Steam 0x00591a70  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10dc*} | Steam {0x10dc*}
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
prologue for 0x00589bf0: 16-byte aligned: YES
  00589bf0  55             PUSH EBP
  00589bf1  8B EC          MOV EBP,ESP
  00589bf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00589bf0
string literals (0):
direct callees (2):
  0x00590180  CSWSCombatRound::GetSpellActionPending
  0x00591A70  CSWSCombatRound::RemoveSpellAction
struct offsets touched (top 1):
  +0x10DC [dec 4316]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00589bf0 FUN_00589bf0 size=58 ret=0 callers=2 callees=2

void __fastcall FUN_00589bf0(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  uVar2 = *(undefined4 *)(param_1 + 0x10dc);
  iVar1 = FUN_00590180();
  if (iVar1 == 1) {
    FUN_00591a70(param_1,*(undefined4 *)(param_1 + 0x10dc),uVar2);
  }
  return;
}
```

### decoy: 0x00589b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00589b20: 16-byte aligned: YES
  00589b20  55             PUSH EBP
  00589b21  8B EC          MOV EBP,ESP
  00589b23  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00589b20
string literals (0):
direct callees (3):
  0x00591510  (unnamed)
  0x00591450  (unnamed)
  0x005D7220  CSWSObject::GetAQActionIDByID
struct offsets touched (top 1):
  +0x10DC [dec 4316]  x2  via EDX
```
