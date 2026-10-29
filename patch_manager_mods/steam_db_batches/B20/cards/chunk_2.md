# Review cards B20 chunk 2

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-2-01

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
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection  (not in this batch, no trusted mapping)
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

## Card B20-2-02

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
- GOG-order neighbour before: 0x0090c8f0 CSWGuiExtent::Intersection  (not in this batch, no trusted mapping)
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

## Card B20-2-03

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x004356e0 (size 54, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004356c0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0xc*,0x18*} | Steam {0xc*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x004356e0: 16-byte aligned: YES
  004356e0  55             PUSH EBP
  004356e1  8B EC          MOV EBP,ESP
  004356e3  FF 15 3C 63 98 00 CALL dword ptr [0x0098633c]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x004356e0
string literals (0):
direct callees (1):
  0x0091C630  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 004356e0 FUN_004356e0 size=54 ret=0 callers=1 callees=3

void FUN_004356e0(void)

{
  DAT_00a1dd24 = wglGetCurrentContext();
  DAT_00a1bcb8 = wglGetCurrentDC();
  _memset [GHIDRA-AUTO-LABEL: unreliable](&DAT_00a1dd0c,0,0x18);
  DAT_00a1dd70 = 0;
  return;
}
```

### decoy: 0x004356c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004356c0: 16-byte aligned: YES
  004356c0  55             PUSH EBP
  004356c1  8B EC          MOV EBP,ESP
  004356c3  8B 45 0C       MOV EAX,dword ptr [EBP + 0xc]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004356c0
string literals (0):
direct callees (1):
  0x00435470  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 54 B, RET 0
- callers 1: 0x00476be0
- callees: 0x00000043 (CALL), 0x0000003d (CALL), 0x0091c630 (CALL)
- imports: wglGetCurrentContext (OPENGL32.DLL), wglGetCurrentDC (OPENGL32.DLL)
- twins: none

## Card B20-2-04

GOG function: CExoTimers::GetLowResolutionTimer  gog 0x00608330  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x00736200 (size 21, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 5 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 5 (callers: AIUpdate_2 0x7664b0, ShowLoadScreen 0x798190, UpdateLoadingScreenHint 0x79fa50)
- best decoy (neighbour/other candidate) scores: 0x007361b0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006082e0 CExoTimers::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00608350 CExoTimers::GetHighResolutionTimer  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060de70 CExoTimersInternal::GetLowResolutionTimer -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 14): the card's A-CALLEE credit may come from these, not from callees
  0x00412180 CSWCCreature::AIUpdate_2 -> Steam 0x007664b0  in candidate callers: yes
  0x004447b0 CClientExoAppInternal::ShowLoadScreen -> Steam 0x00798190  in candidate callers: yes
  0x0044c050 CClientExoAppInternal::UpdateLoadingScreenHint -> Steam 0x0079fa50  in candidate callers: yes
  0x007b7980 CServerExoAppInternal::StallEventSaveGame -> Steam 0x00534600  in candidate callers: yes
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
prologue for 0x00736200: 16-byte aligned: YES
  00736200  55             PUSH EBP
  00736201  8B EC          MOV EBP,ESP
  00736203  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00736200
string literals (0):
direct callees (1):
  0x0073B7F0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00736200 FUN_00736200 size=21 ret=0 callers=14 callees=1
THUNK -> 0x0073B7F0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00736200(undefined4 param_1)

{
  FUN_0073b7f0(param_1);
  return;
}
```

### decoy: 0x007361b0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007361b0: 16-byte aligned: YES
  007361b0  55             PUSH EBP
  007361b1  8B EC          MOV EBP,ESP
  007361b3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007361b0
string literals (0):
direct callees (2):
  0x00623320  CSWSEncounter::AIUpdate
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 21 B, RET 0
- callers 14: 0x00410aa0, 0x00534600 CServerExoAppInternal::StallEventSaveGame, 0x00539c20 CServerExoAppInternal::UpdateAutoSaveTimer, 0x00705d70, 0x0070dfa0, ...
- callees: 0x0073b7f0 (CALL)
- twins: 0x00736220 (distinguishing: 0x0073620c: CALL 0x0073b7f0)

## Card B20-2-05

GOG function: CSWGuiInGameJournal::PopulateItemListBox  gog 0x00591bd0  callconv=(blank)  param_size_bytes=0  gog_size=1005  gog_retn=0
Candidate Steam address: 0x007fc880 (size 1005, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.65) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 1.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 3 (callees: GetQuestJournal 0x73f070, GetClientLanguage 0x73f350, SetSelectedControl 0x41e870); SIG 1.00
- best decoy (neighbour/other candidate) scores: 0x007fc790:STR0/CALLEE0/SIG0.16/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00591ae0 CSWGuiJournalItemEntry::SetNew  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00591fc0 CSWCJournal::ClearDoneHasChanged  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (27), mapped to Steam through trusted rows only:
  0x0040c530 CClientExoApp::GetQuestJournal -> Steam 0x0073f070  in candidate callees: yes
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam 0x0073f350  in candidate callees: yes
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: yes
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x005115b0 CSWTlkTable::GetObjectGender_2 -> Steam ?  in candidate callees: n/a
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  ... 19 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 17 common): GOG {0x4*,0x8*,0xc*,0x2c*,0x3c*,0x40*,0x58*,0x5c*,0x70*,0x98*,0x1d0*,0x1d4*,0x2a4*,0x2d0*,...(+3)} | Steam {0x4*,0x8*,0xc*,0x2c*,0x3c*,0x40*,0x58*,0x5c*,0x70*,0x98*,0x1d0*,0x1d4*,0x2a4*,0x2d0*,...(+3)}
imm sets (* = in both, 11 common): GOG {0x2*,0x3*,0x4*,0x3c*,0x7b*,0x80*,0x178*,0x1d8*,0x590*,0x5e8*,0xfffffffe*} | Steam {0x2*,0x3*,0x4*,0x3c*,0x7b*,0x80*,0x178*,0x1d8*,0x590*,0x5e8*,0xfffffffe*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007fc880: 16-byte aligned: YES
  007fc880  55             PUSH EBP
  007fc881  8B EC          MOV EBP,ESP
  007fc883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x007fc880
string literals (0):
direct callees (25):
  0x0073F070  CClientExoApp::GetQuestJournal
  0x00733540  CExoString::Constructor
  0x0073F350  CClientExoApp::GetClientLanguage
  0x0084A640  (unnamed)
  0x00479AB0  Global::GetResolutionTextScalar
  0x0091C860  _ftol2_sse
  0x00919723  operator new
  0x0041A510  ButtonCtor
  0x00418AF0  CSWGuiControl::AddEvent
  0x007356B0  CExoLocString::GetString_3
  0x00734190  CExoString::Find_2
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x007FEEC0  CSWCItem::non_virtual_thunk_to_dtor_CSWCItem
  0x0071EBD0  CTlkTable::ParseStr
  0x00416E30  CSWGuiTextParams::SetText
  0x007FC790  (unnamed)
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0041FC30  CSWGuiListBox::AddControls
  0x0041E870  CSWGuiListBox::SetSelectedControl
struct offsets touched (top 10):
  +0x4 [dec 4]  x5  via EAX,EDX
  +0x1A24 [dec 6692]  x3  via ECX,EDX
  +0x5C [dec 92]  x2  via EAX,EDX
  +0x1D0 [dec 464]  x2  via EAX,EDX
  +0x1D4 [dec 468]  x2  via EAX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EAX
  +0x2C [dec 44]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
  +0x40 [dec 64]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007fc880 FUN_007fc880 size=1005 ret=0 callers=3 callees=28

void __fastcall FUN_007fc880(int *param_1)

{
  undefined1 uVar1;
  int iVar2;
  float10 fVar3;
  code *local_90;
  int *local_8c;
  undefined1 local_64 [8];
  int *local_5c;
  int *local_58;
  undefined4 local_54;
  int *local_50;
  int local_4c;
  float local_48;
  int local_44;
  undefined1 local_40 [4];
  int local_3c;
  undefined4 local_34;
  undefined1 local_30 [8];
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0096c1a3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_24 = FUN_0073f070(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_44 = 0;
  FUN_00733540();
  local_8 = 0;
  local_34 = FUN_0073f350();
  FUN_0084a640(0);
  local_8 = CONCAT31(local_8._1_3_,1);
  local_4c = param_1[0x231 = byte +0x8c4];
  local_20 = *(undefined4 *)(local_4c + 4);
  local_1c = *(undefined4 *)(local_4c + 8);
  local_14 = *(undefined4 *)(local_4c + 0x10);
  local_18 = param_1[0x223 = byte +0x88c] + (uint)*(byte *)(param_1 + 0x22e) * -2;
  fVar3 = (float10)FUN_00479ab0();
  local_48 = (float)fVar3;
  local_14 = FUN_0091c860();
  local_28 = 0;
  do {
    local_44 = (**(code **)(*param_1 + 0x70))(local_28);
    if (local_44 == 0) {
      FUN_0041fc30(local_40,1,1,0);
      if (param_1[0x689 = byte +0x1a24] != 0) {
        (**(code **)(*(int *)param_1[0x689 = byte +0x1a24] + 0x40))(0);
      }
      if (local_3c < 1) {
        FUN_0041e870(0xffffffff,0);
        FUN_00733570(&DAT_00986925);
        local_8._0_1_ = 3;
        FUN_00416e30(local_64);
        local_8 = CONCAT31(local_8._1_3_,1);
        FUN_00733780();
        FUN_007fc390(0);
      }
      else {
        FUN_0041e870(0,0);
      }
      iVar2 = FUN_0073f750();
      if ((*(uint *)(iVar2 + 0xfc4) & 1) == 0) {
        local_90 = FUN_007fcc70;
      }
      else {
        local_90 = FUN_007fcc90;
      }
      (*local_90)();
      local_8 = local_8 & 0xffffff00;
      Free();
/* ... 51 more lines truncated by --max-lines */
```

### decoy: 0x007fc790 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007fc790: 16-byte aligned: YES
  007fc790  55             PUSH EBP
  007fc791  8B EC          MOV EBP,ESP
  007fc793  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007fc790
string literals (0):
direct callees (0):
struct offsets touched (top 4):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
  +0x1D4 [dec 468]  x4  via EAX,EDX
  +0x94 [dec 148]  x1  via ECX
```

### Static facts (script output, report-only)
- size 1005 B, RET 0
- callers 3: 0x007fbce0, 0x007fbd90, 0x007fc290
- callees: 0x0073f070 CClientExoApp::GetQuestJournal (CALL), 0x00733540 CExoString::DefaultConstructor (CALL), 0x0073f350 CClientExoApp::GetClientLanguage (CALL), 0x0084a640 (CALL), 0x00479ab0 Global::GetResolutionTextScalar (CALL), 0x0091c860 (CALL), ...
- indirect: 0x007fc985 slot 0x70, 0x007fcb87 slot 0x40
- globals: 0x00a1b4b4 READ
- twins: none

## Card B20-2-06

GOG function: CResRef::operator==  gog 0x0061a490  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710b70 (size 164, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.38 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710b40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a420 CResRef::operator!=_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009228ac unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 24): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x10*} | Steam {0x4*,0xc*,0x10*,0x18}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710b70: 16-byte aligned: YES
  00710b70  55             PUSH EBP
  00710b71  8B EC          MOV EBP,ESP
  00710b73  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710b70
string literals (0):
direct callees (3):
  0x00734010  CExoString::CStr
  0x00919F10  _strlen
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710b70 FUN_00710b70 size=164 ret=4 callers=1 callees=3

bool __thiscall FUN_00710b70(void *param_1,int *param_2)

{
  void *_Buf1 [GHIDRA-AUTO-LABEL: unreliable];
  int iVar1;
  size_t local_1c;
  size_t local_10;
  
  _Buf1 [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00734010();
  if (*param_2 == 0) {
    local_10 = 0;
  }
  else {
    local_10 = _strlen [GHIDRA-AUTO-LABEL: unreliable]((char *)*param_2);
  }
  if (local_10 + 1 < 0x11) {
    local_1c = local_10 + 1;
  }
  else {
    local_1c = 0x10;
  }
  iVar1 = _memcmp [GHIDRA-AUTO-LABEL: unreliable](_Buf1 [GHIDRA-AUTO-LABEL: unreliable],param_1,local_1c);
  return iVar1 == 0;
}
```

### decoy: 0x00710b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710b40: 16-byte aligned: YES
  00710b40  55             PUSH EBP
  00710b41  8B EC          MOV EBP,ESP
  00710b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710b40
string literals (0):
direct callees (1):
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 164 B, RET 4
- callers 1: 0x00710aa0
- callees: 0x00734010 (CALL), 0x00919f10 _strlen (CALL), 0x0091ac7c (CALL)
- twins: none

## Card B20-2-07

GOG function: CExoFile::Read  gog 0x0060ba20  callconv=(blank)  param_size_bytes=12  gog_size=35  gog_retn=12
Candidate Steam address: 0x00739990 (size 35, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 7 (callers: ReadPlayBackInputEvents 0x7b0290, ServiceFromDirectory 0x7133a0, ServiceFromDirectoryRaw 0x714630)
- best decoy (neighbour/other candidate) scores: 0x007397f0:STR0/CALLEE6/SIG0.167/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba00 CExoFile::GetSize  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060ba50 CExoFile::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610220 CExoFileInternal::Read_2 -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 22): the card's A-CALLEE credit may come from these, not from callees
  0x0045cea0 CClientExoAppInternal::ReadPlayBackInputEvents -> Steam 0x007b0290  in candidate callers: yes
  0x0061cd00 CExoResMan::ServiceFromDirectory -> Steam 0x007133a0  in candidate callers: yes
  0x0061df90 CExoResMan::ServiceFromDirectoryRaw -> Steam 0x00714630  in candidate callers: yes
  0x0061f0a0 CExoResMan::FileCopy -> Steam 0x00715740  in candidate callers: yes
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
prologue for 0x00739990: 16-byte aligned: YES
  00739990  55             PUSH EBP
  00739991  8B EC          MOV EBP,ESP
  00739993  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00739990
string literals (0):
direct callees (1):
  0x0073DD20  CExoFileInternal::Read_2
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00739990 FUN_00739990 size=35 ret=12 callers=22 callees=1
THUNK -> 0x0073DD20 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00739990(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_0073dd20(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x007397f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007397f0: 16-byte aligned: YES
  007397f0  55             PUSH EBP
  007397f1  8B EC          MOV EBP,ESP
  007397f3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007397f0
string literals (0):
direct callees (2):
  0x00919723  operator new
  0x0073DA40  Global::operator/
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 35 B, RET 12
- callers 22: 0x00632220, 0x00638bd0, 0x007133a0 CExoResMan::ServiceFromDirectory, 0x00714630 CExoResMan::ServiceFromDirectoryRaw, 0x00715740 CExoResMan::FileCopy, ...
- callees: 0x0073dd20 (CALL)
- twins: 0x007399e0 (distinguishing: 0x007399a8: CALL 0x0073dd20)

## Card B20-2-08

GOG function: GLRender::RenderDrawableSquares  gog 0x00827df0  callconv=(blank)  param_size_bytes=0  gog_size=54  gog_retn=0
Candidate Steam address: 0x00435e80 (size 113, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00435f00:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00827dd0 GLRender::RemoveSquareToRender  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00827e30 GLRender::FrameBufferModifications  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00924260 Global::memset -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc}
imm sets (* = in both, 0 common): GOG {0xc,0x18} | Steam {0x2,0x8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00435e80: 16-byte aligned: YES
  00435e80  55             PUSH EBP
  00435e81  8B EC          MOV EBP,ESP
  00435e83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00435e80
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via ECX,EDX
  +0x8 [dec 8]  x1  via EAX
  +0xC [dec 12]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00435e80 FUN_00435e80 size=113 ret=0 callers=1 callees=0

undefined4 FUN_00435e80(void)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  
  uVar1 = (**(code **)(*DAT_00a1dd7c + 4))(0,8,2);
  puVar2 = (undefined4 *)(**(code **)(*DAT_00a1dd7c + 8))(uVar1,0,0,1);
  *puVar2 = 0;
  puVar2[1 = byte +0x4] = 0;
  (**(code **)(*DAT_00a1dd7c + 0xc))(uVar1);
  return uVar1;
}
```

### decoy: 0x00435f00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00435f00: 16-byte aligned: YES
  00435f00  55             PUSH EBP
  00435f01  8B EC          MOV EBP,ESP
  00435f03  81 EC E0 00 00 00 SUB ESP,0xe0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00435f00
string literals (0):
direct callees (3):
  0x00518390  (unnamed)
  0x004360F0  (unnamed)
  0x0091E620  (unnamed)
struct offsets touched (top 4):
  +0x4 [dec 4]  x3  via EAX,ECX,EDX
  +0x8 [dec 8]  x3  via EAX,ECX,EDX
  +0x988550 [dec 9995600]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via ECX
```

### Static facts (script output, report-only)
- size 113 B, RET 0
- callers 1: 0x004cdf40
- indirect: 0x00435e9c slot 0x4, 0x00435ebc slot 0x8, 0x00435ee8 slot 0xc
- twins: none

## Card B20-2-09

GOG function: CSWGuiPowersLevelUp::Destructor  gog 0x005f6ed0  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00908040 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00908070:STR0/CALLEE0/SIG0.008/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005f6370 CSWGuiPowersLevelUp::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005f6f00 CSWGuiPowersLevelUp::Destructor_2 -> Steam ?  in candidate callees: n/a
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
prologue for 0x00908040: 16-byte aligned: YES
  00908040  55             PUSH EBP
  00908041  8B EC          MOV EBP,ESP
  00908043  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00908040
string literals (0):
direct callees (2):
  0x00908070  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00908040 FUN_00908040 size=44 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_00908040(undefined4 param_1,uint param_2)

{
  FUN_00908070();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x00908070 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00908070: 16-byte aligned: YES
  00908070  55             PUSH EBP
  00908071  8B EC          MOV EBP,ESP
  00908073  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00908070
string literals (0):
direct callees (11):
  0x0041D290  CSWGuiListBox::ClearItems
  0x0089A680  (unnamed)
  0x0064DDB0  Free(ptr*) delete-and-zero helper
  0x00733780  CExoString::~CExoString
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0041A3D0  CSWGuiButton::Destructor
  0x0041D190  (unnamed)
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callees: 0x00908070 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B20-2-10

GOG function: CExoStreamingSoundSourceInternal::Pause  gog 0x0063cd80  callconv=(blank)  param_size_bytes=0  gog_size=237  gog_retn=0
Candidate Steam address: 0x0070ded0 (size 198, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: rotation:0x0063ccb0:0.62>0.41
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.39 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070dde0:STR0/CALLEE2/SIG0.8/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063ccb0 CExoStreamingSoundSourceInternal::Stop  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063ce70 CExoStreamingSoundSourceInternal::Resume  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> Steam 0x00741070  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00607070 CExoString::CompareNoCase_2 -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x0063ca40 CExoStreamingSoundSourceInternal::Update3DStream -> Steam 0x0070daa0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '=evil_ending'
disp sets (* = in both, 2 common): GOG {0x4,0xc*,0x24*,0xc4} | Steam {0x8,0xc*,0x24*,0x144,0x14a}
imm sets (* = in both, 1 common): GOG {0xd,0x18*} | Steam {0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070ded0: 16-byte aligned: YES
  0070ded0  55             PUSH EBP
  0070ded1  8B EC          MOV EBP,ESP
  0070ded3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070ded0
string literals (0):
direct callees (2):
  0x0070DAA0  CExoStreamingSoundSourceInternal::Update3DStream
  0x0070DFE0  (unnamed)
struct offsets touched (top 5):
  +0x144 [dec 324]  x2  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x14A [dec 330]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0070ded0 FUN_0070ded0 size=198 ret=0 callers=4 callees=5

void __fastcall FUN_0070ded0(int *param_1)

{
  int iVar1;
  
  if (((*(int *)(param_1[9 = byte +0x24] + 0xc) != 0) && (*param_1 != 0)) && (iVar1 = FUN_0070daa0(), iVar1 == 0)
     ) {
    FUN_0070dfe0(*(undefined1 *)((int)param_1 + 0x14a),0,0x3f800000);
    _AIL_set_stream_loop_count_8 [GHIDRA-AUTO-LABEL: unreliable](*param_1,param_1[2 = byte +0x8] == 0);
    _AIL_set_stream_position_8 [GHIDRA-AUTO-LABEL: unreliable](*param_1,param_1[0x51 = byte +0x144]);
    _AIL_pause_stream_8 [GHIDRA-AUTO-LABEL: unreliable](*param_1,0);
    param_1[0x51 = byte +0x144] = 0;
  }
  return;
}
```

### decoy: 0x0070dde0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070dde0: 16-byte aligned: YES
  0070dde0  55             PUSH EBP
  0070dde1  8B EC          MOV EBP,ESP
  0070dde3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0070dde0
string literals (1):
  0x0099C35C  at 0070de26  'evil_ending'
direct callees (6):
  0x00733540  CExoString::Constructor
  0x00710930  (unnamed)
  0x00734F60  CExoString::CompareNoCase(char*)
  0x00741070  CClientExoApp::GetCreditSequenceInProgress
  0x00733780  CExoString::~CExoString
  0x0070DAA0  CExoStreamingSoundSourceInternal::Update3DStream
struct offsets touched (top 4):
  +0x4 [dec 4]  x1  via ECX
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x144 [dec 324]  x1  via ECX
```

### Static facts (script output, report-only)
- size 198 B, RET 0
- callers 4: 0x007051c0, 0x00709ee0, 0x0070a050, 0x0070b1d0
- callees: 0x0070daa0 CExoStreamingSoundSourceInternal::Update3DStream (CALL), 0x0070dfe0 (CALL), 0x00000081 (CALL), 0x0000007c (CALL), 0x0000007e (CALL)
- imports: _AIL_pause_stream@8 (MSS32.DLL), _AIL_set_stream_loop_count@8 (MSS32.DLL), _AIL_set_stream_position@8 (MSS32.DLL)
- twins: none

## Card B20-2-11

GOG function: CAuroraInterface::GetProcessedTextureInternal_thunk  gog 0x009080d0  callconv=(blank)  param_size_bytes=4  gog_size=17  gog_retn=0
Candidate Steam address: 0x0093da90 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093dab0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x009080b0 CAuroraInterface::ReleaseCompressedTextureInternal_thunk  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009080f0 CAuroraInterface::ReleaseProcessedTextureInternal_thunk  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00908ea0 CAuroraInterface::GetProcessedTextureInternal -> Steam 0x0093e880  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0093da90: 16-byte aligned: YES
  0093da90  55             PUSH EBP
  0093da91  8B EC          MOV EBP,ESP
  0093da93  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093da90
string literals (0):
direct callees (1):
  0x0093E7D0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093da90 FUN_0093da90 size=17 ret=0 callers=1 callees=1
THUNK -> 0x0093E7D0 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0093da90(undefined4 param_1)

{
  FUN_0093e7d0(param_1);
  return;
}
```

### decoy: 0x0093dab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093dab0: 16-byte aligned: YES
  0093dab0  55             PUSH EBP
  0093dab1  8B EC          MOV EBP,ESP
  0093dab3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093dab0
string literals (0):
direct callees (1):
  0x0093E880  CAuroraInterface::GetProcessedTextureInternal
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 1: 0x0093bad0
- callees: 0x0093e7d0 (CALL)
- twins: 0x0093d9b0, 0x0093d9d0, 0x0093d9f0, 0x0093da10, 0x0093da30, ... (distinguishing: 0x0093da97: CALL 0x0093e7d0)

## Card B20-2-12

GOG function: CExoIni::ReadIniEntry  gog 0x00608450  callconv=(blank)  param_size_bytes=16  gog_size=39  gog_retn=16
Candidate Steam address: 0x00736320 (size 39, retn 16)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 7 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007362c0:STR0/CALLEE7/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006083f0 CExoIni::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00608480 CExoIni::WriteIniEntry  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060e100 CExoIniInternal::ReadIniEntry -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (7 of 18): the card's A-CALLEE credit may come from these, not from callees
  0x00427150 CSWCCreature::LipSync -> Steam 0x0077b190  in candidate callers: yes
  0x00432410 CClientExoAppInternal::SetEventDescriptions -> Steam 0x00785df0  in candidate callers: yes
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: yes
  0x00436cf0 CClientExoAppInternal::ReadVideoModeSettings -> Steam 0x0078a7a0  in candidate callers: yes
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
prologue for 0x00736320: 16-byte aligned: YES
  00736320  55             PUSH EBP
  00736321  8B EC          MOV EBP,ESP
  00736323  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00736320
string literals (0):
direct callees (1):
  0x0073BA80  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00736320 FUN_00736320 size=39 ret=16 callers=18 callees=1
THUNK -> 0x0073BA80 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_00736320(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  FUN_0073ba80(param_1,param_2,param_3,param_4);
  return;
}
```

### decoy: 0x007362c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007362c0: 16-byte aligned: YES
  007362c0  55             PUSH EBP
  007362c1  8B EC          MOV EBP,ESP
  007362c3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007362c0
string literals (0):
direct callees (2):
  0x0073B8E0  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 39 B, RET 16
- callers 18: 0x00407920, 0x00409750, 0x0040a460, 0x00737470, 0x0077b190, ...
- callees: 0x0073ba80 (CALL)
- twins: 0x00736350 (distinguishing: 0x0073633c: CALL 0x0073ba80)

## Card B20-2-13

GOG function: CResRef::operator==  gog 0x0061a490  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710ad0 (size 110, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.33 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710b40:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a420 CResRef::operator!=_3  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x009228ac unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 24): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0xc,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710ad0: 16-byte aligned: YES
  00710ad0  55             PUSH EBP
  00710ad1  8B EC          MOV EBP,ESP
  00710ad3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710ad0
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710ad0 FUN_00710ad0 size=110 ret=4 callers=45 callees=1

undefined4 __thiscall FUN_00710ad0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint local_8;
  
  local_8 = 0;
  while( true ) {
    if (0xf < local_8) {
      return 0;
    }
    iVar1 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((int)*(char *)(param_2 + local_8));
    iVar2 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((uint)*(byte *)(param_1 + local_8));
    if (iVar1 != iVar2) break;
    if (iVar1 == 0) {
      return 0;
    }
    local_8 = local_8 + 1;
  }
  return 1;
}
```

### decoy: 0x00710b40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710b40: 16-byte aligned: YES
  00710b40  55             PUSH EBP
  00710b41  8B EC          MOV EBP,ESP
  00710b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710b40
string literals (0):
direct callees (1):
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 110 B, RET 4
- callers 45: 0x0052e590, 0x0052e640, 0x00561190, 0x00561240, 0x00631560, ...
- callees: 0x0091e01a (CALL)
- twins: none

## Card B20-2-14

GOG function: CSWGuiContainer::Destructor  gog 0x005a6190  callconv=(blank)  param_size_bytes=4  gog_size=46  gog_retn=4
Candidate Steam address: 0x008b25f0 (size 46, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 1 (callees: Destructor_2 0x8b2620)
- best decoy (neighbour/other candidate) scores: 0x008b2850:STR0/CALLEE0/SIG0.1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x005a5a40 CSWGuiContainer::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005a61c0 CSWGuiContainer::Destructor_2 -> 0x008b2620  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x005a61c0 CSWGuiContainer::Destructor_2 -> Steam 0x008b2620  in candidate callees: yes
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x1c*} | Steam {0x4*,0x1c*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x008b25f0: 16-byte aligned: YES
  008b25f0  55             PUSH EBP
  008b25f1  8B EC          MOV EBP,ESP
  008b25f3  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x008b25f0
string literals (0):
direct callees (2):
  0x008B2620  CSWGuiContainer::Destructor_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 008b25f0 FUN_008b25f0 size=46 ret=4 callers=0 callees=2

undefined4 __thiscall FUN_008b25f0(undefined4 param_1,uint param_2)

{
  FUN_008b2620();
  if ((param_2 & 1) != 0) {
    FUN_009196fd(param_1);
  }
  return param_1;
}
```

### decoy: 0x008b2850 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008b2850: 16-byte aligned: YES
  008b2850  55             PUSH EBP
  008b2851  8B EC          MOV EBP,ESP
  008b2853  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008b2850
string literals (0):
direct callees (8):
  0x004122A0  CSWGuiManager::PlayGuiSound
  0x0073F810  CClientExoApp::GetSWCMessage
  0x0087A550  (unnamed)
  0x0073F750  CClientExoApp::GetInGameGui
  0x007CAD30  CGuiInGame::HideContainerGui
  0x008B2F70  CSWGuiContainer::SetupGiveMode
  0x008B29E0  CSWGuiContainer::SetContainer
  0x0040EAA0  CSWGuiPanel::HandleInputEvent
struct offsets touched (top 5):
  +0x4 [dec 4]  x4  via ECX,EDX
  +0x1C [dec 28]  x4  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via EAX,EDX
  +0x70 [dec 112]  x3  via EAX,EDX
  +0x6C [dec 108]  x2  via ECX,EDX
```

### Static facts (script output, report-only)
- size 46 B, RET 4
- callees: 0x008b2620 CSWGuiContainer::Destructor_2 (CALL), 0x009196fd operator delete (CALL)
- twins: none

## Card B20-2-15

GOG function: CSWGuiObject::SetActiveControl  gog 0x005c0410  callconv=(blank)  param_size_bytes=0  gog_size=13  gog_retn=8
Candidate Steam address: 0x0042a470 (size 13, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 12 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 12 (callers: StartServices 0x788d90, StopServices 0x78ac40, ? 0x74d520)
- best decoy (neighbour/other candidate) scores: 0x0042a3d0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x005c0340 CSWUpgradeItemEntry::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005c0420 CSWUpgradeItemEntry::Destructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (12 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x004353b0 CClientExoAppInternal::StartServices -> Steam 0x00788d90  in candidate callers: yes
  0x00437190 CClientExoAppInternal::StopServices -> Steam 0x0078ac40  in candidate callers: yes
  0x0051eca0 CSWGuiMainInterface::PopupActionMenu -> Steam 0x0074d520  in candidate callers: yes
  0x00687b00 CSWSCreature::EventHandler -> Steam 0x0056efd0  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 8)
```

### prologue
```
prologue for 0x0042a470: 16-byte aligned: YES
  0042a470  55             PUSH EBP
  0042a471  8B EC          MOV EBP,ESP
  0042a473  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0042a470
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0042a470 FUN_0042a470 size=13 ret=8 callers=12 callees=0

void FUN_0042a470(void)

{
  return;
}
```

### decoy: 0x0042a3d0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0042a3d0: 16-byte aligned: YES
  0042a3d0  55             PUSH EBP
  0042a3d1  8B EC          MOV EBP,ESP
  0042a3d3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0042a3d0
string literals (0):
direct callees (2):
  0x004491C0  CExoPtrList::Add (doubling)
  0x0091C6AA  operator delete[]
struct offsets touched (top 4):
  +0x2C [dec 44]  x2  via EAX,ECX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0x10 [dec 16]  x1  via ECX
  +0x5C [dec 92]  x1  via EAX
```

### Static facts (script output, report-only)
- size 13 B, RET 8
- callers 12: 0x00550e30 CSWSArea::PlotGridPath, 0x005532f0 CSWSArea::PlotPathPointPath, 0x0056efd0 CSWSCreature::EventHandler, 0x005bfc00 CSWSCreature::AIActionMoveToPoint, 0x005c3f70 CSWSCreature::WalkUpdateLocation_QuickWalk, ...
- twins: none

## Card B20-2-16

GOG function: CAuroraInterface::GetProcessedTextureInternal_thunk  gog 0x009080d0  callconv=(blank)  param_size_bytes=4  gog_size=17  gog_retn=0
Candidate Steam address: 0x0093dad0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0093dab0:STR0/CALLEE1/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x009080b0 CAuroraInterface::ReleaseCompressedTextureInternal_thunk  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009080f0 CAuroraInterface::ReleaseProcessedTextureInternal_thunk  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00908ea0 CAuroraInterface::GetProcessedTextureInternal -> Steam 0x0093e880  in candidate callees: no
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x4*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x0093dad0: 16-byte aligned: YES
  0093dad0  55             PUSH EBP
  0093dad1  8B EC          MOV EBP,ESP
  0093dad3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093dad0
string literals (0):
direct callees (1):
  0x0093EA10  CAuroraInterface::ReleaseProcessedTextureInternal
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093dad0 FUN_0093dad0 size=17 ret=0 callers=2 callees=1
THUNK -> 0x0093EA10 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0093dad0(undefined4 param_1)

{
  FUN_0093ea10(param_1);
  return;
}
```

### decoy: 0x0093dab0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093dab0: 16-byte aligned: YES
  0093dab0  55             PUSH EBP
  0093dab1  8B EC          MOV EBP,ESP
  0093dab3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093dab0
string literals (0):
direct callees (1):
  0x0093E880  CAuroraInterface::GetProcessedTextureInternal
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 2: 0x0093bad0, 0x0093c130
- callees: 0x0093ea10 CAuroraInterface::ReleaseProcessedTextureInternal (CALL)
- twins: 0x0093d9b0, 0x0093d9d0, 0x0093d9f0, 0x0093da10, 0x0093da30, ... (distinguishing: 0x0093dad7: CALL 0x0093ea10)

## Card B20-2-17

GOG function: CExoStreamingSoundSourceInternal::Pause  gog 0x0063cd80  callconv=(blank)  param_size_bytes=0  gog_size=237  gog_retn=0
Candidate Steam address: 0x0070dc60 (size 71, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: fail:trusted_holder=CExoStreamingSoundSourceInternal::IsDone | G7 mac: n/a | G8 mac_rotation: rotation:0x0063ccb0:0.62>0.41
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0070dcb0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063ccb0 CExoStreamingSoundSourceInternal::Stop  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063ce70 CExoStreamingSoundSourceInternal::Resume  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> Steam 0x00741070  in candidate callees: no
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  0x00607070 CExoString::CompareNoCase_2 -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x0063ca40 CExoStreamingSoundSourceInternal::Update3DStream -> Steam 0x0070daa0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '=evil_ending'
disp sets (* = in both, 2 common): GOG {0x4,0xc*,0x24*,0xc4} | Steam {0xc*,0x24*}
imm sets (* = in both, 0 common): GOG {0xd,0x18} | Steam {0x2,0xc}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070dc60: 16-byte aligned: YES
  0070dc60  55             PUSH EBP
  0070dc61  8B EC          MOV EBP,ESP
  0070dc63  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070dc60
string literals (0):
direct callees (0):
struct offsets touched (top 2):
  +0xC [dec 12]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0070dc60 FUN_0070dc60 size=71 ret=0 callers=1 callees=1

bool __fastcall FUN_0070dc60(int *param_1)

{
  int iVar1;
  bool bVar2;
  
  if ((*(int *)(param_1[9 = byte +0x24] + 0xc) == 0) || (*param_1 == 0)) {
    bVar2 = false;
  }
  else {
    iVar1 = _AIL_stream_status_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
    bVar2 = iVar1 == 2;
  }
  return bVar2;
}
```

### decoy: 0x0070dcb0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070dcb0: 16-byte aligned: YES
  0070dcb0  55             PUSH EBP
  0070dcb1  8B EC          MOV EBP,ESP
  0070dcb3  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070dcb0
string literals (0):
direct callees (1):
  0x0070DAA0  CExoStreamingSoundSourceInternal::Update3DStream
struct offsets touched (top 1):
  +0x8 [dec 8]  x2  via EAX,EDX
```

### Static facts (script output, report-only)
- size 71 B, RET 0
- callers 1: 0x0070b110
- callees: 0x0000007f (CALL)
- imports: _AIL_stream_status@4 (MSS32.DLL)
- twins: none

## Card B20-2-18

GOG function: CExoFile::ReadAsyncComplete  gog 0x0060baa0  callconv=(blank)  param_size_bytes=0  gog_size=21  gog_retn=0
Candidate Steam address: 0x00739a10 (size 21, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: Demand 0x711c20, Update 0x714300)
- best decoy (neighbour/other candidate) scores: 0x007399e0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0060ba70 CExoFile::ReadAsync  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0060bac0 CExoFile::Write  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00610300 CExoFileInternal::ReadAsyncComplete -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x0061b580 CExoResMan::Demand -> Steam 0x00711c20  in candidate callers: yes
  0x0061dc60 CExoResMan::Update -> Steam 0x00714300  in candidate callers: yes
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
prologue for 0x00739a10: 16-byte aligned: YES
  00739a10  55             PUSH EBP
  00739a11  8B EC          MOV EBP,ESP
  00739a13  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00739a10
string literals (0):
direct callees (1):
  0x0073DE00  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00739a10 FUN_00739a10 size=21 ret=0 callers=2 callees=1
THUNK -> 0x0073DE00 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void __fastcall FUN_00739a10(undefined4 param_1)

{
  FUN_0073de00(param_1);
  return;
}
```

### decoy: 0x007399e0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007399e0: 16-byte aligned: YES
  007399e0  55             PUSH EBP
  007399e1  8B EC          MOV EBP,ESP
  007399e3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x007399e0
string literals (0):
direct callees (1):
  0x0073DDD0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 21 B, RET 0
- callers 2: 0x00711c20 CExoResMan::Demand, 0x00714300 CExoResMan::Update
- callees: 0x0073de00 (CALL)
- twins: 0x00739950, 0x00739970, 0x00739cf0, 0x00739d10 (distinguishing: 0x00739a1c: CALL 0x0073de00)
