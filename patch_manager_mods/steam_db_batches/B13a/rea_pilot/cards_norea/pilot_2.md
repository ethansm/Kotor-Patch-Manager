# Review cards B13a R1 pilot part 2

Follow steam_db_batches/REVIEWER_BRIEF.md. One TSV row per card, card order, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | label_suspect | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence.

## Card B13a-1-41

GOG function: CSWCCreature::GetHostileItems_2  gog 0x00428690  callconv=(blank)  param_size_bytes=8  gog_size=107  gog_retn=8
Candidate Steam address: 0x0077c780 (size 1271, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::PartyMenuUseItemSelect | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077c710:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00428620 CSWCCreature::GetHostileItems -> 0x0077c6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00428700 CSWCCreature::PartyMenuUseItemSelect -> 0x0077c780  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004282c0 CSWCCreature::CreateHostileItemEntry_2 -> Steam ?  in candidate callees: n/a
  0x00429780 CSWCCreature::GetServerCreature -> Steam ?  in candidate callees: n/a
  0x00669930 CItemRepository::ItemListGetItem -> Steam 0x00606100  in candidate callees: no
  0x006a2da0 CSWSCreature::GetItemRepository -> Steam 0x0058a2c0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x10} | Steam {0x4,0x8,0x98,0xac}
imm sets (* = in both, 2 common): GOG {0x8*,0x14*} | Steam {0x8*,0x14*,0x24,0x2f,0xe4,0xbfffffff}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077c780: 16-byte aligned: YES
  0077c780  55             PUSH EBP
  0077c781  8B EC          MOV EBP,ESP
  0077c783  81 EC E4 00 00 00 SUB ESP,0xe4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077c780
string literals (0):
direct callees (10):
  0x0051C8B0  CServerExoApp::ClientToServerObjectId
  0x0051C0E0  CServerExoApp::GetItemByGameObjectID
  0x006D6E30  CSWItem::GetBaseItem
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005D2F80  CSWSCreature::CanUseItem
  0x0077CC80  CSWCCreature::PartyMenuUseItem
  0x0073F750  ClientApp::GetGuiInGame
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x00775600  CSWCCreature::PerformUseItem
struct offsets touched (top 7):
  +0x4 [dec 4]  x15  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0x98 [dec 152]  x6  via EAX,ECX,EDX
  +0x15230 [dec 86576]  x3  via EAX,ECX,EDX
  +0xAC [dec 172]  x2  via ECX,EDX
  +0x15234 [dec 86580]  x2  via EAX,ECX
  +0x15238 [dec 86584]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077c780 FUN_0077c780 size=1271 ret=8 callers=1 callees=10

void __thiscall FUN_0077c780(int param_1,uint param_2,undefined4 param_3)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  undefined4 uVar5;
  
  if (DAT_00a7ead4 == '\0') {
    iVar2 = FUN_0073f750();
    iVar2 = *(int *)(*(int *)(iVar2 + 0x98) + 0x15234);
    param_2 = param_2 & 0xbfffffff;
    uVar1 = FUN_0051c8b0(param_2);
    piVar4 = (int *)FUN_0073fb90();
    if (*piVar4 != 1) {
      FUN_0051c0e0(uVar1);
      iVar3 = FUN_006d6e30();
      if (*(char *)(iVar3 + 0xac) != '/') {
        iVar3 = FUN_0073f750();
        if (*(int *)(*(int *)(iVar3 + 0x98) + 0x15230) == 0) {
          FUN_0051c0e0(uVar1);
          iVar2 = FUN_006d6e30();
          if (*(char *)(iVar2 + 0xac) == '\x14') {
            FUN_00775600(param_2,0,*(undefined4 *)(param_1 + 4),param_1 + 0x24);
          }
          else {
            iVar2 = FUN_0073f750();
            *(uint *)(*(int *)(iVar2 + 0x98) + 0x15238) = param_2;
            iVar2 = FUN_0073f750();
            *(undefined4 *)(*(int *)(iVar2 + 0x98) + 0x15230) = 1;
          }
        }
        else {
          if (iVar2 == -1) {
            return;
          }
          iVar3 = iVar2;
          FUN_0073fb90(iVar2);
          iVar3 = FUN_007e5da0(iVar3);
          uVar5 = *(undefined4 *)(iVar3 + 4);
          iVar3 = FUN_0073f750();
          *(undefined4 *)(*(int *)(iVar3 + 0x98) + 0x15230) = 0;
          iVar3 = iVar2;
          FUN_0073fb90(iVar2);
          FUN_007e5da0(iVar3);
          FUN_0077d800();
          uVar1 = FUN_0051c0e0(uVar1);
          iVar3 = FUN_005d2f80(uVar1,0,1);
          if (iVar3 != 0) {
            FUN_0073fb90(iVar2);
            iVar2 = FUN_007e5da0(iVar2);
            FUN_00775600(param_2,0,uVar5,iVar2 + 0x24);
          }
        }
        iVar2 = FUN_0073f750();
        *(undefined4 *)(*(int *)(iVar2 + 0x98) + 0x15234) = 0xffffffff;
        return;
      }
    }
    if (iVar2 == -1) {
      uVar5 = 0;
      FUN_0073fb90(0);
      FUN_007e5da0(uVar5);
    }
    else {
      FUN_0073fb90(iVar2);
      FUN_007e5da0(iVar2);
    }
    FUN_0077d800();
    uVar1 = FUN_0051c0e0(uVar1);
    iVar2 = FUN_005d2f80(uVar1,0,1);
    if (iVar2 != 0) {
      FUN_00775600(param_2,0,*(undefined4 *)(param_1 + 4),param_1 + 0x24);
    }
  }
  else {
    uVar1 = FUN_0051c8b0(param_2 & 0xbfffffff);
    iVar2 = FUN_0051c0e0(uVar1);
/* ... 10 more lines truncated by --max-lines */
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

## Card B13a-1-49

GOG function: CSWCCreature::`scalar  gog 0x00410e50  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00765110 (size 61, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00765180:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00410e20 CSWCCreature::IsHostile -> 0x00765150  gate_class=AUTO
- GOG-order neighbour after:  0x00410e80 CSWCCreature::Destructor -> 0x007651b0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00410e80 CSWCCreature::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x310}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x14}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00765110: 16-byte aligned: YES
  00765110  55             PUSH EBP
  00765111  8B EC          MOV EBP,ESP
  00765113  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00765110
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 1):
  +0x310 [dec 784]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00765110 FUN_00765110 size=61 ret=4 callers=0 callees=1

undefined4 __thiscall FUN_00765110(int param_1,undefined4 param_2)

{
  FUN_007335f0(*(int *)(param_1 + 0x310) + 0x14);
  return param_2;
}
```

### decoy: 0x00765180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00765180: 16-byte aligned: YES
  00765180  55             PUSH EBP
  00765181  8B EC          MOV EBP,ESP
  00765183  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00765180
string literals (0):
direct callees (2):
  0x007651B0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
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

## Card B13a-1-55

GOG function: CSWCCreature::GetHeadTurnVertical  gog 0x004200d0  callconv=(blank)  param_size_bytes=0  gog_size=93  gog_retn=0
Candidate Steam address: 0x00774260 (size 458, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.42) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.18 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00774200:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00420070 CSWCCreature::GetHeadTurnHorizontal -> 0x00774200  gate_class=REVIEW
- GOG-order neighbour after:  0x00420130 CSWCCreature::ApplyShadowBlob -> 0x00774260  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x10*,0x18*,0x130*,0x224*} | Steam {0x4,0x8,0x10*,0x18*,0x68,0xa4,0xb4,0xd4,0x108,0x130*,0x224*}
imm sets (* = in both, 0 common): GOG {0x14} | Steam {0x3,0x4,0xc,0x1c,0x48,0xd8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00774260: 16-byte aligned: YES
  00774260  55             PUSH EBP
  00774261  8B EC          MOV EBP,ESP
  00774263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00774260
string literals (4):
  0x0099F1A4  at 00774328  'Fx_Shadow_S'
  0x0099F198  at 00774337  'Fx_Shadow_M'
  0x0099F18C  at 00774346  'Fx_Shadow_L'
  0x0099F198  at 00774355  'Fx_Shadow_M'
direct callees (5):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x0071B8B0  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x005160C0  (unnamed)
struct offsets touched (top 10):
  +0x68 [dec 104]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via ECX,EDX
  +0x10 [dec 16]  x1  via EAX
  +0x18 [dec 24]  x1  via EDX
  +0xA4 [dec 164]  x1  via EDX
  +0xB4 [dec 180]  x1  via EDX
  +0xD4 [dec 212]  x1  via ECX
  +0x108 [dec 264]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00774260 FUN_00774260 size=458 ret=0 callers=0 callees=5

undefined4 __fastcall FUN_00774260(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_40 [16];
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [8];
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974ab8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (param_1[0x1a = byte +0x68] == 0) {
    local_24 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar2 = local_24;
  }
  else {
    iVar1 = FUN_0071b8b0(*(undefined2 *)(param_1[0x89 = byte +0x224] + 0x18),DAT_00a0fdf4,&local_14);
    if (iVar1 == 0) {
      local_14 = 3;
    }
    switch(local_14) {
    case 1:
    case 2:
      FUN_007338d0("Fx_Shadow_S");
      break;
    case 3:
      FUN_007338d0("Fx_Shadow_M");
      break;
    case 4:
    case 5:
      FUN_007338d0("Fx_Shadow_L");
      break;
    default:
      FUN_007338d0("Fx_Shadow_M");
    }
    local_20 = (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xa4))(local_1c);
    local_30 = 0;
    local_2c = 0;
    local_28 = 0x3f800000;
    uVar2 = FUN_005160c0(local_40,0,0,0x3f800000,param_1[0x36 = byte +0xd8],param_1[0x37 = byte +0xdc],param_1[0x38 = byte +0xe0]);
    (**(code **)(*(int *)param_1[0x1a = byte +0x68] + 0xb4))(uVar2);
    if (param_1[0x35 = byte +0xd4] == 0) {
      (**(code **)(*param_1 + 0x108))();
    }
    uVar2 = local_20;
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return uVar2;
}
```

### decoy: 0x00774200 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00774200: 16-byte aligned: YES
  00774200  55             PUSH EBP
  00774201  8B EC          MOV EBP,ESP
  00774203  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00774200
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 4):
  +0x10 [dec 16]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x130 [dec 304]  x1  via EAX
  +0x224 [dec 548]  x1  via EAX
```

## Card B13a-1-15

GOG function: CSWCCreature::ForceAppearanceUpdate  gog 0x00413e30  callconv=(blank)  param_size_bytes=0  gog_size=59  gog_retn=0
Candidate Steam address: 0x00768060 (size 59, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: GetLoopingAnimation 0x7ed980)
- best decoy (neighbour/other candidate) scores: 0x00767b20:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004138f0 CSWCCreature::HandleConjuringCreature -> 0x00767b20  gate_class=REVIEW
- GOG-order neighbour after:  0x00413e70 CSWCCreature::UpdateRollingFootstepSound -> 0x007680a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00472ae0 CSWCObject::GetLoopingAnimation -> Steam 0x007ed980  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x9c*} | Steam {0x9c*}
imm sets (* = in both, 1 common): GOG {0x2711*} | Steam {0x2711*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00768060: 16-byte aligned: YES
  00768060  55             PUSH EBP
  00768061  8B EC          MOV EBP,ESP
  00768063  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00768060
string literals (0):
direct callees (1):
  0x007ED980  CSWCObject::GetLoopingAnimation
struct offsets touched (top 1):
  +0x9C [dec 156]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00768060 FUN_00768060 size=59 ret=0 callers=1 callees=1

undefined4 __fastcall FUN_00768060(int *param_1)

{
  short sVar1;
  
  sVar1 = FUN_007ed980();
  if (sVar1 == 0x2711) {
    (**(code **)(*param_1 + 0x9c))(0x2711,0,0);
  }
  return 1;
}
```

### decoy: 0x00767b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00767b20: 16-byte aligned: YES
  00767b20  55             PUSH EBP
  00767b21  8B EC          MOV EBP,ESP
  00767b23  81 EC 9C 00 00 00 SUB ESP,0x9c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00767b20
string literals (0):
direct callees (9):
  0x006C1450  (unnamed)
  0x007107F0  (unnamed)
  0x00710EB0  (unnamed)
  0x00710CA0  (unnamed)
  0x00859CA0  (unnamed)
  0x00859DC0  (unnamed)
  0x00859520  (unnamed)
  0x00859EF0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 10):
  +0x48 [dec 72]  x25  via EAX,ECX,EDX
  +0x308 [dec 776]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x8  via EAX,ECX,EDX
  +0x8 [dec 8]  x8  via EAX,ECX,EDX
  +0xC [dec 12]  x8  via EAX,ECX,EDX
  +0x2D8 [dec 728]  x5  via EAX,ECX,EDX
  +0x20 [dec 32]  x4  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0x1C [dec 28]  x2  via EAX,EDX
  +0x2DC [dec 732]  x2  via ECX
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

## Card B13a-1-05

GOG function: CSWCCreature::ResetAnimationState  gog 0x0041f730  callconv=(blank)  param_size_bytes=4  gog_size=116  gog_retn=0
Candidate Steam address: 0x007737e0 (size 116, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.40) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00773860:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0041f6b0 CSWCCreature::UnloadMonkVisuals -> 0x007737e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f7b0 CSWCCreature::ResetDesiredPositionTimer -> 0x007738e0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00620e90 Global::GetFLOATEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 5 common): GOG {0x10*,0x18*,0x70*,0x130*,0x224*} | Steam {0x10*,0x18*,0x70*,0x130*,0x224*}
imm sets (* = in both, 1 common): GOG {0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x007737e0: 16-byte aligned: YES
  007737e0  55             PUSH EBP
  007737e1  8B EC          MOV EBP,ESP
  007737e3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007737e0
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007737e0 FUN_007737e0 size=116 ret=0 callers=0 callees=1

void FUN_007737e0(int *param_1,undefined4 param_2,undefined4 *param_3)

{
  int iVar1;
  undefined4 local_8;
  
  local_8 = 0;
  iVar1 = (**(code **)(*param_1 + 0x70))();
  if (iVar1 != 0) {
    FUN_0071b3f0(*(undefined2 *)(*(int *)(iVar1 + 0x224) + 0x18),DAT_00a0fde8,&local_8);
    *param_3 = local_8;
  }
  return;
}
```

### decoy: 0x00773860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773860: 16-byte aligned: YES
  00773860  55             PUSH EBP
  00773861  8B EC          MOV EBP,ESP
  00773863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773860
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

## Card B13a-1-31

GOG function: CSWCCreature::GetCameraOffset  gog 0x00421880  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775930 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::DeleteSpellLikeAbilityList | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775950:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
prologue for 0x00775930: 16-byte aligned: YES
  00775930  55             PUSH EBP
  00775931  8B EC          MOV EBP,ESP
  00775933  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775930
string literals (0):
direct callees (1):
  0x0085D260  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775930 FUN_00775930 size=25 ret=0 callers=0 callees=1
THUNK -> 0x0085D260 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775930(undefined4 param_1)

{
  FUN_0085d260(param_1);
  return;
}
```

### decoy: 0x00775950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775950: 16-byte aligned: YES
  00775950  55             PUSH EBP
  00775951  8B EC          MOV EBP,ESP
  00775953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775950
string literals (0):
direct callees (1):
  0x0085D2D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
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

## Card B13a-1-56

GOG function: CSWCCreature::SetThrownLightsaber  gog 0x004213c0  callconv=(blank)  param_size_bytes=0  gog_size=66  gog_retn=0
Candidate Steam address: 0x007754d0 (size 66, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 1 (callers: LoadArea 0x7a2fb0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00775520:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004213a0 CSWSPlaceable::SetEffectSpellId  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00421410 CSWCCreature::UpdatePopUpBubble -> 0x00775520  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x20*,0x108*,0x3b4*} | Steam {0x20*,0x108*,0x3b4*}
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
prologue for 0x007754d0: 16-byte aligned: YES
  007754d0  55             PUSH EBP
  007754d1  8B EC          MOV EBP,ESP
  007754d3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007754d0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x3B4 [dec 948]  x2  via EAX,ECX
  +0x108 [dec 264]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007754d0 FUN_007754d0 size=66 ret=0 callers=2 callees=0

void __fastcall FUN_007754d0(int param_1)

{
  if (*(int *)(param_1 + 0x20) == 0) {
    *(undefined4 *)(param_1 + 0x3b4) = DAT_00994484;
  }
  else {
    *(undefined4 *)(param_1 + 0x3b4) = *(undefined4 *)(*(int *)(param_1 + 0x20) + 0x108);
  }
  return;
}
```

### decoy: 0x00775520 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775520: 16-byte aligned: YES
  00775520  55             PUSH EBP
  00775521  8B EC          MOV EBP,ESP
  00775523  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775520
string literals (0):
direct callees (2):
  0x0073F430  CClientExoApp::GetModule
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via EAX
  +0x48 [dec 72]  x1  via EDX
  +0x108 [dec 264]  x1  via ECX
  +0x3B4 [dec 948]  x1  via EAX
```

## Card B13a-1-54

GOG function: CSWCCreature::SetEngagedState  gog 0x00422e80  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00778510 (size 706, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::CanSee | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: Update 0x7ac470)
- best decoy (neighbour/other candidate) scores: 0x007783b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00422ea0 Global::SetQuededAnimationCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x13c} | Steam {0x4,0x8,0xc,0x14,0x20,0x38,0x4c,0x5c,0x70,0x98,0x140,0x188,0x31c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x8,0xc,0x24,0x50,0xfe,0xff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00778510: 16-byte aligned: YES
  00778510  55             PUSH EBP
  00778511  8B EC          MOV EBP,ESP
  00778513  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00778510
string literals (0):
direct callees (4):
  0x00465650  (unnamed)
  0x004491C0  CExoPtrList::Add (doubling)
  0x007783B0  (unnamed)
  0x0087BE80  CSWCDoor::GetServerDoor
struct offsets touched (top 10):
  +0x98 [dec 152]  x6  via EAX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX
  +0x8 [dec 8]  x4  via EAX,ECX
  +0x14 [dec 20]  x3  via EDX
  +0x4C [dec 76]  x2  via EDX
  +0xC [dec 12]  x1  via ECX
  +0x20 [dec 32]  x1  via EDX
  +0x38 [dec 56]  x1  via ECX
  +0x5C [dec 92]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00778510 FUN_00778510 size=706 ret=4 callers=3 callees=4

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool __thiscall FUN_00778510(int *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  float fVar3;
  int iVar4;
  int iVar5;
  float fVar6;
  float fVar7;
  undefined4 uVar8;
  int iVar9;
  int *piVar10;
  bool bVar11;
  float10 fVar12;
  int local_1c;
  
  if (param_2 == (int *)0x0) {
    bVar11 = false;
  }
  else {
    iVar1 = param_1[9 = byte +0x24];
    iVar2 = param_1[10 = byte +0x28];
    fVar3 = (float)param_1[0xb = byte +0x2c];
    fVar7 = (float)_DAT_00989f10;
    iVar4 = param_2[9 = byte +0x24];
    iVar5 = param_2[10 = byte +0x28];
    fVar6 = (float)param_2[0xb = byte +0x2c];
    fVar12 = (float10)(**(code **)(*param_2 + 0x140))();
    FUN_00465650();
    DAT_00a2015c = 1;
    DAT_00a2010c = 0;
    uVar8 = (**(code **)(*param_1 + 0x98))(0xff,1);
    FUN_004491c0(uVar8);
    iVar9 = (**(code **)(*param_1 + 0x98))(0xfe,1);
    if (iVar9 != 0) {
      FUN_004491c0(iVar9);
    }
    uVar8 = (**(code **)(*param_2 + 0x98))(0xff,1);
    FUN_004491c0(uVar8);
    iVar9 = (**(code **)(*param_2 + 0x98))(0xfe,1);
    if (iVar9 != 0) {
      FUN_004491c0(iVar9);
    }
    bVar11 = true;
    while (bVar11) {
      bVar11 = false;
      FUN_00465650();
      local_1c = FUN_007783b0(*(undefined4 *)(param_1[8 = byte +0x20] + 0x188),iVar1,iVar2,fVar3 + fVar7,iVar4,
                              iVar5,(float)(fVar12 + (float10)fVar6),0,&DAT_00a20100);
      if (local_1c == 2) {
        piVar10 = (int *)(**(code **)(*DAT_00a20138 + 0x70))();
        if (piVar10 == (int *)0x0) {
          local_1c = 1;
        }
        else {
          iVar9 = (**(code **)(*piVar10 + 0x14))();
          if (iVar9 == 0) {
            iVar9 = (**(code **)(*piVar10 + 0x4c))();
            if (iVar9 != 0) {
              piVar10 = (int *)(**(code **)(*piVar10 + 0x4c))();
              uVar8 = (**(code **)(*piVar10 + 0x98))(0xff,1);
              FUN_004491c0(uVar8);
              bVar11 = true;
            }
          }
          else {
            (**(code **)(*piVar10 + 0x14))();
            iVar9 = FUN_0087be80();
            if ((iVar9 == 0) || (*(char *)(iVar9 + 0x31c) != '\0')) {
              piVar10 = (int *)(**(code **)(*piVar10 + 0x14))();
              uVar8 = (**(code **)(*piVar10 + 0x98))(0xff,1);
              FUN_004491c0(uVar8);
              bVar11 = true;
            }
            else {
              local_1c = 1;
/* ... 9 more lines truncated by --max-lines */
```

### decoy: 0x007783b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007783b0: 16-byte aligned: YES
  007783b0  55             PUSH EBP
  007783b1  8B EC          MOV EBP,ESP
  007783b3  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007783b0
string literals (0):
direct callees (6):
  0x0064E080  (unnamed)
  0x0051BD90  CServerExoApp::GetModule
  0x00557EF0  CSWSModule::GetArea
  0x0054B370  CSWSArea::GetRoom_2
  0x0064E0C0  (unnamed)
  0x005FF6A0  (unnamed)
struct offsets touched (top 3):
  +0x8 [dec 8]  x5  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x38 [dec 56]  x1  via EDX
```

## Card B13a-1-33

GOG function: CSWCCreature::UpdateRangedAttackData  gog 0x0041ac10  callconv=(blank)  param_size_bytes=4  gog_size=365  gog_retn=4
Candidate Steam address: 0x0076eda0 (size 365, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0076eb20:STR0/CALLEE0/SIG0.727/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041a990 CSWCCreature::SetLastInstant -> 0x0076eb20  gate_class=AUTO
- GOG-order neighbour after:  0x0041ade0 CSWCCreature::GetLastInstant -> 0x0076ef70  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 15 common): GOG {0x280*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x29c*,0x2a0*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,0x2b4*,...(+1)} | Steam {0x280*,0x284*,0x288*,0x28c*,0x290*,0x294*,0x298*,0x29c*,0x2a0*,0x2a4*,0x2a8*,0x2ac*,0x2b0*,0x2b4*,...(+1)}
imm sets (* = in both, 11 common): GOG {0x4*,0x8*,0x3f*,0x80*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*} | Steam {0x4*,0x8*,0x3f*,0x80*,0x100*,0x200*,0x400*,0x4000*,0x8000*,0x10000*,0x20000*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0076eda0: 16-byte aligned: YES
  0076eda0  55             PUSH EBP
  0076eda1  8B EC          MOV EBP,ESP
  0076eda3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0076eda0
string literals (0):
direct callees (0):
struct offsets touched (top 10):
  +0x280 [dec 640]  x1  via ECX
  +0x284 [dec 644]  x1  via EDX
  +0x288 [dec 648]  x1  via EAX
  +0x28C [dec 652]  x1  via ECX
  +0x290 [dec 656]  x1  via EDX
  +0x294 [dec 660]  x1  via EAX
  +0x298 [dec 664]  x1  via ECX
  +0x29C [dec 668]  x1  via EDX
  +0x2A0 [dec 672]  x1  via EAX
  +0x2A4 [dec 676]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0076eda0 FUN_0076eda0 size=365 ret=4 callers=2 callees=0

undefined4 __thiscall FUN_0076eda0(int param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (param_2 < 0x81) {
    if (param_2 == 0x80) {
      return *(undefined4 *)(param_1 + 0x29c);
    }
    switch(param_2) {
    case 1:
      uVar1 = *(undefined4 *)(param_1 + 0x280);
      break;
    case 2:
      uVar1 = *(undefined4 *)(param_1 + 0x284);
      break;
    default:
      goto switchD_0076ede2_caseD_3;
    case 4:
      uVar1 = *(undefined4 *)(param_1 + 0x288);
      break;
    case 8:
      uVar1 = *(undefined4 *)(param_1 + 0x28c);
      break;
    case 0x10:
      uVar1 = *(undefined4 *)(param_1 + 0x290);
      break;
    case 0x20:
      uVar1 = *(undefined4 *)(param_1 + 0x294);
      break;
    case 0x40:
      uVar1 = *(undefined4 *)(param_1 + 0x298);
    }
  }
  else {
    if (param_2 < 0x4001) {
      if (param_2 == 0x4000) {
        return *(undefined4 *)(param_1 + 0x2ac);
      }
      if (param_2 == 0x100) {
        return *(undefined4 *)(param_1 + 0x2a0);
      }
      if (param_2 == 0x200) {
        return *(undefined4 *)(param_1 + 0x2a4);
      }
      if (param_2 == 0x400) {
        return *(undefined4 *)(param_1 + 0x2a8);
      }
    }
    else {
      if (param_2 == 0x8000) {
        return *(undefined4 *)(param_1 + 0x2b0);
      }
      if (param_2 == 0x10000) {
        return *(undefined4 *)(param_1 + 0x2b4);
      }
      if (param_2 == 0x20000) {
        return *(undefined4 *)(param_1 + 0x2b8);
      }
    }
switchD_0076ede2_caseD_3:
    uVar1 = 0;
  }
  return uVar1;
}
```

### decoy: 0x0076eb20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0076eb20: 16-byte aligned: YES
  0076eb20  55             PUSH EBP
  0076eb21  8B EC          MOV EBP,ESP
  0076eb23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B13a-1-29

GOG function: CSWCCreature::SetAlignmentGoodEvil  gog 0x004291f0  callconv=(blank)  param_size_bytes=2  gog_size=155  gog_retn=4
Candidate Steam address: 0x0077d310 (size 124, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077d270:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004290a0 CSWCCreature::ActionMenuForm -> 0x0077d120  gate_class=AUTO
- GOG-order neighbour after:  0x00429290 CSWCCreature::SetBaseCNPCAlignment -> 0x0077d310  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0048d4c0 CSWCCreatureAppearance::SetAppearanceAlignment -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x80,0x224,0x310*} | Steam {0x82,0x310*}
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
prologue for 0x0077d310: 16-byte aligned: YES
  0077d310  55             PUSH EBP
  0077d311  8B EC          MOV EBP,ESP
  0077d313  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077d310
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x82 [dec 130]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0077d310 FUN_0077d310 size=124 ret=4 callers=1 callees=0

void __thiscall FUN_0077d310(int param_1,short param_2)

{
  short sVar1;
  
  if (*(int *)(param_1 + 0x310) != 0) {
    sVar1 = param_2;
    if (param_2 < -1) {
      sVar1 = -1;
    }
    if (sVar1 < 0x65) {
      if (param_2 < -1) {
        param_2 = -1;
      }
    }
    else {
      param_2 = 100;
    }
    *(short *)(*(int *)(param_1 + 0x310) + 0x82) = param_2;
  }
  return;
}
```

### decoy: 0x0077d270 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077d270: 16-byte aligned: YES
  0077d270  55             PUSH EBP
  0077d271  8B EC          MOV EBP,ESP
  0077d273  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077d270
string literals (0):
direct callees (1):
  0x0085D430  (unnamed)
struct offsets touched (top 3):
  +0x224 [dec 548]  x2  via EAX,ECX
  +0x310 [dec 784]  x2  via EAX,ECX
  +0x80 [dec 128]  x1  via EDX
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

## Card B13a-1-53

GOG function: CSWCCreature::GetCameraOffset  gog 0x00421880  callconv=(blank)  param_size_bytes=0  gog_size=25  gog_retn=0
Candidate Steam address: 0x00775970 (size 25, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetFollowingBody | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775950:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
prologue for 0x00775970: 16-byte aligned: YES
  00775970  55             PUSH EBP
  00775971  8B EC          MOV EBP,ESP
  00775973  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00775970
string literals (0):
direct callees (1):
  0x0085D320  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00775970 FUN_00775970 size=25 ret=0 callers=0 callees=1
THUNK -> 0x0085D320 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00775970(undefined4 param_1)

{
  FUN_0085d320(param_1);
  return;
}
```

### decoy: 0x00775950 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775950: 16-byte aligned: YES
  00775950  55             PUSH EBP
  00775951  8B EC          MOV EBP,ESP
  00775953  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00775950
string literals (0):
direct callees (1):
  0x0085D2D0  (unnamed)
struct offsets touched (top 1):
  +0x224 [dec 548]  x1  via EAX
```

## Card B13a-1-51

GOG function: CSWCCreature::UpdateRollingFootstepSound  gog 0x00413e70  callconv=(blank)  param_size_bytes=4  gog_size=516  gog_retn=8
Candidate Steam address: 0x007680a0 (size 516, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.06) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: PlayerFlourishWeapons 0x79f430, ExecuteCommandCreatureFlourishWeapon 0x66b8a0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007682b0:STR0/CALLEE0/SIG0.171/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00413e30 CSWCCreature::ForceAppearanceUpdate -> 0x00768060  gate_class=REVIEW
- GOG-order neighbour after:  0x00414080 CSWCCreature::SetCombatState -> 0x007682b0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x00472990 CSWCObject::GetAnimationBase -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 9): the card's A-CALLEE credit may come from these, not from callees
  0x0044ba30 CClientExoAppInternal::PlayerFlourishWeapons -> Steam 0x0079f430  in candidate callers: yes
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 6 common): GOG {0x4*,0x4c*,0xc5*,0x138*,0x2e8*,0x3b0*} | Steam {0x4*,0x4c*,0xc5*,0x138*,0x2e8*,0x3b0*}
imm sets (* = in both, 12 common): GOG {0x8*,0x10*,0x20*,0x40*,0x272e*,0x2736*,0x2737*,0x2738*,0x2739*,0x273a*,0x27aa*,0x27ab*} | Steam {0x8*,0x10*,0x20*,0x40*,0x272e*,0x2736*,0x2737*,0x2738*,0x2739*,0x273a*,0x27aa*,0x27ab*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 8)
```

### prologue
```
prologue for 0x007680a0: 16-byte aligned: YES
  007680a0  55             PUSH EBP
  007680a1  8B EC          MOV EBP,ESP
  007680a3  83 EC 40       SUB ESP,0x40
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 007680a0 FUN_007680a0 size=516 ret=8 callers=9 callees=3

undefined4 __thiscall FUN_007680a0(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  int *piVar3;
  
  iVar1 = FUN_007ed830();
  if (iVar1 == 0) {
    uVar2 = 0;
  }
  else if (param_2 == 1) {
    piVar3 = (int *)FUN_007ed830();
    iVar1 = (**(code **)(*piVar3 + 0x4c))(0x272e);
    if (iVar1 != 1) {
      piVar3 = (int *)FUN_007ed830();
      iVar1 = (**(code **)(*piVar3 + 0x4c))(0x27ab);
      if (iVar1 != 1) {
        piVar3 = (int *)FUN_007ed830();
        iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2736);
        if (iVar1 != 1) {
          piVar3 = (int *)FUN_007ed830();
          iVar1 = (**(code **)(*piVar3 + 0x4c))(0x27aa);
          if (iVar1 != 1) {
            piVar3 = (int *)FUN_007ed830();
            iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2737);
            if (iVar1 != 1) {
              piVar3 = (int *)FUN_007ed830();
              iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2738);
              if (iVar1 != 1) {
                piVar3 = (int *)FUN_007ed830();
                iVar1 = (**(code **)(*piVar3 + 0x4c))(0x2739);
                if (iVar1 != 1) {
                  piVar3 = (int *)FUN_007ed830();
                  iVar1 = (**(code **)(*piVar3 + 0x4c))(0x273a);
                  if ((iVar1 != 1) && (*(int *)(param_1 + 0x3b0) == 0)) {
                    if (*(short *)(param_1 + 0x2e8) == 0) {
                      return 1;
                    }
                    uVar2 = FUN_0076e610(0x20);
                    FUN_0073f530(uVar2);
                    if (*(int *)(param_1 + 0x138) == 1) {
                      return 1;
                    }
                    FUN_007ed830();
                    uVar2 = FUN_0076e610(0x10);
                    FUN_0073f530(uVar2);
                    return 1;
                  }
                }
              }
            }
          }
        }
      }
    }
    uVar2 = 1;
  }
  else {
    uVar2 = 1;
  }
  return uVar2;
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

## Card B13a-1-14

GOG function: CSWCCreature::SetEngagedState  gog 0x00422e80  callconv=(blank)  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x00776db0 (size 84, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00775aa0:STR0/CALLEE0/SIG0.022/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00422e00 CSWCCreature::InCreatureCombat -> 0x00776e40  gate_class=REVIEW
- GOG-order neighbour after:  0x00422ea0 Global::SetQuededAnimationCallback  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x13c} | Steam {0x3c0}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00776db0: 16-byte aligned: YES
  00776db0  55             PUSH EBP
  00776db1  8B EC          MOV EBP,ESP
  00776db3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00776db0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C0 [dec 960]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00776db0 FUN_00776db0 size=84 ret=4 callers=3 callees=0

void __thiscall FUN_00776db0(int param_1,int param_2)

{
  if (param_2 == 0) {
    *(undefined4 *)(param_1 + 0x3c0) = 0;
  }
  else if (param_2 == 1) {
    *(undefined4 *)(param_1 + 0x3c0) = 1;
  }
  else if (param_2 == 2) {
    *(undefined4 *)(param_1 + 0x3c0) = 2;
  }
  return;
}
```

### decoy: 0x00775aa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00775aa0: 16-byte aligned: YES
  00775aa0  55             PUSH EBP
  00775aa1  8B EC          MOV EBP,ESP
  00775aa3  81 EC A4 05 00 00 SUB ESP,0x5a4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

## Card B13a-1-40

GOG function: CSWCCreature::GetPersonalActions  gog 0x0042a5d0  callconv=(blank)  param_size_bytes=8  gog_size=129  gog_retn=8
Candidate Steam address: 0x0077dba0 (size 1005, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::GetCombatFeatsSkills | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077df90:STR0/CALLEE0/SIG0.167/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x1a,0x34,0x2e4}
imm sets (* = in both, 1 common): GOG {0x2,0x4,0x5,0x8*} | Steam {0x8*,0xb,0x10,0x11,0x12,0x13,0x14,0x1a,0x1c,0x1d,0x1e,0x1f,0x35,0x4d,...(+8)}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0077dba0: 16-byte aligned: YES
  0077dba0  55             PUSH EBP
  0077dba1  8B EC          MOV EBP,ESP
  0077dba3  81 EC A4 00 00 00 SUB ESP,0xa4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077dba0
string literals (0):
direct callees (7):
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x005453C0  CSWSObject::GetArea
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F4D0  CClientExoApp::GetGameObject
  0x006D5200  (unnamed)
  0x007787E0  CSWCCreature::CheckFeat
  0x00778940  Global::SceneHitCheck
struct offsets touched (top 6):
  +0x4 [dec 4]  x8  via ECX,EDX
  +0xC [dec 12]  x8  via ECX
  +0x8 [dec 8]  x7  via ECX
  +0x34 [dec 52]  x3  via EAX,EDX
  +0x1A [dec 26]  x1  via EAX
  +0x2E4 [dec 740]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077dba0 FUN_0077dba0 size=1005 ret=8 callers=1 callees=7

void __thiscall FUN_0077dba0(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  
  iVar2 = FUN_0077d800(param_1);
  if (iVar2 != 0) {
    iVar2 = FUN_005453c0();
    if (*(int *)(iVar2 + 0x2e4) == 0) {
      iVar2 = FUN_0076e610(0x10);
      bVar1 = false;
      if (iVar2 != 0x7f000000) {
        piVar3 = (int *)FUN_0073f4d0(iVar2);
        if (piVar3 != (int *)0x0) {
          iVar2 = (**(code **)(*piVar3 + 0x34))();
          if (iVar2 != 0) {
            iVar2 = (**(code **)(*piVar3 + 0x34))();
            iVar2 = FUN_006d5200(*(undefined4 *)(iVar2 + 0xc));
            if ((iVar2 != 0) && (*(char *)(iVar2 + 0x1a) != '\0')) {
              bVar1 = true;
            }
          }
        }
      }
      if (bVar1) {
        uVar4 = FUN_007787e0(0x52,0x12,0x1d);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x1a,0x5c,0x1e);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x4d,0x14,0x1f);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
      }
      else {
        uVar4 = FUN_007787e0(0x51,0x13,8);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x35,0x5b,0xb);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0x53,0x11,0x1c);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
        uVar4 = FUN_007787e0(0xdc,0xffffffff,0xffffffff);
        FUN_00778940(uVar4,param_2,FUN_0077b0c0,0,0,0,param_3);
      }
    }
  }
  return;
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

## Card B13a-1-26

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077fb10 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::UpdateRollingFootstepSound_2 | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0077fac0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042ba20 CSWCCreature::PlayRollingFootstepSound -> 0x0077fa00  gate_class=AUTO
- GOG-order neighbour after:  0x0042bb30 CSWCCreature::UpdateRollingFootstepSound_2 -> 0x0077fb10  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00639f20 CExoSoundSource::Stop -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x448*,0x450,0x454} | Steam {0x448*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x83,0x2712}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0077fb10: 16-byte aligned: YES
  0077fb10  55             PUSH EBP
  0077fb11  8B EC          MOV EBP,ESP
  0077fb13  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0077fb10
string literals (0):
direct callees (3):
  0x007ED7D0  CSWCObject::GetCurrentAnimation
  0x0077FA00  CSWCCreature::PlayRollingFootstepSound
  0x0077FAC0  CSWCCreature::IsInjured
struct offsets touched (top 2):
  +0x448 [dec 1096]  x1  via EAX
  +0x77FB8C [dec 7863180]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077fb10 FUN_0077fb10 size=111 ret=0 callers=1 callees=3

void __fastcall FUN_0077fb10(int param_1)

{
  undefined2 uVar1;
  
  if (*(int *)(param_1 + 0x448) != 0) {
    uVar1 = FUN_007ed7d0();
    switch(uVar1) {
    case 0x2712:
    case 0x2713:
    case 0x276d:
    case 0x2795:
      FUN_0077fa00(0);
      break;
    case 0x2714:
    case 0x276e:
      FUN_0077fa00(1);
      break;
    default:
      FUN_0077fac0();
    }
  }
  return;
}
```

### decoy: 0x0077fac0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0077fac0: 16-byte aligned: YES
  0077fac0  55             PUSH EBP
  0077fac1  8B EC          MOV EBP,ESP
  0077fac3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0077fac0
string literals (0):
direct callees (1):
  0x0070AF80  (unnamed)
struct offsets touched (top 3):
  +0x448 [dec 1096]  x2  via EAX,EDX
  +0x450 [dec 1104]  x2  via EAX,ECX
  +0x454 [dec 1108]  x1  via ECX
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

## Card B13a-1-01

GOG function: CSWCCreature::ResetDesiredPositionTimer  gog 0x0041f7b0  callconv=(blank)  param_size_bytes=0  gog_size=34  gog_retn=0
Candidate Steam address: 0x007738e0 (size 34, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00773860:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0041f730 CSWCCreature::ResetAnimationState -> 0x007737e0  gate_class=REVIEW
- GOG-order neighbour after:  0x0041f7e0 CSWCCreature::SetSpecialWalkAnimation -> 0x00773910  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4*,0x8*} | Steam {0x4*,0x8*}
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
prologue for 0x007738e0: 16-byte aligned: YES
  007738e0  55             PUSH EBP
  007738e1  8B EC          MOV EBP,ESP
  007738e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007738e0
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007738e0 FUN_007738e0 size=34 ret=0 callers=0 callees=0

void FUN_007738e0(undefined4 param_1,undefined4 param_2,int param_3)

{
  *(undefined4 *)(param_3 + 4) = 0;
  *(undefined4 *)(param_3 + 8) = 0;
  return;
}
```

### decoy: 0x00773860 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00773860: 16-byte aligned: YES
  00773860  55             PUSH EBP
  00773861  8B EC          MOV EBP,ESP
  00773863  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00773860
string literals (0):
direct callees (1):
  0x0071B3F0  (unnamed)
struct offsets touched (top 5):
  +0x10 [dec 16]  x1  via ECX
  +0x18 [dec 24]  x1  via EAX
  +0x70 [dec 112]  x1  via EDX
  +0x130 [dec 304]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

## Card B13a-1-25

GOG function: CSWCCreature::IsInjured  gog 0x0042bae0  callconv=(blank)  param_size_bytes=0  gog_size=77  gog_retn=0
Candidate Steam address: 0x0077f6c0 (size 819, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CSWCCreature::LoadRollingFootstepSound | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
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
disp sets (* = in both, 3 common): GOG {0x448*,0x450*,0x454*} | Steam {0x4,0x10,0x18,0x5c,0x130,0x224,0x448*,0x450*,0x454*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x4,0x8,0x12,0x13,0x14,0x24,0x84,0x44c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0077f6c0: 16-byte aligned: YES
  0077f6c0  55             PUSH EBP
  0077f6c1  8B EC          MOV EBP,ESP
  0077f6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0077f6c0
string literals (0):
direct callees (13):
  0x00733540  CExoString::Constructor
  0x0071B8B0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0071AFA0  C2DA::GetCExoStringEntry
  0x00919723  operator new
  0x00710810  (unnamed)
  0x0070AB90  (unnamed)
  0x0070ACF0  (unnamed)
  0x0073F470  CClientExoApp::GetPlayerCreatureId
  0x0070ACC0  (unnamed)
  0x0070AE30  (unnamed)
  0x0070AE90  (unnamed)
  0x0071B3F0  (unnamed)
struct offsets touched (top 9):
  +0x448 [dec 1096]  x9  via EAX,ECX,EDX
  +0x130 [dec 304]  x4  via EAX,ECX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x18 [dec 24]  x2  via ECX,EDX
  +0x5C [dec 92]  x2  via EAX
  +0x224 [dec 548]  x2  via EAX,EDX
  +0x450 [dec 1104]  x1  via EDX
  +0x454 [dec 1108]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0077f6c0 FUN_0077f6c0 size=819 ret=0 callers=1 callees=13

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_0077f6c0(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 local_94;
  undefined1 local_4c [16];
  int local_3c;
  undefined4 local_38;
  uint local_34;
  uint local_30;
  undefined4 *local_2c;
  undefined4 *local_28;
  int local_24;
  undefined4 local_20;
  undefined1 local_1c [11];
  undefined1 local_11;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00965443;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  if (*(int *)(param_1 + 0x448) != 0) {
    local_2c = *(undefined4 **)(param_1 + 0x448);
    local_28 = local_2c;
    if (local_2c != (undefined4 *)0x0) {
      (**(code **)*local_2c)(1);
    }
    *(undefined4 *)(param_1 + 0x448) = 0;
  }
  iVar1 = FUN_0071b8b0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdbc,&local_20);
  if (iVar1 == 0) {
    local_30 = 0;
    local_8 = 0xffffffff;
    FUN_00733780();
    uVar3 = local_30;
  }
  else {
    iVar1 = FUN_0071afa0(local_20,DAT_00a0fe6c,local_1c);
    if (iVar1 == 0) {
      local_34 = 0;
      local_8 = 0xffffffff;
      FUN_00733780();
      uVar3 = local_34;
    }
    else {
      local_3c = FUN_00919723(8);
      local_8._0_1_ = 1;
      if (local_3c == 0) {
        local_94 = 0;
      }
      else {
        FUN_00710810(local_1c);
        local_94 = FUN_0070ab90(local_4c);
      }
      local_38 = local_94;
      local_8 = (uint)local_8._1_3_ << 8;
      *(undefined4 *)(param_1 + 0x448) = local_94;
      FUN_0070acf0(1);
      iVar1 = *(int *)(param_1 + 4);
      iVar2 = FUN_0073f470();
      if (iVar1 == iVar2) {
        local_11 = 0x13;
      }
      else {
        FUN_0071b8b0(*(undefined2 *)(*(int *)(param_1 + 0x224) + 0x18),DAT_00a0fdf4,&local_24);
        if (((local_24 == 1) || (local_24 == 2)) || (local_24 == 3)) {
          local_11 = 0x14;
        }
        else {
          local_11 = 0x12;
/* ... 16 more lines truncated by --max-lines */
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
