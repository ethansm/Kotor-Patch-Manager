# Review cards B20 chunk 6

Answer each card with one TSV row in `steam_db_batches/B20/review/<your-name>.tsv`, header:
`gog_addr<TAB>steam_addr<TAB>verdict<TAB>corrected_addr<TAB>content_anchor<TAB>confidence<TAB>reason`
(verdict = match | mismatch | unsure).

Rules (RE_AGENT_GUIDE.md section 4): assume the candidate is a neighbouring function or thunk with an identical RET
signature. You are FORBIDDEN to answer `match` on `ret N`, size or name alone. To answer `match` you MUST cite one
content anchor from the evidence: a string literal (address + text), a confirmed callee (address + name), or a member
offset with its access (asm line). Otherwise answer `unsure`. Ghidra auto labels are not evidence. Decimal numbers in
decompiles are decimal; `Stack Slot Match` below is already computed for you.

## Card B20-6-01

GOG function: Global::`eh_vector_constructor_iterator'  gog 0x009258bf  callconv=__stdcall  param_size_bytes=20  gog_size=77  gog_retn=20
Candidate Steam address: 0x0091d67e (size 77, retn 20)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: warn:autolabel | G2 align: fail:unaligned | G3 prologue: fail:no_standard_prologue | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 50 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE,SIG  (content level C2)
- evidence summary: CALLEE 50 (callers: Constructor 0x780460, LoadArea 0x7a2fb0, InitializeRepliesData 0x7c06f0); SIG 0.50
- best decoy (neighbour/other candidate) scores: 0x008103c0:STR0/CALLEE0/SIG0.125/SRV0/VT0
- flags: autolabel:`eh_vector_constructor_iterato
- GOG-order neighbour before: 0x00925530 Global::memcpy  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00925936 Global::rand -> 0x0091eb8b  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (3), mapped to Steam through trusted rows only:
  0x0092590c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca2c unnamed -> Steam ?  in candidate callees: n/a
  0x0092ca71 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (51 of 135): the card's A-CALLEE credit may come from these, not from callees
  0x0042c9e0 CClientExoAppInternal::Constructor -> Steam 0x00780460  in candidate callers: yes
  0x0044f9b0 CSWCArea::LoadArea -> Steam 0x007a2fb0  in candidate callers: yes
  0x004d0d80 CGuiInGame::InitializeRepliesData -> Steam 0x007c06f0  in candidate callers: yes
  0x004ed490 CSWSpellArray::Load -> Steam 0x006c1490  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 3 common): GOG {0x10*,0x14*,0xfffffffe*} | Steam {0x10*,0x14*,0xfffffffe*}

### abi
```
GOG param bytes: 20; callconv: stdcall
expected_ret = ceil(20/4)*4 = 20 (x86 4-byte stack-slot promotion)
observed: dump header ret=20 ; asm RET = 20
Stack Slot Match: TRUE (GOG 20 bytes -> x86 slots 20; ASM RET 20)
```

### prologue
```
prologue for 0x0091d67e: 16-byte aligned: NO (addr % 16 = 14)
  0091d67e  6A 10          PUSH 0x10
  0091d680  68 08 03 9F 00 PUSH 0x9f0308
  0091d685  E8 42 77 00 00 CALL 0x00924dcc
standard prologue: NO
```

### anchors
```
anchors for 0x0091d67e
string literals (0):
direct callees (3):
  0x00924DCC  (unnamed)
  0x0091D6CB  (unnamed)
  0x00924E11  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0091d67e `eh_vector_constructor_iterator' size=77 ret=20 callers=134 callees=3

/* WARNING: Function: __SEH_prolog4 replaced with injection: SEH_prolog4 */
/* WARNING: Function: __SEH_epilog4 replaced with injection: EH_epilog3 */
/* Library Function - Single Match [GHIDRA-AUTO-LABEL: unreliable]
    void __stdcall `eh vector constructor iterator'(void *,unsigned int,int,void (__thiscall*)(void
   *),void (__thiscall*)(void *))
   
   Libraries: Visual Studio 2005 Release, Visual Studio 2008 Release, Visual Studio 2010 Release */

void _eh_vector_constructor_iterator_ [GHIDRA-AUTO-LABEL: unreliable]
               (void *param_1,uint param_2,int param_3,_func_void_void_ptr [GHIDRA-AUTO-LABEL: unreliable] *param_4,
               _func_void_void_ptr [GHIDRA-AUTO-LABEL: unreliable] *param_5)

{
  void *in_stack_ffffffcc;
  undefined4 local_20;
  
  for (local_20 = 0; local_20 < param_3; local_20 = local_20 + 1) {
    (*param_4)(in_stack_ffffffcc);
  }
  FUN_0091d6cb();
  return;
}
```

### decoy: 0x008103c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x008103c0: 16-byte aligned: YES
  008103c0  55             PUSH EBP
  008103c1  8B EC          MOV EBP,ESP
  008103c3  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x008103c0
string literals (0):
direct callees (7):
  0x008935A0  CSWCMessage::ReadOBJECTIDClient
  0x005E36A0  (unnamed)
  0x005E3DE0  (unnamed)
  0x0073F4B0  CClientExoApp::GetGameObjectArray
  0x0053DED0  (unnamed)
  0x0053DFB0  CGameObjectArray::GetGameObject
  0x007FD840  CSWCItem::Destructor_2
struct offsets touched (top 5):
  +0x34 [dec 52]  x3  via EDX
  +0x10 [dec 16]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via EAX
  +0x84 [dec 132]  x1  via EAX
  +0x94 [dec 148]  x1  via EDX
```

### Static facts (script output, report-only)
- size 77 B, RET 20
- callers 134: 0x0040e170, 0x00451a80, 0x00464040, 0x004641a0, 0x00474230, ...
- callees: 0x00924dcc (CALL), 0x0091d6cb (CALL), 0x00924e11 (CALL)
- indirect: 0x0091d6a2 unresolved
- twins: none

## Card B20-6-02

GOG function: CExoMoviePlayer::CancelMovie  gog 0x009211a0  callconv=(blank)  param_size_bytes=8  gog_size=31  gog_retn=8
Candidate Steam address: 0x0093b550 (size 31, retn 8)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callees: CancelMovie 0x40cc30; callers: WaitForMovie 0x798820, CancelMovie 0x7a0500)
- best decoy (neighbour/other candidate) scores: 0x0093b4f0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00921140 CExoMoviePlayer::Destructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009211c0 CExoMoviePlayer::IsMoviePlaying  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0091fe20 CExoMoviePlayerInternal::CancelMovie -> Steam 0x0040cc30  in candidate callees: yes
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x00444e40 CClientExoAppInternal::WaitForMovie -> Steam 0x00798820  in candidate callers: yes
  0x0044cb00 CClientExoAppInternal::CancelMovie -> Steam 0x007a0500  in candidate callers: yes
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
prologue for 0x0093b550: 16-byte aligned: YES
  0093b550  55             PUSH EBP
  0093b551  8B EC          MOV EBP,ESP
  0093b553  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093b550
string literals (0):
direct callees (1):
  0x0040CC30  CExoMoviePlayerInternal::CancelMovie
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093b550 FUN_0093b550 size=31 ret=8 callers=3 callees=1
THUNK -> 0x0040CC30 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0093b550(undefined4 param_1,undefined4 param_2)

{
  FUN_0040cc30(param_1,param_2);
  return;
}
```

### decoy: 0x0093b4f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093b4f0: 16-byte aligned: YES
  0093b4f0  55             PUSH EBP
  0093b4f1  8B EC          MOV EBP,ESP
  0093b4f3  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093b4f0
string literals (0):
direct callees (2):
  0x0040C9F0  CExoMoviePlayerInternal::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 31 B, RET 8
- callers 3: 0x00798820 CClientExoAppInternal::WaitForMovie, 0x00798cd0, 0x007a0500 CClientExoAppInternal::CancelMovie
- callees: 0x0040cc30 CExoMoviePlayerInternal::CancelMovie (CALL)
- twins: none

## Card B20-6-03

GOG function: CSWGuiLegalScreen::Destructor  gog 0x0053a810  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0087fe20 (size 812, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.05 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0087fdf0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053a760 CSWGuiLegalScreen::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053ab70 CSWDemoScreen::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x20,0x50,0x60,0x68,0x6c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x3,0x4*,0x5,0xc,0x14,0x32,0x41,0x50,0xfffffffc}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0087fe20: 16-byte aligned: YES
  0087fe20  55             PUSH EBP
  0087fe21  8B EC          MOV EBP,ESP
  0087fe23  83 EC 50       SUB ESP,0x50
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087fe20
string literals (0):
direct callees (4):
  0x00736200  (unnamed)
  0x00409ED0  Global::UpdateScreen
  0x0040F390  CSWGuiPanel::SetBackground
  0x00710A10  (unnamed)
struct offsets touched (top 6):
  +0x4 [dec 4]  x6  via EAX,ECX,EDX
  +0x50 [dec 80]  x4  via EAX,ECX,EDX
  +0x68 [dec 104]  x3  via ECX,EDX
  +0x20 [dec 32]  x2  via EAX,ECX
  +0x60 [dec 96]  x1  via ECX
  +0x6C [dec 108]  x1  via ECX
```

### annotate (candidate, first 80 lines)
```
// FN 0087fe20 FUN_0087fe20 size=812 ret=4 callers=1 callees=5

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0087fe20(int param_1,undefined4 param_2)

{
  uint uVar1;
  int iVar2;
  undefined4 uVar3;
  float local_54;
  float local_50;
  float local_4c;
  undefined4 local_40;
  undefined4 local_3c;
  float local_38;
  float local_34;
  float local_30;
  float local_2c;
  float local_1c;
  float local_14;
  int local_10;
  float local_c;
  int local_8;
  
  local_c = *(float *)(param_1 + 0x50);
  if (*(int *)(param_1 + 0x68) != 0) {
    while (uVar1 = FUN_00736200(),
          uVar1 < (uint)(*(int *)(param_1 + 0x68) + *(int *)(param_1 + 0x6c))) {
      Sleep(0x32);
    }
    local_10 = FUN_00736200();
    local_8 = local_10;
    while ((float)_DAT_00987538 < local_c) {
      local_c = (float)((float10)local_c -
                       (float10)(uint)(local_8 - local_10) / (float10)_DAT_0099e208);
      local_10 = local_8;
      local_8 = FUN_00736200();
      local_14 = local_c;
      if (local_c < (float)_DAT_00986b38) {
        local_14 = 0.0;
      }
      if ((float)_DAT_00986b50 <= local_14) {
        local_2c = 1.0;
      }
      else {
        local_2c = local_14;
      }
      if (local_2c <= (float)_DAT_00986b38) {
        local_34 = 0.0;
      }
      else {
        if ((float)_DAT_00986b50 <= local_14) {
          local_30 = 1.0;
        }
        else {
          local_30 = local_14;
        }
        local_34 = local_30;
      }
      *(float *)(param_1 + 0x50) = local_34;
      FUN_00409ed0(_DAT_00986da8,0,0);
      Sleep(0x14);
    }
  }
  local_c = 0.0;
  if ((float)_DAT_00986b50 <= 0.0) {
    local_38 = 1.0;
  }
  else {
    local_38 = 0.0;
  }
  if (local_38 <= (float)_DAT_00986b38) {
    local_40 = 0;
  }
  else {
    if ((float)_DAT_00986b50 <= 0.0) {
      local_3c = 0x3f800000;
    }
    else {
      local_3c = 0;
/* ... 48 more lines truncated by --max-lines */
```

### decoy: 0x0087fdf0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087fdf0: 16-byte aligned: YES
  0087fdf0  55             PUSH EBP
  0087fdf1  8B EC          MOV EBP,ESP
  0087fdf3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0087fdf0
string literals (0):
direct callees (2):
  0x0040E860  CSWGuiPanel::Destructor
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 812 B, RET 4
- callers 1: 0x007813a0 CClientExoAppInternal::BeginIntro
- callees: 0x00736200 (CALL), 0x000000b6 (CALL), 0x00409ed0 Global::UpdateScreen (CALL), 0x0040f390 CSWGuiPanel::SetBackground (CALL), 0x00710a10 (CALL)
- globals: 0x00a1b488 READ, 0x00a1b488 READ, 0x00a1b488 READ
- imports: Sleep (KERNEL32.DLL)
- twins: none

## Card B20-6-04

GOG function: CExoMoviePlayer::PlayMoviesAsync  gog 0x009211e0  callconv=(blank)  param_size_bytes=12  gog_size=35  gog_retn=12
Candidate Steam address: 0x0093b590 (size 35, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 3 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 3 (callees: PlayMoviesAsync 0x40da40; callers: PlayLoadModuleMovies 0x7986a0, PlayMoviesInExoArrayList 0x7a0fb0)
- best decoy (neighbour/other candidate) scores: 0x0093b5c0:STR0/CALLEE2/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x009211c0 CExoMoviePlayer::IsMoviePlaying  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00921210 CExoMoviePlayer::ShowFadePanel  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00920c30 CExoMoviePlayerInternal::PlayMoviesAsync -> Steam 0x0040da40  in candidate callees: yes
GOG callers mapped through trusted rows (2 of 2): the card's A-CALLEE credit may come from these, not from callees
  0x00444cc0 CClientExoAppInternal::PlayLoadModuleMovies -> Steam 0x007986a0  in candidate callers: yes
  0x0044d5b0 CClientExoAppInternal::PlayMoviesInExoArrayList -> Steam 0x007a0fb0  in candidate callers: yes
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
prologue for 0x0093b590: 16-byte aligned: YES
  0093b590  55             PUSH EBP
  0093b591  8B EC          MOV EBP,ESP
  0093b593  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0093b590
string literals (0):
direct callees (1):
  0x0040DA40  CExoMoviePlayerInternal::PlayMoviesAsync
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 0093b590 FUN_0093b590 size=35 ret=12 callers=2 callees=1
THUNK -> 0x0040DA40 (Ghidra may omit __thiscall on thunks; ECX is passed through)

void FUN_0093b590(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  FUN_0040da40(param_1,param_2,param_3);
  return;
}
```

### decoy: 0x0093b5c0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0093b5c0: 16-byte aligned: YES
  0093b5c0  55             PUSH EBP
  0093b5c1  8B EC          MOV EBP,ESP
  0093b5c3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0093b5c0
string literals (0):
direct callees (1):
  0x0040DAC0  (unnamed)
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 35 B, RET 12
- callers 2: 0x007986a0 CClientExoAppInternal::PlayLoadModuleMovies, 0x007a0fb0 CClientExoAppInternal::PlayMoviesInExoArrayList
- callees: 0x0040da40 CExoMoviePlayerInternal::PlayMoviesAsync (CALL)
- twins: none

## Card B20-6-05

GOG function: List_SafePointer_WindManager___::Destructor  gog 0x008fe2a0  callconv=__thiscall  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x00463c80 (size 63, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00463c40:STR0/CALLEE0/SIG0.25/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fe280 Global::AurVertexProgramMakeCurrent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fe2d0 PointSourceWind::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (24 of 466): the card's A-CALLEE credit may come from these, not from callees
  0x0041e770 CSWCCreature::DoFootstepVisual -> Steam 0x00772900  in candidate callers: no
  0x0044f6b0 CSWCArea::Destructor_2 -> Steam 0x007a2cb0  in candidate callers: no
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {}
imm sets (* = in both, 0 common): GOG {0x4,0x8} | Steam {0x3,0x10}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00463c80: 16-byte aligned: YES
  00463c80  55             PUSH EBP
  00463c81  8B EC          MOV EBP,ESP
  00463c83  83 EC 10       SUB ESP,0x10
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00463c80
string literals (0):
direct callees (1):
  0x004518C0  (unnamed)
struct offsets touched (top 0):
```

### annotate (candidate, first 80 lines)
```
// FN 00463c80 FUN_00463c80 size=63 ret=0 callers=6 callees=1

void __fastcall FUN_00463c80(int *param_1)

{
  if ((*param_1 != 0) && (*param_1 != 0)) {
    FID_conflict__vector_deleting_destructor_ [GHIDRA-AUTO-LABEL: unreliable](3);
  }
  return;
}
```

### decoy: 0x00463c40 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463c40: 16-byte aligned: YES
  00463c40  55             PUSH EBP
  00463c41  8B EC          MOV EBP,ESP
  00463c43  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00463c40
string literals (0):
direct callees (1):
  0x00451A80  (unnamed)
struct offsets touched (top 2):
  +0x4 [dec 4]  x1  via EAX
  +0x8 [dec 8]  x1  via ECX
```

### Static facts (script output, report-only)
- size 63 B, RET 0
- callers 6: 0x00455240, 0x0050c7a0, 0x00949928, 0x00949978, 0x0094d421, ...
- callees: 0x004518c0 (CALL)
- twins: 0x00463d00, 0x00463df0 (distinguishing: 0x00463caa: CALL 0x004518c0)

## Card B20-6-06

GOG function: CClientAIMaster::AddObject  gog 0x0044eeb0  callconv=(blank)  param_size_bytes=4  gog_size=36  gog_retn=4
Candidate Steam address: 0x007a2540 (size 36, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x007a2510:STR0/CALLEE5/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0044ed40 CClientAIMaster::UpdateState  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: no
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: no
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: no
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

## Card B20-6-07

GOG function: CRes::GetData  gog 0x00912f70  callconv=(blank)  param_size_bytes=0  gog_size=17  gog_retn=0
Candidate Steam address: 0x00414ba0 (size 17, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 2 | A-SIG 0.50 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: CALLEE  (content level C1)
- evidence summary: CALLEE 2 (callers: WriteResource_2 0x721bb0, CreateParty 0x5f9220)
- best decoy (neighbour/other candidate) scores: 0x00414b70:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00912f40 CSWGuiObject::SetExtent -> 0x00414b70  trusted
- GOG-order neighbour after:  0x00912fc0 CSWGuiBorder::Destructor -> 0x00414bf0  trusted

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (0), mapped to Steam through trusted rows only:
GOG callers mapped through trusted rows (2 of 3): the card's A-CALLEE credit may come from these, not from callees
  0x0062bb40 CERFFile::WriteResource_2 -> Steam 0x00721bb0  in candidate callers: yes
  0x006fed90 CSWPartyTable::CreateParty -> Steam 0x005f9220  in candidate callers: yes
GOG strings (0): 
disp sets (* = in both, 1 common): GOG {0x10*} | Steam {0x10*}
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
prologue for 0x00414ba0: 16-byte aligned: YES
  00414ba0  55             PUSH EBP
  00414ba1  8B EC          MOV EBP,ESP
  00414ba3  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00414ba0
string literals (0):
direct callees (0):
struct offsets touched (top 1):
  +0x10 [dec 16]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00414ba0 FUN_00414ba0 size=17 ret=0 callers=3 callees=0

undefined4 __fastcall FUN_00414ba0(int param_1)

{
  return *(undefined4 *)(param_1 + 0x10);
}
```

### decoy: 0x00414b70 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00414b70: 16-byte aligned: YES
  00414b70  55             PUSH EBP
  00414b71  8B EC          MOV EBP,ESP
  00414b73  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00414b70
string literals (0):
direct callees (0):
struct offsets touched (top 3):
  +0x4 [dec 4]  x2  via EAX,ECX
  +0x8 [dec 8]  x2  via EAX,ECX
  +0xC [dec 12]  x2  via EAX,ECX
```

### Static facts (script output, report-only)
- size 17 B, RET 0
- callers 3: 0x005f9220 CSWPartyTable::CreateParty, 0x00721bb0 CERFFile::WriteResource_2, 0x0085ee80
- twins: none

## Card B20-6-08

GOG function: CExoInputInternal::GetEvents  gog 0x006134e0  callconv=(blank)  param_size_bytes=12  gog_size=2768  gog_retn=None
Candidate Steam address: 0x0072c6c0 (size 217, retn 12)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: n/a | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.10 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0072c500:STR0/CALLEE0/SIG0.254/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x006134a0 CExoInputInternal::GetActive  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00613fb2 CExoInputInternal::GetEvents_2  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (16), mapped to Steam through trusted rows only:
  0x0044e290 CSWPlayerList::GetFirstPlayer -> Steam 0x00521060  in candidate callees: no
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
disp sets (* = in both, 2 common): GOG {0x4,0x8,0xc*,0x14,0x1c,0x24,0x30,0x34,0x188,0x1a0,0x1b0*,0x1b8,0x1c4,0x410} | Steam {0xc*,0x10,0x1a4,0x1b0*}
imm sets (* = in both, 1 common): GOG {0x2,0x5,0x6,0xa,0xc*,0x10,0x14,0x1a4,0x254,0x601} | Steam {0x4,0xc*,0x18,0x1c,0x49}

### abi
```
GOG param bytes: 12; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(12/4)*4 = 12 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=12 ; asm RET = 12
Stack Slot Match: TRUE (GOG 12 bytes -> x86 slots 12; ASM RET 12)
```

### prologue
```
prologue for 0x0072c6c0: 16-byte aligned: YES
  0072c6c0  55             PUSH EBP
  0072c6c1  8B EC          MOV EBP,ESP
  0072c6c3  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
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

### annotate (candidate, first 80 lines)
```
// FN 0072c6c0 FUN_0072c6c0 size=217 ret=12 callers=1 callees=4

void __thiscall FUN_0072c6c0(int param_1,undefined4 param_2,int param_3,int param_4)

{
  int iVar1;
  undefined1 local_18 [8];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00950448;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  if (((param_4 != 0) || (param_3 < 0)) || (*(int *)(param_1 + 0x1b0) <= param_3)) {
    iVar1 = *(int *)(param_3 * 0x10 + *(int *)(param_1 + 0x1a4) + 0xc);
    FUN_00733540(DAT_00a11f20 ^ (uint)&stack0xfffffffc);
    local_8 = 0;
    FUN_00734270(local_18,"%d %c %d %d\n",param_2,0x49,*(undefined4 *)(iVar1 + 0x10),
                 *(undefined4 *)(param_3 * 0x10 + *(int *)(param_1 + 0x1a4)));
    FUN_00739aa0(local_18);
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x0072c500 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0072c500: 16-byte aligned: YES
  0072c500  55             PUSH EBP
  0072c501  8B EC          MOV EBP,ESP
  0072c503  83 EC 28       SUB ESP,0x28
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0072c500
string literals (0):
direct callees (8):
  0x007312A0  (unnamed)
  0x007311C0  (unnamed)
  0x007A1720  (unnamed)
  0x00561430  (unnamed)
  0x0058C370  (unnamed)
  0x007A1780  (unnamed)
  0x007F2A90  (unnamed)
  0x009196FD  operator delete
struct offsets touched (top 9):
  +0xC [dec 12]  x4  via EAX,ECX
  +0x4 [dec 4]  x3  via ECX,EDX
  +0x1A0 [dec 416]  x3  via EAX,ECX,EDX
  +0x24 [dec 36]  x2  via EAX,EDX
  +0x188 [dec 392]  x2  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x18 [dec 24]  x1  via ECX
  +0x34 [dec 52]  x1  via EAX
  +0x18C [dec 396]  x1  via ECX
```

### Static facts (script output, report-only)
- size 217 B, RET 12
- callers 1: 0x0072fa10
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00734270 CExoString::Format (CALL), 0x00739aa0 (CALL), 0x00733780 CExoString::Destructor (CALL)
- strings: 0x0099c800 "%d %c %d %d\n"
- twins: none

## Card B20-6-09

GOG function: CSWGuiLegalScreen::Destructor  gog 0x0053a810  callconv=(blank)  param_size_bytes=4  gog_size=44  gog_retn=4
Candidate Steam address: 0x0087fae0 (size 600, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.07 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x0087f9a0:STR0/CALLEE0/SIG0/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x0053a760 CSWGuiLegalScreen::Constructor  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x0053ab70 CSWDemoScreen::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (2), mapped to Steam through trusted rows only:
  0x0090cce0 CSWGuiPanel::Destructor -> Steam 0x0040e860  in candidate callees: no
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 0): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4,0x8,0xc,0x20,0x24,0x28,0x68,0x6c,0x6e,0x10c}
imm sets (* = in both, 1 common): GOG {0x4*} | Steam {0x2,0x4*,0x5,0x8,0xc,0xac,0xff}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: TRUE (GOG 4 bytes -> x86 slots 4; ASM RET 4)
```

### prologue
```
prologue for 0x0087fae0: 16-byte aligned: YES
  0087fae0  55             PUSH EBP
  0087fae1  8B EC          MOV EBP,ESP
  0087fae3  81 EC AC 00 00 00 SUB ESP,0xac
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x0087fae0
string literals (1):
  0x0099EDC4  at 0087fb3e  'lookathook'
direct callees (5):
  0x0073F490  CClientExoApp::GetModuleCamera
  0x00463120  (unnamed)
  0x0044B8A0  Vector::Dot
  0x0073FEA0  CClientExoApp::GetSWGuiManager
  0x0087E410  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x11  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x68 [dec 104]  x2  via EAX,EDX
  +0xC [dec 12]  x1  via EDX
  +0x20 [dec 32]  x1  via EDX
  +0x24 [dec 36]  x1  via EAX
  +0x28 [dec 40]  x1  via EAX
  +0x6C [dec 108]  x1  via EAX
  +0x6E [dec 110]  x1  via EAX
  +0x10C [dec 268]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 0087fae0 FUN_0087fae0 size=600 ret=4 callers=0 callees=5

uint __thiscall FUN_0087fae0(int param_1,float *param_2)

{
  float *pfVar1;
  undefined4 uVar2;
  int *piVar3;
  int iVar4;
  float10 fVar5;
  float10 fVar6;
  undefined1 *puVar7;
  undefined1 local_64 [12];
  undefined1 local_58 [12];
  undefined1 local_4c [12];
  undefined1 local_40 [16];
  int *local_30;
  float local_2c;
  float local_28;
  float local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  uint local_10;
  int *local_c;
  int *local_8;
  
  local_c = (int *)0x0;
  local_8 = (int *)(**(code **)(**(int **)(param_1 + 0x68) + 8))(0xff);
  local_10 = 0;
  if (local_8 != (int *)0x0) {
    local_14 = 0;
    local_18 = 0;
    local_1c = 0;
    local_20 = 0x3f800000;
    local_c = (int *)(**(code **)(*local_8 + 0x10c))("lookathook");
    if (local_c != (int *)0x0) {
      (**(code **)(*local_c + 0xc))(param_2,&local_20);
      local_30 = (int *)FUN_0073f490();
      puVar7 = local_40;
      (**(code **)(*local_30 + 0x24))(puVar7,local_4c);
      pfVar1 = (float *)FUN_00463120(puVar7);
      local_2c = -*pfVar1;
      local_28 = -pfVar1[1 = byte +0x4];
      local_24 = -pfVar1[2 = byte +0x8];
      fVar5 = (float10)FUN_0044b8a0(param_2,&local_2c);
      uVar2 = (**(code **)(*local_30 + 0x20))(local_58,&local_2c);
      fVar6 = (float10)FUN_0044b8a0(uVar2);
      if ((float10)(double)fVar5 <= fVar6) {
        local_10 = 2;
      }
      else {
        local_10 = 1;
      }
      piVar3 = (int *)FUN_0073f490();
      pfVar1 = (float *)(**(code **)(*piVar3 + 0x28))(local_64,*param_2,param_2[1 = byte +0x4],param_2[2 = byte +0x8],1);
      *param_2 = *pfVar1;
      param_2[1 = byte +0x4] = pfVar1[1 = byte +0x4];
      param_2[2 = byte +0x8] = pfVar1[2 = byte +0x8];
      iVar4 = FUN_0073fea0();
      *param_2 = (float)(int)*(short *)(iVar4 + 0x6c) * *param_2;
      iVar4 = FUN_0073fea0();
      param_2[1 = byte +0x4] = (float)(int)*(short *)(iVar4 + 0x6e) * param_2[1 = byte +0x4];
    }
  }
  if (local_c == (int *)0x0) {
    local_10 = FUN_0087e410(param_2);
    local_10 = local_10 & 0xff;
  }
  return local_10 & 0xff;
}
```

### decoy: 0x0087f9a0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x0087f9a0: 16-byte aligned: YES
  0087f9a0  55             PUSH EBP
  0087f9a1  8B EC          MOV EBP,ESP
  0087f9a3  83 EC 20       SUB ESP,0x20
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x0087f9a0
string literals (0):
direct callees (1):
  0x005E7B10  Global::IntersectLineSegments
struct offsets touched (top 5):
  +0x134 [dec 308]  x4  via ECX
  +0x4 [dec 4]  x3  via EAX,EDX
  +0x130 [dec 304]  x2  via EAX
  +0x8 [dec 8]  x1  via EAX
  +0x20 [dec 32]  x1  via EAX
```

### Static facts (script output, report-only)
- size 600 B, RET 4
- callees: 0x0073f490 CClientExoApp::GetModuleCamera (CALL), 0x00463120 (CALL), 0x0044b8a0 Vector::Dot (CALL), 0x0073fea0 CClientExoApp::GetSWGuiManager (CALL), 0x0087e410 (CALL)
- indirect: 0x0087fb14 slot 0x8, 0x0087fb51 slot 0x10c, 0x0087fb73 slot 0xc, 0x0087fb9f slot 0x24, 0x0087fc06 slot 0x20, ...
- strings: 0x0099edc4 "lookathook"
- twins: none

## Card B20-6-10

GOG function: CSWCVisualEffectOnObject::ApplyLightsaberThrow  gog 0x004fec40  callconv=(blank)  param_size_bytes=0  gog_size=1948  gog_retn=4
Candidate Steam address: 0x00873070 (size 750, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.67) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.34 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00873360:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004fe950 CSWCVisualEffectOnObject::ApplySpellVisual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ff3e0 CSWCVisualEffectOnObject::ApplyTextureReplace  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: no
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: no
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam 0x0076e610  in candidate callees: no
  0x004213a0 CSWSPlaceable::SetEffectSpellId -> Steam ?  in candidate callees: n/a
  0x0042bd00 CSWCCreature::SetSWAnimationSpeed -> Steam ?  in candidate callees: n/a
  0x005036a0 CSWCVisualEffectOnObject::GetTargetCSWCObject -> Steam ?  in candidate callees: n/a
  0x005036f0 CSWCVisualEffectOnObject::GetSourceCSWCObject -> Steam ?  in candidate callees: n/a
  0x00503740 CSWCVisualEffectOnObject::GetOwnerCSWCObject -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'impact'; 'rhand'
disp sets (* = in both, 5 common): GOG {0x4*,0x8*,0x2c,0x48,0x78,0x88,0x8c*,0x94*,0x98*,0xe0,0x158,0x188,0x1c4} | Steam {0x4*,0x8*,0xc,0x18,0x50,0x8c*,0x94*,0x98*}
imm sets (* = in both, 7 common): GOG {0x2*,0x3*,0x4*,0xc*,0x10*,0x24*,0x30,0x31,0x44,0xff*,0x114,0x128,0x148,0x1c8} | Steam {0x2*,0x3*,0x4*,0x8,0xc*,0x10*,0x24*,0x2c,0x41,0x80,0xff*,0x4b0}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00873070: 16-byte aligned: YES
  00873070  55             PUSH EBP
  00873071  8B EC          MOV EBP,ESP
  00873073  81 EC 80 00 00 00 SUB ESP,0x80
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00873070
string literals (3):
  0x009A2C6C  at 008730b6  'handconjure'
  0x009914B8  at 008730c8  'impact'
  0x009914B8  at 0087333a  'impact'
direct callees (9):
  0x007338D0  CExoString::operator=_2
  0x00877E10  (unnamed)
  0x00877DC0  (unnamed)
  0x004452B0  Vector::operator- (Vector3 subtract)
  0x0044B8A0  Vector::Dot
  0x005143C0  (unnamed)
  0x00515D50  (unnamed)
  0x00515620  (unnamed)
  0x00734010  CExoString::CStr
struct offsets touched (top 8):
  +0x8C [dec 140]  x11  via EAX,ECX,EDX
  +0x4 [dec 4]  x5  via EAX,ECX
  +0x8 [dec 8]  x5  via EAX,ECX
  +0x98 [dec 152]  x3  via EDX
  +0xC [dec 12]  x2  via EAX,ECX
  +0x50 [dec 80]  x2  via EDX
  +0x18 [dec 24]  x1  via EDX
  +0x94 [dec 148]  x1  via EDX
```

### annotate (candidate, first 80 lines)
```
// FN 00873070 FUN_00873070 size=750 ret=4 callers=2 callees=9

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00873070(int param_1,short param_2)

{
  undefined4 *puVar1;
  int *piVar2;
  undefined4 uVar3;
  float10 fVar4;
  undefined1 local_68 [16];
  undefined1 local_58 [12];
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined1 local_40 [12];
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_2 == 0x4b0) {
    FUN_007338d0("headconjure");
  }
  else if (param_2 == 0x4b1) {
    FUN_007338d0("handconjure");
  }
  else if (param_2 == 0x4b2) {
    FUN_007338d0("impact");
  }
  if (*(int *)(param_1 + 0x8c) != 0) {
    if (param_2 == 0x4b2) {
      local_18 = 0;
      local_1c = 0;
      local_20 = 0;
      local_24 = 0x3f800000;
      local_10 = 0;
      local_c = 0;
      local_8 = 0;
      local_14 = FUN_00877e10();
      local_28 = FUN_00877dc0();
      (**(code **)(**(int **)(param_1 + 0x8c) + 0x98))(&DAT_0098b534,&local_10,&local_24);
      if ((local_14 != 0) && (local_28 != 0)) {
        local_34 = 0;
        local_30 = 0;
        local_2c = 0;
        puVar1 = (undefined4 *)FUN_004452b0(local_40,local_14 + 0x24,local_28 + 0x24);
        local_34 = *puVar1;
        local_30 = puVar1[1 = byte +0x4];
        local_2c = puVar1[2 = byte +0x8];
        fVar4 = (float10)FUN_0044b8a0(&local_34,&local_34);
        if (fVar4 < (float10)_DAT_0098c1e8 != (fVar4 == (float10)_DAT_0098c1e8)) {
          local_4c = 0;
          local_48 = 0;
          local_44 = 0;
          local_34 = 0;
          local_30 = 0;
          local_2c = 0;
        }
        puVar1 = (undefined4 *)FUN_005143c0(local_58,&local_34);
        local_34 = *puVar1;
        local_30 = puVar1[1 = byte +0x4];
        local_2c = puVar1[2 = byte +0x8];
        fVar4 = (float10)FUN_00515d50(&local_34,0,0);
        puVar1 = (undefined4 *)FUN_00515620(local_68,(float)fVar4);
        local_24 = *puVar1;
        local_20 = puVar1[1 = byte +0x4];
        local_1c = puVar1[2 = byte +0x8];
        local_18 = puVar1[3 = byte +0xc];
      }
      (**(code **)(**(int **)(param_1 + 0x8c) + 0x94))
                (&DAT_0098b534,local_10,local_c,local_8,local_24,local_20,local_1c,local_18);
/* ... 15 more lines truncated by --max-lines */
```

### decoy: 0x00873360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00873360: 16-byte aligned: YES
  00873360  55             PUSH EBP
  00873361  8B EC          MOV EBP,ESP
  00873363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00873360
string literals (3):
  0x0099EDD8  at 008735b5  'rhand'
  0x009914B8  at 008735f7  'impact'
  0x0099EDD8  at 00873929  'rhand'
direct callees (22):
  0x00877DC0  (unnamed)
  0x00877E10  (unnamed)
  0x00877E60  (unnamed)
  0x0077FCE0  CSWCCreature::SetSWAnimationSpeed
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x00919723  operator new
  0x008E54E0  CSWCProjectile::Constructor
  0x008EDA80  (unnamed)
  0x00458940  identity (basic vtable slot 0x4)
  0x00710840  (unnamed)
  0x005EF180  CSWSPlaceable::SetEffectSpellId
  0x005195A0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x008EE300  (unnamed)
  0x00733780  CExoString::~CExoString
  0x005142F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x008E7200  (unnamed)
  0x008EE260  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x98 [dec 152]  x5  via EDX
  +0x2C [dec 44]  x4  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x78 [dec 120]  x1  via EDX
  +0x88 [dec 136]  x1  via EDX
  +0x8C [dec 140]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
```

### Static facts (script output, report-only)
- size 750 B, RET 4
- callers 2: 0x00872910, 0x00878190
- callees: 0x007338d0 (CALL), 0x00877e10 (CALL), 0x00877dc0 (CALL), 0x004452b0 (CALL), 0x0044b8a0 Vector::Dot (CALL), 0x005143c0 (CALL), ...
- indirect: 0x00873157 slot 0x98, 0x00873294 slot 0x94, 0x008732c4 slot 0x98, 0x008732de slot 0x50, 0x00873310 slot 0x98, ...
- twins: none

## Card B20-6-11

GOG function: List_SafePointer_WindManager___::Destructor  gog 0x008fe2a0  callconv=__thiscall  param_size_bytes=0  gog_size=41  gog_retn=0
Candidate Steam address: 0x00463a90 (size 136, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.25 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00463b20:STR0/CALLEE12/SIG0.5/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x008fe280 Global::AurVertexProgramMakeCurrent  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x008fe2d0 PointSourceWind::Constructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921321 unnamed -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (24 of 466): the card's A-CALLEE credit may come from these, not from callees
  0x0041e770 CSWCCreature::DoFootstepVisual -> Steam 0x00772900  in candidate callers: no
  0x0044f6b0 CSWCArea::Destructor_2 -> Steam 0x007a2cb0  in candidate callers: no
  0x00459080 CClientExoAppInternal::ProcessInput -> Steam 0x007ac470  in candidate callers: no
  0x005082d0 CSWCDoor::UpdateAreaTransitionDisplay -> Steam 0x0087c9f0  in candidate callers: no
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x4}
imm sets (* = in both, 1 common): GOG {0x4,0x8*} | Steam {0x8*}

### abi
```
GOG param bytes: 0; callconv: thiscall
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: TRUE (GOG 0 bytes -> x86 slots 0; ASM RET 0)
```

### prologue
```
prologue for 0x00463a90: 16-byte aligned: YES
  00463a90  55             PUSH EBP
  00463a91  8B EC          MOV EBP,ESP
  00463a93  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00463a90
string literals (0):
direct callees (3):
  0x00463FF0  (unnamed)
  0x00463BA0  (unnamed)
  0x00463B20  (unnamed)
struct offsets touched (top 1):
  +0x4 [dec 4]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00463a90 FUN_00463a90 size=136 ret=0 callers=3 callees=3

void __fastcall FUN_00463a90(int param_1)

{
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0094a298;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_8 = 0;
  local_14 = *(int *)(param_1 + 4);
  while (local_14 = local_14 + -1, -1 < local_14) {
    FUN_00463ff0(0);
  }
  FUN_00463ba0(0);
  local_8 = 0xffffffff;
  FUN_00463b20();
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00463b20 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00463b20: 16-byte aligned: YES
  00463b20  55             PUSH EBP
  00463b21  8B EC          MOV EBP,ESP
  00463b23  83 EC 08       SUB ESP,0x8
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x00463b20
string literals (0):
direct callees (1):
  0x009196FD  operator delete
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 136 B, RET 0
- callers 3: 0x00459560, 0x004595b0, 0x0045a190
- callees: 0x00463ff0 (CALL), 0x00463ba0 (CALL), 0x00463b20 (CALL)
- twins: none

## Card B20-6-12

GOG function: CSWCVisualEffectOnObject::ApplyLightsaberThrow  gog 0x004fec40  callconv=(blank)  param_size_bytes=0  gog_size=1948  gog_retn=4
Candidate Steam address: 0x00873b00 (size 447, retn 4)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: pass(0.50) | G8 mac_rotation: pass
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.12 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00873360:STR0/CALLEE1/SIG1/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x004fe950 CSWCVisualEffectOnObject::ApplySpellVisual  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x004ff3e0 CSWCVisualEffectOnObject::ApplyTextureReplace  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (22), mapped to Steam through trusted rows only:
  0x0040c8f0 CClientExoApp::GetModule -> Steam 0x0073f430  in candidate callees: no
  0x0040c9f0 CClientExoApp::GetItemByGameObjectID -> Steam 0x0073f530  in candidate callees: yes
  0x0041a480 CSWCCreature::GetEquippedItemID -> Steam 0x0076e610  in candidate callees: yes
  0x004213a0 CSWSPlaceable::SetEffectSpellId -> Steam ?  in candidate callees: n/a
  0x0042bd00 CSWCCreature::SetSWAnimationSpeed -> Steam ?  in candidate callees: n/a
  0x005036a0 CSWCVisualEffectOnObject::GetTargetCSWCObject -> Steam ?  in candidate callees: n/a
  0x005036f0 CSWCVisualEffectOnObject::GetSourceCSWCObject -> Steam ?  in candidate callees: n/a
  0x00503740 CSWCVisualEffectOnObject::GetOwnerCSWCObject -> Steam ?  in candidate callees: n/a
  ... 14 more GOG callees
GOG callers mapped through trusted rows (0 of 1): the card's A-CALLEE credit may come from these, not from callees
GOG strings (2): 'impact'; 'rhand'
disp sets (* = in both, 2 common): GOG {0x4*,0x8,0x2c*,0x48,0x78,0x88,0x8c,0x94,0x98,0xe0,0x158,0x188,0x1c4} | Steam {0x4*,0xc,0x10,0x14,0x2c*}
imm sets (* = in both, 2 common): GOG {0x2,0x3,0x4*,0xc,0x10*,0x24,0x30,0x31,0x44,0xff,0x114,0x128,0x148,0x1c8} | Steam {0x4*,0xa,0x10*,0x1e,0x60,0x3e8}

### abi
```
GOG param bytes: 0; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(0/4)*4 = 0 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=4 ; asm RET = 4
Stack Slot Match: FALSE (GOG 0 bytes -> x86 slots 0; ASM RET 4)
```

### prologue
```
prologue for 0x00873b00: 16-byte aligned: YES
  00873b00  55             PUSH EBP
  00873b01  8B EC          MOV EBP,ESP
  00873b03  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
```

### anchors
```
anchors for 0x00873b00
string literals (3):
  0x009A4F8C  at 00873b90  'vdu_tex_stone'
  0x009A4F7C  at 00873ba9  'vdu_tex_grstone'
  0x009A4F6C  at 00873bc2  'vdu_tex_bark'
direct callees (9):
  0x00733540  CExoString::Constructor
  0x00877DC0  (unnamed)
  0x00733780  CExoString::~CExoString
  0x007338D0  CExoString::operator=_2
  0x00710810  (unnamed)
  0x00710840  (unnamed)
  0x007ED8D0  CSWCObject::ReplaceTexture_2
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
struct offsets touched (top 6):
  +0xC [dec 12]  x2  via EAX,EDX
  +0x4 [dec 4]  x1  via ECX
  +0x10 [dec 16]  x1  via ECX
  +0x14 [dec 20]  x1  via EDX
  +0x2C [dec 44]  x1  via EDX
  +0x873CD4 [dec 8862932]  x1  via EAX
```

### annotate (candidate, first 80 lines)
```
// FN 00873b00 FUN_00873b00 size=447 ret=4 callers=2 callees=9

void FUN_00873b00(short param_1)

{
  uint uVar1;
  undefined4 uVar2;
  undefined4 extraout_var;
  undefined1 local_64 [16];
  undefined1 local_54 [16];
  undefined1 local_44 [16];
  undefined1 local_34 [16];
  int local_24;
  int local_20;
  undefined1 local_1c [8];
  int *local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_00973f58;
  local_10 = ExceptionList;
  uVar1 = DAT_00a11f20 ^ (uint)&stack0xfffffffc;
  ExceptionList = &local_10;
  FUN_00733540(uVar1);
  local_8 = 0;
  uVar2 = extraout_var;
  local_14 = (int *)FUN_00877dc0();
  if (local_14 == (int *)0x0) {
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  else {
    switch(param_1) {
    case 0x3f2:
      FUN_007338d0("vdu_tex_stone");
      local_14[3 = byte +0xc] = 1;
      break;
    default:
      local_8 = 0xffffffff;
      FUN_00733780(uVar1,(ushort)(param_1 - 0x3e8 [dec 1000]) - 0xa [dec 10],uVar2);
      ExceptionList = local_10;
      return;
    case 0x3fc:
      FUN_007338d0("vdu_tex_grstone");
      local_14[3 = byte +0xc] = 1;
      break;
    case 0x406:
      FUN_007338d0("vdu_tex_bark");
      local_14[4 = byte +0x10] = 1;
      break;
    case 0x410:
      FUN_007338d0("vdu_tex_shade");
      local_14[5 = byte +0x14] = 1;
    }
    FUN_00710810(local_1c);
    FUN_00710840(&DAT_00986925);
    FUN_007ed8d0(local_44,local_34,0);
    local_20 = (**(code **)(*local_14 + 0x2c))();
    if (local_20 != 0) {
      uVar2 = FUN_0076e610(1);
      local_24 = FUN_0073f530(uVar2);
      if (local_24 != 0) {
        FUN_00710810(local_1c);
        FUN_00710840(&DAT_00986925);
        FUN_007ed8d0(local_64,local_54,0);
      }
    }
    local_8 = 0xffffffff;
    FUN_00733780();
  }
  ExceptionList = local_10;
  return;
}
```

### decoy: 0x00873360 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x00873360: 16-byte aligned: YES
  00873360  55             PUSH EBP
  00873361  8B EC          MOV EBP,ESP
  00873363  6A FF          PUSH -0x1
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP; 6A FF 68  SEH frame (PUSH -1; PUSH handler) after frame setup
anchors for 0x00873360
string literals (3):
  0x0099EDD8  at 008735b5  'rhand'
  0x009914B8  at 008735f7  'impact'
  0x0099EDD8  at 00873929  'rhand'
direct callees (22):
  0x00877DC0  (unnamed)
  0x00877E10  (unnamed)
  0x00877E60  (unnamed)
  0x0077FCE0  CSWCCreature::SetSWAnimationSpeed
  0x0076E610  CSWCCreature::GetEquippedItemID
  0x0073F530  CClientExoApp::GetItemByGameObjectID
  0x00919723  operator new
  0x008E54E0  CSWCProjectile::Constructor
  0x008EDA80  (unnamed)
  0x00458940  identity (basic vtable slot 0x4)
  0x00710840  (unnamed)
  0x005EF180  CSWSPlaceable::SetEffectSpellId
  0x005195A0  (unnamed)
  0x00733570  CExoString::CExoString_char
  0x008EE300  (unnamed)
  0x00733780  CExoString::~CExoString
  0x005142F0  (unnamed)
  0x0091C860  _ftol2_sse
  0x008E7200  (unnamed)
  0x008EE260  (unnamed)
struct offsets touched (top 10):
  +0x4 [dec 4]  x14  via EAX,ECX,EDX
  +0x8 [dec 8]  x6  via EAX,ECX,EDX
  +0x98 [dec 152]  x5  via EDX
  +0x2C [dec 44]  x4  via EDX
  +0x48 [dec 72]  x1  via EAX
  +0x78 [dec 120]  x1  via EDX
  +0x88 [dec 136]  x1  via EDX
  +0x8C [dec 140]  x1  via EAX
  +0x94 [dec 148]  x1  via EAX
  +0xE0 [dec 224]  x1  via EDX
```

### Static facts (script output, report-only)
- size 447 B, RET 4
- callers 2: 0x00872910, 0x00878190
- callees: 0x00733540 CExoString::DefaultConstructor (CALL), 0x00877dc0 (CALL), 0x00733780 CExoString::Destructor (CALL), 0x007338d0 (CALL), 0x00710810 (CALL), 0x00710840 (CALL), ...
- indirect: 0x00873c3e slot 0x2c
- strings: 0x009a4f6c "vdu_tex_bark", 0x009a4f7c "vdu_tex_grstone", 0x009a4f8c "vdu_tex_stone"
- twins: none

## Card B20-6-13

GOG function: GLU32_DLL::gluPerspective  gog 0x009433e6  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b436 (size 6, retn None)  THUNK -> 0x00000145 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: fail:unaligned | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000145 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
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
prologue: no asm for 0x00000145
```

### anchors
```
anchors: no asm for 0x00000145
```

### annotate (candidate, first 80 lines)
```
(no decompile for 0x00000145)
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

## Card B20-6-14

GOG function: GLU32_DLL::gluBuild2DMipmaps  gog 0x009433e0  callconv=(blank)  param_size_bytes=0  gog_size=6  gog_retn=None
Candidate Steam address: 0x0093b430 (size 6, retn None)  THUNK -> 0x00000144 (everything below is judged on the target)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: n/a | G4 ret: n/a | G5 thunk: 0x00000144 | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR n/a | A-CALLEE n/a | A-SIG n/a | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x00401000:STRn/a/CALLEEn/a/SIGn/a/SRV0/VT0
- flags: none
- GOG-order neighbour before: 0x00943360 DINPUT8_DLL::DirectInput8Create  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x009433e6 GLU32_DLL::gluPerspective -> 0x0093b436  gate_class=REVIEW

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
(no GOG fingerprint available for 0x009433e0)

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

## Card B20-6-15

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
- GOG-order neighbour after:  0x0044eee0 CClientAIMaster::RemoveObject  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x0044ead0 CClientAIList::AddObject -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (5 of 13): the card's A-CALLEE credit may come from these, not from callees
  0x004a1640 CSWCMessage::HandleServerToPlayerPlaceableUpdate_Add -> Steam 0x0080d050  in candidate callers: no
  0x004bdea0 CSWCMessage::HandleServerToPlayerSafeProjectileSpell -> Steam 0x00829850  in candidate callers: no
  0x004f90c0 CSWCVisualEffect::Constructor -> Steam 0x0086d770  in candidate callers: no
  0x0055b480 CSWCSoundObject::AddToArea -> Steam 0x00800cb0  in candidate callers: no
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

## Card B20-6-16

GOG function: MaxTree::FreeTheFreeList_2  gog 0x00844e00  callconv=(blank)  param_size_bytes=4  gog_size=17  gog_retn=0
Candidate Steam address: 0x00452810 (size 31, retn 0)
Candidate sources: (withheld on every card; judge the body)

### Computed facts (script output, do not recompute)
- G1 start: pass | G2 align: pass | G3 prologue: pass | G4 ret: pass | G5 thunk: none | G6 dup: pass | G7 mac: n/a | G8 mac_rotation: n/a
- anchors: A-STR 0 | A-CALLEE 0 | A-SIG 0.00 | A-SRV 0 | A-VT 0
- families that discriminate against the neighbours: none  (content level C0)
- evidence summary: no content anchor (C0)
- best decoy (neighbour/other candidate) scores: 0x004527f0:STR0/CALLEE0/SIG0.5/SRV0/VT0
- flags: gog_db_param_bad
- GOG-order neighbour before: 0x00844d40 MaxTree::FreeTheFreeList  (not in this batch, no trusted mapping)
- GOG-order neighbour after:  0x00844e20 MaxTree::InternalDestructor  (not in this batch, no trusted mapping)

DECOY NOTE: assume this candidate is a neighbouring function or thunk with the same RET signature until a content anchor proves otherwise.

### GOG side (from the server GOG fingerprint)
GOG callees (1), mapped to Steam through trusted rows only:
  0x00921347 Global::operator_new_uint -> Steam ?  in candidate callees: n/a
GOG callers mapped through trusted rows (0 of 2): the card's A-CALLEE credit may come from these, not from callees
GOG strings (0): 
disp sets (* = in both, 0 common): GOG {} | Steam {0x28}
imm sets (* = in both, 0 common): GOG {0x4} | Steam {}

### abi
```
GOG param bytes: 4; callconv: thiscall (GOG callconv blank -> assumed thiscall)
expected_ret = ceil(4/4)*4 = 4 (x86 4-byte stack-slot promotion; ECX = this)
observed: dump header ret=0 ; asm RET = 0
Stack Slot Match: FALSE (GOG 4 bytes -> x86 slots 4; ASM RET 0)
```

### prologue
```
prologue for 0x00452810: 16-byte aligned: YES
  00452810  55             PUSH EBP
  00452811  8B EC          MOV EBP,ESP
  00452813  51             PUSH ECX
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
```

### anchors
```
anchors for 0x00452810
string literals (0):
direct callees (1):
  0x004536B0  (unnamed)
struct offsets touched (top 1):
  +0x28 [dec 40]  x2  via EAX,ECX
```

### annotate (candidate, first 80 lines)
```
// FN 00452810 FUN_00452810 size=31 ret=0 callers=2 callees=1

void __fastcall FUN_00452810(int param_1)

{
  if (*(int *)(param_1 + 0x28) != 0) {
    FUN_004536b0();
  }
  return;
}
```

### decoy: 0x004527f0 (neighbour that must NOT be confused with the candidate)
```
prologue for 0x004527f0: 16-byte aligned: YES
  004527f0  55             PUSH EBP
  004527f1  8B EC          MOV EBP,ESP
  004527f3  8B 45 08       MOV EAX,dword ptr [EBP + 0x8]
standard prologue: YES - 55 8B EC  PUSH EBP; MOV EBP,ESP
anchors for 0x004527f0
string literals (0):
direct callees (1):
  0x00919723  operator new
struct offsets touched (top 0):
```

### Static facts (script output, report-only)
- size 31 B, RET 0
- callers 2: 0x00451ef0, 0x00452be0
- callees: 0x004536b0 (CALL)
- twins: none
