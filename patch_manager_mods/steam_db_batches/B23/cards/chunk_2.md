# Review cards B23 chunk 2

Answer each card with one TSV row in `steam_db_batches/B23/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B23-2-01

GOG function: CExoInput::UpdateMouseDelta  gog 0x00616990  callconv=(blank)  param_size_bytes=4  gog_size=31  gog_retn=4
Candidate Steam address: 0x0072fd90 (size 31, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callees: UpdateMouseDelta 0x72f110; callers: ProcessInput 0x7ac470)
- best decoy (neighbour/other candidate) scores: 0x0072fd70:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00616970 CExoInput::GetMouseDelta  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x006169b0 CExoInput::GetJoystickNStick  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00615ec0 CExoInputInternal::UpdateMouseDelta -> Steam 0x0072f110  in candidate callees: yes
GOG callers mapped through trusted rows (1 of 1): the card's A-CALLEE credit may come from these, not from callees
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: yes
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
prologue for 0x0072fd90: 16-byte aligned: YES
  0072fd90  55             PUSH EBP
  0072fd91  8B EC          MOV EBP,ESP
  0072fd93  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0072fd90
string literals (0):
direct callees (1):
  0x0072F110  CExoInputInternal::UpdateMouseDelta
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0072fd90 FUN_0072fd90 size=31 ret=4 callers=1 callees=1
THUNK -> 0x0072F110 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0072fd90(undefined4 param_1)

{
  FUN_0072f110(param_1);
  return;
}
```

### decoy: 0x0072fd70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072fd70: 16-byte aligned: YES
  0072fd70  55             PUSH EBP
  0072fd71  8B EC          MOV EBP,ESP
  0072fd73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072fd70
string literals (0):
direct callees (1):
  0x0072ED40  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EDX
```

### Static facts (script output, report-only)
- size 31 B, RET 4
- callers 1: 0x007ac470 CClientExoAppInternal::ProcessInput
- callees: 0x0072f110 CExoInputInternal::UpdateMouseDelta (CALL)
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0072fd90: static only
- xref | no live-only facts | probable | rea_facts 0x0072fd90: checked

## Card B23-2-02

GOG function: CAuroraModel::GetModelDataPtr  gog 0x0090bab0  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x00941420 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00941440:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090ba60 CAurora16BitTexture::GetTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090baf0 CAurora16BitTexture::Load -> 0x009414d0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x007f7ad0 CResVIS::GetVISDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {}
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
prologue for 0x00941420: 16-byte aligned: YES
  00941420  55             PUSH EBP
  00941421  8B EC          MOV EBP,ESP
  00941423  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00941420
string literals (0):
direct callees (1):
  0x00941660  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00941420 FUN_00941420 size=28 ret=0 callers=3 callees=1

void __fastcall FUN_00941420(undefined4 *param_1)

{
  *param_1 = CAurora16BitTexture::vftable;
  FUN_00941660();
  return;
}
```

### decoy: 0x00941440 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00941440: 16-byte aligned: YES
  00941440  55             PUSH EBP
  00941441  8B EC          MOV EBP,ESP
  00941443  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00941440
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00942100  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 28 B, RET 0
- callers 3: 0x0093c550, 0x009413f0, 0x00982d50
- callees: 0x00941660 (CALL)
- twins: 0x00941020 (distinguishing: 0x0094142a: MOV dword ptr [EAX],0x9b95e8)

### Live evidence (REA, report-only)
- (no REA rows for 0x00941420)

## Card B23-2-03

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e750 (size 20, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e5a0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection -> 0x0040e360  gate_class=REVIEW
- GOG-order neighbour after:  0x0090cb30 CSWGuiPanel::Constructor -> 0x0040e5a0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc} | Steam {}
imm sets (* = in both, 0 common): GOG {0x28} | Steam {}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040e750: 16-byte aligned: YES
  0040e750  55             PUSH EBP
  0040e751  8B EC          MOV EBP,ESP
  0040e753  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e750
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0040e750 FUN_0040e750 size=20 ret=0 callers=4 callees=0

void __fastcall FUN_0040e750(undefined4 *param_1)

{
  *param_1 = CSWGuiObject::vftable;
  return;
}
```

### decoy: 0x0040e5a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e5a0: 16-byte aligned: YES
  0040e5a0  55             PUSH EBP
  0040e5a1  8B EC          MOV EBP,ESP
  0040e5a3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0040e5a0
string literals (0):
direct callees (3):
  0x0084A640  (unnamed)
  0x00919723  operator new
  0x00414A50  CSWGuiBorder::Constructor
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via EDX
  +0x18 [dec 24]  x1  via EAX
  +0x1C [dec 28]  x1  via EAX
  +0x20 [dec 32]  x1  via EDX
  +0x30 [dec 48]  x1  via EDX
  +0x4C [dec 76]  x1  via ECX
  +0x50 [dec 80]  x1  via EAX
```

### Static facts (script output, report-only)
- size 20 B, RET 0
- callers 4: 0x00948290, 0x009482d0, 0x00948910, 0x00948950
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0040e750)

## Card B23-2-04

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2380 (size 21, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2360:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject -> 0x007a2540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004990c0 CSWCMessage::HandleServerToPlayerCreatureUpdate_Add -> Steam 0x00804ad0  in candidate callers: no
  0x0049af50 CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect -> Steam 0x00806960  in candidate callers: no
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a2380: 16-byte aligned: YES
  007a2380  55             PUSH EBP
  007a2381  8B EC          MOV EBP,ESP
  007a2383  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2380
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2380 FUN_007a2380 size=21 ret=4 callers=2 callees=0

void __thiscall FUN_007a2380(undefined4 *param_1,undefined4 param_2)

{
  *param_1 = param_2;
  return;
}
```

### decoy: 0x007a2360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2360: 16-byte aligned: YES
  007a2360  55             PUSH EBP
  007a2361  8B EC          MOV EBP,ESP
  007a2363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2360
string literals (0):
direct callees (1):
  0x007A2040  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 21 B, RET 4
- callers 2: 0x005363a0 CServerExoAppInternal::StartServices, 0x00788d90 CClientExoAppInternal::StartServices
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x007a2380)

## Card B23-2-05

GOG function: CAuroraTexture::GetTextureDataPtr  gog 0x00909e50  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f830 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093f7e0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00909e00 CAuroraTexture::GetTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00909e90 CAuroraTexture::Load  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x00943a40 CResTGA::GetTGADataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093f830: 16-byte aligned: YES
  0093f830  55             PUSH EBP
  0093f831  8B EC          MOV EBP,ESP
  0093f833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f830
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453F0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093f830 FUN_0093f830 size=51 ret=0 callers=3 callees=2

undefined4 __fastcall FUN_0093f830(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_009453f0();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093f7e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f7e0: 16-byte aligned: YES
  0093f7e0  55             PUSH EBP
  0093f7e1  8B EC          MOV EBP,ESP
  0093f7e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f7e0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453A0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 3: 0x0093b910, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x0093fab0
- callees: 0x00711250 (CALL), 0x009453f0 (CALL)
- twins: 0x0093f870 (distinguishing: 0x0093f85a: CALL 0x009453f0)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093f830: static only
- xref | no live-only facts | probable | rea_facts 0x0093f830: checked

## Card B23-2-06

GOG function: CAuroraLayeredTexture::GetTextureDataPtr  gog 0x0090a510  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093ff30 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetPtrToResHelperInfo 0x9402d0)
- best decoy (neighbour/other candidate) scores: 0x0093fed0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a4b0 CAuroraProcessedTexture::GetProcessedTextureAttrib  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x0090c1c0 CResTPC::GetTPCDataPtr -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 5): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093ff30: 16-byte aligned: YES
  0093ff30  55             PUSH EBP
  0093ff31  8B EC          MOV EBP,ESP
  0093ff33  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093ff30
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00455820  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093ff30 FUN_0093ff30 size=51 ret=0 callers=5 callees=2

undefined4 __fastcall FUN_0093ff30(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_00455820();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093fed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093fed0: 16-byte aligned: YES
  0093fed0  55             PUSH EBP
  0093fed1  8B EC          MOV EBP,ESP
  0093fed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093fed0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945DB0  CResTPC::GetTPCAttrib
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 5: 0x0093b860, 0x0093b9c0, 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x0093f3d0, 0x009402d0 CAuroraProcessedTexture::GetPtrToResHelperInfo
- callees: 0x00711250 (CALL), 0x00455820 (CALL)
- twins: 0x0093ff70 (distinguishing: 0x0093ff5a: CALL 0x00455820)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093ff30: static only
- xref | no live-only facts | probable | rea_facts 0x0093ff30: checked

## Card B23-2-07

GOG function: CAuroraProcessedTexture::GetProcessedTextureDataSize  gog 0x0090a550  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093feb0 (size 28, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093fed0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a510 CAuroraLayeredTexture::GetTextureDataPtr -> 0x0093ff30  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a590 CAuroraTexture::GetTextureDataSize -> 0x0093f870  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
  0x009444c0 CResTPC::GetTPCSize -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x8} | Steam {}
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
prologue for 0x0093feb0: 16-byte aligned: YES
  0093feb0  55             PUSH EBP
  0093feb1  8B EC          MOV EBP,ESP
  0093feb3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093feb0
string literals (0):
direct callees (1):
  0x009404A0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093feb0 FUN_0093feb0 size=28 ret=0 callers=3 callees=1

void __fastcall FUN_0093feb0(undefined4 *param_1)

{
  *param_1 = CAuroraProcessedTexture::vftable;
  FUN_009404a0();
  return;
}
```

### decoy: 0x0093fed0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093fed0: 16-byte aligned: YES
  0093fed0  55             PUSH EBP
  0093fed1  8B EC          MOV EBP,ESP
  0093fed3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093fed0
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945DB0  CResTPC::GetTPCAttrib
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX
```

### Static facts (script output, report-only)
- size 28 B, RET 0
- callers 3: 0x0093c300, 0x0093fe80, 0x00982cc0
- callees: 0x009404a0 (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x0093feb0)

## Card B23-2-08

GOG function: CSWGuiSaveNamePanel::OnPanelRemoved  gog 0x005d7fe0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x00858a70 (size 225, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00858b60:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d7ef0 CSWGuiSaveNamePanel::OnPanelAdded  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8000 CSWGuiSaveNamePanel::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x2c,0x40c}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00858a70: 16-byte aligned: YES
  00858a70  55             PUSH EBP
  00858a71  8B EC          MOV EBP,ESP
  00858a73  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00858a70
string literals (0):
direct callees (5):
  0x004215E0  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
  0x0073EEE0  (unnamed)
  0x00414150  (unnamed)
  0x00414230  (unnamed)
struct offsets touched (top 3):
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via ECX
  +0xC [dec 12]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00858a70 FUN_00858a70 size=225 ret=0 callers=0 callees=5

void __fastcall FUN_00858a70(int param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  char cVar3;
  
  FUN_004215e0();
  FUN_0040f3c0();
  cVar3 = FUN_0073eee0();
  if (cVar3 == '\0') {
    uVar1 = *(undefined4 *)(param_1 + 4);
    uVar2 = *(undefined4 *)(param_1 + 8);
    FUN_00414150();
    FUN_00414230(uVar1,uVar2);
  }
  return;
}
```

### decoy: 0x00858b60 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00858b60: 16-byte aligned: YES
  00858b60  55             PUSH EBP
  00858b61  8B EC          MOV EBP,ESP
  00858b63  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00858b60
string literals (0):
direct callees (2):
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
  0x00414180  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 225 B, RET 0
- callees: 0x004215e0 (CALL), 0x0040f3c0 CSWGuiPanel::OnPanelAdded (CALL), 0x0073eee0 (CALL), 0x00414150 (CALL), 0x00414230 (CALL)
- globals: 0x00a1b49c READ
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x00858a70)

## Card B23-2-09

GOG function: CSWGuiContainer::OnPanelRemoved  gog 0x005a72d0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x008b3630 (size 243, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWGuiContainer::OnPanelAdded | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b3730:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a71d0 CSWGuiContainer::OnPanelAdded -> 0x008b3630  trusted
- GOG-order neighbour after:  0x005a72f0 CSWGuiStoreItemEntry::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 0 common): GOG {} | Steam {0x4,0x2c,0xb18}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b3630: 16-byte aligned: YES
  008b3630  55             PUSH EBP
  008b3631  8B EC          MOV EBP,ESP
  008b3633  83 EC 2C       SUB ESP,0x2c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008b3630
string literals (0):
direct callees (4):
  0x0073EEE0  (unnamed)
  0x00414150  (unnamed)
  0x00414230  (unnamed)
  0x0040F3C0  CSWGuiPanel::OnPanelAdded
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX
  +0x8 [dec 8]  x2  via ECX
  +0xC [dec 12]  x2  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 008b3630 FUN_008b3630 size=243 ret=0 callers=0 callees=4

void __fastcall FUN_008b3630(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  char cVar7;
  
  cVar7 = FUN_0073eee0();
  if (cVar7 == '\0') {
    iVar1 = *(int *)(param_1 + 4);
    iVar2 = *(int *)(param_1 + 8);
    iVar3 = *(int *)(param_1 + 0xb18);
    iVar4 = *(int *)(param_1 + 0xb1c);
    iVar5 = *(int *)(param_1 + 0xb20);
    iVar6 = *(int *)(param_1 + 0xb24);
    FUN_00414150();
    FUN_00414230(iVar5 / 2 + iVar1 + iVar3,iVar6 / 2 + iVar2 + iVar4);
    FUN_0040f3c0();
  }
  return;
}
```

### decoy: 0x008b3730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b3730: 16-byte aligned: YES
  008b3730  55             PUSH EBP
  008b3731  8B EC          MOV EBP,ESP
  008b3733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b3730
string literals (0):
direct callees (2):
  0x00414180  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 243 B, RET 0
- callees: 0x0073eee0 (CALL), 0x00414150 (CALL), 0x00414230 (CALL), 0x0040f3c0 CSWGuiPanel::OnPanelAdded (CALL)
- globals: 0x00a1b49c READ
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x008b3630)

## Card B23-2-10

GOG function: CAuroraTexture::GetTextureDataSize  gog 0x0090a590  callconv=(blank)  param_size_bytes=0  gog_size=51  gog_retn=0
Candidate Steam address: 0x0093f870 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callers: GetPtrToResHelperInfo 0x9402d0)
- best decoy (neighbour/other candidate) scores: 0x0093f830:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0090a550 CAuroraProcessedTexture::GetProcessedTextureDataSize -> 0x0093feb0  gate_class=REVIEW
- GOG-order neighbour after:  0x0090a5d0 CAuroraProcessedTexture::Load -> 0x0093ffb0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005915c0 CResTPC::GetTPCTXISize -> Steam ?  in candidate callees: n/a
  0x0061abb0 CRes::GetDemands -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (1 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0090a8f0 CAuroraProcessedTexture::GetPtrToResHelperInfo -> Steam 0x009402d0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}
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
prologue for 0x0093f870: 16-byte aligned: YES
  0093f870  55             PUSH EBP
  0093f871  8B EC          MOV EBP,ESP
  0093f873  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093f870
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x00945EA0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0093f870 FUN_0093f870 size=51 ret=0 callers=2 callees=2

undefined4 __fastcall FUN_0093f870(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  if ((*(int *)(param_1 + 8) != 0) && (iVar1 = FUN_00711250(), 0 < iVar1)) {
    uVar2 = FUN_00945ea0();
    return uVar2;
  }
  return 0;
}
```

### decoy: 0x0093f830 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093f830: 16-byte aligned: YES
  0093f830  55             PUSH EBP
  0093f831  8B EC          MOV EBP,ESP
  0093f833  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093f830
string literals (0):
direct callees (2):
  0x00711250  (unnamed)
  0x009453F0  (unnamed)
struct offsets touched (top 1):
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 2: 0x0093bc00 ResourceProvider(name, ext, &size) (DAT_00A3082C), 0x009402d0 CAuroraProcessedTexture::GetPtrToResHelperInfo
- callees: 0x00711250 (CALL), 0x00945ea0 (CALL)
- twins: 0x0093f830 (distinguishing: 0x0093f89a: CALL 0x00945ea0)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0093f870: static only
- xref | no live-only facts | probable | rea_facts 0x0093f870: checked

## Card B23-2-11

GOG function: CSWGuiContainer::OnPanelRemoved  gog 0x005a72d0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x008b3750 (size 194, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008b3730:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a71d0 CSWGuiContainer::OnPanelAdded -> 0x008b3630  trusted
- GOG-order neighbour after:  0x005a72f0 CSWGuiStoreItemEntry::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1d0,0x3b0,0x3b4}
imm sets (* = in both, 0 common): GOG {} | Steam {0x2,0x3,0x1d4,0x24c,0x2c4,0x33c,0x7f000000}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x008b3750: 16-byte aligned: YES
  008b3750  55             PUSH EBP
  008b3751  8B EC          MOV EBP,ESP
  008b3753  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x008b3750
string literals (0):
direct callees (3):
  0x0041A510  ButtonCtor
  0x00414A50  CSWGuiBorder::Constructor
  0x004171A0  CSWGuiText::Constructor
struct offsets touched (top 3):
  +0x1D0 [dec 464]  x1  via ECX
  +0x3B0 [dec 944]  x1  via EDX
  +0x3B4 [dec 948]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 008b3750 FUN_008b3750 size=194 ret=0 callers=3 callees=3

undefined4 * __fastcall FUN_008b3750(undefined4 *param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00978fc2;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_0041a510(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  *param_1 = CSWGuiStoreItemEntry::vftable;
  FUN_00414a50();
  local_8._0_1_ = 1;
  FUN_00414a50();
  local_8._0_1_ = 2;
  FUN_00414a50();
  local_8 = CONCAT31(local_8._1_3_,3);
  FUN_004171a0();
  param_1[0x74 = byte +0x1d0] = 0x7f000000;
  param_1[0xec = byte +0x3b0] = 0xffffffff;
  param_1[0xed = byte +0x3b4] = 0;
  ExceptionList = local_10;
  return param_1;
}
```

### decoy: 0x008b3730 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b3730: 16-byte aligned: YES
  008b3730  55             PUSH EBP
  008b3731  8B EC          MOV EBP,ESP
  008b3733  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b3730
string literals (0):
direct callees (2):
  0x00414180  (unnamed)
  0x0040F4C0  CSWGuiPanel::OnPanelRemoved
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 194 B, RET 0
- callers 3: 0x008b5db0, 0x008d3900, 0x008d8c50
- callees: 0x0041a510 (CALL), 0x00414a50 CSWGuiBorder::Constructor (CALL), 0x004171a0 CSWGuiText::Constructor (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x008b3750)

## Card B23-2-12

GOG function: CSWGuiSaveNamePanel::OnPanelRemoved  gog 0x005d7fe0  callconv=(blank)  param_size_bytes=0  gog_size=30  gog_retn=0
Candidate Steam address: 0x00859250 (size 24, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x008591d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005d7ef0 CSWGuiSaveNamePanel::OnPanelAdded  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005d8000 CSWGuiSaveNamePanel::HandleInputEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090d940 CSWGuiPanel::OnPanelRemoved -> Steam 0x0040f4c0  in candidate callees: no
  0x00912600 CSWGuiManager::RestoreOldMousePosition -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x1e40}
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
prologue for 0x00859250: 16-byte aligned: YES
  00859250  55             PUSH EBP
  00859251  8B EC          MOV EBP,ESP
  00859253  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00859250
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x1E40 [dec 7744]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00859250 FUN_00859250 size=24 ret=0 callers=2 callees=0

void __fastcall FUN_00859250(int param_1)

{
  *(undefined4 *)(param_1 + 0x1e40) = 1;
  return;
}
```

### decoy: 0x008591d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008591d0: 16-byte aligned: YES
  008591d0  55             PUSH EBP
  008591d1  8B EC          MOV EBP,ESP
  008591d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008591d0
string literals (0):
direct callees (2):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x00852380  (unnamed)
struct offsets touched (top 4):
  +0x1E2C [dec 7724]  x4  via EAX,ECX,EDX
  +0x1C [dec 28]  x1  via EDX
  +0x50 [dec 80]  x1  via EAX
  +0x1E34 [dec 7732]  x1  via EAX
```

### Static facts (script output, report-only)
- size 24 B, RET 0
- callers 2: 0x00760920, 0x00760aa0
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x00859250)

## Card B23-2-13

GOG function: CSWGuiExtent::Union  gog 0x0090ca10  callconv=(blank)  param_size_bytes=12  gog_size=285  gog_retn=0
Candidate Steam address: 0x0040e360 (size 285, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0040e480:STR0/CALLEE0/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection -> 0x0040e360  gate_class=REVIEW
- GOG-order neighbour after:  0x0090cb30 CSWGuiPanel::Constructor -> 0x0040e5a0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 3 common): GOG {0x4*,0x8*,0xc*} | Steam {0x4*,0x8*,0xc*}
imm sets (* = in both, 1 common): GOG {0x28*} | Steam {0x28*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 12 bytes -> x86 slots 12; ASM RET 0)
```

### prologue
```
prologue for 0x0040e360: 16-byte aligned: YES
  0040e360  55             PUSH EBP
  0040e361  8B EC          MOV EBP,ESP
  0040e363  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0040e360
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0040e360 FUN_0040e360 size=285 ret=0 callers=10 callees=0

int * FUN_0040e360(int *param_1,int *param_2,int *param_3)

{
  int local_2c;
  int local_28;
  int local_24;
  int local_20;
  
  if (*param_3 < *param_2) {
    local_20 = *param_2;
  }
  else {
    local_20 = *param_3;
  }
  if (param_3[1 = byte +0x4] < param_2[1 = byte +0x4]) {
    local_24 = param_2[1 = byte +0x4];
  }
  else {
    local_24 = param_3[1 = byte +0x4];
  }
  if (*param_2 + param_2[2 = byte +0x8] < *param_3 + param_3[2 = byte +0x8]) {
    local_28 = *param_2 + param_2[2 = byte +0x8];
  }
  else {
    local_28 = *param_3 + param_3[2 = byte +0x8];
  }
  if (param_2[1 = byte +0x4] + param_2[3 = byte +0xc] < param_3[1 = byte +0x4] + param_3[3 = byte +0xc]) {
    local_2c = param_2[1 = byte +0x4] + param_2[3 = byte +0xc];
  }
  else {
    local_2c = param_3[1 = byte +0x4] + param_3[3 = byte +0xc];
  }
  *param_1 = local_20;
  param_1[1 = byte +0x4] = local_24;
  param_1[2 = byte +0x8] = local_28 - local_20;
  param_1[3 = byte +0xc] = local_2c - local_24;
  return param_1;
}
```

### decoy: 0x0040e480 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0040e480: 16-byte aligned: YES
  0040e480  55             PUSH EBP
  0040e481  8B EC          MOV EBP,ESP
  0040e483  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0040e480
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x5  via EAX
  +0xC [dec 12]  x5  via EAX,ECX
```

### Static facts (script output, report-only)
- size 285 B, RET 0
- callers 10: 0x00419a80, 0x00419c80, 0x00419dc0, 0x00419ed0 CSWGuiButton::SetExtent, 0x0041a670, ...
- twins: none

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x0040e360: static only
- xref | no live-only facts | probable | rea_facts 0x0040e360: checked

## Card B23-2-14

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2540 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2510:STR0/CALLEE7/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject -> 0x007a2540  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004990c0 CSWCMessage::HandleServerToPlayerCreatureUpdate_Add -> Steam 0x00804ad0  in candidate callers: no
  0x0049af50 CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect -> Steam 0x00806960  in candidate callers: no
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: no
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

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x007a2540: static only
- xref | no live-only facts | probable | rea_facts 0x007a2540: checked

## Card B23-2-15

GOG function: CClientAIMaster::RemoveObject  gog 0x0044eee0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2510 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2540:STR0/CALLEE2/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044eeb0 CClientAIMaster::AddObject -> 0x007a2510  gate_class=REVIEW
- GOG-order neighbour after:  0x0044ef10 Global::PlayRoomSoundEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044eb90 CClientAIList::RemoveObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: no
  0x004727b0 CSWCObject::Destructor_3 -> Steam 0x007ed5f0  in candidate callers: no
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
prologue for 0x007a2510: 16-byte aligned: YES
  007a2510  55             PUSH EBP
  007a2511  8B EC          MOV EBP,ESP
  007a2513  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2510
string literals (0):
direct callees (1):
  0x007A2110  CClientExoAppInternal::AddMoveToModuleMovie_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2510 FUN_007a2510 size=36 ret=4 callers=13 callees=1
THUNK -> 0x007A2110 (Ghidra may omit __thiscall on thunks; ECX is passed through)

undefined4 FUN_007a2510(undefined4 param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_007a2110(param_1);
  return uVar1;
}
```

### decoy: 0x007a2540 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2540: 16-byte aligned: YES
  007a2540  55             PUSH EBP
  007a2541  8B EC          MOV EBP,ESP
  007a2543  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007a2540
string literals (0):
direct callees (1):
  0x007A21D0  CClientExoAppInternal::SetGameOver
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 36 B, RET 4
- callers 13: 0x007fddf0, 0x00800cb0 CSWCSoundObject::AddToArea, 0x00804ad0 CSWCMessage::HandleServerToPlayerCreatureUpdate_Add, 0x00806960 CSWCMessage::HandleServerToPlayerCreatureUpdate_VisEffect, 0x0080c6b0, ...
- callees: 0x007a2110 (CALL)
- twins: 0x007a2540 (distinguishing: 0x007a2523: CALL 0x007a2110)

### Live evidence (REA, report-only)
- xref | no live run (static only) | probable | rea_facts 0x007a2510: static only
- xref | no live-only facts | probable | rea_facts 0x007a2510: checked

## Card B23-2-16

GOG function: CClientAIMaster::RemoveObject  gog 0x0044eee0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2c80 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CSWCArea::Destructor | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2cb0:STR0/CALLEE0/SIG0.04/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044eeb0 CClientAIMaster::AddObject -> 0x007a2510  gate_class=REVIEW
- GOG-order neighbour after:  0x0044ef10 Global::PlayRoomSoundEvent  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044eb90 CClientAIList::RemoveObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x004529a0 CSWCArea::UnloadArea -> Steam 0x007a5fa0  in candidate callers: no
  0x004727b0 CSWCObject::Destructor_3 -> Steam 0x007ed5f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
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
prologue for 0x007a2c80: 16-byte aligned: YES
  007a2c80  55             PUSH EBP
  007a2c81  8B EC          MOV EBP,ESP
  007a2c83  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007a2c80
string literals (0):
direct callees (2):
  0x007A2CB0  CSWCArea::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 007a2c80 FUN_007a2c80 size=44 ret=4 callers=1 callees=2

undefined4 __thiscall FUN_007a2c80(undefined4 param_1,uint param_2)

{
  FUN_007a2cb0();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x007a2cb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007a2cb0: 16-byte aligned: YES
  007a2cb0  55             PUSH EBP
  007a2cb1  8B EC          MOV EBP,ESP
  007a2cb3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007a2cb0
string literals (0):
direct callees (9):
  0x007A5FA0  CSWCArea::UnloadArea
  0x00890040  CSWCAmbientSound dtor
  0x009196FD  operator delete
  0x0052DC30  CSWSArea::DetachFromClientArea
  0x00473E50  operator delete[] wrapper (`if (*p) FUN_0091c6aa(*p)`)
  0x0072F5F0  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x005E7970  (unnamed)
struct offsets touched (top 8):
  +0x280 [dec 640]  x3  via EAX,ECX,EDX
  +0x104 [dec 260]  x2  via EAX,ECX
  +0x1E0 [dec 480]  x2  via ECX
  +0x264 [dec 612]  x2  via EAX,EDX
  +0x268 [dec 616]  x2  via ECX,EDX
  +0x288 [dec 648]  x2  via ECX,EDX
  +0x4C [dec 76]  x1  via EDX
  +0x28C [dec 652]  x1  via EAX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callers 1: 0x007ab0c0
- callees: 0x007a2cb0 CSWCArea::Destructor_2 (CALL), 0x009196fd operator delete (CALL)
- twins: none

### Live evidence (REA, report-only)
- (no REA rows for 0x007a2c80)
