# Review cards B18 chunk 2

Answer each card with one TSV row in `steam_db_batches/B18/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B18-2-01

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852a0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00985240:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b6a0 Global::AurATIFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b770 Global::AurAnisotropicFilterAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b2e0 Global::AurBumpMapCardValidate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x009852a0: 16-byte aligned: YES
  009852a0  55             PUSH EBP
  009852a1  8B EC          MOV EBP,ESP
  009852a3  B9 08 BC A7 00 MOV ECX,0xa7bc08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852a0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852a0 FUN_009852a0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852a0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x00985240 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00985240: 16-byte aligned: YES
  00985240  55             PUSH EBP
  00985241  8B EC          MOV EBP,ESP
  00985243  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00985240
string literals (0):
direct callees (2):
  0x004D5A60  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852a3: MOV ECX,0xa7bc08)

## Card B18-2-02

GOG function: Global::AurCubeMapAvailable  gog 0x0086b700  callconv=(blank)  param_size_bytes=0  gog_size=100  gog_retn=0
Candidate Steam address: 0x009852c0 (size 15, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x009852b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086b6a0 Global::AurATIFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086b770 Global::AurAnisotropicFilterAvailable  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0086b2e0 Global::AurBumpMapCardValidate -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
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
prologue for 0x009852c0: 16-byte aligned: YES
  009852c0  55             PUSH EBP
  009852c1  8B EC          MOV EBP,ESP
  009852c3  B9 08 BE A7 00 MOV ECX,0xa7be08
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x009852c0
string literals (0):
direct callees (1):
  0x00463B20  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 009852c0 FUN_009852c0 size=15 ret=0 callers=0 callees=1
THUNK -> 0x00463B20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_009852c0(void)

{
  FUN_00463b20();
  return;
}
```

### decoy: 0x009852b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x009852b0: 16-byte aligned: YES
  009852b0  55             PUSH EBP
  009852b1  8B EC          MOV EBP,ESP
  009852b3  B9 D0 BC A7 00 MOV ECX,0xa7bcd0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x009852b0
string literals (0):
direct callees (1):
  0x00452F80  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 15 B, RET 0
- callees: 0x00463b20 (CALL)
- twins: 0x00984ec0, 0x00984ed0, 0x00984ee0, 0x00984ef0, 0x00985050, ... (distinguishing: 0x009852c3: MOV ECX,0xa7be08)

## Card B18-2-03

GOG function: Global::AurEnableForceFury  gog 0x0086ae30  callconv=(blank)  param_size_bytes=2  gog_size=18  gog_retn=0
Candidate Steam address: 0x00478f40 (size 12, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478f50:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ae20 Global::AurDisableForceSight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ae50 Global::AurSetSaturationParams  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00834760 GLRender::EnableFury -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam 0x0079e550  in candidate callers: no
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam 0x0079e960  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4} | Steam {}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 2 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478f40: 16-byte aligned: YES
  00478f40  55             PUSH EBP
  00478f41  8B EC          MOV EBP,ESP
  00478f43  C6 05 C4 BA A7 00 00 MOV byte ptr [0x00a7bac4],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478f40
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478f40 FUN_00478f40 size=12 ret=0 callers=1 callees=0

void FUN_00478f40(void)

{
  DAT_00a7bac4 = 0;
  return;
}
```

### decoy: 0x00478f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478f50: 16-byte aligned: YES
  00478f50  55             PUSH EBP
  00478f51  8B EC          MOV EBP,ESP
  00478f53  0F B7 45 08    MOVZX EAX,word ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478f50
string literals (0):
direct callees (1):
  0x00442090  SetFuryLevel ([0xA1DD54], loads fx_Fury01..03)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 12 B, RET 0
- callers 1: 0x0079e960 CClientExoAppInternal::DisableVideoEffect
- twins: 0x00478eb0, 0x00478ed0, 0x004791e0 (distinguishing: 0x00478f43: MOV byte ptr [0x00a7bac4],0x0)

## Card B18-2-04

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a220 (size 428, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=RequestPoolManagerVBO::ResizePool | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0042a1f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c0340 CSWUpgradeItemEntry::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c0420 CSWUpgradeItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: no
  0x00437190 CClientExoAppInternal::StopServices -> Steam 0x0078ac40  in candidate callers: no
  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> Steam 0x0074d520  in candidate callers: no
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x2c,0x50,0x5c,0x68}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0xc,0x2c,0x8764,0x88b8,0x88e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0042a220: 16-byte aligned: YES
  0042a220  55             PUSH EBP
  0042a221  8B EC          MOV EBP,ESP
  0042a223  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a220
string literals (0):
direct callees (3):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x0091C6AA  operator delete[]
struct offsets touched (top 4):
  +0x50 [dec 80]  x7  via EAX,EDX
  +0x2C [dec 44]  x5  via EAX,EDX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x5C [dec 92]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a220 FUN_0042a220 size=428 ret=8 callers=0 callees=3

void __thiscall FUN_0042a220(int param_1,int param_2,undefined4 param_3)

{
  undefined4 local_14;
  void *local_10;
  void *local_c;
  size_t local_8;
  
  if (param_2 != -1) {
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32b18)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0x8764,&local_8);
    local_10 = (void *)FUN_0091d288(local_8);
    local_c = (void *)(*DAT_00a32bac)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),35000)
    ;
    _memcpy [GHIDRA-AUTO-LABEL: unreliable](local_10,local_c,local_8);
    (*DAT_00a32c68)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4));
    local_14 = *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d54)(1,&local_14);
    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4) = 0;
    (*DAT_00a32b28)(1,*(int *)(param_1 + 0x2c) + param_2 * 4);
    (*DAT_00a32d4c)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),
                    *(undefined4 *)(*(int *)(param_1 + 0x2c) + param_2 * 4));
    (*DAT_00a32ab4)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),param_3,0,0x88e8);
    (*DAT_00a32ce8)(*(undefined4 *)(*(int *)(param_1 + 0x50) + param_2 * 4),0,local_8,local_10);
    FUN_0091c6aa(*(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4));
    *(undefined4 *)(*(int *)(param_1 + 0x68) + param_2 * 4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x5c) + param_2 * 4) = param_3;
    FUN_0091c6aa(local_10);
  }
  return;
}
```

### decoy: 0x0042a1f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a1f0: 16-byte aligned: YES
  0042a1f0  55             PUSH EBP
  0042a1f1  8B EC          MOV EBP,ESP
  0042a1f3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0042a1f0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x5C [dec 92]  x1  via EAX
```

### Static facts (script output, report-only)
- size 428 B, RET 8
- callees: 0x0091d288 operator new[] (CALL), 0x0091dba0 _memcpy (CALL), 0x0091c6aa operator delete[] (CALL)
- indirect: 0x0042a256 unresolved, 0x0042a272 unresolved, 0x0042a2a7 unresolved, 0x0042a2d1 unresolved, 0x0042a2ec unresolved, ...
- twins: none

## Card B18-2-05

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2540 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2510:STR0/CALLEE4/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (4 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: no
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: no
  0x00578ec0 CSWCProjectile::Constructor -> Steam 0x008e54e0  in candidate callers: no
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
prologue for 0x007a2540: 16-byte aligned: YES
  007a2540  55             PUSH EBP
  007a2541  8B EC          MOV EBP,ESP
  007a2543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2540
string literals (0):
direct callees (1):
  0x007A21D0  CClientExoAppInternal::SetGameOver
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2540 FUN_007a2540 size=36 ret=4 callers=3 callees=1
THUNK -> 0x007A21D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

undefined4 FUN_007a2540(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_007a21d0(param_1);
  return uVar1;
}
```

### decoy: 0x007a2510 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2510: 16-byte aligned: YES
  007a2510  55             PUSH EBP
  007a2511  8B EC          MOV EBP,ESP
  007a2513  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2510
string literals (0):
direct callees (1):
  0x007A2110  CClientExoAppInternal::AddMoveToModuleMovie_2
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 3: 0x007a5fa0 CSWCArea::UnloadArea, 0x007ed5f0, 0x007fd370
- callees: 0x007a21d0 (CALL)
- twins: 0x007a2510 (distinguishing: 0x007a2553: CALL 0x007a21d0)

## Card B18-2-06

GOG function: CSWCollisionMesh::NormalLocalToWorld  gog 0x00728b10  callconv=(blank)  param_size_bytes=12  gog_size=331  gog_retn=12
Candidate Steam address: 0x005eebf0 (size 331, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: rotation:0x007286d0:0.80>0.36
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callers: GetMeshBoundingBox 0x617320, AddToArea 0x5f2a80); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x005ee7b0:STR0/CALLEE0/SIG0.056/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x007286d0 CSWCollisionMesh::LocalToWorld  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00728c60 Global::AddModuleOverrideResourceDirectories  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x00723da0 CSWCollisionMesh::GetVertex -> Steam ?  in candidate callees: n/a
  0x00723e60 CSWCollisionMesh::LoadDefaultMesh -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x007627d0 CSWSDoor::GetMeshBoundingBox -> Steam 0x00617320  in candidate callers: yes
  0x0076ce50 CSWSPlaceable::AddToArea -> Steam 0x005f2a80  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x1c*,0x58*,0x64*} | Steam {0x1c*,0x58*,0x64*}
imm sets (* = in both, 3 common): GOG {0xc*,0x50*,0x55ffff00*} | Steam {0xc*,0x50*,0x55ffff00*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x005eebf0: 16-byte aligned: YES
  005eebf0  55             PUSH EBP
  005eebf1  8B EC          MOV EBP,ESP
  005eebf3  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x005eebf0
string literals (0):
direct callees (2):
  0x005E9DD0  (unnamed)
  0x005E9E90  (unnamed)
struct offsets touched (top 3):
  +0x58 [dec 88]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x64 [dec 100]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 005eebf0 FUN_005eebf0 size=331 ret=12 callers=2 callees=2

/* WARNING: Type propagation algorithm not settling */

void __thiscall FUN_005eebf0(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int local_4c;
  undefined4 local_48;
  int local_44[16 = byte +0x40];
  
  local_44[7 = byte +0x1c] = 0xffffffff;
  local_4c = -1;
  local_44[0] = -1;
  local_44[8 = byte +0x20] = 0;
  local_44[9 = byte +0x24] = 0;
  local_44[10 = byte +0x28] = 0;
  local_44[1 = byte +0x4] = 0;
  local_44[2 = byte +0x8] = 0;
  local_44[3 = byte +0xc] = 0;
  local_44[0xc = byte +0x30] = 0;
  local_44[0xd = byte +0x34] = 0;
  local_44[0xe = byte +0x38] = 0;
  local_44[4 = byte +0x10] = 0;
  local_44[5 = byte +0x14] = 0;
  local_44[6 = byte +0x18] = 0;
  local_44[0xf = byte +0x3c] = (**(code **)(*param_1 + 0x1c))();
  if (param_1[0x16 = byte +0x58] != 0) {
    for (local_44[0xb = byte +0x2c] = 0; local_44[0xb = byte +0x2c] < param_1[0x16 = byte +0x58]; local_44[0xb = byte +0x2c] = local_44[0xb = byte +0x2c] + 1) {
      local_44[7 = byte +0x1c] = -1;
      local_4c = -1;
      local_44[0] = -1;
      local_48 = 0xffffffff;
      FUN_005e9dd0(local_44[0xb = byte +0x2c],local_44 + 0x7 [dec 7] {int[]: byte +0x1c},&local_4c,local_44);
      if (((local_44[7 = byte +0x1c] != -1) && (local_4c != -1)) && (local_44[0] != -1)) {
        if ((*(uint *)(param_1[0x19 = byte +0x64] + local_44[0xb = byte +0x2c] * 4) & local_44[0xf = byte +0x3c]) == 0) {
          local_48 = param_4;
        }
        else {
          local_48 = param_3;
        }
        FUN_005e9e90(local_44[7 = byte +0x1c],local_44 + 0x8 [dec 8] {int[]: byte +0x20});
        FUN_005e9e90(local_4c,local_44 + 0x1 [dec 1] {int[]: byte +0x4});
        FUN_005e9e90(local_44[0],local_44 + 0xc);
      }
    }
  }
  return;
}
```

### decoy: 0x005ee7b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x005ee7b0: 16-byte aligned: YES
  005ee7b0  55             PUSH EBP
  005ee7b1  8B EC          MOV EBP,ESP
  005ee7b3  83 EC 70       SUB ESP,0x70
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x005ee7b0
string literals (0):
direct callees (1):
  0x0044C8A0  Vector::operator+ (Vector3 add)
struct offsets touched (top 3):
  +0x4 [dec 4]  x12  via EAX,ECX,EDX
  +0x8 [dec 8]  x11  via EAX,ECX,EDX
  +0xC [dec 12]  x5  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 331 B, RET 12
- callers 2: 0x005f2a80 CSWSPlaceable::AddToArea, 0x00617320 CSWSDoor::GetMeshBoundingBox
- callees: 0x005e9dd0 (CALL), 0x005e9e90 (CALL)
- indirect: 0x005eec55 slot 0x1c
- twins: none

## Card B18-2-07

GOG function: CExoSound::StopAllOneShots  gog 0x0063aaa0  callconv=(blank)  param_size_bytes=0  gog_size=29  gog_retn=0
Candidate Steam address: 0x0070bb00 (size 29, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: StopAllOneShots 0x709cf0; callers: StartNewModule 0x52fda0)
- best decoy (neighbour/other candidate) scores: 0x0070ba90:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063aa30 CExoSound::Play3DOneShotSound  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063aac0 CExoSound::EnableStreamingEnvironmentEffects  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00638c70 CExoSoundInternal::StopAllOneShots -> Steam 0x00709cf0  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x007b3120 CServerExoAppInternal::StartNewModule -> Steam 0x0052fda0  in candidate callers: yes
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
prologue for 0x0070bb00: 16-byte aligned: YES
  0070bb00  55             PUSH EBP
  0070bb01  8B EC          MOV EBP,ESP
  0070bb03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070bb00
string literals (0):
direct callees (1):
  0x00709CF0  CExoSoundInternal::StopAllOneShots
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0070bb00 FUN_0070bb00 size=29 ret=0 callers=1 callees=1

void __fastcall FUN_0070bb00(int *param_1)

{
  if (*param_1 != 0) {
    FUN_00709cf0();
  }
  return;
}
```

### decoy: 0x0070ba90 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070ba90: 16-byte aligned: YES
  0070ba90  55             PUSH EBP
  0070ba91  8B EC          MOV EBP,ESP
  0070ba93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070ba90
string literals (0):
direct callees (1):
  0x00708510  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 29 B, RET 0
- callers 1: 0x0052fda0 CServerExoAppInternal::StartNewModule
- callees: 0x00709cf0 CExoSoundInternal::StopAllOneShots (CALL)
- twins: 0x0070b8f0, 0x0070bbe0 (distinguishing: 0x0070bb14: CALL 0x00709cf0)

## Card B18-2-08

GOG function: CSWGuiObject::SetExtent  gog 0x00912f40  callconv=__thiscall  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00414920 (size 82, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiBorderParams::SetBorder | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00414840:STR0/CALLEE0/SIG0.225/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00912e20 CSWGuiBorder::Constructor -> 0x00414a50  trusted
- GOG-order neighbour after:  0x00912f70 CRes::GetData  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {0x54}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*,0x24,0x34,0x44}

### abi
```
GOG param bytes: 4; callconv: thiscall
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00414920: 16-byte aligned: YES
  00414920  55             PUSH EBP
  00414921  8B EC          MOV EBP,ESP
  00414923  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00414920
string literals (0):
direct callees (3):
  0x00414680  CSWGuiBorderParams::SetCornerImage
  0x00414760  CSWGuiBorderParams::SetEdgeImage
  0x00414840  CSWGuiControl image setter thiscall(CResRef* name, int force) RET 8; current resref at ctl+0x44; work only when name differs (0x710A70) or force
struct offsets touched (top 1):
  +0x54 [dec 84]  x2  via EAX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00414920 FUN_00414920 size=82 ret=4 callers=2 callees=3

void __thiscall FUN_00414920(int param_1,undefined4 param_2)

{
  *(undefined4 *)(param_1 + 0x54) = param_2;
  if (*(int *)(param_1 + 0x54) != 0) {
    FUN_00414680(param_1 + 0x24,1);
    FUN_00414760(param_1 + 0x34,1);
    FUN_00414840(param_1 + 0x44,1);
  }
  return;
}
```

### decoy: 0x00414840 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414840: 16-byte aligned: YES
  00414840  55             PUSH EBP
  00414841  8B EC          MOV EBP,ESP
  00414843  83 EC 38       SUB ESP,0x38
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414840
string literals (0):
direct callees (6):
  0x00710A70  ResRef16 != (memcmp of 16 bytes)
  0x00710CA0  (unnamed)
  0x00710A10  (unnamed)
  0x00710980  (unnamed)
  0x0047EB60  (unnamed)
  0x009196EE  __security_check_cookie
struct offsets touched (top 3):
  +0x54 [dec 84]  x5  via EAX,ECX,EDX
  +0x74 [dec 116]  x4  via ECX,EDX
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 82 B, RET 4
- callers 2: 0x004144f0 CSWGuiBorderParams::operator=, 0x00414a50 CSWGuiBorder::Constructor
- callees: 0x00414680 CSWGuiBorderParams::SetCornerImage (CALL), 0x00414760 CSWGuiBorderParams::SetEdgeImage (CALL), 0x00414840 CSWGuiBorderParams::SetFillImage (CALL)
- twins: none

## Card B18-2-09

GOG function: GLRender::FrameBufferModifications  gog 0x00827e30  callconv=(blank)  param_size_bytes=4  gog_size=1885  gog_retn=0
Candidate Steam address: 0x00435720 (size 1885, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x004356e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00827df0 GLRender::RenderDrawableSquares -> 0x00435720  gate_class=REVIEW
- GOG-order neighbour after:  0x00828590 GLRender::DrawLightmappedGrass  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0086b640 Global::AurFrameBufferModificationsAvailable -> Steam ?  in candidate callees: n/a
  0x0086b700 Global::AurCubeMapAvailable -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}
imm sets (* = in both, 2 common): GOG {0x6*,0x14*} | Steam {0x6*,0x14*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435720 FUN_00435720 size=1885 ret=0 callers=1 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00435720(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
  }
  if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
      wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1ddc8);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      wglMakeCurrent(DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1dddc);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
    else {
      (*DAT_00a32db4)(DAT_00a1ddc4,DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      (*DAT_00a32db4)(DAT_00a1ddd8,DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
  }
  if (DAT_00a1dde4 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde4);
    DAT_00a1dde4 = 0;
  }
  if (DAT_00a1dde8 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde8);
    DAT_00a1dde8 = 0;
  }
  if (DAT_00a1ddd0 != 0) {
    glDeleteTextures(1,&DAT_00a1ddd0);
    DAT_00a1ddd0 = 0;
  }
  if (DAT_00a1dd44 != 0) {
    glDeleteTextures(1,&DAT_00a1dd44);
    DAT_00a1dd44 = 0;
  }
  if (DAT_00a1dd38 != 0) {
    glDeleteTextures(1,&DAT_00a1dd38);
/* ... 111 more lines truncated by --max-lines */
```

### decoy: 0x004356e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B18-2-10

GOG function: CExoIni::ReadIniEntry  gog 0x00608450  callconv=(blank)  param_size_bytes=16  gog_size=39  gog_retn=16
Candidate Steam address: 0x00736350 (size 39, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00736320:STR0/CALLEE7/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006083f0 CExoIni::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00608480 CExoIni::WriteIniEntry  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060e100 CExoIniInternal::ReadIniEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x00427150 CSWCCreature::LipSync -> Steam 0x0077b190  in candidate callers: no
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam 0x00785df0  in candidate callers: yes
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: yes
  0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> Steam 0x0078a7a0  in candidate callers: no
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
prologue for 0x00736350: 16-byte aligned: YES
  00736350  55             PUSH EBP
  00736351  8B EC          MOV EBP,ESP
  00736353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00736350
string literals (0):
direct callees (1):
  0x0073C0E0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00736350 FUN_00736350 size=39 ret=16 callers=12 callees=1
THUNK -> 0x0073C0E0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00736350(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0073c0e0(param_1,param_2,param_3,param_4);
  return;
}
```

### decoy: 0x00736320 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00736320: 16-byte aligned: YES
  00736320  55             PUSH EBP
  00736321  8B EC          MOV EBP,ESP
  00736323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00736320
string literals (0):
direct callees (1):
  0x0073BA80  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 39 B, RET 16
- callers 12: 0x00407920, 0x0069ba00 Global::ExecuteCommandUnlockAllMusic, 0x00785df0 CClientExoAppInternal::SetEventDescriptions, 0x00788d90 CClientExoAppInternal::StartServices, 0x00796e00 CClientExoAppInternal::InitializeClientOptions, ...
- callees: 0x0073c0e0 (CALL)
- twins: 0x00736320 (distinguishing: 0x0073636c: CALL 0x0073c0e0)

## Card B18-2-11

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a6e0 (size 205, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.08 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0042a600:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c0340 CSWUpgradeItemEntry::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c0420 CSWUpgradeItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: no
  0x00437190 CClientExoAppInternal::StopServices -> Steam 0x0078ac40  in candidate callers: no
  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> Steam 0x0074d520  in candidate callers: no
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x20,0x24}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x4,0x8*,0xc,0x14,0xfff,0xfffff000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0042a6e0: 16-byte aligned: YES
  0042a6e0  55             PUSH EBP
  0042a6e1  8B EC          MOV EBP,ESP
  0042a6e3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a6e0
string literals (0):
direct callees (3):
  0x0091D288  operator new[]
  0x0091DBA0  _memcpy
  0x0091C6AA  operator delete[]
struct offsets touched (top 2):
  +0x24 [dec 36]  x4  via EAX,ECX,EDX
  +0x20 [dec 32]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0042a6e0 FUN_0042a6e0 size=205 ret=8 callers=0 callees=3

void __thiscall FUN_0042a6e0(int param_1,int param_2,int param_3)

{
  void *_Src [GHIDRA-AUTO-LABEL: unreliable];
  void *_Dst [GHIDRA-AUTO-LABEL: unreliable];
  
  DAT_00a1dd80 = (param_3 - *(int *)(*(int *)(param_1 + 0x24) + param_2 * 4)) + DAT_00a1dd80;
  DAT_00a1dd84 = ((param_3 + 0xfffU & 0xfffff000) -
                 (*(int *)(*(int *)(param_1 + 0x24) + param_2 * 4) + 0xfffU & 0xfffff000)) +
                 DAT_00a1dd84;
  _Src [GHIDRA-AUTO-LABEL: unreliable] = *(void **)(*(int *)(param_1 + 0x20) + param_2 * 4);
  _Dst [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_0091d288(param_3);
  _memcpy [GHIDRA-AUTO-LABEL: unreliable](_Dst [GHIDRA-AUTO-LABEL: unreliable],_Src [GHIDRA-AUTO-LABEL: unreliable],*(size_t *)(*(int *)(param_1 + 0x24) + param_2 * 4));
  FUN_0091c6aa(_Src [GHIDRA-AUTO-LABEL: unreliable]);
  *(int *)(*(int *)(param_1 + 0x24) + param_2 * 4) = param_3;
  *(void **)(*(int *)(param_1 + 0x20) + param_2 * 4) = _Dst [GHIDRA-AUTO-LABEL: unreliable];
  return;
}
```

### decoy: 0x0042a600 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a600: 16-byte aligned: YES
  0042a600  55             PUSH EBP
  0042a601  8B EC          MOV EBP,ESP
  0042a603  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0042a600
string literals (0):
direct callees (2):
  0x0091C6AA  operator delete[]
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
struct offsets touched (top 3):
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x1C [dec 28]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### Static facts (script output, report-only)
- size 205 B, RET 8
- callees: 0x0091d288 operator new[] (CALL), 0x0091dba0 _memcpy (CALL), 0x0091c6aa operator delete[] (CALL)
- twins: none

## Card B18-2-12

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x00435720 (size 1885, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004356e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications -> 0x00435720  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x28}
imm sets (* = in both, 0 common): GOG {0xc,0x18} | Steam {0x6,0x14}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00435720: 16-byte aligned: YES
  00435720  55             PUSH EBP
  00435721  8B EC          MOV EBP,ESP
  00435723  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435720
string literals (0):
direct callees (2):
  0x004848B0  (unnamed)
  0x00484970  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435720 FUN_00435720 size=1885 ret=0 callers=1 callees=6

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00435720(int param_1)

{
  int iVar1;
  int local_c;
  int local_8;
  
  if (DAT_00a1dd7c != (int *)0x0) {
    (**(code **)(*DAT_00a1dd7c + 0x28))();
  }
  if (DAT_00a1ddc4 != (HDC)0x0) {
    iVar1 = FUN_004848b0();
    if (iVar1 == 0) {
      wglMakeCurrent(DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1ddc8);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      wglMakeCurrent(DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      wglMakeCurrent(DAT_00a1bcb8,DAT_00a1dd24);
      wglDeleteContext(DAT_00a1dddc);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
    else {
      (*DAT_00a32db4)(DAT_00a1ddc4,DAT_00a1ddc4,DAT_00a1ddc8);
      glDeleteTextures(1,&DAT_00a1ddcc);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddc0,DAT_00a1ddc4);
      (*DAT_00a32b0c)(DAT_00a1ddc0);
      DAT_00a1ddc4 = (HDC)0x0;
      DAT_00a1ddc8 = (HGLRC)0x0;
      DAT_00a1ddc0 = 0;
      DAT_00a1ddcc = 0;
      (*DAT_00a32db4)(DAT_00a1ddd8,DAT_00a1ddd8,DAT_00a1dddc);
      glDeleteTextures(1,&DAT_00a1dde0);
      (*DAT_00a32db4)(DAT_00a1bcb8,DAT_00a1bcb8,DAT_00a1dd24);
      (*DAT_00a32ae8)(DAT_00a1ddd4,DAT_00a1ddd8);
      (*DAT_00a32b0c)(DAT_00a1ddd4);
      DAT_00a1ddd8 = (HDC)0x0;
      DAT_00a1dddc = (HGLRC)0x0;
      DAT_00a1ddd4 = 0;
      DAT_00a1dde0 = 0;
      glDeleteTextures(1,&DAT_00a1ddd0);
      DAT_00a1ddd0 = 0;
    }
  }
  if (DAT_00a1dde4 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde4);
    DAT_00a1dde4 = 0;
  }
  if (DAT_00a1dde8 != 0) {
    (*DAT_00a32af4)(1,&DAT_00a1dde8);
    DAT_00a1dde8 = 0;
  }
  if (DAT_00a1ddd0 != 0) {
    glDeleteTextures(1,&DAT_00a1ddd0);
    DAT_00a1ddd0 = 0;
  }
  if (DAT_00a1dd44 != 0) {
    glDeleteTextures(1,&DAT_00a1dd44);
    DAT_00a1dd44 = 0;
  }
  if (DAT_00a1dd38 != 0) {
    glDeleteTextures(1,&DAT_00a1dd38);
/* ... 111 more lines truncated by --max-lines */
```

### decoy: 0x004356e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 1885 B, RET 0
- callers 1: 0x00478300
- callees: 0x004848b0 (CALL), 0x00000023 (CALL), 0x00000045 (CALL), 0x00000044 (CALL), 0x00484970 (CALL), 0x00000121 (CALL)
- indirect: 0x0043573f slot 0x28, 0x0043576f unresolved, 0x00435796 unresolved, 0x004357aa unresolved, 0x004357b6 unresolved, ...
- globals: 0x00a1b484 READ
- imports: ReleaseDC (USER32.DLL), glDeleteTextures (OPENGL32.DLL), wglDeleteContext (OPENGL32.DLL), ...
- twins: none

## Card B18-2-13

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458a80 (size 90, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x00458ae0:STR0/CALLEE0/SIG0.25/SRV0/VT0
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
imm sets (* = in both, 3 common): GOG {0x4*,0x8*,0x64*} | Steam {0x4*,0x8*,0x64*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00458a80 FUN_00458a80 size=90 ret=0 callers=4 callees=2

undefined4 FUN_00458a80(char *param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = FUN_00458ae0(param_1);
  if ((iVar1 < DAT_00a1ffa0) &&
     (iVar2 = __stricmp [GHIDRA-AUTO-LABEL: unreliable]((char *)(*(int *)(DAT_00a1ff9c + iVar1 * 4) + 0x64 [dec 100]),param_1), iVar2 == 0)) {
    return *(undefined4 *)(DAT_00a1ff9c + iVar1 * 4);
  }
  return 0;
}
```

### decoy: 0x00458ae0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458ae0: 16-byte aligned: YES
  00458ae0  55             PUSH EBP
  00458ae1  8B EC          MOV EBP,ESP
  00458ae3  83 EC 14       SUB ESP,0x14
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458ae0
string literals (0):
direct callees (1):
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 90 B, RET 0
- callers 4: 0x004289d0, 0x00458b70, 0x00463900, 0x00464500
- callees: 0x00458ae0 (CALL), 0x009370c7 _stricmp (CALL)
- twins: none

## Card B18-2-14

GOG function: Global::AurEnableForceFury  gog 0x0086ae30  callconv=(blank)  param_size_bytes=2  gog_size=18  gog_retn=0
Candidate Steam address: 0x00478f70 (size 40, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00478f50:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0086ae20 Global::AurDisableForceSight  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ae50 Global::AurSetSaturationParams  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00834760 GLRender::EnableFury -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0044ab50 CClientExoAppInternal::EnableVideoEffect -> Steam 0x0079e550  in candidate callers: yes
  0x0044af60 CClientExoAppInternal::DisableVideoEffect -> Steam 0x0079e960  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4} | Steam {}

### abi
```
GOG param bytes: 2; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(2/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 2 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00478f70: 16-byte aligned: YES
  00478f70  55             PUSH EBP
  00478f71  8B EC          MOV EBP,ESP
  00478f73  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00478f70
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00478f70 FUN_00478f70 size=40 ret=0 callers=1 callees=0

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00478f70(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  _DAT_00a1de7c = param_1;
  _DAT_00a1de80 = param_2;
  _DAT_00a1de84 = param_3;
  _DAT_00a1dd5c = param_4;
  return;
}
```

### decoy: 0x00478f50 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00478f50: 16-byte aligned: YES
  00478f50  55             PUSH EBP
  00478f51  8B EC          MOV EBP,ESP
  00478f53  0F B7 45 08    MOVZX EAX,word ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00478f50
string literals (0):
direct callees (1):
  0x00442090  SetFuryLevel ([0xA1DD54], loads fx_Fury01..03)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 40 B, RET 0
- callers 1: 0x0079e550 CClientExoAppInternal::EnableVideoEffect
- twins: none

## Card B18-2-15

GOG function: CSWCollisionMesh::not_GetWalkableMaterialMask  gog 0x00713530  callconv=(blank)  param_size_bytes=0  gog_size=26  gog_retn=0
Candidate Steam address: 0x00650de0 (size 26, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: QueryTimer 0x650d40)
- best decoy (neighbour/other candidate) scores: 0x00650d40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00713490 Global::QueryTimer -> 0x00650d40  trusted
- GOG-order neighbour after:  0x00713550 CSWRoomSurfaceMesh::CheckAABBNode  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00713490 Global::QueryTimer -> Steam 0x00650d40  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x00650de0: 16-byte aligned: YES
  00650de0  55             PUSH EBP
  00650de1  8B EC          MOV EBP,ESP
  00650de3  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00650de0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00650de0 FUN_00650de0 size=26 ret=0 callers=1 callees=1

LARGE_INTEGER FUN_00650de0(void)

{
  LARGE_INTEGER local_c;
  
  QueryPerformanceFrequency(&local_c);
  return (LARGE_INTEGER)local_c.QuadPart;
}
```

### decoy: 0x00650d40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00650d40: 16-byte aligned: YES
  00650d40  55             PUSH EBP
  00650d41  8B EC          MOV EBP,ESP
  00650d43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00650d40
string literals (0):
direct callees (3):
  0x00650DE0  (unnamed)
  0x0091FC30  (unnamed)
  0x00920670  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 26 B, RET 0
- callers 1: 0x00650d40 Global::QueryTimer
- callees: 0x000000d6 (CALL)
- imports: QueryPerformanceFrequency (KERNEL32.DLL)
- twins: none

## Card B18-2-16

GOG function: CSWGuiObject::SetExtent  gog 0x00912f40  callconv=__thiscall  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00414bc0 (size 45, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00414bf0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00912e20 CSWGuiBorder::Constructor -> 0x00414a50  trusted
- GOG-order neighbour after:  0x00912f70 CRes::GetData  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
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
prologue for 0x00414bc0: 16-byte aligned: YES
  00414bc0  55             PUSH EBP
  00414bc1  8B EC          MOV EBP,ESP
  00414bc3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00414bc0
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00414bc0 FUN_00414bc0 size=45 ret=4 callers=0 callees=1

undefined4 * __thiscall FUN_00414bc0(undefined4 *param_1,uint param_2)

{
  *param_1 = CSWGuiDrawable::vftable;
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00414bf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414bf0: 16-byte aligned: YES
  00414bf0  55             PUSH EBP
  00414bf1  8B EC          MOV EBP,ESP
  00414bf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414bf0
string literals (0):
direct callees (2):
  0x00414C60  CSWGuiBorder::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 45 B, RET 4
- callees: 0x009196fd operator delete (CALL)
- twins: none
