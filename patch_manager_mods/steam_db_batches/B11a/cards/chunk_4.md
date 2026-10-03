# Review cards B11a chunk 4

Answer each card with one TSV row in `steam_db_batches/B11a/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B11a-4-01

GOG function: CClientExoAppInternal::StallServerCode  gog 0x00430300  callconv=(blank)  param_size_bytes=4  gog_size=49  gog_retn=4
Candidate Steam address: 0x00783d80 (size 49, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00783dc0:STR0/CALLEE1/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042e160 CClientExoAppInternal::MainLoop -> 0x00781be0  gate_class=AUTO
- GOG-order neighbour after:  0x00430340 CClientExoAppInternal::UpdateRoom -> 0x00783dc0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00605700 CExoString::Constructor_3 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 4): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00783d80: 16-byte aligned: YES
  00783d80  55             PUSH EBP
  00783d81  8B EC          MOV EBP,ESP
  00783d83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00783d80
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00783d80 FUN_00783d80 size=49 ret=4 callers=4 callees=1

undefined4 __thiscall FUN_00783d80(int param_1,undefined4 param_2)

{
  FUN_007335f0(param_1 + 0x10);
  return param_2;
}
```

### decoy: 0x00783dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00783dc0: 16-byte aligned: YES
  00783dc0  55             PUSH EBP
  00783dc1  8B EC          MOV EBP,ESP
  00783dc3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00783dc0
string literals (0):
direct callees (6):
  0x00726F80  (unnamed)
  0x007AACE0  CSWCArea::GetRoomIndex
  0x007AAB90  CSWCArea::GetRoom_2
  0x0073F750  ClientApp::GetGuiInGame
  0x0070BC00  (unnamed)
  0x0070BC30  (unnamed)
struct offsets touched (top 9):
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
  +0x44 [dec 68]  x1  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x84 [dec 132]  x1  via EDX
  +0xD8 [dec 216]  x1  via EDX
  +0x188 [dec 392]  x1  via ECX
```

## Card B11a-4-02

GOG function: CClientExoAppInternal::CancelMovie  gog 0x0044cb00  callconv=(blank)  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x007a0580 (size 166, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::BusyWaitMenuMusic | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a0530:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ca90 CClientExoAppInternal::SetMouseMode -> 0x0079fc40  gate_class=REJECT
- GOG-order neighbour after:  0x0044cb30 CClientExoAppInternal::SetLastTarget -> 0x007a0530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009211a0 CExoMoviePlayer::CancelMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040e420 CClientExoApp::CancelMovie -> Steam 0x00740f60  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x130} | Steam {0x4,0x238}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc,0x2ee,0x3e8,0x9c4}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007a0580: 16-byte aligned: YES
  007a0580  55             PUSH EBP
  007a0581  8B EC          MOV EBP,ESP
  007a0583  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a0580
string literals (0):
direct callees (5):
  0x00736220  CExoTimers::GetHighResolutionTimer
  0x0091D220  (unnamed)
  0x0070B320  (unnamed)
  0x0070B8F0  CExoSound::Render wrapper
  0x0070B2F0  (unnamed)
struct offsets touched (top 2):
  +0x238 [dec 568]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007a0580 FUN_007a0580 size=166 ret=0 callers=1 callees=5

void __fastcall FUN_007a0580(int param_1)

{
  int iVar1;
  undefined8 uVar2;
  int local_c;
  
  uVar2 = FUN_00736220();
  iVar1 = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar2,1000,0);
  local_c = iVar1;
  if (*(int *)(param_1 + 0x238) != 0) {
    FUN_0070b320(0x9c4);
  }
  while ((uint)(local_c - iVar1) < 0x2ee) {
    FUN_0070b8f0();
    uVar2 = FUN_00736220();
    local_c = __aulldiv [GHIDRA-AUTO-LABEL: unreliable](uVar2,1000,0);
  }
  if (*(int *)(param_1 + 0x238) != 0) {
    FUN_0070b2f0(0);
  }
  return;
}
```

### decoy: 0x007a0530 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0530: 16-byte aligned: YES
  007a0530  55             PUSH EBP
  007a0531  8B EC          MOV EBP,ESP
  007a0533  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0530
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x2B4 [dec 692]  x2  via EAX,EDX
  +0x37C [dec 892]  x2  via ECX,EDX
```

## Card B11a-4-03

GOG function: CClientExoAppInternal::SendLoadGameRequest  gog 0x00431c80  callconv=(blank)  param_size_bytes=12  gog_size=57  gog_retn=12
Candidate Steam address: 0x00785660 (size 57, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: SendLoadGameRequest 0x73f720)
- best decoy (neighbour/other candidate) scores: 0x00785620:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431c40 CClientExoAppInternal::SendSaveGameRequest -> 0x00785620  gate_class=REVIEW
- GOG-order neighbour after:  0x00431cc0 CClientExoAppInternal::ServerModeChange -> 0x00785030  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00505d90 CSWCMessage::SendPlayerToServerModule_LoadGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040cbe0 CClientExoApp::SendLoadGameRequest -> Steam 0x0073f720  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x150*} | Steam {0x150*}
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
prologue for 0x00785660: 16-byte aligned: YES
  00785660  55             PUSH EBP
  00785661  8B EC          MOV EBP,ESP
  00785663  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00785660
string literals (0):
direct callees (1):
  0x0087A4B0  (unnamed)
struct offsets touched (top 1):
  +0x150 [dec 336]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00785660 FUN_00785660 size=57 ret=12 callers=1 callees=1

undefined4 __thiscall
FUN_00785660(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x150) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_0087a4b0(param_2,param_3,param_4);
  }
  return uVar1;
}
```

### decoy: 0x00785620 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785620: 16-byte aligned: YES
  00785620  55             PUSH EBP
  00785621  8B EC          MOV EBP,ESP
  00785623  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00785620
string literals (0):
direct callees (1):
  0x0087A430  (unnamed)
struct offsets touched (top 1):
  +0x150 [dec 336]  x1  via EAX
```

## Card B11a-4-04

GOG function: CClientExoAppInternal::DestroyWorldJournal  gog 0x004318e0  callconv=(blank)  param_size_bytes=0  gog_size=98  gog_retn=0
Candidate Steam address: 0x007852c0 (size 98, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x00785230:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431850 CClientExoAppInternal::CreateWorldJournal -> 0x00785230  gate_class=REVIEW
- GOG-order neighbour after:  0x00431950 CClientExoAppInternal::JournalUpdated -> 0x00785330  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00547080 CSWCWorldJournal::Destructor -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007852c0: 16-byte aligned: YES
  007852c0  55             PUSH EBP
  007852c1  8B EC          MOV EBP,ESP
  007852c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007852c0
string literals (0):
direct callees (2):
  0x0084B870  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x3C [dec 60]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007852c0 FUN_007852c0 size=98 ret=0 callers=1 callees=2

void __fastcall FUN_007852c0(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x3c) != 0) {
    iVar1 = *(int *)(param_1 + 0x3c);
    if (iVar1 != 0) {
      FUN_0084b870();
      FUN_009196fd(iVar1);
    }
    *(undefined4 *)(param_1 + 0x3c) = 0;
  }
  return;
}
```

### decoy: 0x00785230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785230: 16-byte aligned: YES
  00785230  55             PUSH EBP
  00785231  8B EC          MOV EBP,ESP
  00785233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00785230
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0084B7F0  (unnamed)
struct offsets touched (top 1):
  +0x3C [dec 60]  x2  via EAX,EDX
```

## Card B11a-4-05

GOG function: CClientExoAppInternal::SetHitcheckIgnoreList  gog 0x00439850  callconv=(blank)  param_size_bytes=4  gog_size=1020  gog_retn=4
Candidate Steam address: 0x0078d280 (size 1020, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.93) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078d220:STR0/CALLEE0/SIG0.188/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004397f0 CClientExoAppInternal::EmptyHitcheckIgnoreList -> 0x0078d220  gate_class=REVIEW
- GOG-order neighbour after:  0x00439c50 CClientExoAppInternal::SetDebugMode -> 0x0078d680  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam ?  in candidate callees: n/a
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam ?  in candidate callees: n/a
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
  0x004eafd0 CSWBaseItemArray::GetBaseItem -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x008bfdd0 List_T_::Add -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 10 common): GOG {0x4*,0xc*,0x10*,0x14*,0x18*,0x2c*,0x34*,0x48*,0x98*,0x224*} | Steam {0x4*,0xc*,0x10*,0x14*,0x18*,0x2c*,0x34*,0x48*,0x98*,0x224*}
imm sets (* = in both, 8 common): GOG {0x4*,0x10*,0x20*,0x60*,0xfe*,0xff*,0x190*,0x208*} | Steam {0x4*,0x10*,0x20*,0x60*,0xfe*,0xff*,0x190*,0x208*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078d280: 16-byte aligned: YES
  0078d280  55             PUSH EBP
  0078d281  8B EC          MOV EBP,ESP
  0078d283  83 EC 60       SUB ESP,0x60
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d280
string literals (0):
direct callees (8):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x004491C0  CExoPtrList::Add (doubling)
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x006D5200  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 10):
  +0x98 [dec 152]  x14  via EAX,EDX
  +0x10 [dec 16]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x4  via ECX
  +0x18 [dec 24]  x3  via ECX,EDX
  +0xC [dec 12]  x2  via EAX,EDX
  +0x34 [dec 52]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
  +0x224 [dec 548]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078d280 FUN_0078d280 size=1020 ret=4 callers=1 callees=8

void __fastcall FUN_0078d280(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  int *local_38;
  int local_34;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  undefined4 *local_1c;
  int *local_18;
  int *local_14;
  int local_10;
  int *local_c;
  int local_8;
  
  local_28 = FUN_007a1720();
  local_1c = (undefined4 *)FUN_00521060();
  while (local_28 != 0) {
    if (local_1c != (undefined4 *)0x0) {
      local_38 = (int *)0x0;
      FUN_0053dfb0(*local_1c,&local_38);
      local_c = local_38;
      if ((local_38 != (int *)0x0) &&
         (local_18 = (int *)(**(code **)(*local_38 + 0xc))(), local_18 != (int *)0x0)) {
        local_14 = (int *)(**(code **)(*local_18 + 0x2c))();
        local_10 = (**(code **)(*local_18 + 0x34))();
        iVar1 = (**(code **)(*local_18 + 0x98))(0xff,1);
        if (iVar1 != 0) {
          uVar2 = (**(code **)(*local_18 + 0x98))(0xff,1);
          FUN_004491c0(uVar2);
        }
        if (local_14 != (int *)0x0) {
          local_24 = local_14[0x89 = byte +0x224];
          if ((local_24 != 0) && (local_8 = local_24, local_24 != 0)) {
            local_10 = FUN_0073f530(*(undefined4 *)(local_24 + 0x10));
            if ((local_10 != 0) &&
               (iVar1 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1), iVar1 != 0)) {
              uVar2 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1);
              FUN_004491c0(uVar2);
            }
            iVar1 = (**(code **)(*local_14 + 0x98))(0xfe,1);
            if (iVar1 != 0) {
              uVar2 = (**(code **)(*local_14 + 0x98))(0xfe,1);
              FUN_004491c0(uVar2);
            }
            if (*(int *)(local_8 + 0x10) == DAT_00994484) {
              iVar1 = (**(code **)(*local_14 + 0x98))(0xfe,1);
              if (iVar1 != 0) {
                uVar2 = (**(code **)(*local_14 + 0x98))(0xfe,1);
                FUN_004491c0(uVar2);
              }
            }
            else {
              local_10 = FUN_0073f530(*(undefined4 *)(local_8 + 0x10));
              if ((local_10 != 0) &&
                 (iVar1 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1), iVar1 != 0)) {
                uVar2 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1);
                FUN_004491c0(uVar2);
              }
            }
          }
          uVar2 = FUN_0076e610(0x10);
          local_10 = FUN_0073f530(uVar2);
          if (local_10 != 0) {
            local_20 = FUN_006d5200(*(undefined4 *)(local_10 + 0xc));
            iVar1 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1);
            if (iVar1 != 0) {
              uVar2 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1);
              FUN_004491c0(uVar2);
            }
          }
          uVar2 = FUN_0076e610(0x20);
          local_10 = FUN_0073f530(uVar2);
          if ((local_10 != 0) &&
             (iVar1 = (**(code **)(*(int *)(local_10 + 0x10) + 0x98))(0xff,1), iVar1 != 0)) {
/* ... 21 more lines truncated by --max-lines */
```

### decoy: 0x0078d220 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d220: 16-byte aligned: YES
  0078d220  55             PUSH EBP
  0078d221  8B EC          MOV EBP,ESP
  0078d223  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d220
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-06

GOG function: CClientExoAppInternal::GetDoorByGameObjectID  gog 0x00438600  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bd80:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438590 CClientExoAppInternal::GetPlaceableByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00438670 CClientExoAppInternal::GetAreaOfEffectByGameObjectID -> 0x0078bd80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bd80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B11a-4-07

GOG function: CClientExoAppInternal::CleanUpModuleServices  gog 0x00438230  callconv=(blank)  param_size_bytes=0  gog_size=111  gog_retn=0
Candidate Steam address: 0x0078c170 (size 385, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.30) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078c100:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004381a0 CClientExoAppInternal::ShutDownMoviePlayer -> 0x0078bcc0  gate_class=REVIEW
- GOG-order neighbour after:  0x004382a0 CClientExoAppInternal::GetPlayerByPlayerID -> 0x0078c350  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x004d7720 CGuiInGame::ShutDown -> Steam 0x007c7030  in candidate callees: no
  0x004f0320 CSWPlayerControl::Initialize -> Steam ?  in candidate callees: n/a
  0x00546cc0 CSWCJournal::ClearAll -> Steam ?  in candidate callees: n/a
  0x005476d0 CSWCWorldJournal::DeleteAllEntries -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x38,0x3c,0x40*,0x2a0*} | Steam {0x4,0x40*,0x48,0x94,0x188,0x2a0*,0x328,0x32c,0x330}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {0x4,0x34,0xcc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078c170: 16-byte aligned: YES
  0078c170  55             PUSH EBP
  0078c171  8B EC          MOV EBP,ESP
  0078c173  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078c170
string literals (0):
direct callees (7):
  0x0078CEE0  (unnamed)
  0x00726F80  (unnamed)
  0x007F3770  (unnamed)
  0x008652C0  (unnamed)
  0x00865360  (unnamed)
  0x00919723  operator new
  0x008654D0  (unnamed)
struct offsets touched (top 9):
  +0x2A0 [dec 672]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x48 [dec 72]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0x188 [dec 392]  x1  via EAX
  +0x328 [dec 808]  x1  via EAX
  +0x32C [dec 812]  x1  via EDX
  +0x330 [dec 816]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078c170 FUN_0078c170 size=385 ret=0 callers=2 callees=7

void __fastcall FUN_0078c170(int param_1)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_44;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096652b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  piVar1 = (int *)FUN_0078cee0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (piVar1 != (int *)0x0) {
    iVar2 = FUN_00726f80();
    (**(code **)(*piVar1 + 0x94))(*(undefined4 *)(*(int *)(iVar2 + 0x48) + 0x188));
    if (DAT_00a10c08 == 0) {
      iVar2 = piVar1[1 = byte +0x4];
      uVar3 = 1;
      FUN_00726f80(iVar2,1);
      FUN_007f3770(iVar2,uVar3);
    }
    else if (piVar1[0xca = byte +0x328] != 0) {
      piVar1[0xcb = byte +0x32c] = 1;
      piVar1[0xcc = byte +0x330] = 1;
    }
    if (*(int *)(param_1 + 0x2a0) == 0) {
      iVar2 = FUN_00919723(0xcc);
      local_8 = 0;
      if (iVar2 == 0) {
        local_44 = 0;
      }
      else {
        iVar2 = FUN_00726f80();
        local_44 = FUN_008654d0(piVar1[1 = byte +0x4],*(undefined4 *)(iVar2 + 0x40));
      }
      *(undefined4 *)(param_1 + 0x2a0) = local_44;
    }
    else {
      FUN_008652c0(piVar1[1 = byte +0x4]);
      iVar2 = FUN_00726f80();
      FUN_00865360(*(undefined4 *)(iVar2 + 0x40));
    }
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0078c100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078c100: 16-byte aligned: YES
  0078c100  55             PUSH EBP
  0078c101  8B EC          MOV EBP,ESP
  0078c103  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078c100
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x6C [dec 108]  x1  via EDX
```

## Card B11a-4-08

GOG function: CClientExoAppInternal::GetSoundObjectByGameObjectID  gog 0x004386e0  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bd80 (size 102, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bdf0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438670 CClientExoAppInternal::GetAreaOfEffectByGameObjectID -> 0x0078bd80  gate_class=REVIEW
- GOG-order neighbour after:  0x00438750 CClientExoAppInternal::AttachPlayerGameObjectToArea -> 0x0078bc50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x14*,0x6c} | Steam {0x8*,0x14*,0x34}
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
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bd80 FUN_0078bd80 size=102 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_0078bd80(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 uVar2;
  int *local_8;
  
  local_8 = (int *)0x0;
  uVar2 = *(undefined4 *)(param_1 + 0x14);
  cVar1 = FUN_0053dfb0(param_2,&local_8);
  if (((cVar1 == DAT_0099447f) && (local_8 != (int *)0x0)) && ((char)local_8[2 = byte +0x8] == DAT_00994475)) {
    uVar2 = (**(code **)(*local_8 + 0x34))(param_1,uVar2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0078bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bdf0: 16-byte aligned: YES
  0078bdf0  55             PUSH EBP
  0078bdf1  8B EC          MOV EBP,ESP
  0078bdf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bdf0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B11a-4-09

GOG function: CClientExoAppInternal::CreateWorldJournal  gog 0x00431850  callconv=(blank)  param_size_bytes=0  gog_size=132  gog_retn=0
Candidate Steam address: 0x00785230 (size 132, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007852c0:STR0/CALLEE0/SIG0.833/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004317f0 CClientExoAppInternal::AddToPauseExclusionList_2 -> 0x007851d0  gate_class=REVIEW
- GOG-order neighbour after:  0x004318e0 CClientExoAppInternal::DestroyWorldJournal -> 0x007852c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00547000 CSWCWorldJournal::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x3c*} | Steam {0x3c*}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x20*} | Steam {0x4*,0x10*,0x20*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00785230: 16-byte aligned: YES
  00785230  55             PUSH EBP
  00785231  8B EC          MOV EBP,ESP
  00785233  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00785230
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0084B7F0  (unnamed)
struct offsets touched (top 1):
  +0x3C [dec 60]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00785230 FUN_00785230 size=132 ret=0 callers=1 callees=2

void __fastcall FUN_00785230(int param_1)

{
  int iVar1;
  undefined4 local_20;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961f7b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x3c) == 0) {
    iVar1 = FUN_00919723(0x20,DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    if (iVar1 == 0) {
      local_20 = 0;
    }
    else {
      local_20 = FUN_0084b7f0();
    }
    *(undefined4 *)(param_1 + 0x3c) = local_20;
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x007852c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007852c0: 16-byte aligned: YES
  007852c0  55             PUSH EBP
  007852c1  8B EC          MOV EBP,ESP
  007852c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007852c0
string literals (0):
direct callees (2):
  0x0084B870  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 1):
  +0x3C [dec 60]  x3  via EAX,ECX
```

## Card B11a-4-10

GOG function: CClientExoAppInternal::Initialize  gog 0x0042df60  callconv=__thiscall  param_size_bytes=0  gog_size=511  gog_retn=0
Candidate Steam address: 0x007810a0 (size 767, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::Destructor | G7 mac: pass(0.16) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.02 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007813a0:STR0/CALLEE0/SIG0.175/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042ddc0 CClientExoAppInternal::HandleMessage -> 0x00781590  gate_class=REJECT
- GOG-order neighbour after:  0x0042e160 CClientExoAppInternal::MainLoop -> 0x00781be0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (12), mapped to Steam through trusted rows only:
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam ?  in candidate callees: n/a
  0x00445220 CClientExoAppInternal::IsMoviePlaying -> Steam ?  in candidate callees: n/a
  0x004ce110 CGuiInGame::InitializeGlobal -> Steam ?  in candidate callees: n/a
  0x004e11f0 CGuiInGame::SetGlobalUnpauseableState -> Steam ?  in candidate callees: n/a
  0x0050a4e0 CSWRules::Constructor -> Steam ?  in candidate callees: n/a
  0x0053fba0 CSWGuiLoadScreen::Constructor -> Steam ?  in candidate callees: n/a
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  ... 4 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): 'Walk'
disp sets (* = in both, 0 common): GOG {0x18,0x24,0x40,0xa4,0x130,0x204,0x274,0x278} | Steam {0x1d0,0x270,0x3c4}
imm sets (* = in both, 2 common): GOG {0x2*,0x4*,0x44,0x154,0x6f8} | Steam {0x2*,0x3,0x4*,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf,...(+29)}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007810a0: 16-byte aligned: YES
  007810a0  55             PUSH EBP
  007810a1  8B EC          MOV EBP,ESP
  007810a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007810a0
string literals (0):
direct callees (11):
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
  0x0072F5F0  (unnamed)
  0x0070B080  StreamingSource scalar delete
  0x00754E90  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x00632D40  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x007A1900  CClientExoAppInternal::IsMoviePlaying_2
  0x00733780  CExoString::~CExoString
  0x0040DF30  (unnamed)
struct offsets touched (top 3):
  +0x270 [dec 624]  x3  via EAX,ECX
  +0x3C4 [dec 964]  x3  via ECX,EDX
  +0x1D0 [dec 464]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007810a0 FUN_007810a0 size=767 ret=0 callers=1 callees=11

void __fastcall FUN_007810a0(int *param_1)

{
  int iVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00965768;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0x15;
  if (param_1[0x9c = byte +0x270] != 0) {
    iVar1 = param_1[0x9c = byte +0x270];
    if (iVar1 != 0) {
      FUN_00623320(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
      FUN_009196fd(iVar1);
    }
    param_1[0x9c = byte +0x270] = 0;
  }
  iVar1 = param_1[0x74 = byte +0x1d0];
  if (iVar1 != 0) {
    FUN_0072f5f0();
    FUN_009196fd(iVar1);
  }
  if (*param_1 != 0) {
    FUN_009196fd(*param_1);
    *param_1 = 0;
  }
  if (param_1[0xf1 = byte +0x3c4] != 0) {
    iVar1 = param_1[0xf1 = byte +0x3c4];
    if (iVar1 != 0) {
      FUN_0070b080();
      FUN_009196fd(iVar1);
    }
    param_1[0xf1 = byte +0x3c4] = 0;
  }
  local_8._0_1_ = 0x14;
  FUN_00754e90();
  local_8._0_1_ = 0x13;
  _eh_vector_destructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](param_1 + 0x106,0x10,7,FUN_006c5370);
  local_8._0_1_ = 0x12;
  FUN_00632d40();
  local_8._0_1_ = 0x11;
  Free();
  local_8._0_1_ = 0x10;
  FUN_007a1900();
  local_8._0_1_ = 0xf;
  FUN_00733780();
  local_8._0_1_ = 0xe;
  Free();
  local_8._0_1_ = 0xd;
  FUN_00733780();
  local_8._0_1_ = 0xc;
  FUN_00733780();
  local_8._0_1_ = 0xb;
  Free();
  local_8._0_1_ = 10;
  FUN_0040df30();
  local_8._0_1_ = 9;
  FUN_0040df30();
  local_8._0_1_ = 8;
  Free();
  local_8._0_1_ = 7;
  FUN_00733780();
  local_8._0_1_ = 6;
  FUN_00733780();
  local_8._0_1_ = 5;
  FUN_0040df30();
  local_8._0_1_ = 4;
  FUN_0072f5f0();
  local_8._0_1_ = 3;
  FUN_0072f5f0();
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8._0_1_ = 1;
  FUN_0072f5f0();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00733780();
/* ... 5 more lines truncated by --max-lines */
```

### decoy: 0x007813a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007813a0: 16-byte aligned: YES
  007813a0  55             PUSH EBP
  007813a1  8B EC          MOV EBP,ESP
  007813a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007813a0
string literals (3):
  0x0099F49C  at 007813e5  'leclogo'
  0x0099F490  at 00781416  'obsidianent'
  0x0099F488  at 007814dd  'legal'
direct callees (10):
  0x00733570  CExoString::CExoString_char
  0x007A0F10  CClientExoAppInternal::AddMovieToMovieQueue
  0x00733780  CExoString::~CExoString
  0x007A0F50  CClientExoAppInternal::PlayMovieQueue
  0x00919723  operator new
  0x0087FD40  (unnamed)
  0x00410530  CSWGuiManager::AddPanel
  0x00710840  (unnamed)
  0x0087FE20  (unnamed)
  0x00410670  (unnamed)
struct offsets touched (top 3):
  +0x27C [dec 636]  x8  via EAX,ECX,EDX
  +0x274 [dec 628]  x3  via EAX,ECX,EDX
  +0x1FC [dec 508]  x1  via EAX
```

## Card B11a-4-11

GOG function: CClientExoAppInternal::SetCharacterFile  gog 0x0043a610  callconv=(blank)  param_size_bytes=12  gog_size=166  gog_retn=12
Candidate Steam address: 0x0078e040 (size 166, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078d680:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a5e0 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList -> 0x0078e010  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a6c0 CClientExoAppInternal::ShutDownToMainMenu -> 0x0078e0f0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00925530 Global::memcpy -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1b4*,0x1b8*,0x1bc*} | Steam {0x1b4*,0x1b8*,0x1bc*}
imm sets (* = in both, 2 common): GOG {0x4*,0xc*} | Steam {0x4*,0xc*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0078e040: 16-byte aligned: YES
  0078e040  55             PUSH EBP
  0078e041  8B EC          MOV EBP,ESP
  0078e043  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e040
string literals (0):
direct callees (3):
  0x009196FD  operator delete
  0x00919723  operator new
  0x0091DBA0  _memcpy
struct offsets touched (top 3):
  +0x1B4 [dec 436]  x6  via EAX,ECX
  +0x1B8 [dec 440]  x1  via EAX
  +0x1BC [dec 444]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e040 FUN_0078e040 size=166 ret=12 callers=1 callees=3

void __thiscall FUN_0078e040(int param_1,size_t param_2,void *param_3,undefined4 param_4)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x1b4) != 0) {
    FUN_009196fd(*(undefined4 *)(param_1 + 0x1b4));
    *(undefined4 *)(param_1 + 0x1b4) = 0;
  }
  if (param_3 != (void *)0x0) {
    uVar1 = FUN_00919723(param_2 + 1);
    *(undefined4 *)(param_1 + 0x1b4) = uVar1;
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](*(void **)(param_1 + 0x1b4),param_3,param_2);
    *(undefined1 *)(*(int *)(param_1 + 0x1b4) + param_2) = 0;
  }
  *(size_t *)(param_1 + 0x1b8) = param_2;
  *(undefined4 *)(param_1 + 0x1bc) = param_4;
  return;
}
```

### decoy: 0x0078d680 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d680: 16-byte aligned: YES
  0078d680  55             PUSH EBP
  0078d681  8B EC          MOV EBP,ESP
  0078d683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d680
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x194 [dec 404]  x1  via EAX
```

## Card B11a-4-12

GOG function: CClientExoAppInternal::StopServices  gog 0x00437190  callconv=(blank)  param_size_bytes=0  gog_size=3811  gog_retn=0
Candidate Steam address: 0x0078ac40 (size 3811, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 8 | A-CALLEE 6 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,CALLEE,SIG  (content level C2)
- evidence summary: STR 'RIMS:GLOBAL'@0099f618; 'GLOBAL'@0099f624; 'HD0:MOVIES'@0099f62c; CALLEE 6 (callees: ShutDown 0x7c7030, Destructor 0x7114d0, RemoveResourceImageFile 0x7116d0); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078a7a0:STR0/CALLEE0/SIG0.19/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> 0x0078a7a0  gate_class=REVIEW
- GOG-order neighbour after:  0x00438080 CServerInfo::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (35), mapped to Steam through trusted rows only:
  0x0040c660 CClientExoApp::ResetCharacterFile -> Steam ?  in candidate callees: n/a
  0x004316e0 CClientExoAppInternal::DestroyQuestJournal -> Steam ?  in candidate callees: n/a
  0x004318e0 CClientExoAppInternal::DestroyWorldJournal -> Steam ?  in candidate callees: n/a
  0x00438080 CServerInfo::Destructor -> Steam ?  in candidate callees: n/a
  0x00438110 unnamed -> Steam ?  in candidate callees: n/a
  0x004381a0 CClientExoAppInternal::ShutDownMoviePlayer -> Steam ?  in candidate callees: n/a
  0x004452f0 CClientExoAppInternal::StopMovie -> Steam ?  in candidate callees: n/a
  0x0044db90 CClientExoAppInternal::UnloadSuperModelSet -> Steam ?  in candidate callees: n/a
  ... 27 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (11): 'RIMS:GLOBAL'; 'GLOBAL'; 'HD0:MOVIES'; 'RIMS:' ...
disp sets (* = in both, 22 common): GOG {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x24*,0x28*,0x2c*,0x30*,0x40*,0xe0*,0x150*,0x1c0*,...(+8)} | Steam {0x4*,0x8*,0xc*,0x10*,0x14*,0x18*,0x24*,0x28*,0x2c*,0x30*,0x40*,0xe0*,0x150*,0x1c0*,...(+8)}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0x1c*,0x1b4*,0xbba*} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0x1c*,0x1b4*,0xbba*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078ac40: 16-byte aligned: YES
  0078ac40  55             PUSH EBP
  0078ac41  8B EC          MOV EBP,ESP
  0078ac43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078ac40
string literals (11):
  0x0099F728  at 0078b538  'PORTRAITS:'
  0x00993EF0  at 0078b573  'SERVERVAULT:'
  0x0099F644  at 0078b5ae  'HD0:CHITIN'
  0x0099415C  at 0078b5f8  'OVERRIDE:'
  0x0099F62C  at 0078b633  'HD0:MOVIES'
  0x0099F718  at 0078b66e  'HD0:STREAMMUSIC'
  0x0099F708  at 0078b6a9  'HD0:STREAMVOICE'
  0x0099F690  at 0078b6e4  'TEMPCLIENT:'
  0x0099F624  at 0078b71f  'GLOBAL'
  0x0099F618  at 0078b74c  'RIMS:GLOBAL'
  0x00994E68  at 0078b787  'RIMS:'
direct callees (35):
  0x007A1590  CClientExoAppInternal::UnloadSuperModelSet
  0x00798CD0  CClientExoAppInternal::StopMovie
  0x00410800  GuiManager::Contains
  0x0085F3D0  (unnamed)
  0x009196FD  operator delete
  0x0053D780  (unnamed)
  0x007852C0  CClientExoAppInternal::DestroyWorldJournal
  0x0078BB30  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x007BDBC0  (unnamed)
  0x00410380  (unnamed)
  0x007A2360  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x0078BBC0  (unnamed)
  0x00537480  (unnamed)
  0x00704480  CNetLayer::Unlink (mapper: EndServerMode)
  0x00703D70  CNetLayer dtor
  0x0070B080  StreamingSource scalar delete
  0x0070B5A0  (unnamed)
struct offsets touched (top 10):
  +0x1C4 [dec 452]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0x40 [dec 64]  x4  via EAX,ECX,EDX
  +0x274 [dec 628]  x4  via EAX,ECX
  +0x280 [dec 640]  x4  via ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078ac40 FUN_0078ac40 size=3811 ret=0 callers=1 callees=35

void __fastcall FUN_0078ac40(int param_1)

{
  int iVar1;
  undefined1 local_118 [8];
  undefined1 local_110 [8];
  undefined1 local_108 [16];
  undefined1 local_f8 [8];
  undefined1 local_f0 [8];
  undefined1 local_e8 [8];
  undefined1 local_e0 [8];
  undefined1 local_d8 [8];
  undefined1 local_d0 [8];
  undefined1 local_c8 [8];
  undefined1 local_c0 [8];
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  int local_a4;
  undefined4 *local_a0;
  undefined4 *local_9c;
  int local_98;
  int local_94;
  int local_90;
  int local_8c;
  int local_88;
  int local_84;
  undefined4 *local_80;
  undefined4 *local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c;
  int local_68;
  int local_64;
  int local_60;
  int local_5c;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  int local_48;
  int local_44;
  int local_40;
  int local_3c;
  undefined4 *local_38;
  undefined4 *local_34;
  undefined4 local_30;
  int local_2c;
  int local_28;
  undefined4 *local_24;
  undefined4 *local_20;
  undefined4 *local_1c;
  undefined4 *local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009664be;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_007a1590(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  FUN_00798cd0();
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x274),
                     *(CSize **)(param_1 + 0x280));
  if (iVar1 == 0) {
    *(undefined4 *)(param_1 + 0x280) = 0;
  }
  else {
    local_1c = *(undefined4 **)(param_1 + 0x280);
    local_18 = local_1c;
    if (local_1c != (undefined4 *)0x0) {
      (**(code **)*local_1c)(1);
    }
/* ... 294 more lines truncated by --max-lines */
```

### decoy: 0x0078a7a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078a7a0: 16-byte aligned: YES
  0078a7a0  55             PUSH EBP
  0078a7a1  8B EC          MOV EBP,ESP
  0078a7a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078a7a0
string literals (8):
  0x009982A4  at 0078a7ec  'Width'
  0x00986C84  at 0078a800  'Graphics Options'
  0x00986B98  at 0078a814  '.\\\\swkotor2.ini'
  0x00986C84  at 0078a8be  'Graphics Options'
  0x00986B98  at 0078a8d2  '.\\\\swkotor2.ini'
  0x0099F6FC  at 0078a96b  'RefreshRate'
  0x00986C84  at 0078a97f  'Graphics Options'
  0x00986B98  at 0078a993  '.\\\\swkotor2.ini'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x0093CA00  (unnamed)
  0x00402540  (unnamed)
  0x0091C630  (unnamed)
  0x0093C9C0  (unnamed)
  0x0093CA30  (unnamed)
  0x007362C0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

## Card B11a-4-13

GOG function: CClientExoApp::GetPlayerMovementInput  gog 0x0040df60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740ac0 (size 124, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740aa0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df30 CClientExoApp::LookUpAndPerformRumbleWithCutOff -> 0x00740a70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> 0x00761570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0xc,0x2a0}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x44}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00740ac0: 16-byte aligned: YES
  00740ac0  55             PUSH EBP
  00740ac1  8B EC          MOV EBP,ESP
  00740ac3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740ac0
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x2A0 [dec 672]  x5  via EAX,ECX
  +0x4 [dec 4]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00740ac0 FUN_00740ac0 size=124 ret=0 callers=2 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00740ac0(int param_1)

{
  float10 fVar1;
  undefined4 local_c;
  
  if (*(int *)(param_1 + 0x2a0) == 0) {
    local_c = 0;
  }
  else {
    fVar1 = (float10)(**(code **)(**(int **)(param_1 + 0x2a0) + 0xc))();
    if ((fVar1 == (float10)_DAT_00986b38) &&
       (fVar1 = (float10)(**(code **)(**(int **)(param_1 + 0x2a0) + 4))(),
       fVar1 == (float10)_DAT_00986b38)) {
      local_c = 0;
    }
    else {
      local_c = 1;
    }
  }
  return local_c;
}
```

### decoy: 0x00740aa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740aa0: 16-byte aligned: YES
  00740aa0  55             PUSH EBP
  00740aa1  8B EC          MOV EBP,ESP
  00740aa3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740aa0
string literals (0):
direct callees (1):
  0x00740AC0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-4-14

GOG function: CClientExoAppInternal::LoadItemDropSoundList  gog 0x00436970  callconv=(blank)  param_size_bytes=0  gog_size=522  gog_retn=0
Candidate Steam address: 0x0078a390 (size 142, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078a420:STR2/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004368e0 CServerInfo::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00436b80 CClientExoAppInternal::ReadAndSetVideoMode -> 0x0078a630  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (11), mapped to Steam through trusted rows only:
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x0061a140 CResRef::Constructor_2 -> Steam ?  in candidate callees: n/a
  0x0061a190 CResRef::Constructor_4 -> Steam ?  in candidate callees: n/a
  0x0061a800 CResRef::operator=_4 -> Steam ?  in candidate callees: n/a
  0x006207e0 C2DA::Constructor -> Steam ?  in candidate callees: n/a
  ... 3 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'InventorySound'; 'inventorysnds'
disp sets (* = in both, 0 common): GOG {0x24,0x1c8} | Steam {0x104,0x108}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x10,0x54,0x6c} | Steam {0x110,0x118,0x120}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078a390: 16-byte aligned: YES
  0078a390  55             PUSH EBP
  0078a391  8B EC          MOV EBP,ESP
  0078a393  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078a390
string literals (0):
direct callees (1):
  0x00733540  CExoString::Constructor
struct offsets touched (top 2):
  +0x104 [dec 260]  x1  via ECX
  +0x108 [dec 264]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078a390 FUN_0078a390 size=142 ret=0 callers=1 callees=1

int __fastcall FUN_0078a390(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00733540();
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_00733540();
  *(undefined4 *)(param_1 + 0x108) = 0;
  *(undefined4 *)(param_1 + 0x104) = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078a420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078a420: 16-byte aligned: YES
  0078a420  55             PUSH EBP
  0078a421  8B EC          MOV EBP,ESP
  0078a423  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078a420
string literals (2):
  0x0099F6EC  at 0078a47d  'inventorysnds'
  0x0099F6DC  at 0078a581  'InventorySound'
direct callees (11):
  0x00919723  operator new
  0x00710840  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x00733540  CExoString::Constructor
  0x007107F0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x00734010  CExoString::CStr
  0x00710EB0  (unnamed)
struct offsets touched (top 2):
  +0x1C8 [dec 456]  x3  via EAX,EDX
  +0x24 [dec 36]  x2  via EAX
```

## Card B11a-4-15

GOG function: CClientExoAppInternal::EmptyHitcheckIgnoreList  gog 0x004397f0  callconv=(blank)  param_size_bytes=0  gog_size=91  gog_retn=0
Candidate Steam address: 0x0078d220 (size 91, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078d190:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00439760 CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList -> 0x0078d190  gate_class=REVIEW
- GOG-order neighbour after:  0x00439850 CClientExoAppInternal::SetHitcheckIgnoreList -> 0x0078d280  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00477c30 unnamed -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x007c0300 CExoLinkedList_CAurObject_::RemoveHead -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x190*} | Steam {0x4*,0x10*,0x190*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078d220: 16-byte aligned: YES
  0078d220  55             PUSH EBP
  0078d221  8B EC          MOV EBP,ESP
  0078d223  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d220
string literals (0):
direct callees (4):
  0x007A1720  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078d220 FUN_0078d220 size=91 ret=0 callers=3 callees=4

void __fastcall FUN_0078d220(undefined4 param_1)

{
  undefined4 uVar1;
  int iVar2;
  
  uVar1 = FUN_007a1720(param_1);
  while (iVar2 = FUN_007a1780(), iVar2 == 0) {
    iVar2 = FUN_007f2a90();
    if (iVar2 != 0) {
      FUN_009196fd(iVar2,param_1,iVar2,uVar1);
    }
  }
  return;
}
```

### decoy: 0x0078d190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d190: 16-byte aligned: YES
  0078d190  55             PUSH EBP
  0078d191  8B EC          MOV EBP,ESP
  0078d193  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d190
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0064DDE0  (unnamed)
  0x009196FD  operator delete
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-4-16

GOG function: CClientExoAppInternal::UpdateCamera  gog 0x00438930  callconv=(blank)  param_size_bytes=4  gog_size=2883  gog_retn=4
Candidate Steam address: 0x0078bd20 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.04 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bd80:STR0/CALLEE0/SIG0.105/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438910 CClientExoAppInternal::InitializeInGameGUI -> 0x0078bc50  gate_class=REVIEW
- GOG-order neighbour after:  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> 0x0078cea0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (32), mapped to Steam through trusted rows only:
  0x0040c380 Global::IsJoystickConnected -> Steam ?  in candidate callees: n/a
  0x0040c770 CClientExoApp::GetCaptureEventFile -> Steam ?  in candidate callees: n/a
  0x0040c790 CClientExoApp::GetCaptureEvents -> Steam ?  in candidate callees: n/a
  0x0040c870 CClientExoApp::GetWorldTimer -> Steam 0x0073f3b0  in candidate callees: no
  0x0040c890 CClientExoApp::GetPauseTimer -> Steam ?  in candidate callees: n/a
  0x0040ccf0 CClientExoApp::GetClientOptions -> Steam 0x0072fb00  in candidate callees: no
  0x0040d380 CClientExoApp::GetSWGuiManager -> Steam ?  in candidate callees: n/a
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
  ... 24 more GOG callees
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
GOG strings (1): '%d %c %f %f %f\\n'
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x18,0x20,0x2c,0x40,0x48,0x64,0x68,0x6c,0x6e,0x75,0x80,0xac,...(+7)} | Steam {0x28}
imm sets (* = in both, 1 common): GOG {0x2,0x4*,0x5,0x8,0xc,0x28,0x41,0x44,0x56,0x11c,0x17c,0x3e8} | Steam {0x4*,0x10,0x1c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bd20: 16-byte aligned: YES
  0078bd20  55             PUSH EBP
  0078bd21  8B EC          MOV EBP,ESP
  0078bd23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bd20
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bd20 FUN_0078bd20 size=93 ret=4 callers=1 callees=3

int __thiscall FUN_0078bd20(undefined4 param_1,int param_2)

{
  undefined4 local_c;
  int local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = FUN_00521060();
  while( true ) {
    if (local_8 == 0) {
      return 0;
    }
    if (*(int *)(local_8 + 0x28) == param_2) break;
    local_8 = FUN_0058c370(&local_c);
  }
  return local_8;
}
```

### decoy: 0x0078bd80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B11a-4-17

GOG function: CClientExoAppInternal::ClearCachedCreatures  gog 0x00439530  callconv=(blank)  param_size_bytes=4  gog_size=53  gog_retn=4
Candidate Steam address: 0x0078cf50 (size 53, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078cee0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004394c0 CClientExoAppInternal::GetPlayerCreature -> 0x0078cea0  gate_class=REVIEW
- GOG-order neighbour after:  0x00439570 CClientExoAppInternal::ContinueMessageProcessing -> 0x0078cf90  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x3bc*,0x3c0*} | Steam {0x3bc*,0x3c0*}
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
prologue for 0x0078cf50: 16-byte aligned: YES
  0078cf50  55             PUSH EBP
  0078cf51  8B EC          MOV EBP,ESP
  0078cf53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078cf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C0 [dec 960]  x2  via EAX
  +0x3BC [dec 956]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078cf50 FUN_0078cf50 size=53 ret=4 callers=1 callees=0

void __thiscall FUN_0078cf50(int param_1,int param_2)

{
  if (*(int *)(param_1 + 0x3c0) == param_2) {
    *(undefined4 *)(param_1 + 0x3bc) = 0x7f000000;
    *(undefined4 *)(param_1 + 0x3c0) = 0;
  }
  return;
}
```

### decoy: 0x0078cee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078cee0: 16-byte aligned: YES
  0078cee0  55             PUSH EBP
  0078cee1  8B EC          MOV EBP,ESP
  0078cee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078cee0
string literals (0):
direct callees (1):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
struct offsets touched (top 4):
  +0x20 [dec 32]  x3  via EAX,ECX
  +0x3C0 [dec 960]  x3  via EAX,EDX
  +0x3BC [dec 956]  x2  via EAX
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-18

GOG function: CClientExoAppInternal::RestartMoviesIfNecessary  gog 0x0043a820  callconv=(blank)  param_size_bytes=0  gog_size=109  gog_retn=0
Candidate Steam address: 0x0078e250 (size 109, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078e2c0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a6c0 CClientExoAppInternal::ShutDownToMainMenu -> 0x0078e0f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a890 CClientExoAppInternal::AddObjectModelDirectlyToIgnoreList -> 0x0078e2c0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x0040b960 Global::ChangeResolutionForMovie -> Steam ?  in candidate callees: n/a
  0x0040ba50 Global::RestoreResolutionFromMovie -> Steam ?  in candidate callees: n/a
  0x00444e40 CClientExoAppInternal::WaitForMovie -> Steam ?  in candidate callees: n/a
  0x004604e0 CClientExoAppInternal::HideMouse -> Steam ?  in candidate callees: n/a
  0x00460540 CClientExoAppInternal::ShowMouse -> Steam ?  in candidate callees: n/a
  0x00921230 CExoMoviePlayer::HideFadePanel -> Steam ?  in candidate callees: n/a
  0x00921250 CExoMoviePlayer::GetNumberOfMoviesToPlay -> Steam ?  in candidate callees: n/a
  0x00921280 CExoMoviePlayer::RestartMoviesAsync -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x130*} | Steam {0x130*}
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
prologue for 0x0078e250: 16-byte aligned: YES
  0078e250  55             PUSH EBP
  0078e251  8B EC          MOV EBP,ESP
  0078e253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e250
string literals (0):
direct callees (8):
  0x0093B600  (unnamed)
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0040BE50  Movie::PrepareWindowForMovie
  0x0093B630  (unnamed)
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0093B5E0  (unnamed)
struct offsets touched (top 1):
  +0x130 [dec 304]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e250 FUN_0078e250 size=109 ret=0 callers=1 callees=8

void __fastcall FUN_0078e250(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x130) != 0) {
    iVar1 = FUN_0093b600();
    if (iVar1 != 0) {
      FUN_007b38d0(8);
      FUN_0040be50();
      FUN_0093b630();
      FUN_00798820(1);
      FUN_0040bf40();
      FUN_007b3930(8);
      FUN_0093b5e0();
    }
  }
  return;
}
```

### decoy: 0x0078e2c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e2c0: 16-byte aligned: YES
  0078e2c0  55             PUSH EBP
  0078e2c1  8B EC          MOV EBP,ESP
  0078e2c3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e2c0
string literals (0):
direct callees (4):
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x0078E350  CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList_2
  0x0078E520  CClientExoAppInternal::AddItemModelDirectlyToIgnoreList
  0x0078E5A0  CClientExoAppInternal::AddSimpleObjectModelDirectlyToIgnoreList
struct offsets touched (top 2):
  +0x8 [dec 8]  x1  via EDX
  +0x14 [dec 20]  x1  via EAX
```

## Card B11a-4-19

GOG function: CClientExoAppInternal::SetDebugMode  gog 0x00439c50  callconv=__thiscall  param_size_bytes=4  gog_size=25  gog_retn=4
Candidate Steam address: 0x0078d680 (size 25, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078cf50:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: autolabel:SetCompletionList
- GOG-order neighbour before: 0x00439850 CClientExoAppInternal::SetHitcheckIgnoreList -> 0x0078d280  gate_class=REVIEW
- GOG-order neighbour after:  0x00439c70 CClientExoAppInternal::SetTexturePack -> 0x0078cf50  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x194*} | Steam {0x194*}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078d680: 16-byte aligned: YES
  0078d680  55             PUSH EBP
  0078d681  8B EC          MOV EBP,ESP
  0078d683  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d680
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x194 [dec 404]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078d680 Concurrency::details::UMSThreadScheduler::SetCompletionList size=25 ret=4 callers=1 callees=0

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: void __thiscall Concurrency::details::UMSThreadScheduler::SetCompletionList(struct
   Concurrency::IUMSCompletionList *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::UMSThreadScheduler::SetCompletionList
          (UMSThreadScheduler *this,IUMSCompletionList *param_1)

{
  *(IUMSCompletionList **)(this + 0x194) = param_1;
  return;
}
```

### decoy: 0x0078cf50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078cf50: 16-byte aligned: YES
  0078cf50  55             PUSH EBP
  0078cf51  8B EC          MOV EBP,ESP
  0078cf53  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078cf50
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0x3C0 [dec 960]  x2  via EAX
  +0x3BC [dec 956]  x1  via EDX
```

## Card B11a-4-20

GOG function: CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList  gog 0x00439760  callconv=(blank)  param_size_bytes=4  gog_size=132  gog_retn=4
Candidate Steam address: 0x0078d190 (size 132, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078d100:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004396d0 CClientExoAppInternal::AddObjectToHitcheckIgnoreList -> 0x0078d100  gate_class=REVIEW
- GOG-order neighbour after:  0x004397f0 CClientExoAppInternal::EmptyHitcheckIgnoreList -> 0x0078d220  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x006207c0 CExoLinkedListInternal::Remove_2 -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x190*} | Steam {0x4*,0x10*,0x190*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078d190: 16-byte aligned: YES
  0078d190  55             PUSH EBP
  0078d191  8B EC          MOV EBP,ESP
  0078d193  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d190
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0064DDE0  (unnamed)
  0x009196FD  operator delete
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078d190 FUN_0078d190 size=132 ret=4 callers=1 callees=5

void __thiscall FUN_0078d190(undefined4 param_1,int param_2)

{
  int local_c;
  int *local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = (int *)FUN_00521060();
  while( true ) {
    if (local_c == 0) {
      return;
    }
    if ((local_8 != (int *)0x0) && (*local_8 == param_2)) break;
    local_8 = (int *)FUN_0058c370(&local_c);
  }
  FUN_0064dde0(local_c);
  FUN_009196fd(local_8);
  return;
}
```

### decoy: 0x0078d100 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d100: 16-byte aligned: YES
  0078d100  55             PUSH EBP
  0078d101  8B EC          MOV EBP,ESP
  0078d103  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d100
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x0054AA30  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-4-21

GOG function: CClientExoAppInternal::SetVideoMode  gog 0x0043a1c0  callconv=(blank)  param_size_bytes=5  gog_size=839  gog_retn=8
Candidate Steam address: 0x0078d6a0 (size 1345, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::SetTexturePack | G7 mac: pass(0.14) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.11 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078dbf0:STR0/CALLEE2/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00439c70 CClientExoAppInternal::SetTexturePack -> 0x0078cf50  gate_class=REJECT
- GOG-order neighbour after:  0x0043a510 CClientExoAppInternal::EndLoadBar -> 0x0078df40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00409260 Global::ReInitAurora -> Steam ?  in candidate callees: n/a
  0x004e03a0 CGuiInGame::ResetInterfaceForSize -> Steam 0x007cfca0  in candidate callees: no
  0x0053c430 CSWGuiMainMenu::LoadFromLayout -> Steam ?  in candidate callees: n/a
  0x0053fba0 CSWGuiLoadScreen::Constructor -> Steam ?  in candidate callees: n/a
  0x0090ec80 CSWGuiManager::PanelExists -> Steam ?  in candidate callees: n/a
  0x0090ff00 CSWGuiManager::SetSize -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d0b0 CClientExoApp::SetVideoMode -> Steam 0x0073fbd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x18,0x40,0x1a0,0x274,0x278,0x280} | Steam {0x24,0x12c,0x130,0x1a4}
imm sets (* = in both, 4 common): GOG {0x2*,0x4*,0x8*,0xc*,0x14,0x9c,0xfc,0x6f8} | Steam {0x2*,0x3,0x4*,0x5,0x6,0x7,0x8*,0x9,0xa,0xb,0xc*,0x10,0x54,0xe4}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078d6a0: 16-byte aligned: YES
  0078d6a0  55             PUSH EBP
  0078d6a1  8B EC          MOV EBP,ESP
  0078d6a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078d6a0
string literals (5):
  0x0099F770  at 0078d7f3  'texpacks'
  0x0099F768  at 0078d935  'Texture'
  0x0099F758  at 0078d999  'DynMemRatio'
  0x0099F744  at 0078da02  'TEXTUREPACKS:'
  0x0099F744  at 0078da83  'TEXTUREPACKS:'
direct callees (16):
  0x00733540  CExoString::Constructor
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x00710840  (unnamed)
  0x0071AD40  CRes2DA helper ctor (name, flag)
  0x0071C3D0  C2DA::LoadText
  0x0071D0A0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x0071B1D0  (unnamed)
  0x0071B670  (unnamed)
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00427D90  Global::AurTextureResetAll
  0x00733DF0  CExoString::operator+
  0x0093C7A0  (unnamed)
  0x004218C0  Global::AurTextureSetDynamicMemoryRatio
  0x00427DE0  Global::AurTextureSetMem
struct offsets touched (top 4):
  +0x1A4 [dec 420]  x3  via EAX,ECX
  +0x24 [dec 36]  x2  via EAX,ECX
  +0x130 [dec 304]  x2  via ECX,EDX
  +0x12C [dec 300]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078d6a0 FUN_0078d6a0 size=1345 ret=8 callers=3 callees=16

undefined4 FUN_0078d6a0(byte param_1)

{
  undefined1 uVar1;
  int iVar2;
  undefined1 auStack_10c [4];
  undefined4 uStack_108;
  uint uStack_104;
  undefined1 *puStack_100;
  char *local_fc;
  uint uStack_f8;
  undefined4 local_f4;
  undefined4 local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 *local_c8;
  int local_c4;
  int local_c0;
  int local_bc;
  int local_b8;
  int local_b4;
  undefined4 local_b0;
  undefined4 *local_ac;
  undefined4 *local_a8;
  undefined1 local_a4 [16];
  undefined1 local_94 [16];
  undefined1 local_84 [8];
  undefined1 local_7c [8];
  undefined1 local_74 [8];
  undefined1 local_6c [8];
  undefined4 local_64;
  undefined4 *local_60;
  undefined4 *local_5c;
  undefined4 *local_58;
  undefined4 *local_54;
  undefined1 *local_50;
  int local_4c;
  undefined4 *local_48;
  undefined4 local_44;
  undefined1 local_40 [8];
  undefined4 local_38[5 = byte +0x14];
  undefined4 local_24;
  undefined4 *local_20;
  undefined1 local_1c [8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_009665f7;
  local_10 = ExceptionList;
  uStack_f8 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_fc = (char *)0x78d6d6;
  FUN_00733540();
  local_8 = 0;
  local_fc = (char *)0x78d6e5;
  FUN_00733540();
  local_8._0_1_ = 1;
  local_fc = (char *)0x78d6f1;
  FUN_00733540();
  local_8._0_1_ = 2;
  local_fc = (char *)0x78d6fd;
  FUN_00733540();
  local_8._0_1_ = 3;
  uVar1 = (undefined1)local_8;
  local_8._0_1_ = 3;
  local_20 = (undefined4 *)0x0;
  local_14 = 0;
  local_24 = 0;
  local_38[0] = 0;
/* ... 224 more lines truncated by --max-lines */
```

### decoy: 0x0078dbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078dbf0: 16-byte aligned: YES
  0078dbf0  55             PUSH EBP
  0078dbf1  8B EC          MOV EBP,ESP
  0078dbf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078dbf0
string literals (0):
direct callees (9):
  0x0091C630  (unnamed)
  0x00409750  ChangeDisplayMode/RecreateWindow
  0x00411A80  CSWGuiManager::SetScreenSize
  0x00410800  GuiManager::Contains
  0x00881970  (unnamed)
  0x00919723  operator new
  0x008850B0  (unnamed)
  0x007CFCA0  CGuiInGame::ResetInterfaceForSize
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x274 [dec 628]  x5  via EAX,ECX,EDX
  +0x278 [dec 632]  x3  via EAX,ECX
  +0x280 [dec 640]  x3  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x1A0 [dec 416]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
```

## Card B11a-4-22

GOG function: CClientExoApp::ResetDriveAcceleration  gog 0x0040d700  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740290 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::IsSoundPlayingInDialog | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740260:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d6d0 CClientExoApp::ReattachCreatureToScene -> 0x00740210  gate_class=AUTO
- GOG-order neighbour after:  0x0040d720 CClientExoApp::BarkString -> 0x007404d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004388e0 CClientExoAppInternal::ResetDriveAcceleration -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x007a3d70 Global::ExecuteCommandPlayMovie -> Steam 0x0068e720  in candidate callers: no
  0x007b34f0 CServerExoAppInternal::ExecuteLoadModule -> Steam 0x00530170  in candidate callers: no
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
prologue for 0x00740290: 16-byte aligned: YES
  00740290  55             PUSH EBP
  00740291  8B EC          MOV EBP,ESP
  00740293  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740290
string literals (0):
direct callees (1):
  0x0079BCC0  CClientExoAppInternal::IsSoundPlayingInDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740290 FUN_00740290 size=22 ret=0 callers=1 callees=1
THUNK -> 0x0079BCC0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740290(undefined4 param_1)

{
  FUN_0079bcc0(param_1);
  return;
}
```

### decoy: 0x00740260 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740260: 16-byte aligned: YES
  00740260  55             PUSH EBP
  00740261  8B EC          MOV EBP,ESP
  00740263  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740260
string literals (0):
direct callees (1):
  0x0079BC40  CClientExoAppInternal::BarkString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-23

GOG function: CClientExoAppInternal::GetItemByGameObjectID  gog 0x00438360  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438300 CClientExoAppInternal::GetPlayerByGameObjectID -> -  gate_class=NOCAND
- GOG-order neighbour after:  0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x34} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-24

GOG function: CClientExoAppInternal::InitializeInGameGUI  gog 0x00438910  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x0078bc50 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004388e0 CClientExoAppInternal::ResetDriveAcceleration -> 0x0078bc50  gate_class=REVIEW
- GOG-order neighbour after:  0x00438930 CClientExoAppInternal::UpdateCamera -> 0x0078bd20  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004ceb50 CGuiInGame::Initialize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x40*} | Steam {0x38,0x3c,0x40*,0x2a0}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bc50 FUN_0078bc50 size=111 ret=0 callers=1 callees=4

void __fastcall FUN_0078bc50(int param_1)

{
  if (*(int *)(param_1 + 0x3c) != 0) {
    FUN_0084bda0();
  }
  if (*(int *)(param_1 + 0x38) != 0) {
    FUN_0084b490(param_1,*(undefined4 *)(param_1 + 0x38));
  }
  if (*(int *)(param_1 + 0x40) != 0) {
    FUN_007c7030();
  }
  if (*(int *)(param_1 + 0x2a0) != 0) {
    FUN_00865220();
  }
  return;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-25

GOG function: CClientExoApp::GetPlayerMovementInput  gog 0x0040df60  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00740a00 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::DelayNextEnemyAutoPause | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740a20:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040df30 CClientExoApp::LookUpAndPerformRumbleWithCutOff -> 0x00740a70  gate_class=REVIEW
- GOG-order neighbour after:  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> 0x00761570  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040df80 CClientExoAppInternal::GetPlayerMovementInput -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00780e80 Global::ExecuteCommandCreatureFlourishWeapon -> Steam 0x0066b8a0  in candidate callers: no
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
prologue for 0x00740a00: 16-byte aligned: YES
  00740a00  55             PUSH EBP
  00740a01  8B EC          MOV EBP,ESP
  00740a03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740a00
string literals (0):
direct callees (1):
  0x0079E9A0  CClientExoAppInternal::DelayNextEnemyAutoPause
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740a00 FUN_00740a00 size=22 ret=0 callers=3 callees=1
THUNK -> 0x0079E9A0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00740a00(undefined4 param_1)

{
  FUN_0079e9a0(param_1);
  return;
}
```

### decoy: 0x00740a20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740a20: 16-byte aligned: YES
  00740a20  55             PUSH EBP
  00740a21  8B EC          MOV EBP,ESP
  00740a23  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740a20
string literals (0):
direct callees (1):
  0x0079E9D0  CClientExoAppInternal::ShowTutorialWindow
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-26

GOG function: CClientExoAppInternal::JournalUpdated  gog 0x00431950  callconv=(blank)  param_size_bytes=12  gog_size=45  gog_retn=12
Candidate Steam address: 0x00785330 (size 45, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00785030:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004318e0 CClientExoAppInternal::DestroyWorldJournal -> 0x007852c0  gate_class=REVIEW
- GOG-order neighbour after:  0x00431980 CClientExoAppInternal::ExitProgram -> 0x00785360  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00546e40 CSWCJournal::JournalUpdated -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x38*} | Steam {0x38*}
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
prologue for 0x00785330: 16-byte aligned: YES
  00785330  55             PUSH EBP
  00785331  8B EC          MOV EBP,ESP
  00785333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00785330
string literals (0):
direct callees (1):
  0x0084B630  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00785330 FUN_00785330 size=45 ret=12 callers=1 callees=1

void __thiscall FUN_00785330(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  if (*(int *)(param_1 + 0x38) != 0) {
    FUN_0084b630(param_2,param_3,param_4);
  }
  return;
}
```

### decoy: 0x00785030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785030: 16-byte aligned: YES
  00785030  55             PUSH EBP
  00785031  8B EC          MOV EBP,ESP
  00785033  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00785030
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0084ABF0  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via EAX,EDX
```

## Card B11a-4-27

GOG function: CClientExoAppInternal::GetPlayerCreature  gog 0x004394c0  callconv=(blank)  param_size_bytes=0  gog_size=101  gog_retn=0
Candidate Steam address: 0x0078cea0 (size 57, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078cee0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438930 CClientExoAppInternal::UpdateCamera -> 0x0078bd20  gate_class=REVIEW
- GOG-order neighbour after:  0x00439530 CClientExoAppInternal::ClearCachedCreatures -> 0x0078cf50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callees: no
GOG callers mapped through trusted rows (2 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x0040c910 CClientExoApp::GetPlayerCreature -> Steam 0x0073f450  in candidate callers: no
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x20,0x3bc,0x3c0} | Steam {0x18}
imm sets (* = in both, 0 common): GOG {0x8} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078cea0: 16-byte aligned: YES
  0078cea0  55             PUSH EBP
  0078cea1  8B EC          MOV EBP,ESP
  0078cea3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078cea0
string literals (0):
direct callees (2):
  0x007F3EA0  (unnamed)
  0x007F4220  (unnamed)
struct offsets touched (top 1):
  +0x18 [dec 24]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078cea0 FUN_0078cea0 size=57 ret=0 callers=1 callees=2

void FUN_0078cea0(void)

{
  FUN_007f3ea0(0,0);
  FUN_007f4220(0,0);
  return;
}
```

### decoy: 0x0078cee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078cee0: 16-byte aligned: YES
  0078cee0  55             PUSH EBP
  0078cee1  8B EC          MOV EBP,ESP
  0078cee3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078cee0
string literals (0):
direct callees (1):
  0x0073F550  CClientExoApp::GetCreatureByGameObjectID
struct offsets touched (top 4):
  +0x20 [dec 32]  x3  via EAX,ECX
  +0x3C0 [dec 960]  x3  via EAX,EDX
  +0x3BC [dec 956]  x2  via EAX
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-28

GOG function: CClientExoApp::SetPausedByCombat  gog 0x0040d810  callconv=(blank)  param_size_bytes=12  gog_size=37  gog_retn=12
Candidate Steam address: 0x00740440 (size 36, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoApp::GetCanSendDialog | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740420:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: autolabel:Statistics
- GOG-order neighbour before: 0x0040d7f0 CClientExoApp::GetPausedByCombat -> 0x00740330  gate_class=REVIEW
- GOG-order neighbour after:  0x0040d840 CClientExoApp::ChangeCharacter -> 0x00740380  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00448540 CClientExoAppInternal::SetPausedByCombat -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (11 of 31): the card's A-CALLEE credit may come from these, not from callees
  0x004dae30 CGuiInGame::ShowGalaxyMapGui -> Steam 0x007ca730  in candidate callers: no
  0x004db020 CGuiInGame::HideGalaxyMapGui -> Steam 0x007ca920  in candidate callers: no
  0x004db430 CGuiInGame::HideContainerGui -> Steam 0x007cad30  in candidate callers: no
  0x004dd630 CGuiInGame::HideStoreGui -> Steam 0x007ccf30  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
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
prologue for 0x00740440: 16-byte aligned: YES
  00740440  55             PUSH EBP
  00740441  8B EC          MOV EBP,ESP
  00740443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740440
string literals (0):
direct callees (1):
  0x0079C5B0  CClientExoAppInternal::GetCanSendDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740440 Concurrency::details::SchedulerResourceManagement::Statistics size=36 ret=12 callers=1 callees=1
THUNK -> 0x0079C5B0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    public: virtual void __thiscall
   Concurrency::details::SchedulerResourceManagement::Statistics(unsigned int *,unsigned int
   *,unsigned int *)
   
   Library: Visual Studio 2010 Debug */

void __thiscall
Concurrency::details::SchedulerResourceManagement::Statistics
          (SchedulerResourceManagement *this,uint *param_1,uint *param_2,uint *param_3)

{
  FUN_0079c5b0(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x00740420 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740420: 16-byte aligned: YES
  00740420  55             PUSH EBP
  00740421  8B EC          MOV EBP,ESP
  00740423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740420
string literals (0):
direct callees (1):
  0x0079C550  CClientExoAppInternal::SetCanSendDialog
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-29

GOG function: CClientExoAppInternal::GetTriggerByGameObjectID  gog 0x004384b0  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bc50:STR0/CALLEE0/SIG0.333/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438440 CClientExoAppInternal::GetAreaByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00438520 CClientExoAppInternal::GetProjectileByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x3c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

## Card B11a-4-30

GOG function: CClientExoAppInternal::GetPlaceableByGameObjectID  gog 0x00438590  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bd80:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438520 CClientExoAppInternal::GetProjectileByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00438600 CClientExoAppInternal::GetDoorByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x4c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bd80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

## Card B11a-4-31

GOG function: CClientExoApp::ResetDriveAcceleration  gog 0x0040d700  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007401c0 (size 38, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CClientExoApp::RemoveMoveToModuleMovies | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007401a0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040d6d0 CClientExoApp::ReattachCreatureToScene -> 0x00740210  gate_class=AUTO
- GOG-order neighbour after:  0x0040d720 CClientExoApp::BarkString -> 0x007404d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004388e0 CClientExoAppInternal::ResetDriveAcceleration -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x007a3d70 Global::ExecuteCommandPlayMovie -> Steam 0x0068e720  in candidate callers: no
  0x007b34f0 CServerExoAppInternal::ExecuteLoadModule -> Steam 0x00530170  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 0 common): GOG {} | Steam {0x8,0x2d8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007401c0: 16-byte aligned: YES
  007401c0  55             PUSH EBP
  007401c1  8B EC          MOV EBP,ESP
  007401c3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007401c0
string literals (0):
direct callees (1):
  0x007413E0  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007401c0 FUN_007401c0 size=38 ret=0 callers=1 callees=1
THUNK -> 0x007413E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007401c0(void)

{
  FUN_007413e0(0);
  return;
}
```

### decoy: 0x007401a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007401a0: 16-byte aligned: YES
  007401a0  55             PUSH EBP
  007401a1  8B EC          MOV EBP,ESP
  007401a3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007401a0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-4-32

GOG function: CClientExoAppInternal::ExitProgram  gog 0x00431980  callconv=(blank)  param_size_bytes=0  gog_size=19  gog_retn=0
Candidate Steam address: 0x00785360 (size 19, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: ExitProgram 0x73f290)
- best decoy (neighbour/other candidate) scores: 0x00785330:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431950 CClientExoAppInternal::JournalUpdated -> 0x00785330  gate_class=REVIEW
- GOG-order neighbour after:  0x004319a0 CClientExoAppInternal::CreateModule -> 0x00785380  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040c750 CClientExoApp::ExitProgram -> Steam 0x0073f290  in candidate callers: yes
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
prologue for 0x00785360: 16-byte aligned: YES
  00785360  55             PUSH EBP
  00785361  8B EC          MOV EBP,ESP
  00785363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00785360
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00785360 FUN_00785360 size=19 ret=0 callers=2 callees=1
THUNK -> 0x0000011A (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00785360(void)

{
  PostQuitMessage(0);
  return;
}
```

### decoy: 0x00785330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785330: 16-byte aligned: YES
  00785330  55             PUSH EBP
  00785331  8B EC          MOV EBP,ESP
  00785333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00785330
string literals (0):
direct callees (1):
  0x0084B630  (unnamed)
struct offsets touched (top 1):
  +0x38 [dec 56]  x2  via EAX,ECX
```

## Card B11a-4-33

GOG function: CClientExoAppInternal::ReadAndSetVideoMode  gog 0x00436b80  callconv=(blank)  param_size_bytes=8  gog_size=360  gog_retn=8
Candidate Steam address: 0x0078a630 (size 360, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078a7a0:STR0/CALLEE0/SIG0.158/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00436970 CClientExoAppInternal::LoadItemDropSoundList -> 0x0078a390  gate_class=REVIEW
- GOG-order neighbour after:  0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> 0x0078a7a0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> Steam ?  in candidate callees: n/a
  0x0043a1c0 CClientExoAppInternal::SetVideoMode -> Steam ?  in candidate callees: n/a
  0x0043a510 CClientExoAppInternal::EndLoadBar -> Steam ?  in candidate callees: n/a
  0x0043a5e0 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList -> Steam ?  in candidate callees: n/a
  0x00462c00 CClientOptions::SetFullScreenEnabled -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00608370 CExoIni::Constructor -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}
imm sets (* = in both, 7 common): GOG {0x2*,0x8*,0x20*,0x28*,0x3c*,0x300*,0x400*} | Steam {0x2*,0x8*,0x20*,0x28*,0x3c*,0x300*,0x400*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0078a630: 16-byte aligned: YES
  0078a630  55             PUSH EBP
  0078a631  8B EC          MOV EBP,ESP
  0078a633  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078a630
string literals (0):
direct callees (9):
  0x00733540  CExoString::Constructor
  0x00736240  (unnamed)
  0x0078A7A0  CClientExoAppInternal::ReadVideoModeSettings
  0x007B5E90  (unnamed)
  0x0078E010  CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList
  0x0078DF40  CClientExoAppInternal::EndLoadBar
  0x0078DBF0  (unnamed)
  0x007362C0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078a630 FUN_0078a630 size=360 ret=8 callers=1 callees=9

void __thiscall FUN_0078a630(undefined4 param_1,undefined4 param_2,int param_3)

{
  int iVar1;
  undefined4 local_2c;
  undefined1 local_25;
  undefined4 local_24;
  int local_20;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00966390;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  param_2 = 0;
  local_24 = 0;
  local_2c = 0;
  local_14 = 0;
  local_25 = 0;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0;
  FUN_00736240();
  local_8 = CONCAT31(local_8._1_3_,1);
  DAT_00a106b8 = param_3 != 0;
  iVar1 = FUN_0078a7a0(&local_24,&local_2c,&local_25,&local_14);
  FUN_007b5e90(param_3);
  local_20 = FUN_0078e010(local_24,local_2c,local_25,local_14);
  if (((iVar1 == 0) && (param_2 == 0)) || (local_20 == -1)) {
    local_20 = FUN_0078df40(0x400,0x300,0x20,0x3c);
  }
  if (param_3 == 0) {
    FUN_0078dbf0(local_20,1);
  }
  else {
    FUN_0078dbf0(local_20,2);
  }
  if (param_2 != 0) {
    FUN_0078a7a0(&local_24,&local_2c,&local_25,&local_14);
    DAT_009f42a4 = local_24;
    DAT_009f42a8 = local_2c;
    DAT_009f42b0 = local_25;
  }
  local_8 = local_8 & 0xffffff00;
  FUN_007362c0();
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0078a7a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078a7a0: 16-byte aligned: YES
  0078a7a0  55             PUSH EBP
  0078a7a1  8B EC          MOV EBP,ESP
  0078a7a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078a7a0
string literals (8):
  0x009982A4  at 0078a7ec  'Width'
  0x00986C84  at 0078a800  'Graphics Options'
  0x00986B98  at 0078a814  '.\\\\swkotor2.ini'
  0x00986C84  at 0078a8be  'Graphics Options'
  0x00986B98  at 0078a8d2  '.\\\\swkotor2.ini'
  0x0099F6FC  at 0078a96b  'RefreshRate'
  0x00986C84  at 0078a97f  'Graphics Options'
  0x00986B98  at 0078a993  '.\\\\swkotor2.ini'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x0093CA00  (unnamed)
  0x00402540  (unnamed)
  0x0091C630  (unnamed)
  0x0093C9C0  (unnamed)
  0x0093CA30  (unnamed)
  0x007362C0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

## Card B11a-4-34

GOG function: CClientExoAppInternal::GetPauseState  gog 0x00430a40  callconv=(blank)  param_size_bytes=1  gog_size=29  gog_retn=4
Candidate Steam address: 0x00783d80 (size 49, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::StallServerCode | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.17 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00783dc0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004309a0 Quaternion::GetYAxis  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00430a60 CClientExoAppInternal::GetActivePauseState -> 0x00784440  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1cc} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x10}

### abi
```
GOG param bytes: 1; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(1/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 1 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00783d80: 16-byte aligned: YES
  00783d80  55             PUSH EBP
  00783d81  8B EC          MOV EBP,ESP
  00783d83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00783d80
string literals (0):
direct callees (1):
  0x007335F0  CExoString::CExoString_copy
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00783d80 FUN_00783d80 size=49 ret=4 callers=4 callees=1

undefined4 __thiscall FUN_00783d80(int param_1,undefined4 param_2)

{
  FUN_007335f0(param_1 + 0x10);
  return param_2;
}
```

### decoy: 0x00783dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00783dc0: 16-byte aligned: YES
  00783dc0  55             PUSH EBP
  00783dc1  8B EC          MOV EBP,ESP
  00783dc3  83 EC 34       SUB ESP,0x34
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00783dc0
string literals (0):
direct callees (6):
  0x00726F80  (unnamed)
  0x007AACE0  CSWCArea::GetRoomIndex
  0x007AAB90  CSWCArea::GetRoom_2
  0x0073F750  ClientApp::GetGuiInGame
  0x0070BC00  (unnamed)
  0x0070BC30  (unnamed)
struct offsets touched (top 9):
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x30 [dec 48]  x1  via ECX
  +0x34 [dec 52]  x1  via ECX
  +0x44 [dec 68]  x1  via ECX
  +0x7C [dec 124]  x1  via EDX
  +0x84 [dec 132]  x1  via EDX
  +0xD8 [dec 216]  x1  via EDX
  +0x188 [dec 392]  x1  via ECX
```

## Card B11a-4-35

GOG function: CClientExoAppInternal::ReadVideoModeSettings  gog 0x00436cf0  callconv=(blank)  param_size_bytes=16  gog_size=1172  gog_retn=16
Candidate Steam address: 0x0078a7a0 (size 1172, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.88) | G8 mac_rotation: pass
- anchors: A-STR 1 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR,SIG  (content level C2)
- evidence summary: STR 'RefreshRate'@0099f6fc; SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0078ac40:STR0/CALLEE0/SIG0.19/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00436b80 CClientExoAppInternal::ReadAndSetVideoMode -> 0x0078a630  gate_class=REVIEW
- GOG-order neighbour after:  0x00437190 CClientExoAppInternal::StopServices -> 0x0078ac40  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (13), mapped to Steam through trusted rows only:
  0x00402500 GraphicsSettings::GetQualityLevel -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605680 CExoString::Constructor_2 -> Steam 0x00733570  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00608370 CExoIni::Constructor -> Steam ?  in candidate callees: n/a
  0x006083f0 CExoIni::Destructor -> Steam ?  in candidate callees: n/a
  0x00608450 CExoIni::ReadIniEntry -> Steam ?  in candidate callees: n/a
  0x00906fe0 CAuroraInterface::IsWithinDesktopResolution -> Steam ?  in candidate callees: n/a
  ... 5 more GOG callees
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (5): '.\\\\swkotor2.ini'; 'Graphics Options'; 'RefreshRate'; 'Height' ...
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 18 common): GOG {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0x10*,0x20*,0x3c*,0x9c*,...(+4)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6*,0x7*,0x8*,0x9*,0xa*,0xc*,0x10*,0x20*,0x3c*,0x9c*,...(+4)}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0078a7a0: 16-byte aligned: YES
  0078a7a0  55             PUSH EBP
  0078a7a1  8B EC          MOV EBP,ESP
  0078a7a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078a7a0
string literals (8):
  0x009982A4  at 0078a7ec  'Width'
  0x00986C84  at 0078a800  'Graphics Options'
  0x00986B98  at 0078a814  '.\\\\swkotor2.ini'
  0x00986C84  at 0078a8be  'Graphics Options'
  0x00986B98  at 0078a8d2  '.\\\\swkotor2.ini'
  0x0099F6FC  at 0078a96b  'RefreshRate'
  0x00986C84  at 0078a97f  'Graphics Options'
  0x00986B98  at 0078a993  '.\\\\swkotor2.ini'
direct callees (13):
  0x00733540  CExoString::Constructor
  0x00736240  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x00736320  (unnamed)
  0x00733780  CExoString::~CExoString
  0x0091CF18  (unnamed)
  0x0093CA00  (unnamed)
  0x00402540  (unnamed)
  0x0091C630  (unnamed)
  0x0093C9C0  (unnamed)
  0x0093CA30  (unnamed)
  0x007362C0  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078a7a0 FUN_0078a7a0 size=1172 ret=16 callers=2 callees=14

void FUN_0078a7a0(uint *param_1,uint *param_2,char *param_3,uint *param_4)

{
  int iVar1;
  BOOL BVar2;
  uint local_134;
  uint local_130;
  uint local_12c;
  undefined1 local_124 [8];
  undefined1 local_11c [8];
  undefined1 local_114 [8];
  undefined1 local_10c [8];
  undefined1 local_104 [8];
  undefined1 local_fc [8];
  undefined1 local_f4 [8];
  undefined1 local_ec [8];
  undefined1 local_e4 [8];
  uint local_dc;
  DEVMODEA local_d8;
  uint local_34;
  uint local_30;
  uint local_2c;
  DWORD local_28;
  undefined4 local_24;
  int local_1c[2 = byte +0x8];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00966423;
  local_10 = ExceptionList;
  local_34 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(local_34);
  local_8 = 0;
  FUN_00736240();
  local_8._0_1_ = 1;
  FUN_00733570("Width");
  local_8._0_1_ = 2;
  FUN_00733570("Graphics Options");
  local_8._0_1_ = 3;
  FUN_00733570(".\\swkotor2.ini");
  local_8._0_1_ = 4;
  local_24 = FUN_00736320(local_1c,local_f4,local_ec,local_e4);
  local_8._0_1_ = 3;
  FUN_00733780();
  local_8._0_1_ = 2;
  FUN_00733780();
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_00733780();
  if (local_1c[0] == 0) {
    local_12c = 0;
  }
  else {
    local_12c = FUN_0091cf18(local_1c[0]);
  }
  *param_1 = local_12c;
  FUN_00733570("Height");
  local_8._0_1_ = 5;
  FUN_00733570("Graphics Options");
  local_8._0_1_ = 6;
  FUN_00733570(".\\swkotor2.ini");
  local_8._0_1_ = 7;
  FUN_00736320(local_1c,local_10c,local_104,local_fc);
  local_8._0_1_ = 6;
  FUN_00733780();
  local_8._0_1_ = 5;
  FUN_00733780();
  local_8 = CONCAT31(local_8._1_3_,1);
  FUN_00733780();
  if (local_1c[0] == 0) {
    local_130 = 0;
  }
  else {
    local_130 = FUN_0091cf18(local_1c[0]);
  }
  *param_2 = local_130;
/* ... 85 more lines truncated by --max-lines */
```

### decoy: 0x0078ac40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078ac40: 16-byte aligned: YES
  0078ac40  55             PUSH EBP
  0078ac41  8B EC          MOV EBP,ESP
  0078ac43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078ac40
string literals (11):
  0x0099F728  at 0078b538  'PORTRAITS:'
  0x00993EF0  at 0078b573  'SERVERVAULT:'
  0x0099F644  at 0078b5ae  'HD0:CHITIN'
  0x0099415C  at 0078b5f8  'OVERRIDE:'
  0x0099F62C  at 0078b633  'HD0:MOVIES'
  0x0099F718  at 0078b66e  'HD0:STREAMMUSIC'
  0x0099F708  at 0078b6a9  'HD0:STREAMVOICE'
  0x0099F690  at 0078b6e4  'TEMPCLIENT:'
  0x0099F624  at 0078b71f  'GLOBAL'
  0x0099F618  at 0078b74c  'RIMS:GLOBAL'
  0x00994E68  at 0078b787  'RIMS:'
direct callees (35):
  0x007A1590  CClientExoAppInternal::UnloadSuperModelSet
  0x00798CD0  CClientExoAppInternal::StopMovie
  0x00410800  GuiManager::Contains
  0x0085F3D0  (unnamed)
  0x009196FD  operator delete
  0x0053D780  (unnamed)
  0x007852C0  CClientExoAppInternal::DestroyWorldJournal
  0x0078BB30  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x007BDBC0  (unnamed)
  0x00410380  (unnamed)
  0x007A2360  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x0078BBC0  (unnamed)
  0x00537480  (unnamed)
  0x00704480  CNetLayer::Unlink (mapper: EndServerMode)
  0x00703D70  CNetLayer dtor
  0x0070B080  StreamingSource scalar delete
  0x0070B5A0  (unnamed)
struct offsets touched (top 10):
  +0x1C4 [dec 452]  x6  via EAX,ECX,EDX
  +0x10 [dec 16]  x4  via EAX,ECX,EDX
  +0x40 [dec 64]  x4  via EAX,ECX,EDX
  +0x274 [dec 628]  x4  via EAX,ECX
  +0x280 [dec 640]  x4  via ECX,EDX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX
  +0xC [dec 12]  x3  via EAX,EDX
  +0x14 [dec 20]  x3  via EAX,EDX
  +0x18 [dec 24]  x3  via EAX,ECX
```

## Card B11a-4-36

GOG function: CClientExoAppInternal::GetAreaOfEffectByGameObjectID  gog 0x00438670  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bd80 (size 102, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bdf0:STR0/CALLEE0/SIG0.75/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438600 CClientExoAppInternal::GetDoorByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x004386e0 CClientExoAppInternal::GetSoundObjectByGameObjectID -> 0x0078bd80  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x8*,0x14*,0x54} | Steam {0x8*,0x14*,0x34}
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
prologue for 0x0078bd80: 16-byte aligned: YES
  0078bd80  55             PUSH EBP
  0078bd81  8B EC          MOV EBP,ESP
  0078bd83  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bd80
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x34 [dec 52]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bd80 FUN_0078bd80 size=102 ret=4 callers=2 callees=1

undefined4 __thiscall FUN_0078bd80(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 uVar2;
  int *local_8;
  
  local_8 = (int *)0x0;
  uVar2 = *(undefined4 *)(param_1 + 0x14);
  cVar1 = FUN_0053dfb0(param_2,&local_8);
  if (((cVar1 == DAT_0099447f) && (local_8 != (int *)0x0)) && ((char)local_8[2 = byte +0x8] == DAT_00994475)) {
    uVar2 = (**(code **)(*local_8 + 0x34))(param_1,uVar2);
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}
```

### decoy: 0x0078bdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bdf0: 16-byte aligned: YES
  0078bdf0  55             PUSH EBP
  0078bdf1  8B EC          MOV EBP,ESP
  0078bdf3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bdf0
string literals (0):
direct callees (1):
  0x0053DFB0  CGameObjectArray::GetGameObject
struct offsets touched (top 3):
  +0x8 [dec 8]  x1  via ECX
  +0x14 [dec 20]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
```

## Card B11a-4-37

GOG function: CClientExoAppInternal::CancelMovie  gog 0x0044cb00  callconv=(blank)  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x0079fa50 (size 487, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::UpdateLoadingScreenHint | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0079f770:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ca90 CClientExoAppInternal::SetMouseMode -> 0x0079fc40  gate_class=REJECT
- GOG-order neighbour after:  0x0044cb30 CClientExoAppInternal::SetLastTarget -> 0x007a0530  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009211a0 CExoMoviePlayer::CancelMovie -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040e420 CClientExoApp::CancelMovie -> Steam 0x00740f60  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x130} | Steam {0x4,0x8,0x488,0x48c}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x4,0x58,0x2710,0xfffffffe}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0079fa50: 16-byte aligned: YES
  0079fa50  55             PUSH EBP
  0079fa51  8B EC          MOV EBP,ESP
  0079fa53  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0079fa50
string literals (0):
direct callees (8):
  0x0079CF70  (unnamed)
  0x00736200  (unnamed)
  0x0051BDB0  CServerExoApp::GetModuleName
  0x00919F10  _strlen
  0x00733780  CExoString::~CExoString
  0x00733570  CExoString::CExoString_char
  0x0079FC40  (unnamed)
  0x0079CF40  CClientExoAppInternal::SetLoadScreenHint
struct offsets touched (top 4):
  +0x488 [dec 1160]  x4  via ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x48C [dec 1164]  x2  via ECX,EDX
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0079fa50 FUN_0079fa50 size=487 ret=0 callers=1 callees=8

void __fastcall FUN_0079fa50(int param_1)

{
  bool bVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  char local_60;
  size_t local_54;
  undefined1 local_38 [8];
  undefined1 local_30 [8];
  undefined1 local_28 [11];
  char local_1d;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00967b20;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  bVar1 = false;
  iVar2 = FUN_0079cf70(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  if (iVar2 == 0) {
    ExceptionList = local_10;
    return;
  }
  local_18 = FUN_00736200();
  local_14 = *(int *)(param_1 + 0x48c);
  if (local_14 == 0) {
    local_14 = local_18;
  }
  local_1c = local_18 - local_14;
  *(int *)(param_1 + 0x48c) = local_18;
  *(int *)(param_1 + 0x488) = *(int *)(param_1 + 0x488) + local_1c;
  if (*(uint *)(param_1 + 0x488) < 10000) {
    ExceptionList = local_10;
    return;
  }
  *(undefined4 *)(param_1 + 0x488) = 0;
  if (*(int *)(DAT_00a1b4a4 + 8) != 0) {
    piVar3 = (int *)FUN_0051bdb0(local_28);
    bVar1 = true;
    if (*piVar3 == 0) {
      local_54 = 0;
    }
    else {
      local_54 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*piVar3);
    }
    if (0 < (int)local_54) {
      local_60 = '\0';
      goto LAB_0079fb7f;
    }
  }
  local_60 = '\x01';
LAB_0079fb7f:
  local_1d = local_60;
  if (bVar1) {
    FUN_00733780();
  }
  if (local_1d == '\0') {
    uVar4 = FUN_0051bdb0(local_38);
    local_8 = 1;
    uVar4 = FUN_0079fc40(uVar4,2);
    FUN_0079cf40(uVar4);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    FUN_00733570(&DAT_00986925);
    local_8 = 0;
    uVar4 = FUN_0079fc40(local_30,1);
    FUN_0079cf40(uVar4);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
/* ... 3 more lines truncated by --max-lines */
```

### decoy: 0x0079f770 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0079f770: 16-byte aligned: YES
  0079f770  55             PUSH EBP
  0079f771  8B EC          MOV EBP,ESP
  0079f773  83 EC 78       SUB ESP,0x78
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0079f770
string literals (0):
direct callees (9):
  0x0051C8F0  CServerExoApp::GetPlayerCreatureId
  0x0051C600  CServerExoApp::GetClientObjectByObjectId
  0x007E5D80  (unnamed)
  0x0051C100  CServerExoApp::GetCreatureByGameObjectID
  0x00797F30  CClientExoAppInternal::ChangeCharacter
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x007A12E0  CClientExoAppInternal::PlayCharacterSelectSound
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E6BE0  (unnamed)
struct offsets touched (top 7):
  +0x4 [dec 4]  x4  via ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x38 [dec 56]  x3  via EAX
  +0x270 [dec 624]  x3  via EAX,EDX
  +0x94 [dec 148]  x2  via EAX
  +0x9C [dec 156]  x2  via EDX
  +0x11AC [dec 4524]  x2  via EAX
```

## Card B11a-4-38

GOG function: CClientExoAppInternal::ShutDownMoviePlayer  gog 0x004381a0  callconv=(blank)  param_size_bytes=4  gog_size=129  gog_retn=4
Candidate Steam address: 0x0078bcc0 (size 93, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bc50:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438080 CServerInfo::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00438230 CClientExoAppInternal::CleanUpModuleServices -> 0x0078c170  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4}
imm sets (* = in both, 1 common): GOG {0x4*,0xc,0x14,0x30} | Steam {0x4*,0x10,0x1c}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bcc0: 16-byte aligned: YES
  0078bcc0  55             PUSH EBP
  0078bcc1  8B EC          MOV EBP,ESP
  0078bcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bcc0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bcc0 FUN_0078bcc0 size=93 ret=4 callers=1 callees=3

int __thiscall FUN_0078bcc0(undefined4 param_1,int param_2)

{
  undefined4 local_c;
  int local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = FUN_00521060();
  while( true ) {
    if (local_8 == 0) {
      return 0;
    }
    if (*(int *)(local_8 + 4) == param_2) break;
    local_8 = FUN_0058c370(&local_c);
  }
  return local_8;
}
```

### decoy: 0x0078bc50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

## Card B11a-4-39

GOG function: CClientExoApp::GetDisplayCharacterForKey  gog 0x0040e7c0  callconv=(blank)  param_size_bytes=8  gog_size=53  gog_retn=8
Candidate Steam address: 0x00741300 (size 53, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741030:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e750 CClientExoApp::GetInFreeLook -> 0x00741290  gate_class=AUTO
- GOG-order neighbour after:  0x0040e800 CClientExoApp::GetSoundPausedByCombat -> 0x00741340  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d980 CClientExoAppInternal::GetDisplayCharacterForKey -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00741300: 16-byte aligned: YES
  00741300  55             PUSH EBP
  00741301  8B EC          MOV EBP,ESP
  00741303  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741300
string literals (0):
direct callees (1):
  0x007A1380  CClientExoAppInternal::GetDisplayCharacterForKey
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00741300 FUN_00741300 size=53 ret=8 callers=2 callees=1

undefined4 FUN_00741300(undefined4 param_1,undefined4 param_2)

{
  FUN_007a1380(param_1,param_2);
  return param_1;
}
```

### decoy: 0x00741030 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00741030: 16-byte aligned: YES
  00741030  55             PUSH EBP
  00741031  8B EC          MOV EBP,ESP
  00741033  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00741030
string literals (0):
direct callees (1):
  0x007A1130  CClientExoAppInternal::StartCreditSequence
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-4-40

GOG function: CClientExoAppInternal::MovieWindowActivated  gog 0x0044d710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007a10f0 (size 30, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::MovieWindowDeactivated | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a0fb0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d6f0 CClientExoAppInternal::MovieWindowDeactivated -> 0x007a10f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044d730 CClientExoAppInternal::StartCreditSequence -> 0x007a1130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ab80 CExoSound::ResumeMovieStreamingSounds -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x007a10f0: 16-byte aligned: YES
  007a10f0  55             PUSH EBP
  007a10f1  8B EC          MOV EBP,ESP
  007a10f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a10f0
string literals (0):
direct callees (1):
  0x0070BC60  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a10f0 FUN_007a10f0 size=30 ret=0 callers=1 callees=1
THUNK -> 0x0070BC60 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_007a10f0(void)

{
  FUN_0070bc60(DAT_0099c2bc,0);
  return;
}
```

### decoy: 0x007a0fb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a0fb0: 16-byte aligned: YES
  007a0fb0  55             PUSH EBP
  007a0fb1  8B EC          MOV EBP,ESP
  007a0fb3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a0fb0
string literals (0):
direct callees (15):
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x0072FA60  (unnamed)
  0x00797D90  CClientExoAppInternal::SetInputActive
  0x0072F9F0  (unnamed)
  0x0070BC60  (unnamed)
  0x007B38D0  CClientExoAppInternal::HideMouse
  0x0040BE50  Movie::PrepareWindowForMovie
  0x0093B5C0  (unnamed)
  0x0093B590  (unnamed)
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0093B5E0  (unnamed)
  0x007413E0  (unnamed)
  0x006169E0  (unnamed)
struct offsets touched (top 3):
  +0x130 [dec 304]  x3  via ECX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x1F8 [dec 504]  x1  via EAX
```

## Card B11a-4-41

GOG function: CClientExoAppInternal::DestroyQuestJournal_2  gog 0x00431750  callconv=(blank)  param_size_bytes=0  gog_size=146  gog_retn=0
Candidate Steam address: 0x00785130 (size 146, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00784740:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004316e0 CClientExoAppInternal::DestroyQuestJournal -> 0x007850c0  gate_class=AUTO
- GOG-order neighbour after:  0x004317f0 CClientExoAppInternal::AddToPauseExclusionList_2 -> 0x007851d0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044de70 CClientExoAppInternal::IsMoviePlaying_2 -> Steam ?  in candidate callees: n/a
  0x005e2810 CExoArrayList_CMusicItemInfo_::Destructor -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x8*,0xc*,0x20*,0x44*} | Steam {0x2*,0x3*,0x8*,0xc*,0x20*,0x44*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00785130: 16-byte aligned: YES
  00785130  55             PUSH EBP
  00785131  8B EC          MOV EBP,ESP
  00785133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00785130
string literals (0):
direct callees (3):
  0x007A1740  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00785130 FUN_00785130 size=146 ret=0 callers=1 callees=3

void __fastcall FUN_00785130(int param_1)

{
  uint uVar1;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_009658a3;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_007a1740(uVar1,param_1);
  local_8._1_3_ = (uint3)((uint)local_8 >> 8);
  local_8._0_1_ = 2;
  Free(uVar1,param_1,param_1 + 0x20);
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_006d2cb0();
  local_8 = 3;
  Free();
  local_8 = 0xffffffff;
  FUN_006d2cb0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00784740 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00784740: 16-byte aligned: YES
  00784740  55             PUSH EBP
  00784741  8B EC          MOV EBP,ESP
  00784743  81 EC 8C 01 00 00 SUB ESP,0x18c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00784740
string literals (0):
direct callees (16):
  0x00784420  (unnamed)
  0x0051B050  CWorldTimer::PauseWorldTimer
  0x0051A860  (unnamed)
  0x00784440  CClientExoAppInternal::GetActivePauseState
  0x0073F430  CClientExoApp::GetModule
  0x0051AE30  CWorldTimer::ResetTimer
  0x0051A920  (unnamed)
  0x0051B0A0  CWorldTimer::UnpauseWorldTimer
  0x00726F80  (unnamed)
  0x007845B0  CClientExoAppInternal::ClearPauseExclusionList
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x00784680  CClientExoAppInternal::AddToPauseExclusionList
  0x0058C370  (unnamed)
  0x0072F5F0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 10):
  +0x28 [dec 40]  x15  via EAX,ECX,EDX
  +0x24 [dec 36]  x13  via EAX,ECX,EDX
  +0x2C [dec 44]  x12  via EAX,ECX,EDX
  +0x30 [dec 48]  x12  via EAX,ECX,EDX
  +0x14 [dec 20]  x8  via EAX,ECX,EDX
  +0x18 [dec 24]  x8  via EAX,ECX,EDX
  +0x48 [dec 72]  x7  via EAX,ECX
  +0x188 [dec 392]  x6  via EAX,ECX,EDX
  +0x1CC [dec 460]  x4  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,EDX
```

## Card B11a-4-42

GOG function: CClientExoAppInternal::MovieWindowActivated  gog 0x0044d710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x007a12c0 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: review:held_by=CClientExoAppInternal::StopCreditSequence | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a1130:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044d6f0 CClientExoAppInternal::MovieWindowDeactivated -> 0x007a10f0  gate_class=REVIEW
- GOG-order neighbour after:  0x0044d730 CClientExoAppInternal::StartCreditSequence -> 0x007a1130  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0063ab80 CExoSound::ResumeMovieStreamingSounds -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4c0}
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
prologue for 0x007a12c0: 16-byte aligned: YES
  007a12c0  55             PUSH EBP
  007a12c1  8B EC          MOV EBP,ESP
  007a12c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a12c0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4C0 [dec 1216]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007a12c0 FUN_007a12c0 size=24 ret=0 callers=1 callees=0

void __fastcall FUN_007a12c0(int param_1)

{
  *(undefined4 *)(param_1 + 0x4c0) = 0;
  return;
}
```

### decoy: 0x007a1130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a1130: 16-byte aligned: YES
  007a1130  55             PUSH EBP
  007a1131  8B EC          MOV EBP,ESP
  007a1133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a1130
string literals (0):
direct callees (6):
  0x00919723  operator new
  0x00892090  (unnamed)
  0x00410800  GuiManager::Contains
  0x00410530  CSWGuiManager::AddPanel
  0x00726F80  (unnamed)
  0x00890260  (unnamed)
struct offsets touched (top 6):
  +0x48 [dec 72]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x2  via ECX,EDX
  +0x1E0 [dec 480]  x2  via EAX,ECX
  +0x4C0 [dec 1216]  x2  via EAX
  +0x4C4 [dec 1220]  x1  via EAX
```

## Card B11a-4-43

GOG function: CClientExoAppInternal::AddToPauseExclusionList_2  gog 0x004317f0  callconv=(blank)  param_size_bytes=0  gog_size=86  gog_retn=0
Candidate Steam address: 0x007851d0 (size 86, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00785130:STR0/CALLEE0/SIG0.083/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431750 CClientExoAppInternal::DestroyQuestJournal_2 -> 0x00785130  gate_class=REVIEW
- GOG-order neighbour after:  0x00431850 CClientExoAppInternal::CreateWorldJournal -> 0x00785230  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0044de70 CClientExoAppInternal::IsMoviePlaying_2 -> Steam ?  in candidate callees: n/a
  0x0060b670 CExoArrayList_T_::Destructor -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 4): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007851d0: 16-byte aligned: YES
  007851d0  55             PUSH EBP
  007851d1  8B EC          MOV EBP,ESP
  007851d3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007851d0
string literals (0):
direct callees (2):
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007851d0 FUN_007851d0 size=86 ret=0 callers=4 callees=2

void __fastcall FUN_007851d0(undefined4 param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_009658c8;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  Free(DAT_00a11f20 ^ (uint)&stack0xfffffffc,param_1);
  local_8 = 0xffffffff;
  FUN_006d2cb0();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00785130 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785130: 16-byte aligned: YES
  00785130  55             PUSH EBP
  00785131  8B EC          MOV EBP,ESP
  00785133  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00785130
string literals (0):
direct callees (3):
  0x007A1740  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x006D2CB0  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-4-44

GOG function: CClientExoApp::MovieWindowDeactivated  gog 0x0040e710  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00741250 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741230:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e6f0 CClientExoApp::PlayMovieQueue -> 0x00741230  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e730 CClientExoApp::MovieWindowActivated -> 0x00741270  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d6f0 CClientExoAppInternal::MovieWindowDeactivated -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00741250: 16-byte aligned: YES
  00741250  55             PUSH EBP
  00741251  8B EC          MOV EBP,ESP
  00741253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741250
string literals (0):
direct callees (1):
  0x007A10F0  CClientExoAppInternal::MovieWindowDeactivated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00741250 FUN_00741250 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007A10F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00741250(undefined4 param_1)

{
  FUN_007a10f0(param_1);
  return;
}
```

### decoy: 0x00741230 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00741230: 16-byte aligned: YES
  00741230  55             PUSH EBP
  00741231  8B EC          MOV EBP,ESP
  00741233  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00741230
string literals (0):
direct callees (1):
  0x007A0F50  CClientExoAppInternal::PlayMovieQueue
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via ECX
```

## Card B11a-4-45

GOG function: CClientExoAppInternal::GetPlayerByPlayerID  gog 0x004382a0  callconv=(blank)  param_size_bytes=4  gog_size=93  gog_retn=4
Candidate Steam address: 0x0078c350 (size 2883, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078c330:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438230 CClientExoAppInternal::CleanUpModuleServices -> 0x0078c170  gate_class=REVIEW
- GOG-order neighbour after:  0x00438300 CClientExoAppInternal::GetPlayerByGameObjectID -> -  gate_class=NOCAND

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x8,0x18,0x20,0x2c,0x40,0x48,0x64,0x68,0x6c,0x6e,0x75,0x80,0xac,...(+7)}
imm sets (* = in both, 1 common): GOG {0x4*,0x10,0x1c} | Steam {0x2,0x4*,0x5,0x8,0xc,0x28,0x41,0x44,0x56,0x11c,0x17c,0x3e8}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078c350: 16-byte aligned: YES
  0078c350  55             PUSH EBP
  0078c351  8B EC          MOV EBP,ESP
  0078c353  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078c350
string literals (1):
  0x0099F734  at 0078cdcc  '%d %c %f %f %f\\n'
direct callees (32):
  0x0073F3D0  CClientExoApp::GetPauseTimer
  0x0051A940  (unnamed)
  0x0072FB00  CClientExoApp::GetClientOptions
  0x007BBFF0  (unnamed)
  0x0072FD70  (unnamed)
  0x0072F9D0  (unnamed)
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0091C860  _ftol2_sse
  0x0072E840  (unnamed)
  0x0072E850  (unnamed)
  0x0073EEE0  (unnamed)
  0x0072FDE0  (unnamed)
  0x007F3DE0  (unnamed)
  0x007F4220  (unnamed)
  0x007F3EA0  (unnamed)
  0x007F3C70  (unnamed)
  0x007F4550  (unnamed)
  0x0078CEE0  (unnamed)
  0x007FAAE0  (unnamed)
  0x007F4610  (unnamed)
struct offsets touched (top 10):
  +0x18 [dec 24]  x21  via EAX,ECX,EDX
  +0x4 [dec 4]  x16  via EAX,ECX,EDX
  +0xAC [dec 172]  x6  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x6C [dec 108]  x3  via ECX,EDX
  +0x80 [dec 128]  x3  via EAX,EDX
  +0x2C [dec 44]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via ECX,EDX
  +0x48 [dec 72]  x2  via ECX,EDX
  +0x75 [dec 117]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0078c350 FUN_0078c350 size=2883 ret=4 callers=1 callees=32

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0078c350(int param_1)

{
  longlong lVar1;
  char cVar2;
  int iVar3;
  float *pfVar4;
  undefined4 uVar5;
  float10 fVar6;
  float10 fVar7;
  float10 fVar8;
  undefined8 uVar9;
  double dVar10;
  float local_158;
  int local_150;
  int local_14c;
  float local_148;
  float local_144;
  float local_140;
  undefined1 local_90 [8];
  float local_88;
  undefined4 local_84;
  float local_80;
  int local_7c;
  float local_78;
  int *local_74;
  uint local_70;
  float local_6c;
  float local_68;
  undefined4 local_64;
  int local_60;
  int local_5c;
  float local_58;
  float local_54;
  uint local_50;
  undefined1 local_4c [12];
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  uint local_2c;
  uint local_28;
  undefined4 local_24;
  uint local_20;
  undefined1 local_19;
  int local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096655b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x18) != 0) {
    FUN_0073f3d0(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    lVar1 = FUN_0051a940();
    local_30 = (float)((float10)lVar1 / (float10)_DAT_00996518);
    local_19 = DAT_00a30828;
    DAT_00a30828 = 0;
    local_74 = *(int **)(*(int *)(param_1 + 0x18) + 0x40);
    local_84 = *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x18) + 0x48) + 0x188);
    local_38 = 0;
    FUN_0072fb00();
    fVar6 = (float10)FUN_007bbff0();
    local_88 = (float)fVar6;
    local_34 = 0;
    local_54 = 0.0;
    FUN_0072fd70();
    iVar3 = FUN_0072fb00();
    if ((*(uint *)(iVar3 + 8) >> 4 & 1) != 0) {
      local_54 = local_54 * (float)_DAT_00986d98;
    }
    fVar6 = (float10)FUN_0072f9d0();
    if ((float10)_DAT_00986d98 <= fVar6) {
/* ... 167 more lines truncated by --max-lines */
```

### decoy: 0x0078c330 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078c330: 16-byte aligned: YES
  0078c330  55             PUSH EBP
  0078c331  8B EC          MOV EBP,ESP
  0078c333  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078c330
string literals (0):
direct callees (1):
  0x007BE4C0  CGuiInGame::Initialize
struct offsets touched (top 1):
  +0x40 [dec 64]  x1  via EAX
```

## Card B11a-4-46

GOG function: CClientExoApp::MovieWindowActivated  gog 0x0040e730  callconv=(blank)  param_size_bytes=0  gog_size=22  gog_retn=0
Candidate Steam address: 0x00741270 (size 22, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00741250:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e710 CClientExoApp::MovieWindowDeactivated -> 0x00741250  gate_class=REVIEW
- GOG-order neighbour after:  0x0040e750 CClientExoApp::GetInFreeLook -> 0x00741290  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044d710 CClientExoAppInternal::MovieWindowActivated -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x00741270: 16-byte aligned: YES
  00741270  55             PUSH EBP
  00741271  8B EC          MOV EBP,ESP
  00741273  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00741270
string literals (0):
direct callees (1):
  0x007A1110  CClientExoAppInternal::MovieWindowActivated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00741270 FUN_00741270 size=22 ret=0 callers=1 callees=1
THUNK -> 0x007A1110 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00741270(undefined4 param_1)

{
  FUN_007a1110(param_1);
  return;
}
```

### decoy: 0x00741250 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00741250: 16-byte aligned: YES
  00741250  55             PUSH EBP
  00741251  8B EC          MOV EBP,ESP
  00741253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00741250
string literals (0):
direct callees (1):
  0x007A10F0  CClientExoAppInternal::MovieWindowDeactivated
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-4-47

GOG function: CClientExoAppInternal::EndLoadBar  gog 0x0043a510  callconv=(blank)  param_size_bytes=16  gog_size=208  gog_retn=16
Candidate Steam address: 0x0078df40 (size 208, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0078e010:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a1c0 CClientExoAppInternal::SetVideoMode -> 0x0078d6a0  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a5e0 CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList -> 0x0078e010  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x00907060 CAuroraInterface::GetNearestVideoMode -> Steam ?  in candidate callees: n/a
  0x00921312 Global::@__security_check_cookie@4 -> Steam ?  in candidate callees: n/a
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0xc*,0x10*,0x9c*,0xac*} | Steam {0xc*,0x10*,0x9c*,0xac*}

### abi
```
GOG param bytes: 16; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(16/4)*4 = 16 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=16 ; asm RET = 16
Stack Slot Match: TRUE (GOG 16 bytes -> x86 slots 16; ASM RET 16)
```

### prologue
```
prologue for 0x0078df40: 16-byte aligned: YES
  0078df40  55             PUSH EBP
  0078df41  8B EC          MOV EBP,ESP
  0078df43  81 EC AC 00 00 00 SUB ESP,0xac
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078df40
string literals (0):
direct callees (3):
  0x0093CA40  (unnamed)
  0x0091C630  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078df40 FUN_0078df40 size=208 ret=16 callers=1 callees=4

void __thiscall FUN_0078df40(undefined4 param_1,int param_2,int param_3,char param_4,int param_5)

{
  DEVMODEA local_ac;
  uint local_c;
  DWORD local_8;
  
  local_c = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  if (param_2 == 0) {
    param_2 = DAT_009f42a4;
  }
  if (param_3 == 0) {
    param_3 = DAT_009f42a8;
  }
  if (param_4 == '\0') {
    param_4 = DAT_009f42b0;
  }
  if (param_5 == 0) {
    param_5 = DAT_009f42ac;
  }
  local_8 = FUN_0093ca40(param_2,param_3,param_4,param_5,param_1);
  _memset [GHIDRA-AUTO-LABEL: unreliable](&local_ac,0,0x9c);
  local_ac.dmSize = 0x9c;
  EnumDisplaySettingsA((LPCSTR)0x0,local_8,&local_ac);
  __security_check_cookie [GHIDRA-AUTO-LABEL: unreliable](local_c ^ (uint)&stack0xfffffffc);
  return;
}
```

### decoy: 0x0078e010 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078e010: 16-byte aligned: YES
  0078e010  55             PUSH EBP
  0078e011  8B EC          MOV EBP,ESP
  0078e013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078e010
string literals (0):
direct callees (1):
  0x0093CD00  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-4-48

GOG function: CClientExoAppInternal::GetModuleCamera  gog 0x0042dd90  callconv=(blank)  param_size_bytes=0  gog_size=46  gog_retn=0
Candidate Steam address: 0x007819e0 (size 511, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.27) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00781be0:STR0/CALLEE0/SIG0.032/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0042db10 CClientExoAppInternal::DisplayMainMenu -> 0x00781810  gate_class=REJECT
- GOG-order neighbour after:  0x0042ddc0 CClientExoAppInternal::HandleMessage -> 0x00781590  gate_class=REJECT

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x40*} | Steam {0x18,0x24,0x40*,0xa4,0x130,0x204,0x274,0x278}
imm sets (* = in both, 0 common): GOG {0xc} | Steam {0x2,0x4,0x44,0x154,0x6f8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007819e0: 16-byte aligned: YES
  007819e0  55             PUSH EBP
  007819e1  8B EC          MOV EBP,ESP
  007819e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007819e0
string literals (0):
direct callees (12):
  0x00919723  operator new
  0x008850B0  (unnamed)
  0x0069DEA0  CSWRules::CSWRules (loads EXPTABLE 51 rows at +0x38)
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x007BDE80  (unnamed)
  0x00785DF0  CClientExoAppInternal::SetEventDescriptions
  0x00798C00  IsMoviePlaying
  0x007D0AF0  CGuiInGame::SetGlobalUnpauseableState
  0x0072F9F0  (unnamed)
  0x0070BC60  (unnamed)
struct offsets touched (top 8):
  +0x18 [dec 24]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0xA4 [dec 164]  x2  via EAX,ECX
  +0x130 [dec 304]  x2  via EAX,ECX
  +0x278 [dec 632]  x2  via EAX,ECX
  +0x24 [dec 36]  x1  via ECX
  +0x204 [dec 516]  x1  via EAX
  +0x274 [dec 628]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 007819e0 FUN_007819e0 size=511 ret=0 callers=1 callees=12

undefined4 __fastcall FUN_007819e0(int param_1)

{
  uint uVar1;
  int iVar2;
  int local_54;
  undefined4 local_50;
  undefined1 local_30 [8];
  int local_28;
  int local_24;
  int local_20;
  undefined4 local_1c;
  byte local_15;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096581e;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  if (*(int *)(param_1 + 0x278) == 0) {
    local_20 = FUN_00919723(0x6f8,uVar1);
    local_8 = 0;
    if (local_20 == 0) {
      local_50 = 0;
    }
    else {
      local_50 = FUN_008850b0(*(undefined4 *)(param_1 + 0x274));
    }
    local_1c = local_50;
    *(undefined4 *)(param_1 + 0x278) = local_50;
  }
  local_8 = 0xffffffff;
  if (DAT_00a1b4d0 == 0) {
    local_28 = FUN_00919723(0x154,uVar1);
    local_8 = 1;
    if (local_28 == 0) {
      local_54 = 0;
    }
    else {
      local_54 = FUN_0069dea0();
    }
    local_24 = local_54;
    DAT_00a1b4d0 = local_54;
  }
  local_8 = 0xffffffff;
  if (DAT_00a1b4d0 != 0) {
    local_15 = 0;
    while( true ) {
      if (*(int *)(*(int *)(*(int *)(DAT_00a1b4d0 + 0x130) + 0x18) + 0x24) <= (int)(uint)local_15)
      break;
      FUN_00733570(&DAT_00993904);
      local_8 = 2;
      FUN_0071bcf0(local_15,local_30,&local_14);
      local_8 = 0xffffffff;
      FUN_00733780();
      *(uint *)(param_1 + 0xa4) = local_14 << (local_15 & 0x1f) | *(uint *)(param_1 + 0xa4);
      local_15 = local_15 + 1;
    }
  }
  FUN_007bde80();
  FUN_00785df0();
  iVar2 = FUN_00798c00();
  if (iVar2 != 0) {
    FUN_007d0af0(1);
    FUN_0072f9f0();
    FUN_0070bc60(DAT_0099c2b4,0);
  }
  *(undefined4 *)(param_1 + 0x204) = 1;
  ExceptionList = local_10;
  return 1;
}
```

### decoy: 0x00781be0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00781be0: 16-byte aligned: YES
  00781be0  55             PUSH EBP
  00781be1  8B EC          MOV EBP,ESP
  00781be3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00781be0
string literals (2):
  0x0099F4B4  at 00782c55  'Playing next anim in queue: %s'
  0x0099F4B4  at 00782c71  'Playing next anim in queue: %s'
direct callees (106):
  0x007CE740  CGuiInGame::SetAreaTransition
  0x0078CEE0  (unnamed)
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
  0x0079ED60  CClientExoAppInternal::CrownFutureLeader
  0x007B3050  CClientExoAppInternal::SetInputClass
  0x00865250  (unnamed)
  0x007C9C40  CGuiInGame::SetSWGuiStatus
  0x007CEEC0  CGuiInGame::InitializeMap
  0x007B3390  CClientExoAppInternal::EnableInput
  0x00783D80  CClientExoAppInternal::StallServerCode
  0x00733780  CExoString::~CExoString
  0x00798820  CClientExoAppInternal::WaitForMovie
  0x00798C00  IsMoviePlaying
  0x007983E0  (unnamed)
  0x007413E0  (unnamed)
  0x007B3930  CClientExoAppInternal::ShowMouse
  0x0040BF40  Movie::RestoreAfterMovie (handler of WM_USER+10 0x40A and sync call)
  0x0093B5E0  (unnamed)
  0x0051B050  CWorldTimer::PauseWorldTimer
  0x0051AE30  CWorldTimer::ResetTimer
struct offsets touched (top 10):
  +0x40 [dec 64]  x39  via EAX,ECX,EDX
  +0x4 [dec 4]  x30  via EAX,ECX,EDX
  +0x8 [dec 8]  x17  via EAX,ECX,EDX
  +0x10 [dec 16]  x13  via EAX,ECX,EDX
  +0x18 [dec 24]  x13  via EAX,ECX,EDX
  +0x24 [dec 36]  x10  via EAX,ECX,EDX
  +0x14 [dec 20]  x9  via EAX,ECX,EDX
  +0x2C [dec 44]  x9  via EAX,ECX,EDX
  +0x16C [dec 364]  x8  via EAX,ECX,EDX
  +0x370 [dec 880]  x8  via EAX,ECX,EDX
```

## Card B11a-4-49

GOG function: CClientExoApp::GetNextLoadScreenHintSTRREF  gog 0x0040e280  callconv=(blank)  param_size_bytes=5  gog_size=33  gog_retn=8
Candidate Steam address: 0x00740d90 (size 41, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.75 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00740dc0:STR0/CALLEE5/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0040e250 CClientExoApp::SetStoryHint -> 0x00740d90  gate_class=AUTO
- GOG-order neighbour after:  0x0040e2b0 CClientExoApp::SetModuleResourceLoaded -> 0x00740df0  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044c240 CClientExoAppInternal::GetNextLoadScreenHintSTRREF -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x004e1650 CGuiInGame::DoQuickLoad -> Steam 0x007d0f50  in candidate callers: yes
  0x005d33b0 CSWGuiSaveLoad::LoadGame -> Steam 0x00853a00  in candidate callers: yes
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: no
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x491}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 5; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(5/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 5 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x00740d90: 16-byte aligned: YES
  00740d90  55             PUSH EBP
  00740d91  8B EC          MOV EBP,ESP
  00740d93  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00740d90
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00740d90 FUN_00740d90 size=41 ret=8 callers=2 callees=0

void __thiscall FUN_00740d90(int param_1,undefined1 param_2,byte param_3)

{
  *(undefined1 *)(*(int *)(param_1 + 4) + 0x491 + (uint)param_3) = param_2;
  return;
}
```

### decoy: 0x00740dc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00740dc0: 16-byte aligned: YES
  00740dc0  55             PUSH EBP
  00740dc1  8B EC          MOV EBP,ESP
  00740dc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00740dc0
string literals (0):
direct callees (1):
  0x0079FC40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

## Card B11a-4-50

GOG function: CClientExoAppInternal::ShutDownToMainMenu  gog 0x0043a6c0  callconv=(blank)  param_size_bytes=0  gog_size=339  gog_retn=0
Candidate Steam address: 0x0078e0f0 (size 339, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.80) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: UnloadArea 0x7a5fa0; callers: ShutDownToMainMenu 0x73fb40); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x0078dbf0:STR0/CALLEE0/SIG0.121/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a610 CClientExoAppInternal::SetCharacterFile -> 0x0078e040  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a820 CClientExoAppInternal::RestartMoviesIfNecessary -> 0x0078e250  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (9), mapped to Steam through trusted rows only:
  0x00401c30 CAppManager::DestroyServer -> Steam ?  in candidate callees: n/a
  0x0042db10 CClientExoAppInternal::DisplayMainMenu -> Steam ?  in candidate callees: n/a
  0x00431a30 CClientExoAppInternal::UnloadModule -> Steam ?  in candidate callees: n/a
  0x0043abf0 CClientExoAppInternal::StartMenuMusic -> Steam ?  in candidate callees: n/a
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callees: yes
  0x00469b40 CGameObjectArray::Delete_2 -> Steam ?  in candidate callees: n/a
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  ... 1 more GOG callees
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0040d020 CClientExoApp::ShutDownToMainMenu -> Steam 0x0073fb40  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 9 common): GOG {0x4*,0x14*,0x40*,0x48*,0x108*,0x18c*,0x1ec*,0x250*,0x254*} | Steam {0x4*,0x14*,0x40*,0x48*,0x108*,0x18c*,0x1ec*,0x250*,0x254*}
imm sets (* = in both, 3 common): GOG {0x2*,0x2c*,0x7f*} | Steam {0x2*,0x2c*,0x7f*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078e0f0: 16-byte aligned: YES
  0078e0f0  55             PUSH EBP
  0078e0f1  8B EC          MOV EBP,ESP
  0078e0f3  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e0f0
string literals (0):
direct callees (9):
  0x00401C70  (unnamed)
  0x00726F80  (unnamed)
  0x0053DED0  (unnamed)
  0x007A5FA0  CSWCArea::UnloadArea
  0x00785410  CClientExoAppInternal::UnloadModule
  0x00781590  (unnamed)
  0x0070BC60  (unnamed)
  0x0078E620  (unnamed)
  0x00623320  CSWSEncounter::AIUpdate
struct offsets touched (top 9):
  +0x14 [dec 20]  x4  via EAX,EDX
  +0x18C [dec 396]  x2  via ECX,EDX
  +0x254 [dec 596]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x40 [dec 64]  x1  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x108 [dec 264]  x1  via ECX
  +0x1EC [dec 492]  x1  via ECX
  +0x250 [dec 592]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078e0f0 FUN_0078e0f0 size=339 ret=0 callers=2 callees=9

void __fastcall FUN_0078e0f0(int param_1)

{
  undefined4 *puVar1;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_c = 0;
  local_8 = 0;
  local_10 = 0;
  if (*(int *)(param_1 + 0x254) != 0) {
    *(undefined4 *)(param_1 + 0x250) = 1;
  }
  FUN_00401c70();
  local_8 = FUN_00726f80();
  if (local_8 != 0) {
    if (*(int *)(local_8 + 0x48) != 0) {
      local_c = *(undefined4 *)(param_1 + 0x40);
      FUN_0053ded0(*(undefined4 *)(*(int *)(local_8 + 0x48) + 0x108),&local_10);
      FUN_007a5fa0();
    }
    FUN_00785410();
  }
  if (*(int *)(param_1 + 0x254) == 0) {
    FUN_00781590(0);
    FUN_0070bc60(DAT_0099c2a8,0);
    *(undefined4 *)(param_1 + 0x1ec) = 1;
    FUN_0078e620(1,0x7f,2);
    if (*(int *)(param_1 + 0x18c) != 0) {
      FUN_00623320();
      *(undefined4 *)(*(int *)(DAT_00a1b4a4 + 0x14) + 0x14) = 0;
      puVar1 = *(undefined4 **)(DAT_00a1b4a4 + 0x14);
      *puVar1 = 0;
      puVar1[1 = byte +0x4] = 0;
      *(undefined4 *)(param_1 + 0x18c) = 0;
    }
  }
  return;
}
```

### decoy: 0x0078dbf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078dbf0: 16-byte aligned: YES
  0078dbf0  55             PUSH EBP
  0078dbf1  8B EC          MOV EBP,ESP
  0078dbf3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078dbf0
string literals (0):
direct callees (9):
  0x0091C630  (unnamed)
  0x00409750  ChangeDisplayMode/RecreateWindow
  0x00411A80  CSWGuiManager::SetScreenSize
  0x00410800  GuiManager::Contains
  0x00881970  (unnamed)
  0x00919723  operator new
  0x008850B0  (unnamed)
  0x007CFCA0  CGuiInGame::ResetInterfaceForSize
  0x009196EE  __security_check_cookie
struct offsets touched (top 7):
  +0x274 [dec 628]  x5  via EAX,ECX,EDX
  +0x278 [dec 632]  x3  via EAX,ECX
  +0x280 [dec 640]  x3  via EAX,EDX
  +0x40 [dec 64]  x2  via EAX,EDX
  +0x1A0 [dec 416]  x2  via EAX,ECX
  +0x4 [dec 4]  x1  via EAX
  +0x18 [dec 24]  x1  via EAX
```

## Card B11a-4-51

GOG function: CClientExoAppInternal::AttachPlayerGameObjectToArea  gog 0x00438750  callconv=(blank)  param_size_bytes=0  gog_size=385  gog_retn=0
Candidate Steam address: 0x0078bc50 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.75) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bcc0:STR0/CALLEE0/SIG0.156/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004386e0 CClientExoAppInternal::GetSoundObjectByGameObjectID -> 0x0078bd80  gate_class=REVIEW
- GOG-order neighbour after:  0x004388e0 CClientExoAppInternal::ResetDriveAcceleration -> 0x0078bc50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (7), mapped to Steam through trusted rows only:
  0x004394c0 CClientExoAppInternal::GetPlayerCreature -> Steam ?  in candidate callees: n/a
  0x0046a990 CSWCModule::SetCameraFollow -> Steam ?  in candidate callees: n/a
  0x004f03c0 CSWPlayerControl::SetPlayer -> Steam ?  in candidate callees: n/a
  0x004f0460 CSWPlayerControl::SetCamera -> Steam ?  in candidate callees: n/a
  0x004f0520 CSWPlayerControlCamRelative::GetPlayerWalking -> Steam ?  in candidate callees: n/a
  0x004f05f0 CSWPlayerControlCamRelative::Constructor -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0x4,0x40*,0x48,0x94,0x188,0x2a0*,0x328,0x32c,0x330} | Steam {0x38,0x3c,0x40*,0x2a0*}
imm sets (* = in both, 0 common): GOG {0x4,0x34,0xcc} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bc50 FUN_0078bc50 size=111 ret=0 callers=1 callees=4

void __fastcall FUN_0078bc50(int param_1)

{
  if (*(int *)(param_1 + 0x3c) != 0) {
    FUN_0084bda0();
  }
  if (*(int *)(param_1 + 0x38) != 0) {
    FUN_0084b490(param_1,*(undefined4 *)(param_1 + 0x38));
  }
  if (*(int *)(param_1 + 0x40) != 0) {
    FUN_007c7030();
  }
  if (*(int *)(param_1 + 0x2a0) != 0) {
    FUN_00865220();
  }
  return;
}
```

### decoy: 0x0078bcc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bcc0: 16-byte aligned: YES
  0078bcc0  55             PUSH EBP
  0078bcc1  8B EC          MOV EBP,ESP
  0078bcc3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078bcc0
string literals (0):
direct callees (3):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

## Card B11a-4-52

GOG function: CClientExoAppInternal::AddCreatureModelDirectlyToIgnoreList  gog 0x0043a5e0  callconv=(blank)  param_size_bytes=16  gog_size=38  gog_retn=16
Candidate Steam address: 0x0078e010 (size 38, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078df40:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0043a510 CClientExoAppInternal::EndLoadBar -> 0x0078df40  gate_class=REVIEW
- GOG-order neighbour after:  0x0043a610 CClientExoAppInternal::SetCharacterFile -> 0x0078e040  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00907320 CAuroraInterface::GetExactVideoMode -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x0078e010: 16-byte aligned: YES
  0078e010  55             PUSH EBP
  0078e011  8B EC          MOV EBP,ESP
  0078e013  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078e010
string literals (0):
direct callees (1):
  0x0093CD00  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078e010 FUN_0078e010 size=38 ret=16 callers=1 callees=1
THUNK -> 0x0093CD00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __thiscall
FUN_0078e010(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined1 param_4,
            undefined4 param_5)

{
  FUN_0093cd00(param_2,param_3,param_4,param_5,param_1);
  return;
}
```

### decoy: 0x0078df40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078df40: 16-byte aligned: YES
  0078df40  55             PUSH EBP
  0078df41  8B EC          MOV EBP,ESP
  0078df43  81 EC AC 00 00 00 SUB ESP,0xac
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078df40
string literals (0):
direct callees (3):
  0x0093CA40  (unnamed)
  0x0091C630  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

## Card B11a-4-53

GOG function: CClientExoAppInternal::GetCreatureByGameObjectID  gog 0x004383d0  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438360 CClientExoAppInternal::GetItemByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00438440 CClientExoAppInternal::GetAreaByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (1 of 7): the card's A-CALLEE credit may come from these, not from callees
  0x0040ca10 CClientExoApp::GetCreatureByGameObjectID -> Steam 0x0073f550  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x2c} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-54

GOG function: CClientExoAppInternal::AddObjectToHitcheckIgnoreList  gog 0x004396d0  callconv=(blank)  param_size_bytes=4  gog_size=141  gog_retn=4
Candidate Steam address: 0x0078d100 (size 141, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(1.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078d190:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00439570 CClientExoAppInternal::ContinueMessageProcessing -> 0x0078cf90  gate_class=REVIEW
- GOG-order neighbour after:  0x00439760 CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList -> 0x0078d190  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (5), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x0062c800 unnamed -> Steam ?  in candidate callees: n/a
  0x00733b00 CExoLinkedListInternal::AddTail_2 -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0x10*,0x190*} | Steam {0x4*,0x10*,0x190*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078d100: 16-byte aligned: YES
  0078d100  55             PUSH EBP
  0078d101  8B EC          MOV EBP,ESP
  0078d103  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078d100
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0058C370  (unnamed)
  0x00919723  operator new
  0x0054AA30  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078d100 FUN_0078d100 size=141 ret=4 callers=2 callees=5

void __thiscall FUN_0078d100(undefined4 param_1,int param_2)

{
  int local_c;
  int *local_8;
  
  local_c = FUN_007a1720(param_1);
  local_8 = (int *)FUN_00521060();
  while( true ) {
    if (local_c == 0) {
      local_8 = (int *)FUN_00919723(4);
      *local_8 = param_2;
      FUN_0054aa30(local_8);
      return;
    }
    if ((local_8 != (int *)0x0) && (*local_8 == param_2)) break;
    local_8 = (int *)FUN_0058c370(&local_c);
  }
  return;
}
```

### decoy: 0x0078d190 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078d190: 16-byte aligned: YES
  0078d190  55             PUSH EBP
  0078d191  8B EC          MOV EBP,ESP
  0078d193  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078d190
string literals (0):
direct callees (5):
  0x007A1720  (unnamed)
  0x00521060  (unnamed)
  0x0064DDE0  (unnamed)
  0x009196FD  operator delete
  0x0058C370  (unnamed)
struct offsets touched (top 0):
```

## Card B11a-4-55

GOG function: CClientExoAppInternal::SendSaveGameRequest  gog 0x00431c40  callconv=(blank)  param_size_bytes=12  gog_size=57  gog_retn=12
Candidate Steam address: 0x00785620 (size 57, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: SendSaveGameRequest 0x73f6f0)
- best decoy (neighbour/other candidate) scores: 0x00785660:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00431a30 CClientExoAppInternal::UnloadModule -> 0x00785410  gate_class=AUTO
- GOG-order neighbour after:  0x00431c80 CClientExoAppInternal::SendLoadGameRequest -> 0x00785660  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00505d10 CSWCMessage::SendPlayerToServerModule_SaveGame -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0040cbb0 CClientExoApp::SendSaveGameRequest -> Steam 0x0073f6f0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x150*} | Steam {0x150*}
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
prologue for 0x00785620: 16-byte aligned: YES
  00785620  55             PUSH EBP
  00785621  8B EC          MOV EBP,ESP
  00785623  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00785620
string literals (0):
direct callees (1):
  0x0087A430  (unnamed)
struct offsets touched (top 1):
  +0x150 [dec 336]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00785620 FUN_00785620 size=57 ret=12 callers=1 callees=1

undefined4 __thiscall
FUN_00785620(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x150) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = FUN_0087a430(param_2,param_3,param_4);
  }
  return uVar1;
}
```

### decoy: 0x00785660 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00785660: 16-byte aligned: YES
  00785660  55             PUSH EBP
  00785661  8B EC          MOV EBP,ESP
  00785663  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00785660
string literals (0):
direct callees (1):
  0x0087A4B0  (unnamed)
struct offsets touched (top 1):
  +0x150 [dec 336]  x1  via EAX
```

## Card B11a-4-56

GOG function: CClientExoAppInternal::ContinueMessageProcessing  gog 0x00439570  callconv=(blank)  param_size_bytes=0  gog_size=322  gog_retn=0
Candidate Steam address: 0x0078cf90 (size 16, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078cfa0:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00439530 CClientExoAppInternal::ClearCachedCreatures -> 0x0078cf50  gate_class=REVIEW
- GOG-order neighbour after:  0x004396d0 CClientExoAppInternal::AddObjectToHitcheckIgnoreList -> 0x0078d100  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0043a6c0 CClientExoAppInternal::ShutDownToMainMenu -> Steam ?  in candidate callees: n/a
  0x006133b0 Global::return_void -> Steam ?  in candidate callees: n/a
  0x00737a80 CSWCSoundObject::LoadModel -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x10,0x14,0x264} | Steam {}
imm sets (* = in both, 0 common): GOG {0x3,0x4,0x38,0x10bbd,0x10bbe,0x10bbf,0x10c34} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078cf90: 16-byte aligned: YES
  0078cf90  55             PUSH EBP
  0078cf91  8B EC          MOV EBP,ESP
  0078cf93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078cf90
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078cf90 FUN_0078cf90 size=16 ret=0 callers=4 callees=0

undefined4 FUN_0078cf90(void)

{
  return 1;
}
```

### decoy: 0x0078cfa0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078cfa0: 16-byte aligned: YES
  0078cfa0  55             PUSH EBP
  0078cfa1  8B EC          MOV EBP,ESP
  0078cfa3  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0078cfa0
string literals (0):
direct callees (3):
  0x005A65C0  CSWSEffectListHandler::OnApplyNegativeLevel
  0x00623320  CSWSEncounter::AIUpdate
  0x0078E0F0  CClientExoAppInternal::ShutDownToMainMenu
struct offsets touched (top 4):
  +0x14 [dec 20]  x8  via EAX,ECX,EDX
  +0x264 [dec 612]  x3  via ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x10 [dec 16]  x2  via EAX,EDX
```

## Card B11a-4-57

GOG function: CClientExoAppInternal::GetAreaByGameObjectID  gog 0x00438440  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004383d0 CClientExoAppInternal::GetCreatureByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x004384b0 CClientExoAppInternal::GetTriggerByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x24} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-58

GOG function: CClientExoAppInternal::ResetDriveAcceleration  gog 0x004388e0  callconv=(blank)  param_size_bytes=0  gog_size=48  gog_retn=0
Candidate Steam address: 0x0078bc50 (size 111, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00438750 CClientExoAppInternal::AttachPlayerGameObjectToArea -> 0x0078bc50  gate_class=REVIEW
- GOG-order neighbour after:  0x00438910 CClientExoAppInternal::InitializeInGameGUI -> 0x0078bc50  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x2c,0x2a0*} | Steam {0x38,0x3c,0x40,0x2a0*}
imm sets (* = in both, 0 common): GOG {} | Steam {0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0078bc50: 16-byte aligned: YES
  0078bc50  55             PUSH EBP
  0078bc51  8B EC          MOV EBP,ESP
  0078bc53  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0078bc50
string literals (0):
direct callees (4):
  0x0084BDA0  (unnamed)
  0x0084B490  (unnamed)
  0x007C7030  CGuiInGame::ShutDown
  0x00865220  (unnamed)
struct offsets touched (top 4):
  +0x38 [dec 56]  x2  via EAX,ECX
  +0x3C [dec 60]  x2  via EAX,ECX
  +0x40 [dec 64]  x2  via EAX,ECX
  +0x2A0 [dec 672]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0078bc50 FUN_0078bc50 size=111 ret=0 callers=1 callees=4

void __fastcall FUN_0078bc50(int param_1)

{
  if (*(int *)(param_1 + 0x3c) != 0) {
    FUN_0084bda0();
  }
  if (*(int *)(param_1 + 0x38) != 0) {
    FUN_0084b490(param_1,*(undefined4 *)(param_1 + 0x38));
  }
  if (*(int *)(param_1 + 0x40) != 0) {
    FUN_007c7030();
  }
  if (*(int *)(param_1 + 0x2a0) != 0) {
    FUN_00865220();
  }
  return;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

## Card B11a-4-59

GOG function: CClientExoAppInternal::GetActivePauseState  gog 0x00430a60  callconv=(blank)  param_size_bytes=0  gog_size=49  gog_retn=0
Candidate Steam address: 0x00784440 (size 49, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: GetActivePauseState 0x73fe00, MainLoop 0x781be0)
- best decoy (neighbour/other candidate) scores: 0x00783ee0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00430a40 CClientExoAppInternal::GetPauseState -> 0x00783d80  gate_class=REVIEW
- GOG-order neighbour after:  0x00430aa0 CClientExoAppInternal::GetActiveTimer -> 0x00784480  gate_class=AUTO

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00430a40 CClientExoAppInternal::GetPauseState -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0040d2e0 CClientExoApp::GetActivePauseState -> Steam 0x0073fe00  in candidate callers: yes
  0x0042e160 CClientExoAppInternal::MainLoop -> Steam 0x00781be0  in candidate callers: yes
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
prologue for 0x00784440: 16-byte aligned: YES
  00784440  55             PUSH EBP
  00784441  8B EC          MOV EBP,ESP
  00784443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00784440
string literals (0):
direct callees (1):
  0x00784420  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00784440 FUN_00784440 size=49 ret=0 callers=5 callees=1

undefined4 FUN_00784440(void)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = FUN_00784420(2);
  if (iVar1 == 0) {
    iVar1 = FUN_00784420(1);
    if (iVar1 == 0) {
      uVar2 = 0;
    }
    else {
      uVar2 = 1;
    }
  }
  else {
    uVar2 = 2;
  }
  return uVar2;
}
```

### decoy: 0x00783ee0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00783ee0: 16-byte aligned: YES
  00783ee0  55             PUSH EBP
  00783ee1  8B EC          MOV EBP,ESP
  00783ee3  81 EC A4 01 00 00 SUB ESP,0x1a4
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00783ee0
string literals (0):
direct callees (13):
  0x00726F80  (unnamed)
  0x0078CEE0  (unnamed)
  0x007FAE30  GetPlayerObject-like getter `[this+0x48]->+0x268`
  0x00835F50  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x00436200  (unnamed)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0073F750  ClientApp::GetGuiInGame
  0x00463120  (unnamed)
  0x0070B940  (unnamed)
  0x0046CE40  (unnamed)
  0x0070B910  (unnamed)
  0x0070B8F0  CExoSound::Render wrapper
struct offsets touched (top 10):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x9  via EAX,ECX,EDX
  +0x24 [dec 36]  x3  via EAX,ECX,EDX
  +0x80 [dec 128]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EAX
  +0x14 [dec 20]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x40 [dec 64]  x1  via EAX
  +0x64 [dec 100]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
```

## Card B11a-4-60

GOG function: CClientExoAppInternal::GetProjectileByGameObjectID  gog 0x00438520  callconv=(blank)  param_size_bytes=4  gog_size=102  gog_retn=4
Candidate Steam address: 0x0078bb30 (size 138, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0078bbc0:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x004384b0 CClientExoAppInternal::GetTriggerByGameObjectID -> 0x0078bb30  gate_class=REVIEW
- GOG-order neighbour after:  0x00438590 CClientExoAppInternal::GetPlaceableByGameObjectID -> 0x0078bb30  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00469c20 CGameObjectArray::GetGameObject -> Steam 0x0053dfb0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8,0x14,0x44} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*,0xc} | Steam {0x4*,0x110,0x118,0x120}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0078bb30: 16-byte aligned: YES
  0078bb30  55             PUSH EBP
  0078bb31  8B EC          MOV EBP,ESP
  0078bb33  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0078bb30
string literals (0):
direct callees (2):
  0x00733780  CExoString::~CExoString
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0078bb30 FUN_0078bb30 size=138 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_0078bb30(undefined4 param_1,uint param_2)

{
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  puStack_c = &LAB_009664fc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 1;
  FUN_00733780(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = local_8 & 0xffffff00;
  FUN_00733780();
  local_8 = 0xffffffff;
  FUN_00733780();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x0078bbc0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0078bbc0: 16-byte aligned: YES
  0078bbc0  55             PUSH EBP
  0078bbc1  8B EC          MOV EBP,ESP
  0078bbc3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0078bbc0
string literals (0):
direct callees (3):
  0x00733780  CExoString::~CExoString
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```
