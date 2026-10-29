# Review cards B20 chunk 3

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-3-01

GOG function: CSWGuiInGameMessages::ShowEffects  gog 0x0052bec0  callconv=(blank)  param_size_bytes=0  gog_size=1659  gog_retn=None
Candidate Steam address: 0x0075a630 (size 397, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.21) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 1 | A-SIG 0.43 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0075a7c0:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0052bd30 CSWGuiInGameMessages::ShowCombatMessages  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0052c53b Global::SWMSGSetupEffectRow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (24), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: yes
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: no
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  ... 16 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 4 common): GOG {0x4*,0x8*,0x14,0x20,0x40,0x48*,0xa0,0x2a4,0x2d0,0xc14,0xf04,0xfc8*} | Steam {0x4*,0x8*,0x48*,0xfc8*}
imm sets (* = in both, 13 common): GOG {0x2*,0x3,0x4,0x5,0xa,0x68*,0xcc,0xd8,0xf0,0x148,0x15c,0x1c0,0x358*,0x648*,...(+10)} | Steam {0x2*,0x28,0x68*,0x358*,0x648*,0x938*,0xc28*,0x106c*,0x11b4*,0x1df4*,0x1fc4*,0x2194*,0x2364*,0xfffffffd*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0075a630: 16-byte aligned: YES
  0075a630  55             PUSH EBP
  0075a631  8B EC          MOV EBP,ESP
  0075a633  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0075a630
string literals (0):
direct callees (2):
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x0073F750  CClientExoApp::GetInGameGui
struct offsets touched (top 4):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x8 [dec 8]  x1  via EDX
  +0xFC8 [dec 4040]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 0075a630 FUN_0075a630 size=397 ret=0 callers=3 callees=2

void __fastcall FUN_0075a630(int *param_1)

{
  int iVar1;
  
  param_1[0x2c = byte +0xb0] = param_1[0x2c = byte +0xb0] & 0xfffffffd;
  param_1[0xe8 = byte +0x3a0] = param_1[0xe8 = byte +0x3a0] & 0xfffffffd;
  param_1[0x260 = byte +0x980] = param_1[0x260 = byte +0x980] & 0xfffffffd;
  param_1[0x31c = byte +0xc70] = param_1[0x31c = byte +0xc70] & 0xfffffffd;
  param_1[0x1a4 = byte +0x690] = param_1[0x1a4 = byte +0x690] & 0xfffffffdU | 2;
  param_1[0x42d = byte +0x10b4] = param_1[0x42d = byte +0x10b4] & 0xfffffffd;
  param_1[0x47f = byte +0x11fc] = param_1[0x47f = byte +0x11fc] & 0xfffffffd;
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fe24);
  FUN_00417140(&DAT_00a7fdac);
  FUN_00417140(&DAT_00a7fe24);
  (**(code **)(*param_1 + 0x8 [dec 8] {int*: byte +0x20}))(param_1 + 0x192,0);
  iVar1 = FUN_0073f750();
  *(undefined1 *)(iVar1 + 0xfc8) = 2;
  return;
}
```

### decoy: 0x0075a7c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0075a7c0: 16-byte aligned: YES
  0075a7c0  55             PUSH EBP
  0075a7c1  8B EC          MOV EBP,ESP
  0075a7c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0075a7c0
string literals (0):
direct callees (21):
  0x00417140  CSWGuiTextParams::SetDefaultColor
  0x0041D290  CSWGuiListBox::ClearItems
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5DA0  Party member entry thiscall(party, int idx) RET 4 (0x7E5D80 = idx 0)
  0x0077D800  Party entry -> creature thiscall(entry); creature+0x1198 = stats
  0x0084A640  (unnamed)
  0x00733540  CExoString::Constructor
  0x0071DEE0  (unnamed)
  0x007337C0  CExoString::operator=
  0x00733780  CExoString::~CExoString
  0x00919723  operator new
  0x00419740  LabelCtor
  0x00414A50  CSWGuiBorder::Constructor
  0x00416E30  CSWGuiTextParams::SetText
  0x00417010  CSWGuiTextParams::SetColor
  0x00414980  CSWGuiBorder::GetDimension
  0x0083EA60  CExoArrayList<uint32>::Add
  0x0041FC30  CSWGuiListBox::AddControls
  0x0041E870  CSWGuiListBox::SetSelectedControl
  0x0073F750  CClientExoApp::GetInGameGui
struct offsets touched (top 10):
  +0x48 [dec 72]  x14  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x20 [dec 32]  x2  via EAX,EDX
  +0x40 [dec 64]  x2  via EDX
  +0xA0 [dec 160]  x2  via EDX
  +0x2A4 [dec 676]  x2  via EAX,ECX
  +0x2D0 [dec 720]  x2  via ECX,EDX
  +0x14 [dec 20]  x1  via EAX
  +0xC14 [dec 3092]  x1  via EDX
```

### Static facts (script output, report-only)
- size 397 B, RET 0
- callers 3: 0x00759d20, 0x0075a1f0, 0x0075a2b0
- callees: 0x00417140 CSWGuiTextParams::SetDefaultColor (CALL), 0x0073f750 CClientExoApp::GetInGameGui (CALL)
- indirect: 0x0075a796 slot 0x8
- twins: none

## Card B20-3-02

GOG function: GLU32_DLL::gluPerspective  gog 0x009433e6  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b430 (size 6, retn None)  THUNK -> 0x00000144 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000144 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR n/a | A-CALLEE n/a | A-SIG n/a | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00401000:STRn/a/CALLEEn/a/SIGn/a/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009433e0 GLU32_DLL::gluBuild2DMipmaps -> 0x0093b430  gate_class=REVIEW
- GOG-order neighbour after:  0x009433ec VERSION_DLL::VerQueryValueA  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
(no GOG fingerprint available for 0x009433e6)

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=n/a ; asm RET = none
Stack Slot Match: UNKNOWN (GOG 0 bytes -> x86 slots 0; ASM RET none)
```

### prologue
```
prologue: no asm for 0x00000144
```

### anchors
```
anchors: no asm for 0x00000144
```

### annotate (candidate, first 80 lines)
```
(no decompile for 0x00000144)
```

### decoy: 0x00401000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00401000: 16-byte aligned: YES
  00401000  55             PUSH EBP
  00401001  8B EC          MOV EBP,ESP
  00401003  C6 05 20 3C A1 00 00 MOV byte ptr [0x00a13c20],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00401000
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size unknown (not in steam_functions)
- asm: no decomp file in dump
- twins: none

## Card B20-3-03

GOG function: Global::FindGob  gog 0x0084b050  callconv=(blank)  param_size_bytes=4  gog_size=90  gog_retn=0
Candidate Steam address: 0x00458980 (size 253, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00458a80:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
disp sets (* = in both, 0 common): GOG {} | Steam {0x1c,0x20,0x1a4}
imm sets (* = in both, 1 common): GOG {0x4*,0x8,0x64} | Steam {0x4*,0x10}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00458980: 16-byte aligned: YES
  00458980  55             PUSH EBP
  00458981  8B EC          MOV EBP,ESP
  00458983  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00458980
string literals (0):
direct callees (5):
  0x00476620  (unnamed)
  0x004142E0  CExoArrayList::IndexOf
  0x0049B840  (unnamed)
  0x00463B50  (unnamed)
  0x00456EA0  (unnamed)
struct offsets touched (top 3):
  +0x1C [dec 28]  x2  via ECX,EDX
  +0x20 [dec 32]  x1  via EDX
  +0x1A4 [dec 420]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00458980 FUN_00458980 size=253 ret=0 callers=1 callees=5

void __fastcall FUN_00458980(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_00949d08;
  local_10 = ExceptionList;
  uVar2 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  *param_1 = PartLight::vftable;
  local_8 = 0;
  for (local_14 = 0; local_14 < DAT_00a1ffa0; local_14 = local_14 + 1) {
    iVar1 = *(int *)(*(int *)(DAT_00a1ff9c + local_14 * 4) + 0x1a4);
    if (iVar1 != 0) {
      iVar3 = FUN_00476620(param_1);
      if (iVar3 != 0) {
        iVar3 = FUN_004142e0(param_1);
        if (-1 < iVar3) {
          FUN_0049b840(iVar3);
          FUN_00463b50(iVar3);
        }
      }
    }
    if (*(undefined4 **)(iVar1 + 0x1c) == param_1) {
      *(undefined4 *)(iVar1 + 0x1c) = 0;
      *(undefined4 *)(iVar1 + 0x20) = 0;
    }
  }
  local_8 = 0xffffffff;
  FUN_00456ea0(uVar2);
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00458a80 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00458a80: 16-byte aligned: YES
  00458a80  55             PUSH EBP
  00458a81  8B EC          MOV EBP,ESP
  00458a83  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00458a80
string literals (0):
direct callees (2):
  0x00458AE0  (unnamed)
  0x009370C7  _stricmp
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 253 B, RET 0
- callers 1: 0x00458950
- callees: 0x00476620 (CALL), 0x004142e0 CExoArrayList::IndexOf (CALL), 0x0049b840 (CALL), 0x00463b50 (CALL), 0x00456ea0 (CALL)
- twins: none

## Card B20-3-04

GOG function: CSWGuiInGameJournal::PopulateItemListBox  gog 0x00591bd0  callconv=(blank)  param_size_bytes=0  gog_size=1005  gog_retn=0
Candidate Steam address: 0x007fcc70 (size 21, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.03 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007fc880:STR0/CALLEE3/SIG1/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00591ae0 CSWGuiJournalItemEntry::SetNew  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00591fc0 CSWCJournal::ClearDoneHasChanged  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (27), mapped to Steam through trusted rows only:
  0x0040c530 CClientExoApp::GetQuestJournal -> Steam 0x0073f070  in candidate callees: no
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam 0x0073f350  in candidate callees: no
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005115b0 CSWTlkTable::GetObjectGender_2 -> Steam ?  in candidate callees: n/a
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  ... 19 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x4,0x8,0xc,0x2c,0x3c*,0x40,0x58,0x5c,0x70,0x98,0x1d0,0x1d4,0x2a4,0x2d0,...(+3)} | Steam {0x3c*}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x3c,0x7b,0x80,0x178,0x1d8,0x590,0x5e8,0xfffffffe} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007fcc70: 16-byte aligned: YES
  007fcc70  55             PUSH EBP
  007fcc71  8B EC          MOV EBP,ESP
  007fcc73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007fcc70
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x3C [dec 60]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 007fcc70 FUN_007fcc70 size=21 ret=0 callers=1 callees=0

void __fastcall FUN_007fcc70(int param_1)

{
  *(undefined4 *)(param_1 + 0x3c) = 0;
  return;
}
```

### decoy: 0x007fc880 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007fc880: 16-byte aligned: YES
  007fc880  55             PUSH EBP
  007fc881  8B EC          MOV EBP,ESP
  007fc883  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
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

### Static facts (script output, report-only)
- size 21 B, RET 0
- callers 1: 0x007fc880
- twins: none

## Card B20-3-05

GOG function: Global::AuroraUseARBShaders  gog 0x0086ba30  callconv=(blank)  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00484cd0 (size 51, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484ca0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ba10 Global::AurCheckFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ba60 Global::AurClearActiveBins  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {} | Steam {0x9,0x20}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00484cd0: 16-byte aligned: YES
  00484cd0  55             PUSH EBP
  00484cd1  8B EC          MOV EBP,ESP
  00484cd3  83 7D 08 00    CMP dword ptr [EBP + 0x8],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484cd0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484cd0 FUN_00484cd0 size=51 ret=0 callers=13 callees=0

char * FUN_00484cd0(char *param_1)

{
  if (param_1 == (char *)0x0) {
    param_1 = (char *)0x0;
  }
  else {
    for (; (*param_1 == ' ' || (*param_1 == '\t')); param_1 = param_1 + 1) {
    }
  }
  return param_1;
}
```

### decoy: 0x00484ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484ca0: 16-byte aligned: YES
  00484ca0  55             PUSH EBP
  00484ca1  8B EC          MOV EBP,ESP
  00484ca3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484ca0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 51 B, RET 0
- callers 13: 0x00429010, 0x00463900, 0x00485470, 0x00485a40, 0x0048cd30, ...
- twins: none

## Card B20-3-06

GOG function: CResRef::operator==_3  gog 0x0061a570  callconv=(blank)  param_size_bytes=4  gog_size=113  gog_retn=4
Candidate Steam address: 0x00710c20 (size 113, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 10 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 10 (callers: ? 0x765940, GetEquipmentForcePowers_2 0x778ba0, SetEnvMap 0x7f03f0)
- best decoy (neighbour/other candidate) scores: 0x00710930:STR0/CALLEE5/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0061a4c0 CResRef::operator==_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0061a5f0 CResRef::operator=  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00928a62 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (10 of 40): the card's A-CALLEE credit may come from these, not from callees
  0x00411610 CSWCCreature::SetSoundSet -> Steam 0x00765940  in candidate callers: yes
  0x00424b60 CSWCCreature::GetEquipmentForcePowers_2 -> Steam 0x00778ba0  in candidate callers: yes
  0x00475550 CSWCObject::SetEnvMap -> Steam 0x007f03f0  in candidate callers: yes
  0x004756d0 CSWCObject::SetEnvMap_2 -> Steam 0x007f0570  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 2 common): GOG {0x4*,0x10*} | Steam {0x4*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710c20: 16-byte aligned: YES
  00710c20  55             PUSH EBP
  00710c21  8B EC          MOV EBP,ESP
  00710c23  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710c20
string literals (0):
direct callees (1):
  0x0091E01A  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710c20 FUN_00710c20 size=113 ret=4 callers=40 callees=1

undefined4 __thiscall FUN_00710c20(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  uint local_8;
  
  local_8 = 0;
  while( true ) {
    if (0xf < local_8) {
      return 1;
    }
    iVar1 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((int)*(char *)(param_2 + local_8));
    iVar2 = _tolower [GHIDRA-AUTO-LABEL: unreliable]((uint)*(byte *)(param_1 + local_8));
    if (iVar1 != iVar2) break;
    if (iVar1 == 0) {
      return 1;
    }
    local_8 = local_8 + 1;
  }
  return 0;
}
```

### decoy: 0x00710930 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710930: 16-byte aligned: YES
  00710930  55             PUSH EBP
  00710931  8B EC          MOV EBP,ESP
  00710933  83 EC 1C       SUB ESP,0x1c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710930
string literals (0):
direct callees (3):
  0x0091DBA0  _memcpy
  0x007338D0  CExoString::operator=_2
  0x009196EE  __security_check_cookie
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 113 B, RET 4
- callers 40: 0x0052e640, 0x00545760, 0x005460f0, 0x00561240, 0x005ffc00, ...
- callees: 0x0091e01a (CALL)
- twins: none

## Card B20-3-07

GOG function: CExoFileInternal::Read  gog 0x00610130  callconv=(blank)  param_size_bytes=8  gog_size=231  gog_retn=8
Candidate Steam address: 0x0073dc30 (size 231, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x0073dd20:STR0/CALLEE0/SIG0.2/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006100a0 CExoFileInternal::GetSize -> 0x0073dba0  trusted
- GOG-order neighbour after:  0x00610220 CExoFileInternal::Read_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (8), mapped to Steam through trusted rows only:
  0x006059e0 CExoString::operator=_2 -> Steam ?  in candidate callees: n/a
  0x00606120 CExoString::CStr -> Steam ?  in candidate callees: n/a
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
  0x00927e9f unnamed -> Steam ?  in candidate callees: n/a
  0x00927ed3 unnamed -> Steam ?  in candidate callees: n/a
  0x0092866d unnamed -> Steam ?  in candidate callees: n/a
  0x00928942 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 4 common): GOG {0x4*,0x8*,0x10*,0x18*} | Steam {0x4*,0x8*,0x10*,0x18*}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x0073dc30: 16-byte aligned: YES
  0073dc30  55             PUSH EBP
  0073dc31  8B EC          MOV EBP,ESP
  0073dc33  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0073dc30
string literals (0):
direct callees (8):
  0x00919723  operator new
  0x0091F3EF  (unnamed)
  0x00921563  (unnamed)
  0x00734010  CExoString::CStr
  0x0092178F  (unnamed)
  0x0092152F  (unnamed)
  0x007338D0  CExoString::operator=_2
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0073dc30 FUN_0073dc30 size=231 ret=8 callers=1 callees=8

size_t __thiscall FUN_0073dc30(int *param_1,undefined4 param_2,size_t param_3)

{
  void *_DstBuf [GHIDRA-AUTO-LABEL: unreliable];
  size_t sVar1;
  int iVar2;
  
  if (*param_1 == 0) {
    return 0;
  }
  _DstBuf [GHIDRA-AUTO-LABEL: unreliable] = (void *)FUN_00919723(param_3 + 1);
  sVar1 = _fread [GHIDRA-AUTO-LABEL: unreliable](_DstBuf [GHIDRA-AUTO-LABEL: unreliable],1,param_3,(FILE *)*param_1);
  if (sVar1 != param_3) {
    iVar2 = _ferror [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    if (iVar2 != 0) {
      FUN_00734010();
      FUN_0092178f(*param_1);
      return 0;
    }
    iVar2 = _feof [GHIDRA-AUTO-LABEL: unreliable]((FILE *)*param_1);
    if (iVar2 != 0) {
      FUN_00734010();
      FUN_0092178f(*param_1);
      return 0;
    }
  }
  *(undefined1 *)((int)_DstBuf [GHIDRA-AUTO-LABEL: unreliable] + sVar1) = 0;
  FUN_007338d0(_DstBuf [GHIDRA-AUTO-LABEL: unreliable]);
  FUN_009196fd(_DstBuf [GHIDRA-AUTO-LABEL: unreliable]);
  return sVar1;
}
```

### decoy: 0x0073dd20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0073dd20: 16-byte aligned: YES
  0073dd20  55             PUSH EBP
  0073dd21  8B EC          MOV EBP,ESP
  0073dd23  83 EC 0C       SUB ESP,0xc
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0073dd20
string literals (0):
direct callees (5):
  0x0091F3EF  (unnamed)
  0x00921563  (unnamed)
  0x00734010  CExoString::CStr
  0x0092178F  (unnamed)
  0x0092152F  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 231 B, RET 8
- callers 1: 0x007399c0
- callees: 0x00919723 operator new (CALL), 0x0091f3ef (CALL), 0x00921563 (CALL), 0x00734010 (CALL), 0x0092178f (CALL), 0x0092152f (CALL), ...
- twins: none

## Card B20-3-08

GOG function: CExoLinkedListInternal::Remove_2  gog 0x006207c0  callconv=(blank)  param_size_bytes=4  gog_size=27  gog_retn=4
Candidate Steam address: 0x0064dde0 (size 27, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 17 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 17 (callers: RemoveObjectFromHitcheckIgnoreList 0x78d190, UpdateVisualEffectsList 0x7f0b50, SetLockObjectOrientationInDialog 0x7cf5d0)
- best decoy (neighbour/other candidate) scores: 0x0064de00:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00620740 CExoResMan::CacheResource_2 -> 0x00716de0  trusted
- GOG-order neighbour after:  0x006207e0 C2DA::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0060c0a0 CExoLinkedListInternal::Remove -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (18 of 37): the card's A-CALLEE credit may come from these, not from callees
  0x00439760 CClientExoAppInternal::RemoveObjectFromHitcheckIgnoreList -> Steam 0x0078d190  in candidate callers: yes
  0x00475cb0 CSWCObject::UpdateVisualEffectsList -> Steam 0x007f0b50  in candidate callers: yes
  0x004dfcd0 CGuiInGame::SetLockObjectOrientationInDialog -> Steam 0x007cf5d0  in candidate callers: yes
  0x004dfe80 CGuiInGame::SetLockHeadFollowInDialog -> Steam 0x007cf780  in candidate callers: yes
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
prologue for 0x0064dde0: 16-byte aligned: YES
  0064dde0  55             PUSH EBP
  0064dde1  8B EC          MOV EBP,ESP
  0064dde3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0064dde0
string literals (0):
direct callees (1):
  0x0073A130  FnListRemove
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0064dde0 FUN_0064dde0 size=27 ret=4 callers=37 callees=1
THUNK -> 0x0073A130 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0064dde0(undefined4 param_1)

{
  FUN_0073a130(param_1);
  return;
}
```

### decoy: 0x0064de00 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0064de00: 16-byte aligned: YES
  0064de00  55             PUSH EBP
  0064de01  8B EC          MOV EBP,ESP
  0064de03  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0064de00
string literals (0):
direct callees (1):
  0x0064DFF0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 27 B, RET 4
- callers 37: 0x00535c30 CServerExoAppInternal::PlayerListChange, 0x005394c0 CServerExoAppInternal::RemoveFromExclusionList, 0x0053f6a0 CSWSObject::RemoveGroup, 0x00541080 CSWSObject::ClearAllActions, 0x00576ca0, ...
- callees: 0x0073a130 (CALL)
- twins: none

## Card B20-3-09

GOG function: CExoInputInternal::GetEvents  gog 0x006134e0  callconv=(blank)  param_size_bytes=12  gog_size=2768  gog_retn=None
Candidate Steam address: 0x0072c7e0 (size 5830, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.79) | G8 mac_rotation: pass
- anchors: A-STR 2 | A-CALLEE 0 | A-SIG 0.44 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: STR  (content level C1)
- evidence summary: STR '.\\exoinputinternal.cpp'@0099c810; 'CExoInputInternal::GetEvents() Invalid I'@0099c828
- best decoy (neighbour/other candidate) scores: 0x0072c6c0:STR0/CALLEE0/SIG0.098/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006134a0 CExoInputInternal::GetActive  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00613fb2 CExoInputInternal::GetEvents_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam 0x00521060  in candidate callees: yes
  0x00462840 CExoArrayList_CExoInputEvent_::Add -> Steam ?  in candidate callees: n/a
  0x0060c4d0 CExoDebug::Warning -> Steam ?  in candidate callees: n/a
  0x0060f6e0 unnamed -> Steam ?  in candidate callees: n/a
  0x00613fb2 CExoInputInternal::GetEvents_2 -> Steam ?  in candidate callees: n/a
  0x006162b0 unnamed -> Steam ?  in candidate callees: n/a
  0x006162f0 unnamed -> Steam ?  in candidate callees: n/a
  0x00617f90 CExoRawInputInternal::GetJoystickBuffer -> Steam ?  in candidate callees: n/a
  ... 8 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): '.\\\\exoinputinternal.cpp'; 'CExoInputInternal::GetEvents() Invalid I'
disp sets (* = in both, 14 common): GOG {0x4*,0x8*,0xc*,0x14*,0x1c*,0x24*,0x30*,0x34*,0x188*,0x1a0*,0x1b0*,0x1b8*,0x1c4*,0x410*} | Steam {0x4*,0x8*,0xc*,0x10,0x14*,0x18,0x1c*,0x24*,0x30*,0x34*,0x188*,0x19c,0x1a0*,0x1b0*,...(+5)}
imm sets (* = in both, 8 common): GOG {0x2*,0x5*,0x6*,0xa*,0xc*,0x10*,0x14*,0x1a4*,0x254,0x601} | Steam {0x2*,0x4,0x5*,0x6*,0x8,0xa*,0xb,0xc*,0x10*,0x11,0x14*,0x16,0x20,0x27,...(+39)}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0072c7e0: 16-byte aligned: YES
  0072c7e0  55             PUSH EBP
  0072c7e1  8B EC          MOV EBP,ESP
  0072c7e3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0072c7e0
string literals (2):
  0x0099C828  at 0072c8bc  'CExoInputInternal::GetEvents() Invalid InputClass parameter'
  0x0099C810  at 0072c8c1  '.\\\\exoinputinternal.cpp'
direct callees (24):
  0x0091D67E  (unnamed)
  0x0072F5C0  (unnamed)
  0x0072F500  (unnamed)
  0x00919BCC  `eh vector destructor iterator` (CRT)
  0x0073A560  (unnamed)
  0x007A1720  (unnamed)
  0x00732C40  (unnamed)
  0x00731390  (unnamed)
  0x00521060  CSWPlayerList::GetFirstPlayer
  0x0058C370  (unnamed)
  0x00561430  (unnamed)
  0x00732AA0  (unnamed)
  0x0072F540  (unnamed)
  0x0058A1B0  (unnamed)
  0x00732960  (unnamed)
  0x0072E730  (unnamed)
  0x007A1780  (unnamed)
  0x005936D0  (unnamed)
  0x009196FD  operator delete
  0x00736220  CExoTimers::GetHighResolutionTimer
struct offsets touched (top 10):
  +0x4 [dec 4]  x28  via EAX,ECX,EDX
  +0xC [dec 12]  x28  via EAX,ECX,EDX
  +0x1B0 [dec 432]  x14  via EAX,ECX,EDX
  +0x8 [dec 8]  x13  via EAX,ECX,EDX
  +0x10 [dec 16]  x10  via EAX,ECX,EDX
  +0x1A0 [dec 416]  x9  via EAX,EDX
  +0x14 [dec 20]  x8  via EAX,ECX,EDX
  +0x1B8 [dec 440]  x7  via EAX,ECX,EDX
  +0x18 [dec 24]  x6  via ECX,EDX
  +0x1C [dec 28]  x5  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0072c7e0 FUN_0072c7e0 size=5830 ret=12 callers=1 callees=24

undefined4 __thiscall FUN_0072c7e0(int param_1,undefined4 param_2,undefined4 *param_3,int param_4)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  undefined4 *puVar8;
  uint *puVar9;
  undefined8 uVar10;
  int local_24c;
  int local_244;
  undefined4 *local_240;
  int local_1f8;
  uint local_1f4;
  uint local_1dc;
  int local_1d8;
  uint local_1d4;
  int local_1c0;
  int local_190;
  int local_18c;
  int local_188;
  int local_184;
  int local_180;
  int local_17c;
  int local_178;
  int local_174;
  int local_170;
  int local_16c;
  int local_168;
  int local_164;
  int local_160;
  int local_15c;
  int *local_158;
  int local_154;
  int local_150;
  int *local_14c;
  int *local_148;
  int *local_144;
  int local_140;
  int local_13c;
  uint auStack_138[10 = byte +0x28];
  undefined4 local_110;
  int local_10c;
  int *local_108;
  int local_104;
  uint local_100[6 = byte +0x18];
  uint local_e8[25 = byte +0x64];
  int local_84;
  int local_80;
  uint local_7c;
  int local_78[10 = byte +0x28];
  int local_50;
  int *local_4c;
  int local_48;
  int aiStack_44[10 = byte +0x28];
  undefined4 local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00961015;
  local_10 = ExceptionList;
  uVar5 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  local_50 = -1;
  _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable](local_100,0xc,10,FUN_007306e0,COleDispParams::~COleDispParams);
  local_8 = 1;
  local_110 = 0;
  local_1c = 0;
  *param_3 = 0;
  FUN_0072f5c0(0);
  if (*(int *)(param_1 + 0x1a0) == 0) {
/* ... 445 more lines truncated by --max-lines */
```

### decoy: 0x0072c6c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072c6c0: 16-byte aligned: YES
  0072c6c0  55             PUSH EBP
  0072c6c1  8B EC          MOV EBP,ESP
  0072c6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0072c6c0
string literals (1):
  0x0099C800  at 0072c75c  '%d %c %d %d\\n'
direct callees (4):
  0x00733540  CExoString::Constructor
  0x00734270  CExoString::Format
  0x00739AA0  (unnamed)
  0x00733780  CExoString::~CExoString
struct offsets touched (top 4):
  +0x1A4 [dec 420]  x2  via EAX,ECX
  +0xC [dec 12]  x1  via ECX
  +0x10 [dec 16]  x1  via EDX
  +0x1B0 [dec 432]  x1  via EAX
```

### Static facts (script output, report-only)
- size 5830 B, RET 12
- callers 1: 0x0072fa80
- callees: 0x0091d67e (CALL), 0x0072f5c0 (CALL), 0x0072f500 (CALL), 0x00919bcc (CALL), 0x0073a560 (CALL), 0x007a1720 (CALL), ...
- indirect: 0x0072cc4f slot 0x4, 0x0072d3a0 slot 0x0
- strings: 0x0099c810 ".\\exoinputinternal.cpp", 0x0099c828 "CExoInputInternal::GetEvents() Invalid InputClass parameter"
- globals: 0x00a1b488 READ, 0x00a1b488 READ
- twins: none

## Card B20-3-10

GOG function: CExoStreamingSoundSourceInternal::SetVolume  gog 0x0063cf80  callconv=(blank)  param_size_bytes=12  gog_size=412  gog_retn=12
Candidate Steam address: 0x0070dfe0 (size 412, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x0063cd80:0.51>0.35
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.62 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: SIG  (content level C1)
- evidence summary: SIG 0.62
- best decoy (neighbour/other candidate) scores: 0x0070e180:STR0/CALLEE0/SIG0.163/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063cf40 CExoStreamingSoundSourceInternal::FadeAndStop  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063d120 Global::CExoStreamingSoundSourceInternal_volume_scale_eligibility_helper  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x004c72d0 CSWCArea::GetObjectsAtPositionVisible -> Steam ?  in candidate callees: n/a
  0x006392a0 CExoSoundInternal::GetSliderType_2 -> Steam ?  in candidate callees: n/a
  0x00639340 CExoSoundInternal::GetSliderVolume -> Steam ?  in candidate callees: n/a
  0x0063d120 Global::CExoStreamingSoundSourceInternal_volume_scale_eligibility_helper -> Steam ?  in candidate callees: n/a
  0x0063d280 CExoStreamingSoundSourceInternal::DoObstructionWork -> Steam ?  in candidate callees: n/a
  0x00924490 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 6): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 2 common): GOG {0xc*,0x24*,0xca,0xd4,0xd8} | Steam {0xc*,0x24*,0x14a,0x154,0x158}
imm sets (* = in both, 6 common): GOG {0x3*,0x4*,0x5*,0xc*,0x3c*,0x44*} | Steam {0x3*,0x4*,0x5*,0xc*,0x3c*,0x44*}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0070dfe0: 16-byte aligned: YES
  0070dfe0  55             PUSH EBP
  0070dfe1  8B EC          MOV EBP,ESP
  0070dfe3  83 EC 3C       SUB ESP,0x3c
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0070dfe0
string literals (0):
direct callees (6):
  0x0070A320  (unnamed)
  0x0070E180  Global::CExoStreamingSoundSourceInternal_volume_scale_eligibility_helper
  0x0091C860  _ftol2_sse
  0x0078EB80  (unnamed)
  0x0070A3C0  (unnamed)
  0x0070E2E0  StreamSource::Update3D (vol/pan/reverb)
struct offsets touched (top 5):
  +0x24 [dec 36]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x4  via ECX,EDX
  +0x14A [dec 330]  x1  via ECX
  +0x154 [dec 340]  x1  via EDX
  +0x158 [dec 344]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0070dfe0 FUN_0070dfe0 size=412 ret=12 callers=6 callees=7

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0070dfe0(int *param_1,undefined1 param_2,int param_3)

{
  float fVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  float10 fVar5;
  float10 fVar6;
  float local_14;
  int local_c;
  
  local_c = param_3;
  iVar2 = FUN_0070a320(param_1);
  FUN_0070e180();
  iVar3 = FUN_0091c860();
  if (param_3 == 0) {
    *(undefined1 *)((int)param_1 + 0x14a) = param_2;
  }
  if ((*(int *)(param_1[9 = byte +0x24] + 0xc) != 0) && (*param_1 != 0)) {
    iVar4 = FUN_0078eb80(param_1);
    if ((iVar4 == 0) && (param_3 == 0)) {
      fVar5 = (float10)FUN_0070a3c0(iVar2,1);
      fVar6 = (float10)FUN_0070a3c0(iVar2,0);
      if ((float10)(double)fVar5 == fVar6) {
        local_c = 0;
      }
      else {
        local_c = 1;
      }
    }
    fVar5 = (float10)FUN_0070a3c0(iVar2,local_c);
    local_14 = ((float)*(byte *)(param_1 + 0x55) / (float)_DAT_00988490) * (float)fVar5;
    if (((iVar2 == 3) || ((char)param_1[3 = byte +0xc] == '\x04')) || ((char)param_1[3 = byte +0xc] == '\x05')) {
      local_14 = local_14 * *(float *)param_1[9 = byte +0x24];
    }
    fVar1 = ((float)iVar3 * local_14) / (float)_DAT_00988490;
    _AIL_set_sample_volume_levels_12 [GHIDRA-AUTO-LABEL: unreliable](*(undefined4 *)(*param_1 + 0xc),fVar1,fVar1);
    param_1[0x56 = byte +0x158] = (int)fVar1;
    FUN_0070e2e0();
  }
  return;
}
```

### decoy: 0x0070e180 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070e180: 16-byte aligned: YES
  0070e180  55             PUSH EBP
  0070e181  8B EC          MOV EBP,ESP
  0070e183  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0070e180
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0xC [dec 12]  x2  via EAX
```

### Static facts (script output, report-only)
- size 412 B, RET 12
- callers 6: 0x00705d70, 0x00709be0, 0x0070b2a0, 0x0070c0c0, 0x0070d9d0, ...
- callees: 0x0070a320 (CALL), 0x0070e180 (CALL), 0x0091c860 (CALL), 0x0078eb80 (CALL), 0x0070a3c0 (CALL), 0x0000007b (CALL), ...
- imports: _AIL_set_sample_volume_levels@12 (MSS32.DLL)
- twins: none

## Card B20-3-11

GOG function: CSWGuiInGameJournal::PopulateItemListBox  gog 0x00591bd0  callconv=(blank)  param_size_bytes=0  gog_size=1005  gog_retn=0
Candidate Steam address: 0x007fc350 (size 60, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: mac_contra(0.00) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007fc390:STR0/CALLEE1/SIG0.375/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00591ae0 CSWGuiJournalItemEntry::SetNew  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00591fc0 CSWCJournal::ClearDoneHasChanged  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (27), mapped to Steam through trusted rows only:
  0x0040c530 CClientExoApp::GetQuestJournal -> Steam 0x0073f070  in candidate callees: no
  0x0040c810 CClientExoApp::GetClientLanguage -> Steam 0x0073f350  in candidate callees: no
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x004789b0 CSWParty::GetPlayerCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005115b0 CSWTlkTable::GetObjectGender_2 -> Steam ?  in candidate callees: n/a
  0x00512a40 CSWGuiButton::Constructor -> Steam ?  in candidate callees: n/a
  ... 19 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0xc,0x2c,0x3c,0x40,0x58,0x5c,0x70,0x98,0x1d0,0x1d4,0x2a4,0x2d0,...(+3)} | Steam {0x1c,0x1a20}
imm sets (* = in both, 0 common): GOG {0x2,0x3,0x4,0x3c,0x7b,0x80,0x178,0x1d8,0x590,0x5e8,0xfffffffe} | Steam {}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x007fc350: 16-byte aligned: YES
  007fc350  55             PUSH EBP
  007fc351  8B EC          MOV EBP,ESP
  007fc353  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007fc350
string literals (0):
direct callees (3):
  0x00410800  GuiManager::Contains
  0x0075BAA0  (unnamed)
  0x007FCD40  (unnamed)
struct offsets touched (top 2):
  +0x1A20 [dec 6688]  x2  via EAX
  +0x1C [dec 28]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007fc350 FUN_007fc350 size=60 ret=0 callers=0 callees=3

void __fastcall FUN_007fc350(int param_1)

{
  int iVar1;
  
  iVar1 = CMFCVisualManagerBitmapCache::IsCached
                    (*(CMFCVisualManagerBitmapCache **)(param_1 + 0x1c),
                     *(CSize **)(param_1 + 0x1a20));
  if (iVar1 != 0) {
    FUN_0075baa0(0);
  }
  FUN_007fcd40();
  return;
}
```

### decoy: 0x007fc390 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007fc390: 16-byte aligned: YES
  007fc390  55             PUSH EBP
  007fc391  8B EC          MOV EBP,ESP
  007fc393  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007fc390
string literals (0):
direct callees (23):
  0x0041D290  CSWGuiListBox::ClearItems
  0x005210A0  (unnamed)
  0x00733540  CExoString::Constructor
  0x0073F350  CClientExoApp::GetClientLanguage
  0x007356B0  CExoLocString::GetString_3
  0x00733570  CExoString::CExoString_char
  0x0071BCF0  C2DA::GetINTEntry_2
  0x00733780  CExoString::~CExoString
  0x0071DEE0  (unnamed)
  0x00733DF0  CExoString::operator+
  0x007337C0  CExoString::operator=
  0x00734190  CExoString::Find_2
  0x0073FB90  ClientApp -> party (*(*(clientApp+4)+0x270)) thiscall(ecx)
  0x007E5D80  GetControlledCreature
  0x007FEEC0  CSWCItem::non_virtual_thunk_to_dtor_CSWCItem
  0x0071EBD0  CTlkTable::ParseStr
  0x00416E30  CSWGuiTextParams::SetText
  0x00919F10  _strlen
  0x004178B0  CSWGuiText::GetFontHeight
  0x00479AB0  Global::GetResolutionTextScalar
struct offsets touched (top 10):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x2  via EDX
  +0x34 [dec 52]  x2  via EAX,ECX
  +0x5C [dec 92]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x3C [dec 60]  x1  via ECX
  +0x58 [dec 88]  x1  via ECX
  +0x70 [dec 112]  x1  via EDX
  +0x108 [dec 264]  x1  via EAX
  +0x130 [dec 304]  x1  via ECX
```

### Static facts (script output, report-only)
- size 60 B, RET 0
- callees: 0x00410800 GuiManager::Contains (CALL), 0x0075baa0 (CALL), 0x007fcd40 (CALL)
- twins: none

## Card B20-3-12

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

## Card B20-3-13

GOG function: CSWGuiOptionsMouse::HandleInputEvent  gog 0x005725e0  callconv=(blank)  param_size_bytes=8  gog_size=413  gog_retn=8
Candidate Steam address: 0x007dc360 (size 163, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.25) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.06 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007dc410:STR0/CALLEE0/SIG0.102/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00572410 CSWGuiOptionsMouse::Destructor_2  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x005727b0 CSWGuiOptionsMouse::OnRevMouseButtons  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (4), mapped to Steam through trusted rows only:
  0x00572940 CSWGuiOptionsMouse::OnDefault -> Steam ?  in candidate callees: n/a
  0x0090cf20 CSWGuiPanel::HandleInputEvent -> Steam 0x0040eaa0  in candidate callees: no
  0x0090ee40 CSWGuiManager::PopModalPanel -> Steam 0x004109c0  in candidate callees: no
  0x00910720 CSWGuiManager::PlayGuiSound -> Steam 0x004122a0  in candidate callees: no
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x1c,0x20,0x3c,0x48,0x5c,0x1d4,0x2f8,0x808} | Steam {0x4,0x8}
imm sets (* = in both, 2 common): GOG {0x3,0x7,0x8*,0x12,0x14,0x28,0x31,0x32,0x200*,0x2f8,0x7b8,0xfffffcff,0xffffff7f,0xfffffffe} | Steam {0x8*,0x98,0x110,0x200*,0x278}

### abi
```
GOG param bytes: 8; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(8/4)*4 = 8 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=8 ; asm RET = 8
Stack Slot Match: TRUE (GOG 8 bytes -> x86 slots 8; ASM RET 8)
```

### prologue
```
prologue for 0x007dc360: 16-byte aligned: YES
  007dc360  55             PUSH EBP
  007dc361  8B EC          MOV EBP,ESP
  007dc363  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x007dc360
string literals (0):
direct callees (1):
  0x0041A7A0  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x4  via EAX,ECX,EDX
  +0x8 [dec 8]  x4  via EAX,ECX,EDX
```

### annotate (candidate, first 80 lines)
```
// FN 007dc360 FUN_007dc360 size=163 ret=8 callers=0 callees=1

void __thiscall FUN_007dc360(int param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_0041a7a0(param_2,param_3);
  *(undefined4 *)(param_1 + 0x98) = DAT_00a7fe24;
  *(undefined4 *)(param_1 + 0x9c) = DAT_00a7fe28;
  *(undefined4 *)(param_1 + 0xa0) = DAT_00a7fe2c;
  *(undefined4 *)(param_1 + 0x200) = DAT_00a7fe24;
  *(undefined4 *)(param_1 + 0x204) = DAT_00a7fe28;
  *(undefined4 *)(param_1 + 0x208) = DAT_00a7fe2c;
  *(undefined4 *)(param_1 + 0x110) = DAT_00a7fdac;
  *(undefined4 *)(param_1 + 0x114) = DAT_00a7fdb0;
  *(undefined4 *)(param_1 + 0x118) = DAT_00a7fdb4;
  *(undefined4 *)(param_1 + 0x278) = DAT_00a7fdac;
  *(undefined4 *)(param_1 + 0x27c) = DAT_00a7fdb0;
  *(undefined4 *)(param_1 + 0x280) = DAT_00a7fdb4;
  return;
}
```

### decoy: 0x007dc410 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x007dc410: 16-byte aligned: YES
  007dc410  55             PUSH EBP
  007dc411  8B EC          MOV EBP,ESP
  007dc413  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x007dc410
string literals (11):
  0x009A1EEC  at 007dc538  'optmouse_p'
  0x009A1EDC  at 007dc558  'SLI_MOUSESEN'
  0x009A1ECC  at 007dc595  'LBL_MOUSESEN'
  0x009A1EBC  at 007dc5d1  'CB_REVBUTTONS'
  0x009A1818  at 007dc60e  'LBL_TITLE'
  0x009A1810  at 007dc64b  'LB_DESC'
  0x009A18AC  at 007dc6c4  'BTN_DEFAULT'
  0x0099E548  at 007dc701  'LBL_BAR1'
  0x0099E53C  at 007dc73d  'LBL_BAR2'
  0x0099E530  at 007dc77a  'LBL_BAR3'
  0x0099E524  at 007dc7b7  'LBL_BAR4'
direct callees (18):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00419740  LabelCtor
  0x0041CE90  (unnamed)
  0x0041A510  ButtonCtor
  0x0041AB60  (unnamed)
  0x0041A330  CSWGuiButtonToggle::Constructor
  0x0091D67E  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x007D2360  (unnamed)
  0x00418AF0  CSWGuiControl::AddEvent
  0x00760EA0  (unnamed)
  0x0040EC00  CSWGuiPanel::SetActiveControl
  0x007DD1F0  (unnamed)
struct offsets touched (top 8):
  +0x48 [dec 72]  x4  via EAX,EDX
  +0x68 [dec 104]  x1  via EDX
  +0x98 [dec 152]  x1  via EAX
  +0x5C9 [dec 1481]  x1  via ECX
  +0x5D4 [dec 1492]  x1  via EAX
  +0x9E4 [dec 2532]  x1  via ECX
  +0xBA8 [dec 2984]  x1  via EDX
  +0xCF0 [dec 3312]  x1  via EAX
```

### Static facts (script output, report-only)
- size 163 B, RET 8
- callees: 0x0041a7a0 (CALL)
- twins: none

## Card B20-3-14

GOG function: CExoStreamingSoundSourceInternal::Pause  gog 0x0063cd80  callconv=(blank)  param_size_bytes=0  gog_size=237  gog_retn=0
Candidate Steam address: 0x0070dde0 (size 237, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: rotation:0x0063ccb0:0.62>0.41
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.80 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 2 (callees: GetCreditSequenceInProgress 0x741070, Update3DStream 0x70daa0); SIG 0.80
- best decoy (neighbour/other candidate) scores: 0x0070ded0:STR0/CALLEE1/SIG0.393/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0063ccb0 CExoStreamingSoundSourceInternal::Stop  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0063ce70 CExoStreamingSoundSourceInternal::Resume  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (6), mapped to Steam through trusted rows only:
  0x0040e530 CClientExoApp::GetCreditSequenceInProgress -> Steam 0x00741070  in candidate callees: yes
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: yes
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: yes
  0x00607070 CExoString::CompareNoCase_2 -> Steam ?  in candidate callees: n/a
  0x0061a280 CResRef::CopyToString -> Steam ?  in candidate callees: n/a
  0x0063ca40 CExoStreamingSoundSourceInternal::Update3DStream -> Steam 0x0070daa0  in candidate callees: yes
GOG callers mapped through trusted rows (0 of 5): the card's A-CALLEE credit may come from these, not from callees
GOG strings (1): '=evil_ending'
disp sets (* = in both, 3 common): GOG {0x4*,0xc*,0x24*,0xc4} | Steam {0x4*,0xc*,0x24*,0x144}
imm sets (* = in both, 2 common): GOG {0xd*,0x18*} | Steam {0xd*,0x18*}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0070dde0: 16-byte aligned: YES
  0070dde0  55             PUSH EBP
  0070dde1  8B EC          MOV EBP,ESP
  0070dde3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0070dde0 FUN_0070dde0 size=237 ret=0 callers=5 callees=8

void __fastcall FUN_0070dde0(int *param_1)

{
  int iVar1;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00974098;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8 = 0;
  FUN_00710930(local_18);
  iVar1 = FUN_00734f60("evil_ending");
  if ((iVar1 != 0) && (iVar1 = FUN_00741070(), iVar1 == 0)) {
    local_8 = 0xffffffff;
    FUN_00733780();
    ExceptionList = local_10;
    return;
  }
  if (((*(int *)(param_1[9 = byte +0x24] + 0xc) != 0) && (*param_1 != 0)) && (iVar1 = FUN_0070daa0(), iVar1 != 0)
     ) {
    _AIL_pause_stream_8 [GHIDRA-AUTO-LABEL: unreliable](*param_1,1);
    iVar1 = _AIL_stream_position_4 [GHIDRA-AUTO-LABEL: unreliable](*param_1);
    param_1[0x51 = byte +0x144] = iVar1;
  }
  local_8 = 0xffffffff;
  FUN_00733780();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0070ded0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0070ded0: 16-byte aligned: YES
  0070ded0  55             PUSH EBP
  0070ded1  8B EC          MOV EBP,ESP
  0070ded3  83 EC 18       SUB ESP,0x18
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
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

### Static facts (script output, report-only)
- size 237 B, RET 0
- callers 5: 0x007051c0, 0x00709e00, 0x00709f80, 0x0070b1b0, 0x0070c0c0
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00710930 (CALL), 0x00734f60 CExoString::CompareNoCase(char*) (CALL), 0x00741070 CClientExoApp::GetCreditSequenceInProgress (CALL), 0x00733780 CExoString::Destructor (CALL), 0x0070daa0 CExoStreamingSoundSourceInternal::Update3DStream (CALL), ...
- globals: 0x00a1b4a4 READ
- imports: _AIL_pause_stream@8 (MSS32.DLL), _AIL_stream_position@4 (MSS32.DLL)
- twins: none

## Card B20-3-15

GOG function: CSWGuiInGameMessages::ShowEffects  gog 0x0052bec0  callconv=(blank)  param_size_bytes=0  gog_size=1659  gog_retn=None
Candidate Steam address: 0x0075b260 (size 263, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.43) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.09 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0075ae40:STR0/CALLEE0/SIG0.139/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0052bd30 CSWGuiInGameMessages::ShowCombatMessages  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0052c53b Global::SWMSGSetupEffectRow  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (24), mapped to Steam through trusted rows only:
  0x0040cc10 CClientExoApp::GetInGameGui -> Steam 0x0073f750  in candidate callees: no
  0x0040d070 CClientExoApp::GetSWParty -> Steam 0x0073fb90  in candidate callees: no
  0x00429780 CSWCCreature::GetServerCreature -> Steam 0x0077d800  in candidate callees: no
  0x004789d0 CSWParty::GetCharacter -> Steam ?  in candidate callees: n/a
  0x004eaed0 CExoString::Constructor -> Steam 0x00733540  in candidate callees: no
  0x005340d0 CExoArrayList_CSWGuiControl__::CExoArrayList -> Steam ?  in candidate callees: n/a
  0x00534170 CExoArrayList::Add_2 -> Steam ?  in candidate callees: n/a
  0x00605890 CExoString::Destructor -> Steam 0x00733780  in candidate callees: no
  ... 16 more GOG callees
GOG callers mapped through trusted rows (0 of 3): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {0x4,0x8,0x14,0x20,0x40,0x48,0xa0,0x2a4,0x2d0,0xc14,0xf04,0xfc8} | Steam {}
imm sets (* = in both, 6 common): GOG {0x2*,0x3*,0x4*,0x5*,0xa,0x68,0xcc,0xd8*,0xf0,0x148,0x15c,0x1c0*,0x358,0x648,...(+10)} | Steam {0x2*,0x3*,0x4*,0x5*,0x6,0x7,0x8,0xc,0x60,0x78,0xd8*,0x1c0*,0x308,0x4d8,...(+1)}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x0075b260: 16-byte aligned: YES
  0075b260  55             PUSH EBP
  0075b261  8B EC          MOV EBP,ESP
  0075b263  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x0075b260
string literals (0):
direct callees (6):
  0x0041D190  (unnamed)
  0x0041A3D0  CSWGuiButton::Destructor
  0x00417290  (unnamed)
  0x00414C60  CSWGuiBorder::Destructor_2
  0x00418640  CSWGuiControl::Destructor
  0x0040E860  CSWGuiPanel::Destructor
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0075b260 FUN_0075b260 size=263 ret=0 callers=18 callees=6

void __fastcall FUN_0075b260(int param_1)

{
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  puStack_c = &LAB_00963fb3;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 4;
  FUN_0041d190(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
  local_8._0_1_ = 3;
  FUN_0041a3d0();
  local_8._0_1_ = 2;
  FUN_0041a3d0();
  *(undefined ***)(param_1 + 0x1c0) = CSWGuiLabel::vftable;
  local_8._0_1_ = 6;
  FUN_00417290();
  local_8._0_1_ = 5;
  FUN_00414c60();
  local_8._0_1_ = 1;
  FUN_00418640();
  *(undefined ***)(param_1 + 0x78) = CSWGuiLabel::vftable;
  local_8._0_1_ = 8;
  FUN_00417290();
  local_8._0_1_ = 7;
  FUN_00414c60();
  local_8 = (uint)local_8._1_3_ << 8;
  FUN_00418640();
  local_8 = 0xffffffff;
  FUN_0040e860();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0075ae40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0075ae40: 16-byte aligned: YES
  0075ae40  55             PUSH EBP
  0075ae41  8B EC          MOV EBP,ESP
  0075ae43  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x0075ae40
string literals (4):
  0x0099E690  at 0075af8a  'confirm_p'
  0x0099E688  at 0075afa7  'BTN_OK'
  0x0099E67C  at 0075afdd  'BTN_CANCEL'
  0x0099E670  at 0075b014  'LB_MESSAGE'
direct callees (15):
  0x0040E5A0  CSWGuiPanel::Constructor
  0x00419740  LabelCtor
  0x0041A510  ButtonCtor
  0x0041CE90  (unnamed)
  0x00710840  (unnamed)
  0x0040EE40  CSWGuiPanel::StartLoadFromLayout
  0x00733570  CExoString::CExoString_char
  0x0040F620  CSWGuiPanel::InitControl
  0x00733780  CExoString::~CExoString
  0x0040F5A0  CSWGuiPanel::StopLoadFromLayout
  0x00418AF0  CSWGuiControl::AddEvent
  0x0040FD80  CSWGuiPanel::AddControl
  0x0040EA00  CSWGuiPanel::CenterPanel
  0x00479AB0  Global::GetResolutionTextScalar
  0x0091C860  _ftol2_sse
struct offsets touched (top 10):
  +0x68 [dec 104]  x10  via EAX,ECX,EDX
  +0x4 [dec 4]  x9  via EAX,ECX,EDX
  +0x8 [dec 8]  x7  via EAX,ECX,EDX
  +0xC [dec 12]  x7  via EAX,ECX,EDX
  +0x48 [dec 72]  x2  via EAX,EDX
  +0x1C0 [dec 448]  x2  via EAX,EDX
  +0x64 [dec 100]  x1  via EAX
  +0x6C [dec 108]  x1  via EDX
  +0x70 [dec 112]  x1  via EAX
  +0x74 [dec 116]  x1  via ECX
```

### Static facts (script output, report-only)
- size 263 B, RET 0
- callers 18: 0x0075b370, 0x008869f0, 0x00888a40, 0x00888ad0, 0x0088c110, ...
- callees: 0x0041d190 (CALL), 0x0041a3d0 CSWGuiButton::Destructor (CALL), 0x00417290 (CALL), 0x00414c60 CSWGuiBorder::Destructor_2 (CALL), 0x00418640 CSWGuiControl::Destructor (CALL), 0x0040e860 CSWGuiPanel::Destructor (CALL)
- twins: none

## Card B20-3-16

GOG function: Global::AuroraUseARBShaders  gog 0x0086ba30  callconv=(blank)  param_size_bytes=0  gog_size=33  gog_retn=0
Candidate Steam address: 0x00484c80 (size 29, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00484ca0:STR0/CALLEE1/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0086ba10 Global::AurCheckFrameBufferModificationsAvailable  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0086ba60 Global::AurClearActiveBins  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (1 of 12): the card's A-CALLEE credit may come from these, not from callees
  0x00878e50 Global::RenderSkinnedVP -> Steam 0x004c3430  in candidate callers: no
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
prologue for 0x00484c80: 16-byte aligned: YES
  00484c80  55             PUSH EBP
  00484c81  8B EC          MOV EBP,ESP
  00484c83  E8 88 FC FF FF CALL 0x00484910
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00484c80
string literals (0):
direct callees (2):
  0x00484910  GL capability test #1 (always returns 0)
  0x00484970  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00484c80 FUN_00484c80 size=29 ret=0 callers=4 callees=2

undefined4 FUN_00484c80(void)

{
  int iVar1;
  
  iVar1 = FUN_00484910();
  if ((iVar1 == 0) && (iVar1 = FUN_00484970(), iVar1 == 0)) {
    return 0;
  }
  return 1;
}
```

### decoy: 0x00484ca0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00484ca0: 16-byte aligned: YES
  00484ca0  55             PUSH EBP
  00484ca1  8B EC          MOV EBP,ESP
  00484ca3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00484ca0
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 29 B, RET 0
- callers 4: 0x006772e0 Global::ExecuteCommandEffectVisualEffect, 0x0076d9e0, 0x007bc330 CClientOptions::SetFrameBuffer, 0x007d8190
- callees: 0x00484910 GL capability test #1 (always returns 0) (CALL), 0x00484970 (CALL)
- twins: 0x00484c60 (distinguishing: 0x00484c83: CALL 0x00484910)

## Card B20-3-17

GOG function: CResRef::operator==  gog 0x0061a490  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x00710b40 (size 44, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00710980:STR0/CALLEE0/SIG0.5/SRV0/VT0
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
imm sets (* = in both, 3 common): GOG {0x4*,0xc*,0x10*} | Steam {0x4*,0xc*,0x10*}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x00710b40: 16-byte aligned: YES
  00710b40  55             PUSH EBP
  00710b41  8B EC          MOV EBP,ESP
  00710b43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00710b40
string literals (0):
direct callees (1):
  0x0091AC7C  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00710b40 FUN_00710b40 size=44 ret=4 callers=24 callees=1

bool __thiscall FUN_00710b40(void *param_1,void *param_2)

{
  int iVar1;
  
  iVar1 = _memcmp [GHIDRA-AUTO-LABEL: unreliable](param_2,param_1,0x10);
  return iVar1 == 0;
}
```

### decoy: 0x00710980 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00710980: 16-byte aligned: YES
  00710980  55             PUSH EBP
  00710981  8B EC          MOV EBP,ESP
  00710983  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00710980
string literals (0):
direct callees (1):
  0x0091DBA0  _memcpy
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EDX
```

### Static facts (script output, report-only)
- size 44 B, RET 4
- callers 24: 0x0052e640, 0x00561240, 0x00703950, 0x0070a8d0, 0x00710a70, ...
- callees: 0x0091ac7c (CALL)
- twins: none

## Card B20-3-18

GOG function: GLU32_DLL::gluPerspective  gog 0x009433e6  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b43c (size 6, retn None)  THUNK -> 0x00000148 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: fail:unaligned | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000148 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR n/a | A-CALLEE n/a | A-SIG n/a | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00401000:STRn/a/CALLEEn/a/SIGn/a/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009433e0 GLU32_DLL::gluBuild2DMipmaps -> 0x0093b430  gate_class=REVIEW
- GOG-order neighbour after:  0x009433ec VERSION_DLL::VerQueryValueA  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
(no GOG fingerprint available for 0x009433e6)

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=n/a ; asm RET = none
Stack Slot Match: UNKNOWN (GOG 0 bytes -> x86 slots 0; ASM RET none)
```

### prologue
```
prologue: no asm for 0x00000148
```

### anchors
```
anchors: no asm for 0x00000148
```

### annotate (candidate, first 80 lines)
```
(no decompile for 0x00000148)
```

### decoy: 0x00401000 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00401000: 16-byte aligned: YES
  00401000  55             PUSH EBP
  00401001  8B EC          MOV EBP,ESP
  00401003  C6 05 20 3C A1 00 00 MOV byte ptr [0x00a13c20],0x0
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00401000
string literals (0):
direct callees (0):
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size unknown (not in steam_functions)
- asm: no decomp file in dump
- twins: none
